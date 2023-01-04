#ifndef DDV_EXE_COROUTINE_IMPL_H_
#define DDV_EXE_COROUTINE_IMPL_H_ 1

#include <exception>
#include <utility>

#include "context/context.h"

#include "exe/coroutine/routine.h"

#include "utils/defer.h"
#include "utils/memory/view.h"

namespace exe::coroutine {

// The basis for suspended executions
class CoroutineImpl : public ::context::ITrampoline {
private:
	Routine routine_;
	::context::ExecutionContext context_;
	::std::exception_ptr throwed_exception_ = nullptr;
	bool is_active_ = false;
	bool is_completed_ = false;

public:
	CoroutineImpl(Routine &&routine, ::utils::memory_view stack) noexcept
		: routine_(::std::move(routine))
		, context_(stack, this)
	{}

	[[nodiscard]] bool isActive() const noexcept
	{
		return is_active_;
	}

	[[nodiscard]] bool isCompleted() const noexcept
	{
		return is_completed_;
	}

	/* external call functions */

	void resume()
	{
		auto clean_up = ::utils::rollback_exchange(is_active_, true);

		context_.switchToSaved();

		if (throwed_exception_) {
			::std::rethrow_exception(throwed_exception_);
		}
	}

	/* internal call functions */

	void suspend() noexcept
	{
		context_.switchToSaved();
	}

	[[noreturn]] void cancel() noexcept
	{
		is_completed_ = true;
		context_.exitToSaved();
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
