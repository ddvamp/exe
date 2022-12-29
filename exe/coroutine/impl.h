#ifndef DDV_EXE_COROUTINE_IMPL_H_
#define DDV_EXE_COROUTINE_IMPL_H_ 1

#include <exception>
#include <new>

#include "context/context.h"

#include "exe/coroutine/routine.h"

#include "utils/memory/view.h"

namespace exe::coroutine {

// The basis for suspended executions
class CoroutineImpl : public ::context::ITrampoline {
private:
	Routine routine_;
	::context::ExecutionContext context_;
	::std::exception_ptr throwed_exception_;
	bool is_completed_;

public:
	CoroutineImpl(Routine routine, ::utils::memory_view stack) noexcept
		: routine_(::std::move(routine))
		, context_(stack, this)
		, throwed_exception_(nullptr)
		, is_completed_(false)
	{}

	// external call functions

	bool isCompleted() const noexcept
	{
		return is_completed_;
	}

	void resume();

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
		try {
			routine_();
		} catch (...) {
			throwed_exception_ = ::std::current_exception();
		}

		cancel();
	}
};

} // namespace exe::coroutine

#endif /* DDV_EXE_COROUTINE_IMPL_H_ */
