// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#include "inline.hpp"

#include <util/debug/assert.hpp>

namespace exe::sched {

/* virtual */ void Inline::Submit(task::TaskBase *task) noexcept {
  UTIL_ASSERT(task, "nullptr instead of the task");
  task->Run();
}

inline class Inline &Inline() noexcept {
  static class Inline instance;
  return instance;
}

}  // namespace exe::sched
