// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_SCHED_TASK_TASK_HPP_INCLUDED_
#define DDVAMP_EXE_SCHED_TASK_TASK_HPP_INCLUDED_ 1

#include <concurrency/intrusive/forward_list.hpp>

namespace exe::sched::task {

struct ITask {
 protected:
  ~ITask() = default;

 public:
  // The user must take care of passing arguments, returning values,
  // and handling exceptions himself
  virtual void Run() noexcept = 0;
};

struct TaskBase : ITask, ::concurrency::IntrusiveForwardListNode<TaskBase> {
 protected:
  ~TaskBase() = default;
};

} // namespace exe::sched::task

#endif /* DDVAMP_EXE_SCHED_TASK_TASK_HPP_INCLUDED_ */
