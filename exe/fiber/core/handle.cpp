// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#include <utility>

#include "exe/fiber/core/fiber.hpp"
#include "exe/fiber/core/handle.hpp"

#include "util/debug.hpp"

namespace exe::fiber {

FiberHandle::~FiberHandle()
{
	UTIL_ASSERT(!isValid(), "fiber is lost");
}

FiberHandle &FiberHandle::operator= (FiberHandle &&that) noexcept
{
	UTIL_ASSERT(!isValid(), "fiber is lost");

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
	UTIL_ASSERT(isValid(), "fiber is missing");

	return release();
}

} // namespace exe::fiber
