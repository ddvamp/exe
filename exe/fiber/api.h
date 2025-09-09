// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FIBER_API_H_
#define DDV_EXE_FIBER_API_H_ 1

#include "exe/executors/executor.h"
#include "exe/fiber/core/awaiter.h"
#include "exe/fiber/core/id.h"
#include "exe/fiber/core/routine.h"

namespace exe::fiber {

using executors::INothrowExecutor;

// Start fiber on where
//
// Precondition: routine == true
void go(INothrowExecutor &where, FiberRoutine &&routine);

// Start fiber on executor of current fiber
//
// Precondition: fiber context && routine == true
void go(FiberRoutine &&routine);

// precondition: fiber context
namespace self {

[[nodiscard]] FiberId getId() noexcept;

[[nodiscard]] INothrowExecutor &getExecutor() noexcept;

// For synchronization primitives
// Do not use directly
void suspend(IAwaiter &) noexcept;

// reschedule current fiber
void yield() noexcept;

// reschedule current fiber and activate next one if it is valid
// otherwise, the call is equivalent to yield
void switchTo(FiberHandle &&next) noexcept;

// reschedule current fiber on executor
void teleportTo(INothrowExecutor &executor);

} // namespace self

} // namespace exe::fiber

#endif /* DDV_EXE_FIBER_API_H_ */
