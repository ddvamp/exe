#include "exe/coroutine/impl.h"

#include "utils/assert.h"

namespace exe::coroutine {

void CoroutineImpl::resume()
{
	UTILS_ASSERT(
		!is_completed_,
		"resume, but the coroutine has already been completed"
	);

	context_.switchTo(context_);
	if (throwed_exception_) {
		::std::rethrow_exception(throwed_exception_);
	}
}

} // namespace exe::coroutine
