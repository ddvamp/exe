//
// task.hpp
// ~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_RUNTIME_TASK_TASK_HPP_INCLUDED_
#define DDVAMP_EXE_RUNTIME_TASK_TASK_HPP_INCLUDED_ 1

#include <concurrency/intrusive/forward_list.hpp>

namespace exe::runtime::task {

struct ITask {
 protected:
  // Lifetime cannot be controlled via ITask *
  ~ITask() = default;

 public:
  // The user must take care of passing arguments, returning values,
  // and handling exceptions himself
  virtual void Run() && noexcept = 0;
};

struct TaskBase : ITask, ::concurrency::IntrusiveForwardListNode<TaskBase> {
 protected:
  // Lifetime cannot be controlled via TaskBase *
  ~TaskBase() = default;
};

} // namespace exe::runtime::task

#endif /* DDVAMP_EXE_RUNTIME_TASK_TASK_HPP_INCLUDED_ */
