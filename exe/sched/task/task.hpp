// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_SCHED_TASK_TASK_HPP_INCLUDED_
#define DDVAMP_EXE_SCHED_TASK_TASK_HPP_INCLUDED_ 1

#include <utils/intrusive/forward_list.hpp>

namespace exe::sched::task {

class ITask {
 protected:
  ~ITask() = default;

 public:
  // The user must take care of passing arguments, returning values,
  // and handling exceptions himself
  virtual void Run() noexcept = 0;
};

class TaskBase
    : public ITask,
      public ::utils::intrusive_concurrent_forward_list_node<TaskBase> {
 protected:
  ~TaskBase() = default;
};

}  // namespace exe::sched::task

#endif  /* DDVAMP_EXE_SCHED_TASK_TASK_HPP_INCLUDED_ */
