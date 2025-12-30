//
// handle.cpp
// ~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#include <exe/fiber/core/fiber.hpp>
#include <exe/fiber/core/handle.hpp>

#include <util/debug/assert.hpp>

#include <utility>

namespace exe::fiber {

FiberHandle::~FiberHandle() {
  UTIL_ASSERT(!IsValid(), "Fiber is lost");
}

FiberHandle &FiberHandle::operator= (FiberHandle that) noexcept {
  ::std::swap(fiber_, that.fiber_);
  return *this;
}

void FiberHandle::Schedule() && noexcept {
  ReleaseChecked()->Schedule();
}

void FiberHandle::Resume() && noexcept {
  ::std::move(*this).Schedule();
}

Fiber *FiberHandle::Release() noexcept {
  return ::std::exchange(fiber_, nullptr);
}

Fiber *FiberHandle::ReleaseChecked() noexcept {
  UTIL_ASSERT(IsValid(), "Fiber is missing, but expected");
  return Release();
}

} // namespace exe::fiber
