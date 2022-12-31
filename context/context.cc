#include "context/context.h"

#include "utils/debug.h"

namespace context {

ExecutionContext::ExecutionContext(::utils::memory_view stack,
	ITrampoline *trampoline) noexcept
{
	UTILS_ASSERT(stack.data(), "invalid stack in ExecutionContext");
	UTILS_ASSERT(!stack.empty(), "invalid stack in ExecutionContext");
	UTILS_ASSERT(trampoline, "invalid trampoline in ExecutionContext");

	machine_ctx_.setup(stack, trampoline);
}

void ExecutionContext::exitTo(ExecutionContext &target) noexcept
{
	switchTo(target);

	UTILS_UNREACHABLE("resuming a completed ExecutionContext");
}

} // namespace context
