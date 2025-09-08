// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_CONTEXT_CONTEXT_H_
#define DDV_CONTEXT_CONTEXT_H_ 1

#include "context/exceptions_context.h"
#include "context/machine_context.h"

#include "util/debug.h"
#include "util/memory/view.h"

namespace context {

// Execution context = machine context (registers state) + exceptions context
class ExecutionContext {
private:
	MachineContext machine_ctx_;
	ExceptionsContext exceptions_ctx_;

public:
	~ExecutionContext() = default;

	ExecutionContext(ExecutionContext const &) = delete;
	void operator= (ExecutionContext const &) = delete;

	ExecutionContext(ExecutionContext &&) = delete;
	void operator= (ExecutionContext &&) = delete;

public:
	// empty slot for current context saving
	ExecutionContext() = default;

	// initializes new context with stack using trampoline
	ExecutionContext(::util::memory_view stack, ITrampoline *trampoline)
		noexcept
	{
		machine_ctx_.setup(stack, trampoline);
	}

	// save current context in this and reset target context
	// (this and target are allowed to be aliased)
	void switchTo(ExecutionContext &target) noexcept
	{
		exceptions_ctx_.switchTo(target.exceptions_ctx_);
		machine_ctx_.switchTo(target.machine_ctx_);
	}

	void switchToSaved() noexcept
	{
		switchTo(*this);
	}

	// last context switch
	[[noreturn]] void exitTo(ExecutionContext &target) noexcept
	{
		switchTo(target);
		UTILS_UNREACHABLE("resuming a completed ExecutionContext");
	}

	[[noreturn]] void exitToSaved() noexcept
	{
		exitTo(*this);
	}
};

} // namespace context

#endif /* DDV_CONTEXT_CONTEXT_H_ */
