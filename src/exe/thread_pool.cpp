//
//
//
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#include <utility>

#include "exe/runtime/tp/thread_pool.hpp"

#include "util/abort.hpp"
#include "util/debug.hpp"
#include "util/utility.hpp"

namespace exe::runtime::tp {

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
	UTIL_ASSERT(state_ == State::STOPPED, "thread pool was not stopped");
}

void ThreadPool::workLoop()
{
	current_pool = this;

	// TODO: exception handling
	try {
		while (auto item = tasks_.Pop()) {
			auto task = *item;

			task->Run();
			task_count_.Done(1, ::std::memory_order_release);
		}
	} catch (...) {
		UTIL_ABORT("exception inside thread pool");
	}
}

void ThreadPool::start()
{
	UTIL_ASSERT(
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
	UTIL_ASSERT(workers != 0, "zero-size thread pool was requested");
}

/* virtual */ void ThreadPool::submit(task::TaskBase *task)
{
	UTIL_ASSERT(
		task,
		"nullptr instead of the task"
	);

	task_count_.Add();
	UTIL_VERIFY(tasks_.Push(task), "using thread pool after stop");
}

void ThreadPool::waitIdle()
{
	task_count_.Wait(::std::memory_order_acquire);
}

void ThreadPool::stop()
{
	UTIL_ASSERT(
		::std::exchange(state_, State::STOPPED) == State::STARTED,
		"attempt to stop non-working thread pool"
	);

	tasks_.Close();
	joinWorkerThreads();
}

} // namespace exe::runtime::tp
