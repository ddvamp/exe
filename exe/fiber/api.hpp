// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_FIBER_API_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_API_HPP_INCLUDED_ 1

#include "core/awaiter.hpp"
#include "core/body.hpp"
#include "core/id.hpp"
#include "core/scheduler.hpp"

namespace exe::fiber {

// Start fiber on where
//
// Precondition: body == true
void Go(IScheduler &where, Body &&body);

// Start fiber on scheduler of current fiber
//
// Precondition: body == true && in fiber context
void Go(Body &&body);

// Precondition: in fiber context
namespace self {

[[nodiscard]] FiberId GetId() noexcept;

[[nodiscard]] IScheduler &GetScheduler() noexcept;

// For synchronization primitives. Do not use directly
void Suspend(IAwaiter &) noexcept;

// Reschedule current fiber
void Yield() noexcept;

// Reschedule current fiber and activate next one if it is valid
// otherwise, the call is equivalent to yield
void SwitchTo(FiberHandle &&next) noexcept;

// Reschedule current fiber on scheduler
void TeleportTo(IScheduler &scheduler);

}  // namespace self

}  // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_API_HPP_INCLUDED_ */
