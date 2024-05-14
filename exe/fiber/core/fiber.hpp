// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_FIBER_CORE_FIBER_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_CORE_FIBER_HPP_INCLUDED_ 1

#include <atomic>

#include "awaiter.hpp"
#include "coroutine.hpp"
#include <exe/sched/task/task.hpp>
#include <exe/fiber/api.hpp>
#include "id.hpp"
#include "scheduler.hpp"
#include "stack.hpp"

namespace exe::fiber {

// Fiber = stackful coroutine + scheduler
class [[nodiscard]] Fiber : public sched::task::TaskBase {
 private:
  Stack stack_;
  Coroutine coroutine_;
  IScheduler *scheduler_;
  IAwaiter *awaiter_ = nullptr;
  FiberId const id_ = GetNextId();

 public:
  // Reference to currently active fiber
  [[nodiscard]] static Fiber &Self() noexcept;

  Fiber(Body &&, Stack &&, IScheduler *) noexcept;

  [[nodiscard]] FiberId GetId() const noexcept {
    return id_;
  }

  [[nodiscard]] IScheduler *GetScheduler() const noexcept {
    return scheduler_;
  }

  // Schedule execution on scheduler set on fiber
  void Schedule() noexcept;

  // Execute fiber immediately
  void Resume() noexcept;

  void Suspend(IAwaiter *) noexcept;

  // Reschedule current fiber on scheduler
  void TeleportTo(IScheduler *) noexcept;

  // ITask
  void Run() noexcept override;

 private:
  [[nodiscard]] Fiber *DoRun() noexcept;

  void Step() noexcept;
  void Stop() noexcept;
  void DestroySelf() noexcept;
  void ReleaseResources() noexcept;

  [[nodiscard]] static FiberId GetNextId() noexcept;
};

// Create an self-ownership fiber
[[nodiscard]] Fiber *CreateFiber(Body &&, IScheduler *);

}  // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_CORE_FIBER_HPP_INCLUDED_ */
