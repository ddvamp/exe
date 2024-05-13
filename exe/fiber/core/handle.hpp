// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_FIBER_CORE_HANDLE_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_CORE_HANDLE_HPP_INCLUDED_ 1

#include "fwd.hpp"

namespace exe::fiber {

// Class for managing fiber in awaiters
class [[nodiscard]] FiberHandle {
  friend class Fiber;
 
 private:
  Fiber *fiber_ = nullptr;

 public:
  // Precondition: IsValid() == false
  ~FiberHandle();

  FiberHandle(FiberHandle const &) = delete;
  FiberHandle(FiberHandle &&that) noexcept : fiber_(that.Release()) {}

  // Precondition: IsValid() == false
  FiberHandle &operator= (FiberHandle) noexcept;

 public:
  FiberHandle() = default;

  static FiberHandle Invalid() noexcept {
    return FiberHandle{};
  }

  [[nodiscard]] bool IsValid() const noexcept {
    return fiber_;
  }

  // Schedule execution on executor set on fiber
  //
  // Precondition: IsValid() == true
  void Schedule() && noexcept;

  // Execute fiber immediately
  //
  // Precondition: isValid() == true
  void Resume() && noexcept;

 private:
  explicit FiberHandle(Fiber *fiber) noexcept : fiber_(fiber) {}

  Fiber *Release() noexcept;
  Fiber *ReleaseChecked() noexcept;
};

}  // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_CORE_HANDLE_HPP_INCLUDED_ */
