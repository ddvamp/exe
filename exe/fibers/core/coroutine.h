#ifndef DDV_EXE_FIBERS_CORE_COROUTINE_H_
#define DDV_EXE_FIBERS_CORE_COROUTINE_H_ 1

#include <utility>

#include "context/context.h"

#include "exe/fibers/core/routine.h"

#include "utils/memory/view.h"

namespace exe::fibers {

// The basis for suspended execution of fibers
class Coroutine : public ::context::ITrampoline {
private:
	FiberRoutine routine_;
	::context::ExecutionContext context_;
	bool is_completed_;

public:
	Coroutine(FiberRoutine routine, ::utils::memory_view stack) noexcept
		: routine_(::std::move(routine))
		, context_(stack, this)
		, is_completed_(false)
	{}

	// external call functions

	bool isCompleted() const noexcept
	{
		return is_completed_;
	}

	void resume() noexcept;

	// internal call functions

	void suspend() noexcept
	{
		context_.switchTo(context_);
	}

	[[noreturn]] void cancel() noexcept
	{
		is_completed_ = true;
		context_.exitTo(context_);
	}

private:
	[[noreturn]] void doRun() noexcept override
	{
		routine_();

		cancel();
	}
};

} // namespace exe::fibers

#endif /* DDV_EXE_FIBERS_CORE_COROUTINE_H_ */
