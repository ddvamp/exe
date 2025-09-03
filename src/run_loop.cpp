//
// run_loop.cpp
// ~~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#include <exe/runtime/run_loop.hpp>
#include <exe/runtime/task/task.hpp>

#include <util/utility.hpp>
#include <util/debug/assert.hpp>

#include <atomic>
#include <cstddef>

namespace exe::runtime {

RunLoop::~RunLoop() {
  UTIL_ASSERT(IsEmpty(), "RunLoop contains tasks when destroyed");
}

/* virtual */ void RunLoop::Submit(task::TaskBase *task) noexcept {
  UTIL_ASSERT(task, "nullptr instead of the task");
  task->Link(nullptr);
  if (IsEmpty()) {
    head_ = tail_ = task;
  } else {
    tail_->Link(task);
    tail_ = task;
  }
}

::std::size_t RunLoop::RunAtMost(::std::size_t const limit) noexcept {
  auto remains = limit;
  while (::util::all_of(!IsEmpty(), remains != 0)) {
    Pop()->Run();
    --remains;
  }
  return limit - remains;
}

task::TaskBase *RunLoop::Pop() noexcept {
  return ::std::exchange(head_, head_->next_.load(::std::memory_order_relaxed));
}

} // namespace exe::runtime
