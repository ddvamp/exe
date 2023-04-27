// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_EXECUTORS_TP_THREAD_POOL_H_
#define DDV_EXE_EXECUTORS_TP_THREAD_POOL_H_ 1

#include <cstddef>
#include <thread>
#include <vector>

#include "concurrency/mpmc_unbounded_blocking_queue.h"
#include "concurrency/wait_point.h"

#include "exe/executors/executor.h"

namespace exe::executors::tp {

struct defer_start_t {
	explicit defer_start_t() = default;
};

inline constexpr defer_start_t defer_start{};

class ThreadPool : public IExecutor {
private:
	::std::vector<::std::thread> workers_;
	::std::size_t worker_count_;
	::concurrency::MPMCUnboundedBlockingQueue<TaskBase *> tasks_;
	::concurrency::WaitPoint task_count_;

#ifndef UTILS_DISABLE_DEBUG
	enum class State {
		CREATED,
		STARTED,
		STOPPED,
	};

	State state_ = State::CREATED;
#endif

public:
	~ThreadPool();

	ThreadPool(ThreadPool const &) = delete;
	void operator= (ThreadPool const &) = delete;

	ThreadPool(ThreadPool &&) = delete;
	void operator= (ThreadPool &&) = delete;

public:
	explicit ThreadPool(::std::size_t workers);

	// requires explicit start of threads
	ThreadPool(::std::size_t workers, defer_start_t);

	// wait until the number of tasks drops to zero
	void waitIdle();

	// initializes and starts worker threads
	void start();

	// wait for all tasks to complete and join threads
	void stop();

	static ThreadPool *current() noexcept;

private:
	void doExecute(TaskBase *task) override;

	void workLoop();

	void joinWorkerThreads();
};

} // namespace exe::executors::tp

#endif /* DDV_EXE_EXECUTORS_TP_THREAD_POOL_H_ */
