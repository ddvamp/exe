#include <utility>

#include "exe/fibers/core/fiber.h"

#include "utils/abort.h"
#include "utils/debug.h"
#include "utils/defer.h"

namespace exe::fibers {

namespace {

thread_local Fiber *current = nullptr;

[[nodiscard]] bool amIFiber() noexcept
{
	return current;
}

class YieldAwaiter : public ISuspendingAwaiter {
public:
	void awaitSuspend(FiberHandle h) noexcept override
	{
		h.schedule();
	}
};

class SwitchAwaiter : public IAwaiter {
private:
	FiberHandle target_;

public:
	explicit SwitchAwaiter(FiberHandle target) noexcept
		: target_(target)
	{}

	void awaitSuspend(FiberHandle) noexcept override
	{
		// nothing
	}

	FiberHandle awaitSymmetricSuspend(FiberHandle from) override
	{
		// prevents race condition
		auto target = target_;

		from.schedule();

		return target;
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
	IExecutor *executor) noexcept
	: stack_(::std::move(stack))
	, coroutine_(::std::move(routine), stack_.view())
	, executor_(executor)
{}

void Fiber::schedule() noexcept
{
	// TODO: exception handling
	try {
		executor_->execute(this);
	} catch (...) {
		UTILS_ABORT("exception while trying to schedule fiber");
	}
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

void Fiber::teleportTo(IExecutor *executor) noexcept
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
	auto rollback = ::utils::rollback_exchange(current, this);

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

	[[assume(awaiter)]];

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

Fiber *createFiber(FiberRoutine &&routine, IExecutor *executor)
{
	return new Fiber{::std::move(routine), allocateStack(), executor};
}


////////////////////////////////////////////////////////////////////////////////


/* API */

namespace self {

IExecutor &getExecutor() noexcept
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

void switchTo(FiberHandle next) noexcept
{
	SwitchAwaiter awaiter{next};
	suspend(awaiter);
}

void teleportTo(IExecutor &executor)
{
	Fiber::self().teleportTo(&executor);
}

} // namespace self

void go(IExecutor &executor, FiberRoutine &&routine)
{
	UTILS_ASSERT(bool(routine), "empty routine for fiber");

	auto *fiber = createFiber(::std::move(routine), &executor);

	fiber->schedule();
}

void go(FiberRoutine &&routine)
{
	go(self::getExecutor(), ::std::move(routine));
}

} // namespace exe::fibers
