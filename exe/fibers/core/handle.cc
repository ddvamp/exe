#include <utility>

#include "exe/fibers/core/fiber.h"
#include "exe/fibers/core/handle.h"

#include "utils/debug.h"

namespace exe::fibers {

FiberHandle::~FiberHandle()
{
	UTILS_ASSERT(!isValid(), "fiber is lost");
}

FiberHandle &FiberHandle::operator= (FiberHandle &&that) noexcept
{
	UTILS_ASSERT(!isValid(), "fiber is lost");

	fiber_ = that.release();

	return *this;
}

void FiberHandle::schedule() && noexcept
{
	releaseChecked()->schedule();
}

void FiberHandle::resume() && noexcept
{
	releaseChecked()->resume();
}

Fiber *FiberHandle::release() noexcept
{
	return ::std::exchange(fiber_, nullptr);
}

Fiber *FiberHandle::releaseChecked() noexcept
{
	UTILS_ASSERT(isValid(), "fiber is missing");

	return release();
}

} // namespace exe::fibers
