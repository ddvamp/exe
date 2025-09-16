//
// scheduler.hpp
// ~~~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_RUNTIME_TASK_SCHEDULER_HPP_INCLUDED_
#define DDVAMP_EXE_RUNTIME_TASK_SCHEDULER_HPP_INCLUDED_ 1

#include <exe/runtime/task/task.hpp>

#include <concepts>

namespace exe::runtime::task {

struct IScheduler {
 protected:
  // Lifetime cannot be controlled via IScheduler *
  ~IScheduler() = default;

 public:
  virtual void Submit(TaskBase *) = 0;
};

/* Safe means nothrow task scheduling */
struct ISafeScheduler : IScheduler {
 protected:
  // Lifetime cannot be controlled via ISafeScheduler *
  ~ISafeScheduler() = default;

 public:
  void Submit(TaskBase *) noexcept override = 0;
};

////////////////////////////////////////////////////////////////////////////////

namespace concepts {

template <typename S>
concept Scheduler = ::std::derived_from<S, IScheduler>;

template <typename S>
concept SafeScheduler = Scheduler<S> && ::std::derived_from<S, ISafeScheduler>;

template <typename S>
concept UnsafeScheduler = Scheduler<S> && !SafeScheduler<S>;

} // namespace concepts

} // namespace exe::runtime::task

#endif /* DDVAMP_EXE_RUNTIME_TASK_SCHEDULER_HPP_INCLUDED_ */
