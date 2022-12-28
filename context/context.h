#ifndef DDV_CONTEXT_CONTEXT_H_
#define DDV_CONTEXT_CONTEXT_H_ 1

#include "context/exceptions_context.h"
#include "context/machine_context.h"

namespace context {

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
	ExecutionContext(void *stack, ITrampoline *trampoline) noexcept
	{
		machine_ctx_.setup(stack, trampoline);
	}

	// external/internal call
	void switchTo(ExecutionContext &target) noexcept
	{
		exceptions_ctx_.switchTo(target.exceptions_ctx_);
		machine_ctx_.switchTo(target.machine_ctx_);
	}

	// internal call
	[[noreturn]] void exitTo(ExecutionContext &target) noexcept;
};

} // namespace context

#endif /* DDV_CONTEXT_CONTEXT_H_ */
