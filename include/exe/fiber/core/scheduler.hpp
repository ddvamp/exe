//
// scheduler.hpp
// ~~~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FIBER_CORE_SCHEDULER_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_CORE_SCHEDULER_HPP_INCLUDED_ 1

#include <exe/runtime/task/scheduler.hpp>

namespace exe::fiber {

using Scheduler = runtime::task::ISafeScheduler;

} // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_CORE_SCHEDULER_HPP_INCLUDED_ */
