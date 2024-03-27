// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_SCHED_TASK_SCHEDULER_HPP_INCLUDED_
#define DDVAMP_EXE_SCHED_TASK_SCHEDULER_HPP_INCLUDED_ 1

#include <concepts>

#include "task.hpp"

namespace exe::sched::task {

class IScheduler {
 public:
  virtual ~IScheduler() = default;

  virtual void Submit(TaskBase *) = 0;
};

class INothrowScheduler : public IScheduler {
 public:
  void Submit(TaskBase *) noexcept override = 0;
};


namespace concepts {

template <typename S>
concept Scheduler = ::std::derived_from<S, IScheduler>;

template <typename S>
concept NothrowScheduler = Scheduler<S> &&
                           ::std::derived_from<S, INothrowScheduler>;

}  // namespace concepts

}  // namespace exe::sched::task

#endif  /* DDVAMP_EXE_SCHED_TASK_SCHEDULER_HPP_INCLUDED_ */
