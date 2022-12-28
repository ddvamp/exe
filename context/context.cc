#include "context/context.h"

#include "utils/abort.h"

namespace context {

void ExecutionContext::exitTo(ExecutionContext &target) noexcept
{
	switchTo(target);

	::utils::abort("resuming a completed ExecutionContext");
}

} // namespace context
