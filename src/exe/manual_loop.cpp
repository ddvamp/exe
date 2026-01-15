//
// manual_loop.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#include <exe/runtime/manual_loop.hpp>

#include <exe/runtime/task/task.hpp>

#include <util/utility.hpp>
#include <util/debug/assert.hpp>

#include <cstddef>

namespace exe::runtime {

ManualLoop::~ManualLoop() {
  UTIL_ASSERT(IsEmpty(), "ManualLoop contains tasks when destroyed");
}

/* virtual */ void ManualLoop::Submit(task::TaskBase *task) noexcept {
  UTIL_ASSERT(task, "nullptr instead of the task");
  tasks_.push(*task);
}

::std::size_t ManualLoop::RunAtMost(::std::size_t const limit) noexcept {
  auto remains = limit;
  while (::util::all_of(!IsEmpty(), remains != 0)) {
    ::std::move(tasks_.pop()).Run();
    --remains;
  }
  return limit - remains;
}

} // namespace exe::runtime
