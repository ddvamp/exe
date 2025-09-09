// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#include <atomic>
#include <new>
#include <utility>

#include "exe/runtime/strand.h"

#include "util/debug/assert.h"

namespace exe::runtime {

class alignas(::std::hardware_destructive_interference_size) Strand::Impl
	: private TaskBase
	, public ::util::RefCounted<Impl> {
private:
	struct DummyTask : TaskBase {
		DummyTask() noexcept
		{
			link(this);
		}

		void run() noexcept override
		{
			// do nothing
		}
	};

private:
	ISafeScheduler &where_;
	DummyTask dummy_;
	TaskBase *head_ = &dummy_;
	::std::atomic<TaskBase *> tail_ = &dummy_;

public:
	[[nodiscard]] static Impl *create(ISafeScheduler &where)
	{
		return ::new Impl(where);
	}

	void destroySelf() const noexcept
	{
		delete this;
	}

public:
	explicit Impl(ISafeScheduler &where) noexcept
		: where_(where)
	{
		static_assert(
			sizeof(Impl) <= ::std::hardware_destructive_interference_size,
			"Size has exceeded the cache line."
			"Think about separating the fields."
		);
	}

	[[nodiscard]] ISafeScheduler &getScheduler() const noexcept
	{
		return where_;
	}

	void submit(TaskBase *task) noexcept;

private:
	void run() noexcept override;

	void runSelf() noexcept;

	void afterAcquire(TaskBase *task) noexcept;

	void submitSelf() noexcept;

	bool isActualTask(TaskBase *task) const noexcept;

	static TaskBase *loadNext(TaskBase *task) noexcept;

	TaskBase *putTask(TaskBase *task) noexcept;

	bool tryAcquire() noexcept;

	bool tryPutDummy(TaskBase *expected) noexcept;

	TaskBase *tryTakeNextTask(TaskBase *task) noexcept;
};

////////////////////////////////////////////////////////////////////////////////

Strand::Strand(ISafeScheduler &where)
	: impl_(Impl::create(where))
{}

Strand::~Strand() noexcept = default;

/* virtual */ void Strand::submit(TaskBase *task) noexcept
{
	UTIL_ASSERT(task, "nullptr instead of task");

	impl_->submit(task);
}

ISafeScheduler &Strand::getScheduler() const noexcept
{
	return impl_->getScheduler();
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
	if (!prev) [[likely]] {
		return;
	}

	if (isActualTask(prev)) [[likely]] {
		submitSelf();
		return;
	}

	dummy_.link(nullptr);

	if (tryPutDummy(task)) {
		afterAcquire(task);
		return;
	}

	if (tryTakeNextTask(task)) {
		submitSelf();
	}
}

/* virtual */ void Strand::Impl::run() noexcept
{
	runSelf();
	decRef();
}

void Strand::Impl::runSelf() noexcept
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

void Strand::Impl::afterAcquire(TaskBase *task) noexcept
{
	head_ = task;
	task->link(&dummy_);

	submitSelf();
}

void Strand::Impl::submitSelf() noexcept
{
	incRef();
	where_.submit(this);
}

bool Strand::Impl::isActualTask(TaskBase *task) const noexcept
{
	return task != &dummy_;
}

/* static */ TaskBase *Strand::Impl::loadNext(TaskBase *task) noexcept
{
	return task->next_.load(::std::memory_order_acquire);
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

} // namespace exe::runtime
