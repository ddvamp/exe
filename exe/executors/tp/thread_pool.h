#ifndef DDV_EXE_EXECUTORS_TP_THREAD_POOL_H_
#define DDV_EXE_EXECUTORS_TP_THREAD_POOL_H_ 1

#include <cstddef>
#include <thread>
#include <vector>

#include "concurrency/mpmc_unbounded_blocking_queue.h"
#include "concurrency/wait_point.h"

#include "exe/executors/executor.h"

namespace exe::executors::tp {

class ThreadPool : public IExecutor {
private:
	::std::vector<::std::thread> workers_;
	::concurrency::MPMCUnboundedBlockingQueue<TaskBase *> tasks_;
	::concurrency::WaitPoint wp_;

#ifndef UTILS_DISABLE_DEBUG
	bool joined_ = false;
#endif

public:
	~ThreadPool();

	ThreadPool(ThreadPool const &) = delete;
	void operator= (ThreadPool const &) = delete;

	ThreadPool(ThreadPool &&) = delete;
	void operator= (ThreadPool &&) = delete;

public:
	explicit ThreadPool(::std::size_t workers);

	void execute(TaskBase *task) override;

	void waitIdle();

	// wait for all tasks to complete and join threads
	void stop();

	static ThreadPool *current() noexcept;

private:
	void workLoop();

	void join();
};

} // namespace exe::executors::tp

#endif /* DDV_EXE_EXECUTORS_TP_THREAD_POOL_H_ */
