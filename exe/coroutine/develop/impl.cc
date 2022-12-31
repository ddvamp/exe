#include "exe/coroutine/develop/impl.h"

#include "utils/debug.h"

namespace exe::coroutine {

void CoroutineImpl::resume() noexcept
{
	UTILS_ASSERT(
		!is_completed_,
		"resume, but the coroutine has already been completed"
	);

	context_.switchTo(context_);
}

} // namespace exe::coroutine
