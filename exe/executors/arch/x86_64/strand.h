#ifndef DDV_EXE_EXECUTORS_ARCH_X86_64_STRAND_H_
#define DDV_EXE_EXECUTORS_ARCH_X86_64_STRAND_H_ 1

#include <atomic>

#include "exe/executors/executor.h"

#include "utils/abort.h"
#include "utils/assert.h"
#include "utils/utility.h"

namespace exe::executors {

// Adapter for an executor that allows to serialize
// asynchronous critical sections without using explicit locks
// 
// Instead of moving the "lock" between threads, it moves critical sections,
// thereby allowing critical data to be in cache all the time
//
// Both sending tasks and their execution are wait-free
class Strand final : public IExecutor, private TaskBase {
private:
	struct DummyTask : public TaskBase {
		DummyTask() noexcept
		{
			next_.store(this, ::std::memory_order_relaxed);
		}

		void run() noexcept override
		{
			next_.store(nullptr, ::std::memory_order_relaxed);
		}
	};

private:
	IExecutor &executor_;
	DummyTask dummy_;
	TaskBase *head_;
	::std::atomic<TaskBase *> tail_;

public:
	~Strand() = default;

	Strand(Strand const &) = delete;
	void operator= (Strand const &) = delete;

	Strand(Strand &&) = delete;
	void operator= (Strand &&) = delete;

public:
	explicit Strand(IExecutor &e) noexcept
		: executor_(e)
		, dummy_()
		, head_(nullptr)
		, tail_(&dummy_)
	{}

	// wait-free task submission
	// 
	// precondition: task && task->next_ == nullptr
	void execute(TaskBase *task) noexcept override
	{
		UTILS_ASSERT(
			task,
			"violation of the execute precondition"
		);
		UTILS_ASSERT(
			!task->next_.load(::std::memory_order_relaxed),
			"violation of the execute precondition"
		);

		auto *prev = tail_.exchange(task);

		auto idle = static_cast<bool>(prev->next_.exchange(task));
		if (idle) {
			// start a new critical section
			// (relying on correct synchronization by executor)
			head_ = prev;

			// TODO: exception handling
			try {
				executor_.execute(this);
			} catch (...) {
				::utils::abort(
					"exception while trying to start "
					"a critical section of strand"
				);
			}
		}
	}

private:
	// wait-free task processing cycle
	void run() noexcept override
	{
		auto *curr = head_;
		auto *next = curr->next_.load(::std::memory_order_relaxed);

		do {
			// at this point curr is the only pointer to its object
			// (after run, curr object doesn't exist)
			curr->run();
			curr = next;
		} while (
			// in case of a high load,
			// we will immediately get the following task
			(next = curr->next_.load(::std::memory_order_acquire)) ||

			(
				// are there any other tasks?
				tail_.compare_exchange_strong(
					::utils::temporary(::utils::decay_copy(curr)),
					&dummy_
				) &&
				// cmpxchg -> true: there are no other tasks

				// at this point curr is the only pointer to its object
				// (after run, curr object doesn't exist)
				(curr->run(), curr = &dummy_),

				// if the task has not been linked yet, we give control
				(next = curr->next_.load(::std::memory_order_acquire)) ||
				(next = curr->next_.exchange(curr))
			)
		);
	}
};

} // namespace exe::executors

#endif /* DDV_EXE_EXECUTORS_ARCH_X86_64_STRAND_H_ */
