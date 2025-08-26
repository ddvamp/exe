// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_FIBER_CORE_FIBER_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_CORE_FIBER_HPP_INCLUDED_ 1

#include "awaiter.hpp"
#include "coroutine.hpp"
#include "id.hpp"
#include "scheduler.hpp"
#include "stack.hpp"
#include <exe/sched/task/task.hpp>

#include <atomic>

namespace exe::fiber {

// Fiber = stackful coroutine + scheduler
class [[nodiscard]] Fiber final : private sched::task::TaskBase {
 private:
  Stack stack_;
  Coroutine coroutine_;
  IScheduler *scheduler_;
  IAwaiter *awaiter_ = nullptr;
  FiberId const id_ = GetNextId();

  inline static ::std::atomic<FiberId> next_id_ = kInvalidFiberId + 1;

  // To guarantee the expected implementation
  static_assert(::std::atomic<FiberId>::is_always_lock_free);

 public:
  // Create an self-ownership fiber
  [[nodiscard]] static Fiber *Create(Body &&, IScheduler *);

  // Reference to currently active fiber
  [[nodiscard]] static Fiber &Self() noexcept;

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

 private:
  Fiber(Body &&, Stack &&, IScheduler *) noexcept;

  // ITask
  void Run() noexcept override;
  [[nodiscard]] Fiber *DoRun() noexcept;

  void Step() noexcept;
  void Stop() noexcept;
  void DestroySelf() noexcept;
  void ReleaseResources() noexcept;

  [[nodiscard]] static FiberId GetNextId() noexcept;
};

} // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_CORE_FIBER_HPP_INCLUDED_ */
