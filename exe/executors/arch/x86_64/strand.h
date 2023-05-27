// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_EXECUTORS_ARCH_X86_64_STRAND_H_
#define DDV_EXE_EXECUTORS_ARCH_X86_64_STRAND_H_ 1

#include <atomic>

#include "exe/executors/executor.h"

namespace exe::executors {

// Adapter for an executor that allows to serialize
// asynchronous critical sections without using explicit locks
// 
// Instead of moving the "lock" between threads, it moves critical sections,
// thereby allowing data to be in cache all the time
//
// Both sending tasks and their execution are wait-free,
// but not the executor_.execute call
class Strand final : public IExecutor, private TaskBase {
private:
	struct DummyTask : TaskBase {
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
	TaskBase *head_ = &dummy_;
	::std::atomic<TaskBase *> tail_ = &dummy_;

public:
	~Strand() = default;

	Strand(Strand const &) = delete;
	void operator= (Strand const &) = delete;

	Strand(Strand &&) = delete;
	void operator= (Strand &&) = delete;

public:
	explicit Strand(IExecutor &where) noexcept
		: executor_(where)
	{}

private:
	// wait-free task submission
	// (excluding the executor_.execute call)
	void doExecute(TaskBase *task) noexcept override;

	// wait-free task processing cycle
	void run() noexcept override;
};

} // namespace exe::executors

#endif /* DDV_EXE_EXECUTORS_ARCH_X86_64_STRAND_H_ */
