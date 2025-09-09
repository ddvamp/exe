// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FIBER_API_H_
#define DDV_EXE_FIBER_API_H_ 1

#include "exe/runtime/task/scheduler.h"
#include "exe/fiber/core/awaiter.h"
#include "exe/fiber/core/id.h"
#include "exe/fiber/core/routine.h"

namespace exe::fiber {

using runtime::ISafeScheduler;

// Start fiber on where
//
// Precondition: routine == true
void go(ISafeScheduler &where, FiberRoutine &&routine);

// Start fiber on scheduler of current fiber
//
// Precondition: fiber context && routine == true
void go(FiberRoutine &&routine);

// precondition: fiber context
namespace self {

[[nodiscard]] FiberId getId() noexcept;

[[nodiscard]] ISafeScheduler &getScheduler() noexcept;

// For synchronization primitives
// Do not use directly
void suspend(IAwaiter &) noexcept;

// reschedule current fiber
void yield() noexcept;

// reschedule current fiber and activate next one if it is valid
// otherwise, the call is equivalent to yield
void switchTo(FiberHandle &&next) noexcept;

// reschedule current fiber on scheduler
void teleportTo(ISafeScheduler &scheduler);

} // namespace self

} // namespace exe::fiber

#endif /* DDV_EXE_FIBER_API_H_ */
