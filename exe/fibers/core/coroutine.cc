#include "exe/fibers/core/coroutine.h"

#include "utils/debug.h"

namespace exe::fibers {

void Coroutine::resume() noexcept
{
	UTILS_ASSERT(!is_completed_, "resuming a completed fiber");

	context_.switchTo(context_);
}

} // namespace exe::fibers
