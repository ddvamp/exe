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

#include "exe/fiber/core/fiber.hpp"

#include "util/abort.hpp"
#include "util/debug.hpp"
#include "util/defer.hpp"

namespace exe::fiber {

namespace {

thread_local Fiber *current = nullptr;

[[nodiscard]] bool amIFiber() noexcept
{
	return current;
}

class YieldAwaiter : public ISuspendingAwaiter {
public:
	void awaitSuspend(FiberHandle &&h) noexcept override
	{
		::std::move(h).schedule();
	}
};

class SwitchAwaiter : public IAwaiter {
private:
	FiberHandle &target_;

public:
	explicit SwitchAwaiter(FiberHandle &target) noexcept
		: target_(target)
	{}

	void awaitSuspend(FiberHandle &&) noexcept override
	{
		// nothing
	}

	[[nodiscard]] FiberHandle awaitSymmetricSuspend(FiberHandle &&from) override
	{
		// prevents race condition
		auto cleanup = ::util::defer{
			[&from]() noexcept {
				::std::move(from).schedule();
			}
		};

		return ::std::move(target_);
	}
};

} // namespace


////////////////////////////////////////////////////////////////////////////////


/* static */ Fiber &Fiber::self() noexcept
{
	UTIL_ASSERT(amIFiber(), "not in the fiber context");

	return *current;
}

Fiber::Fiber(FiberRoutine &&routine, ::context::Stack &&stack,
	ISafeScheduler *scheduler) noexcept
	: stack_(::std::move(stack))
	, coroutine_(::std::move(routine), stack_.View())
	, scheduler_(scheduler)
{}

void Fiber::schedule() noexcept
{
	scheduler_->Submit(this);
}

void Fiber::resume() noexcept
{
	Run();
}

void Fiber::suspend(IAwaiter *awaiter) noexcept
{
	awaiter_ = awaiter;

	stop();
}

void Fiber::teleportTo(ISafeScheduler *scheduler) noexcept
{
	scheduler_ = scheduler;

	self::yield();
}

void Fiber::releaseResources() noexcept
{
	deallocateStack(::std::move(stack_));
}

void Fiber::step() noexcept
{
	auto const prev = ::std::exchange(current, this);
	coroutine_.resume();
	current = prev;
}

Fiber *Fiber::doRun() noexcept
{
	step();

	if (coroutine_.isCompleted()) [[unlikely]] {
		destroySelf();
		return nullptr;
	}

	auto awaiter = ::std::exchange(awaiter_, nullptr);

	UTIL_ASSUME(awaiter, "nullptr instead of awaiter");

	auto next = awaiter->awaitSymmetricSuspend(FiberHandle{this});

	return next.release();
}

/* virtual */ void Fiber::Run() noexcept
{
	auto next = this;

	while ((next = next->doRun()));
}

void Fiber::destroySelf() noexcept
{
	releaseResources();
	delete this;
}

void Fiber::stop() noexcept
{
	coroutine_.suspend();
}

Fiber *createFiber(FiberRoutine &&routine, ISafeScheduler *scheduler)
{
	return new Fiber{::std::move(routine), allocateStack(), scheduler};
}


////////////////////////////////////////////////////////////////////////////////


/* API */

namespace self {

FiberId getId() noexcept
{
	return Fiber::self().getId();
}

ISafeScheduler &getScheduler() noexcept
{
	return *Fiber::self().getScheduler();
}

void suspend(IAwaiter &awaiter) noexcept
{
	Fiber::self().suspend(&awaiter);
}

void yield() noexcept
{
	YieldAwaiter awaiter;
	suspend(awaiter);
}

void switchTo(FiberHandle &&next) noexcept
{
	SwitchAwaiter awaiter{next};
	suspend(awaiter);
}

void teleportTo(ISafeScheduler &scheduler)
{
	Fiber::self().teleportTo(&scheduler);
}

} // namespace self

void go(ISafeScheduler &scheduler, FiberRoutine &&routine)
{
	UTIL_ASSERT(routine, "empty routine for fiber");

	auto fiber = createFiber(::std::move(routine), &scheduler);

	fiber->schedule();
}

void go(FiberRoutine &&routine)
{
	go(self::getScheduler(), ::std::move(routine));
}

} // namespace exe::fiber
