#ifndef DDV_CONTEXT_CONTEXT_H_
#define DDV_CONTEXT_CONTEXT_H_ 1

#include "context/exceptions_context.h"
#include "context/machine_context.h"

#include "utils/memory/view.h"

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
	ExecutionContext(::utils::memory_view stack, ITrampoline *trampoline)
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
	[[noreturn]] void exitTo(ExecutionContext &target) noexcept;

	[[noreturn]] void exitToSaved() noexcept
	{
		exitTo(*this);
	}
};

} // namespace context

#endif /* DDV_CONTEXT_CONTEXT_H_ */
