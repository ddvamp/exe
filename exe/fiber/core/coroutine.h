// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FIBER_CORE_COROUTINE_H_
#define DDV_EXE_FIBER_CORE_COROUTINE_H_ 1

#include <utility>

#include "context/context.h"

#include "exe/fiber/core/routine.h"

#include "util/memory/view.h"

namespace exe::fiber {

// Basis for suspended execution of fiber
class Coroutine : public ::context::ITrampoline {
private:
	FiberRoutine routine_;
	::context::ExecutionContext context_;
	bool is_completed_ = false;

public:
	Coroutine(FiberRoutine &&routine, ::util::memory_view stack) noexcept
		: routine_(::std::move(routine))
		, context_(stack, this)
	{}

	[[nodiscard]] bool isCompleted() const noexcept
	{
		return is_completed_;
	}

	/* external call functions */

	void resume() noexcept
	{
		context_.switchToSaved();
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
		routine_();

		cancel();
	}
};

} // namespace exe::fiber

#endif /* DDV_EXE_FIBER_CORE_COROUTINE_H_ */
