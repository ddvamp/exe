#include "context/trampoline.h"

#include "utils/debug.h"

namespace context {

void ITrampoline::run() noexcept
{
	doRun();

	UTILS_UNREACHABLE("ITrampoline::run out of bounds");
}

} // namespace context
