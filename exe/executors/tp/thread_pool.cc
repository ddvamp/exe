#include <utility>

#include "exe/executors/tp/thread_pool.h"

#include "utils/abort.h"
#include "utils/assert.h"
#include "utils/utility.h"

namespace exe::executors::tp {

namespace {

thread_local ThreadPool *current_pool = nullptr;

} // namespace

ThreadPool *ThreadPool::current() noexcept
{
	return current_pool;
}

ThreadPool::~ThreadPool()
{
	UTILS_ASSERT(joined_, "thread pool was not stopped");
}

void ThreadPool::workLoop()
{
	current_pool = this;

	// TODO: exception handling
	try {
		while (auto item = tasks_.take()) {
			auto task = *item;

			task->run();

			wp_.done();
		}
	} catch (...) {
		::utils::abort("exception inside thread pool");
	}
}

void ThreadPool::join()
{
	UTILS_ASSERT(!joined_, "thread pool already stopped");
	
	joined_ = true;

	for (auto &w : workers_) {
		w.join();
	}
}

ThreadPool::ThreadPool(::std::size_t workers)
{
	workers_.reserve(workers);
	while (workers-- != 0) {
		workers_.emplace_back(&ThreadPool::workLoop, this);
	}
}

void ThreadPool::execute(TaskBase *task)
{
	UTILS_ASSERT(task, "thread pool got nullptr instead of a task");
	UTILS_VERIFY(
		tasks_.put(::utils::builder(
			[&w = wp_, task]() noexcept {
				w.add();
				return task;
			}
		)),
		"using a thread pool after stop"
	);
}

void ThreadPool::waitIdle()
{
	wp_.wait();
}

void ThreadPool::stop()
{
	tasks_.close();
	join();
}

} // namespace exe::executors::tp