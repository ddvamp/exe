// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#include <atomic>
#include <utility>

#include "exe/executors/strand.h"

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
	INothrowExecutor &where_;

	::std::atomic_uint64_t owners_ = 1;

	DummyTask dummy_;
	TaskBase *head_ = &dummy_;
	::std::atomic<TaskBase *> tail_ = &dummy_;

public:
	explicit Impl(INothrowExecutor &where) noexcept
		: where_(where)
	{}

	void submit(TaskBase *task) noexcept;

	void release() noexcept
	{
		if (owners_.fetch_sub(1, ::std::memory_order_acq_rel) == 1) {
			destroySelf();
		}
	}

private:
	void run() noexcept override;

	void submitSelf() noexcept;

	bool isActualTask(TaskBase *task) const noexcept;

	TaskBase *putTask(TaskBase *task) noexcept;

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

Strand::Strand(INothrowExecutor &where)
	: impl_(::new Impl(where))
{}

Strand::~Strand()
{
	impl_->release();
}

/* virtual */ void Strand::submit(TaskBase *task) noexcept
{
	impl_->submit(task);
}

////////////////////////////////////////////////////////////////////////////////

void Strand::Impl::submit(TaskBase *task) noexcept
{
	if (tryAcquire()) {
		afterAcquire(task);
		return;
	}

	task->link(nullptr);

	auto prev = putTask(task);
	
	if (!isActualTask(prev)) [[unlikely]] {
		dummy_.link(nullptr);

		if (tryPutDummy(task)) {
			afterAcquire(task);
			return;
		}

		addRef();
		prev = tryTakeNextTask(task);
	}

	if (prev) {
		submitSelf();
	}
}

/* virtual */ void Strand::Impl::run() noexcept
{
	auto curr = head_;
	auto next = curr->next_.load(::std::memory_order_relaxed);

	if (isActualTask(next)) [[unlikely]] {
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

void Strand::Impl::submitSelf() noexcept
{
	where_.submit(this);
}

bool Strand::Impl::isActualTask(TaskBase *task) const noexcept
{
	return task != &dummy_;
}

TaskBase *Strand::Impl::putTask(TaskBase *task) noexcept
{
	return
		tail_.exchange(task, ::std::memory_order_acq_rel)->
		next_.exchange(task, ::std::memory_order_acq_rel);
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

	submitSelf();
}

} // namespace exe::executors
