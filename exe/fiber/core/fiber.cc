// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#include <utility>

#include "exe/fiber/core/fiber.h"

#include "util/abort.h"
#include "util/debug.h"
#include "util/defer.h"

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
	UTILS_ASSERT(amIFiber(), "not in the fiber context");

	return *current;
}

Fiber::Fiber(FiberRoutine &&routine, ::context::Stack &&stack,
	INothrowExecutor *executor) noexcept
	: stack_(::std::move(stack))
	, coroutine_(::std::move(routine), stack_.view())
	, executor_(executor)
{}

void Fiber::schedule() noexcept
{
	executor_->submit(this);
}

void Fiber::resume() noexcept
{
	run();
}

void Fiber::suspend(IAwaiter *awaiter) noexcept
{
	awaiter_ = awaiter;

	stop();
}

void Fiber::teleportTo(INothrowExecutor *executor) noexcept
{
	executor_ = executor;

	self::yield();
}

void Fiber::releaseResources() noexcept
{
	deallocateStack(::std::move(stack_));
}

void Fiber::step() noexcept
{
	auto rollback = ::util::rollback_exchange(current, this);

	coroutine_.resume();
}

Fiber *Fiber::doRun() noexcept
{
	step();

	if (coroutine_.isCompleted()) [[unlikely]] {
		destroySelf();
		return nullptr;
	}

	auto awaiter = ::std::exchange(awaiter_, nullptr);

	UTILS_ASSUME(awaiter, "nullptr instead of awaiter");

	auto next = awaiter->awaitSymmetricSuspend(FiberHandle{this});

	return next.release();
}

/* virtual */ void Fiber::run() noexcept
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

Fiber *createFiber(FiberRoutine &&routine, INothrowExecutor *executor)
{
	return new Fiber{::std::move(routine), allocateStack(), executor};
}


////////////////////////////////////////////////////////////////////////////////


/* API */

namespace self {

FiberId getId() noexcept
{
	return Fiber::self().getId();
}

INothrowExecutor &getExecutor() noexcept
{
	return *Fiber::self().getExecutor();
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

void teleportTo(INothrowExecutor &executor)
{
	Fiber::self().teleportTo(&executor);
}

} // namespace self

void go(INothrowExecutor &executor, FiberRoutine &&routine)
{
	UTILS_ASSERT(routine, "empty routine for fiber");

	auto fiber = createFiber(::std::move(routine), &executor);

	fiber->schedule();
}

void go(FiberRoutine &&routine)
{
	go(self::getExecutor(), ::std::move(routine));
}

} // namespace exe::fiber
