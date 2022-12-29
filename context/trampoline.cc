#include "context/trampoline.h"

#include "utils/unreachable.h"

namespace context {

void ITrampoline::run() noexcept
{
	doRun();

	UTILS_UNREACHABLE("ITrampoline::run out of bounds");
}

} // namespace context
