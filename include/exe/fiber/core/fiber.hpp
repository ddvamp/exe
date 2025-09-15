//
//
//
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FIBER_CORE_FIBER_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_CORE_FIBER_HPP_INCLUDED_ 1

#include <atomic>

#include "exe/runtime/task/scheduler.hpp"
#include "exe/runtime/task/task.hpp"
#include "exe/fiber/api.hpp"
#include "exe/fiber/core/awaiter.hpp"
#include "exe/fiber/core/coroutine.hpp"
#include "exe/fiber/core/id.hpp"
#include "exe/fiber/core/stack.hpp"

namespace exe::fiber {

// Fiber = stackful coroutine + scheduling + scheduler
class [[nodiscard]] Fiber : public runtime::task::TaskBase {
private:
	::context::Stack stack_;
	Coroutine coroutine_;
	ISafeScheduler *scheduler_;
	IAwaiter *awaiter_ = nullptr;
	FiberId const id_ = getNextId();

	inline static ::std::atomic<FiberId> next_id_ = kInvalidFiberId + 1;

public:
	// reference to currently active fiber
	[[nodiscard]] static Fiber &self() noexcept;

	Fiber(Body &&, ::context::Stack &&, ISafeScheduler *) noexcept;

	[[nodiscard]] FiberId getId() const noexcept
	{
		return id_;
	}

	[[nodiscard]] ISafeScheduler *getScheduler() const noexcept
	{
		return scheduler_;
	}

	// schedule execution on scheduler set on fiber
	void schedule() noexcept;

	// execute fiber immediately
	void resume() noexcept;

	void suspend(IAwaiter *) noexcept;

	// reschedule current fiber on scheduler
	void teleportTo(ISafeScheduler *) noexcept;

	// ITask
	void Run() noexcept override;

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
[[nodiscard]] Fiber *createFiber(Body &&, ISafeScheduler *);

} // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_CORE_FIBER_HPP_INCLUDED_ */
