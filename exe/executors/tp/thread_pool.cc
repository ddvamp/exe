// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#include <utility>

#include "exe/executors/tp/thread_pool.h"

#include "util/abort.h"
#include "util/debug.h"
#include "util/utility.h"

namespace exe::executors::tp {

namespace {

thread_local ThreadPool *current_pool = nullptr;

::std::size_t normalize_thread_count(::std::size_t requested) noexcept
{
	auto const available = ::std::thread::hardware_concurrency();

	return
		available == 0
		? requested
		: requested < available
		? requested
		: available;
}

} // namespace

/* static */ ThreadPool *ThreadPool::current() noexcept
{
	return current_pool;
}

ThreadPool::~ThreadPool()
{
	UTILS_ASSERT(state_ == State::STOPPED, "thread pool was not stopped");
}

void ThreadPool::workLoop()
{
	current_pool = this;

	// TODO: exception handling
	try {
		while (auto item = tasks_.take()) {
			auto task = *item;

			task->run();
			task_count_.done(1, ::std::memory_order_release);
		}
	} catch (...) {
		UTILS_ABORT("exception inside thread pool");
	}
}

void ThreadPool::start()
{
	UTILS_ASSERT(
		::std::exchange(state_, State::STARTED) == State::CREATED,
		"thread pool has already been started"
	);

	auto cnt = worker_count_;

	workers_.reserve(cnt);
	do {
		workers_.emplace_back(&ThreadPool::workLoop, this);
	} while (--cnt != 0);
}

void ThreadPool::joinWorkerThreads()
{
	for (auto &w : workers_) {
		w.join();
	}
}

ThreadPool::ThreadPool(::std::size_t workers)
	: ThreadPool(workers, defer_start)
{
	start();
}

ThreadPool::ThreadPool(::std::size_t workers, defer_start_t)
	: worker_count_(normalize_thread_count(workers))
{
	UTILS_ASSERT(workers != 0, "zero-size thread pool was requested");
}

/* virtual */ void ThreadPool::submit(TaskBase *task)
{
	UTILS_ASSERT(
		task,
		"nullptr instead of the task"
	);

	UTILS_VERIFY(
		tasks_.put(::util::builder{
			[this, task]() noexcept {
				task_count_.add();
				return task;
			}
		}),
		"using thread pool after stop"
	);
}

void ThreadPool::waitIdle()
{
	task_count_.wait(::std::memory_order_acquire);
}

void ThreadPool::stop()
{
	UTILS_ASSERT(
		::std::exchange(state_, State::STOPPED) == State::STARTED,
		"attempt to stop non-working thread pool"
	);

	tasks_.close();
	joinWorkerThreads();
}

} // namespace exe::executors::tp
