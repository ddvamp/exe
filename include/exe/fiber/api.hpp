//
// api.hpp
// ~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FIBER_API_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_API_HPP_INCLUDED_ 1

#include <exe/fiber/core/awaiter.hpp>
#include <exe/fiber/core/body.hpp>
#include <exe/fiber/core/fwd.hpp>
#include <exe/fiber/core/handle.hpp>
#include <exe/fiber/core/id.hpp>
#include <exe/fiber/core/scheduler.hpp>

namespace exe::fiber {

/**
 *  Start fiber on where
 *
 *  Precondition: body == true
 */
void Go(Scheduler &where, Body &&body);

/**
 *  Start fiber on scheduler of current fiber
 *
 *  Precondition: body == true && in fiber context
 */
void Go(Body &&body);

////////////////////////////////////////////////////////////////////////////////

/* Precondition: in fiber context */

namespace self {

[[nodiscard]] FiberId GetId() noexcept;

[[nodiscard]] Scheduler &GetScheduler() noexcept;

/* For synchronization primitives. Do not use directly */
void Suspend(IAwaiter &) noexcept;

/* Reschedule current fiber */
void Yield() noexcept;

/**
 *  Reschedule current fiber and activate next one if it is valid
 *  otherwise, the call is equivalent to yield
 */
void SwitchTo(FiberHandle &&next) noexcept;

/* Reschedule current fiber on scheduler */
void TeleportTo(Scheduler &scheduler) noexcept;

} // namespace self

////////////////////////////////////////////////////////////////////////////////

/* Prevents context switching until the end of the scope */
class NoSwitchContextGuard {
 private:
  Fiber *self;

 public:
  NoSwitchContextGuard() noexcept;
  ~NoSwitchContextGuard();

  NoSwitchContextGuard(NoSwitchContextGuard const &) = delete;
  void operator= (NoSwitchContextGuard const &) = delete;

  NoSwitchContextGuard(NoSwitchContextGuard &&) = delete;
  void operator= (NoSwitchContextGuard &&) = delete;
};

} // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_API_HPP_INCLUDED_ */
