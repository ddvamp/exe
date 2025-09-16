//
// awaiter.hpp
// ~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FIBER_CORE_AWAITER_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_CORE_AWAITER_HPP_INCLUDED_ 1

#include <exe/fiber/core/handle.hpp>

namespace exe::fiber {

struct IAwaiter {
 protected:
  // Lifetime cannot be controlled via IAwaiter *
  ~IAwaiter() = default;

 public:
  virtual FiberHandle AwaitSymmetricSuspend(FiberHandle &&self) noexcept = 0;
};

} // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_CORE_AWAITER_HPP_INCLUDED_ */
