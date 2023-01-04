#ifndef DDV_EXE_FIBERS_CORE_FIBER_H_
#define DDV_EXE_FIBERS_CORE_FIBER_H_ 1

#include "exe/executors/executor.h"
#include "exe/executors/task.h"
#include "exe/fibers/api.h"
#include "exe/fibers/core/awaiter.h"
#include "exe/fibers/core/coroutine.h"
#include "exe/fibers/core/stack.h"

namespace exe::fibers {

// Fiber = stackful coroutine + scheduling + executor
class [[nodiscard]] Fiber : public executors::TaskBase {
private:
	::context::Stack stack_;
	Coroutine coroutine_;
	IExecutor *executor_;
	IAwaiter *awaiter_ = nullptr;

public:
	// reference to currently active fiber
	[[nodiscard]] static Fiber &self() noexcept;

	Fiber(FiberRoutine &&, ::context::Stack &&, IExecutor *) noexcept;

	[[nodiscard]] IExecutor *getExecutor() const noexcept
	{
		return executor_;
	}

	// schedule execution on executor set on fiber
	void schedule() noexcept;

	// execute fiber immediately
	void resume() noexcept;

	void suspend(IAwaiter *) noexcept;

	// reschedule current fiber on executor
	void teleportTo(IExecutor *) noexcept;

	// ITask
	void run() noexcept override;

private:
	[[nodiscard]] Fiber *doRun() noexcept;

	void step() noexcept;
	void stop() noexcept;
	void destroySelf() noexcept;
	void releaseResources() noexcept;
};

// create an self-ownership fiber
[[nodiscard]] Fiber *createFiber(FiberRoutine &&, IExecutor *);

} // namespace exe::fibers

#endif /* DDV_EXE_FIBERS_CORE_FIBER_H_ */
