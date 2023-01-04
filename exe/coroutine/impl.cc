#include "exe/coroutine/impl.h"

#include "utils/debug.h"

namespace exe::coroutine {

void CoroutineImpl::resume()
{
	UTILS_ASSERT(!isCompleted(), "resuming a completed coroutine");

	context_.switchTo(context_);
	if (throwed_exception_) {
		::std::rethrow_exception(throwed_exception_);
	}
}

} // namespace exe::coroutine
