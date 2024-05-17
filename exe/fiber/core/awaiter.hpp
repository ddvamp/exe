// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_FIBER_CORE_AWAITER_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_CORE_AWAITER_HPP_INCLUDED_ 1

#include <utility>

#include "handle.hpp"

namespace exe::fiber {

struct IAwaiter {
  virtual ~IAwaiter() = default;

  virtual void AwaitSuspend(FiberHandle &&) = 0;
  virtual FiberHandle AwaitSymmetricSuspend(FiberHandle &&) = 0;
};

struct ISuspendingAwaiter : IAwaiter {
  FiberHandle AwaitSymmetricSuspend(FiberHandle &&h) override {
    AwaitSuspend(::std::move(h));
    return FiberHandle::Invalid();
  }
};

}  // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_CORE_AWAITER_HPP_INCLUDED_ */
