#include <utility>

#include "exe/fibers/core/fiber.h"
#include "exe/fibers/core/handle.h"

#include "utils/debug.h"

namespace exe::fibers {

// Schedule execution on the executor set on fiber
//
// Precondition: isValid() == true
void FiberHandle::schedule() noexcept
{
	releaseChecked()->schedule();
}

// Execute fiber immediately
//
// Precondition: isValid() == true
void FiberHandle::resume() noexcept
{
	releaseChecked()->resume();
}

Fiber *FiberHandle::release() noexcept
{
	return std::exchange(fiber_, nullptr);
}

Fiber *FiberHandle::releaseChecked() noexcept
{
	UTILS_ASSERT(isValid(), "handle is in moved-from state");

	return release();
}

} // namespace exe::fibers
