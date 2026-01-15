//
// inline.hpp
// ~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_RUNTIME_INLINE_HPP_INCLUDED_
#define DDVAMP_EXE_RUNTIME_INLINE_HPP_INCLUDED_ 1

#include <exe/runtime/task/scheduler.hpp>
#include <exe/runtime/task/task.hpp>

#include <util/debug/assert.hpp>

#include <utility>

namespace exe::runtime {

/* Executes task immediately at place */
class Inline : public task::ISafeScheduler {
 public:
  void Submit(task::TaskBase *task) noexcept override {
    UTIL_ASSERT(task, "nullptr instead of the task");
    ::std::move(*task).Run();
  }
};

[[nodiscard]] inline Inline &GetInline() noexcept {
  static Inline instance;
  return instance;
}

} // namespace exe::runtime

#endif /* DDVAMP_EXE_RUNTIME_INLINE_HPP_INCLUDED_ */
