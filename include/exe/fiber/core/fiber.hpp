//
// fiber.hpp
// ~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FIBER_CORE_FIBER_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_CORE_FIBER_HPP_INCLUDED_ 1

#include <exe/fiber/core/awaiter.hpp>
#include <exe/fiber/core/body.hpp>
#include <exe/fiber/core/coroutine.hpp>
#include <exe/fiber/core/id.hpp>
#include <exe/fiber/core/scheduler.hpp>
#include <exe/fiber/core/stack.hpp>
#include <exe/runtime/task/task.hpp>

#include <atomic>
#include <functional>

namespace exe::fiber {

/* Fiber = stackful coroutine + scheduler */
class [[nodiscard]] Fiber final : private runtime::task::TaskBase {
 private:
  Stack stack_;
  Coroutine coroutine_;
  ::std::reference_wrapper<Scheduler> scheduler_;
  IAwaiter *awaiter_ = nullptr;
  FiberId const id_ = GetNextId();

  inline static ::std::atomic<FiberId> next_id_ = kInvalidFiberId + 1;

  // To guarantee the expected implementation
  static_assert(::std::atomic<FiberId>::is_always_lock_free);

 public:
  // Create an self-ownership fiber
  [[nodiscard]] static Fiber *Create(Body &&, Scheduler &);

  // Reference to currently active fiber
  [[nodiscard]] static Fiber &Self() noexcept;

  [[nodiscard]] FiberId GetId() const noexcept {
    return id_;
  }

  [[nodiscard]] Scheduler &GetScheduler() const noexcept {
    return scheduler_.get();
  }

  // Schedule execution on scheduler set on fiber
  void Schedule() noexcept;

  // Execute fiber immediately
  void Resume() noexcept;

  void Suspend(IAwaiter &) noexcept;

  // Reschedule current fiber on scheduler
  void TeleportTo(Scheduler &) noexcept;

 private:
  Fiber(Body &&, Stack &&, Scheduler &) noexcept;

  // TaskBase
  void Run() && noexcept override;
  [[nodiscard]] Fiber *DoRun() noexcept;

  [[nodiscard]] IAwaiter *Step() noexcept;
  void Stop() noexcept;
  void DestroySelf() noexcept;
  void ReleaseResources() noexcept;

  [[nodiscard]] static FiberId GetNextId() noexcept;
};

} // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_CORE_FIBER_HPP_INCLUDED_ */
