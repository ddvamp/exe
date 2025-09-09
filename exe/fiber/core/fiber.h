// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FIBER_CORE_FIBER_H_
#define DDV_EXE_FIBER_CORE_FIBER_H_ 1

#include <atomic>

#include "exe/executors/executor.h"
#include "exe/executors/task.h"
#include "exe/fiber/api.h"
#include "exe/fiber/core/awaiter.h"
#include "exe/fiber/core/coroutine.h"
#include "exe/fiber/core/id.h"
#include "exe/fiber/core/stack.h"

namespace exe::fiber {

// Fiber = stackful coroutine + scheduling + executor
class [[nodiscard]] Fiber : public executors::TaskBase {
private:
	::context::Stack stack_;
	Coroutine coroutine_;
	INothrowExecutor *executor_;
	IAwaiter *awaiter_ = nullptr;
	FiberId const id_ = getNextId();

	inline static ::std::atomic<FiberId> next_id_ = kInvalidFiberId + 1;

public:
	// reference to currently active fiber
	[[nodiscard]] static Fiber &self() noexcept;

	Fiber(FiberRoutine &&, ::context::Stack &&, INothrowExecutor *) noexcept;

	[[nodiscard]] FiberId getId() const noexcept
	{
		return id_;
	}

	[[nodiscard]] INothrowExecutor *getExecutor() const noexcept
	{
		return executor_;
	}

	// schedule execution on executor set on fiber
	void schedule() noexcept;

	// execute fiber immediately
	void resume() noexcept;

	void suspend(IAwaiter *) noexcept;

	// reschedule current fiber on executor
	void teleportTo(INothrowExecutor *) noexcept;

	// ITask
	void run() noexcept override;

private:
	[[nodiscard]] Fiber *doRun() noexcept;

	void step() noexcept;
	void stop() noexcept;
	void destroySelf() noexcept;
	void releaseResources() noexcept;

	static FiberId getNextId() noexcept
	{
		return next_id_.fetch_add(1, ::std::memory_order_relaxed);
	}
};

// create an self-ownership fiber
[[nodiscard]] Fiber *createFiber(FiberRoutine &&, INothrowExecutor *);

} // namespace exe::fiber

#endif /* DDV_EXE_FIBER_CORE_FIBER_H_ */
