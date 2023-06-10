// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#include <atomic>

#include "exe/executors/strand.h"

#include "utils/abort.h"

namespace exe::executors {

class alignas(64) Strand::Impl : private TaskBase {
private:
	struct DummyTask : TaskBase {
		DummyTask() noexcept
		{
			next_.store(this, ::std::memory_order_relaxed);
		}

		void run() noexcept override {}
	};

private:
	IExecutor &where_;

	::std::atomic_uint64_t owners_ = 1;

	DummyTask dummy_;
	TaskBase *head_ = &dummy_;
	::std::atomic<TaskBase *> tail_ = &dummy_;

public:
	explicit Impl(IExecutor &where) noexcept
		: where_(where)
	{}

	void execute(TaskBase *task) noexcept;

	void release() noexcept
	{
		if (owners_.fetch_sub(1, ::std::memory_order_acq_rel) == 1) {
			destroySelf();
		}
	}

private:
	void run() noexcept override;

	bool enqueue(TaskBase *task) noexcept;

	bool tryAcquire() noexcept;

	bool tryPutDummy(TaskBase *expected) noexcept;

	TaskBase *tryTakeNextTask(TaskBase *task) noexcept;

	static TaskBase *loadNext(TaskBase *task) noexcept;

	void afterAcquire(TaskBase *task) noexcept;

	void addRef() noexcept
	{
		owners_.fetch_add(1, ::std::memory_order_relaxed);
	}

	void destroySelf() noexcept
	{
		delete this;
	}
};

////////////////////////////////////////////////////////////////////////////////

Strand::Strand(IExecutor &where)
	: impl_(::new Impl(where))
{}

Strand::~Strand()
{
	impl_->release();
}

/* virtual */ void Strand::doExecute(TaskBase *task) noexcept
{
	impl_->execute(task);
}

////////////////////////////////////////////////////////////////////////////////

void Strand::Impl::execute(TaskBase *task) noexcept
{
	if (enqueue(task)) {
		// start a new critical section
		// (relying on correct synchronization by executor)

		// TODO: exception handling
		try {
			where_.execute(this);
		} catch (...) {
			UTILS_ABORT(
				"exception while trying to start "
				"a critical section of strand"
			);
		}
	}
}

/* virtual */ void Strand::Impl::run() noexcept
{
	auto curr = head_;
	auto next = curr->next_.load(::std::memory_order_relaxed);

	if (next != &dummy_) [[unlikely]] {
		goto run_task;
	}

dummy_next:
	curr->run();

	if ((curr = tryTakeNextTask(&dummy_))) {
		dummy_.link(nullptr);
		goto take_next;
	}

	release();
	return;

run_task:
	::std::exchange(curr, next)->run();

take_next:
	if ((next = loadNext(curr))) {
		goto run_task;
	}

	if (tryPutDummy(curr)) {
		goto dummy_next;
	}

	if ((next = tryTakeNextTask(curr))) {
		goto run_task;
	}
}

bool Strand::Impl::enqueue(TaskBase *task) noexcept
{
	if (tryAcquire()) {
		afterAcquire(task);
		return true;
	}

	task->link(nullptr);

	auto prev =
		tail_.exchange(task, ::std::memory_order_acq_rel)->
		next_.exchange(task, ::std::memory_order_acq_rel);
	
	if (prev != &dummy_) [[likely]] {
		return prev;
	}

	dummy_.link(nullptr);

	if (tryPutDummy(task)) {
		afterAcquire(task);
		return true;
	}

	addRef();
	return tryTakeNextTask(task);
}

bool Strand::Impl::tryAcquire() noexcept
{
	auto expected = static_cast<TaskBase *>(&dummy_);

	return dummy_.next_.load(::std::memory_order_relaxed) == expected &&
		dummy_.next_.compare_exchange_weak(
			expected,
			nullptr,
			::std::memory_order_acquire,
			::std::memory_order_relaxed
		);
}

bool Strand::Impl::tryPutDummy(TaskBase *expected) noexcept
{
	return tail_.compare_exchange_strong(
		expected,
		&dummy_,
		::std::memory_order_release,
		::std::memory_order_relaxed
	);
}

TaskBase *Strand::Impl::tryTakeNextTask(TaskBase *task) noexcept
{
	auto next = static_cast<TaskBase *>(nullptr);

	task->next_.compare_exchange_strong(
		next,
		head_ = task,
		::std::memory_order_release,
		::std::memory_order_acquire
	);

	return next;
}

/* static */ TaskBase *Strand::Impl::loadNext(TaskBase *task) noexcept
{
	return task->next_.load(::std::memory_order_acquire);
}

void Strand::Impl::afterAcquire(TaskBase *task) noexcept
{
	addRef();

	head_ = task;
	task->link(&dummy_);
}

} // namespace exe::executors
