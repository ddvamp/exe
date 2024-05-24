// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_SCHED_SAFE_SCHEDULER_HPP_INCLUDED_
#define DDVAMP_EXE_SCHED_SAFE_SCHEDULER_HPP_INCLUDED_ 1

#include <type_traits>

#include <utils/abort.hpp>

#include "task/scheduler.hpp"

namespace exe::sched {

// A scheduler decorator that aborts the program when
// an exception is thrown when scheduling a task
template <task::concepts::UnsafeScheduler S>
class SafeScheduler : public task::ISafeScheduler {
 private:
  S &underlying_;

 public:
  explicit SafeScheduler(S &underlying) noexcept : underlying_(underlying) {}

  void Submit(task::TaskBase *task) noexcept override {
    try {
      if constexpr (::std::is_abstract_v<S>) {
        underlying_.Submit(task);
      } else {
        underlying_.S::Submit(task);
      }
    } catch (...) {
      UTILS_ABORT("An exception was thrown when scheduling the task");
    }
  }

  [[nodiscard]] S &GetUnderlying() const noexcept {
    return underlying_;
  }
};

}  // namespace exe::sched

#endif /* DDVAMP_EXE_SCHED_SAFE_SCHEDULER_HPP_INCLUDED_ */
