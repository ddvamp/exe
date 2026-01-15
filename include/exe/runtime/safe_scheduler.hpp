//
// safe_scheduler.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_RUNTIME_SAFE_SCHEDULER_HPP_INCLUDED_
#define DDVAMP_EXE_RUNTIME_SAFE_SCHEDULER_HPP_INCLUDED_ 1

#include <exe/runtime/task/scheduler.hpp>
#include <exe/runtime/task/task.hpp>

#include <util/abort.hpp>

#include <type_traits>

namespace exe::runtime {

/**
 *  A scheduler decorator that aborts the program when
 *  an exception is thrown when scheduling a task
 */
template <task::concepts::UnsafeScheduler S>
class SafeScheduler final : public task::ISafeScheduler {
 private:
  S &underlying_;

 public:
  explicit SafeScheduler(S &underlying) noexcept : underlying_(underlying) {}

  [[nodiscard]] S &GetUnderlying() const noexcept {
    return underlying_;
  }

  void Submit(task::TaskBase *task) noexcept override try {
    if constexpr (::std::is_abstract_v<S>) {
      underlying_.Submit(task);
    } else {
      underlying_.S::Submit(task);
    }
  } catch (...) {
    UTIL_ABORT("An exception was thrown when scheduling the task");
  }
};

} // namespace exe::runtime

#endif /* DDVAMP_EXE_RUNTIME_SAFE_SCHEDULER_HPP_INCLUDED_ */
