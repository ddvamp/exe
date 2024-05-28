// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#include "handle.hpp"

#include <utility>

#include <util/debug/assert.hpp>

#include "fiber.hpp"

namespace exe::fiber {

FiberHandle::~FiberHandle() {
  UTIL_ASSERT(!IsValid(), "fiber is lost");
}

FiberHandle &FiberHandle::operator= (FiberHandle that) noexcept {
  ::std::swap(fiber_, that.fiber_);
  return *this;
}

void FiberHandle::Schedule() && noexcept {
  ReleaseChecked()->Schedule();
}

void FiberHandle::Resume() && noexcept {
  ReleaseChecked()->Resume();
}

Fiber *FiberHandle::Release() noexcept {
  return ::std::exchange(fiber_, nullptr);
}

Fiber *FiberHandle::ReleaseChecked() noexcept {
  UTIL_ASSERT(IsValid(), "fiber is missing");
  return Release();
}

}  // namespace exe::fiber
