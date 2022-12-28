#include "context/context.h"

#include "utils/unreachable.h"

namespace context {

void ExecutionContext::exitTo(ExecutionContext &target) noexcept
{
	switchTo(target);

	UTILS_UNREACHABLE("resuming a completed ExecutionContext");
}

} // namespace context
