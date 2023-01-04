#include "context/context.h"

#include "utils/debug.h"

namespace context {

void ExecutionContext::exitTo(ExecutionContext &target) noexcept
{
	switchTo(target);

	UTILS_UNREACHABLE("resuming a completed ExecutionContext");
}

} // namespace context
