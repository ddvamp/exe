// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#include "fiber.hpp"

#include <utility>

#include <utils/abort.hpp>
#include <utils/debug.hpp>
#include <utils/defer.hpp>

namespace exe::fiber {

namespace {

thread_local Fiber *current = nullptr;

[[nodiscard]] bool AmIFiber() noexcept {
  return current;
}

struct YieldAwaiter final : ISuspendingAwaiter {
  void AwaitSuspend(FiberHandle &&h) noexcept override {
    ::std::move(h).Schedule();
  }
};

struct SwitchAwaiter final : IAwaiter {
  FiberHandle &target_;

  explicit SwitchAwaiter(FiberHandle &target) noexcept : target_(target) {}

  void AwaitSuspend(FiberHandle &&) noexcept override {
    // pass
  }

  FiberHandle AwaitSymmetricSuspend(FiberHandle &&from) override {
    // Prevents race condition
    ::utils::defer cleanup([&from]() noexcept {
        ::std::move(from).Schedule(); 
    });
    return ::std::move(target_);
  }
};

}  // namespace


////////////////////////////////////////////////////////////////////////////////


/* static */ Fiber &Fiber::Self() noexcept {
  UTILS_ASSERT(AmIFiber(), "Not in the fiber context");
  return *current;
}

Fiber::Fiber(Body &&body, Stack &&stack, IScheduler *scheduler) noexcept
    : stack_(::std::move(stack))
    , coroutine_(::std::move(body), stack_.View())
    , scheduler_(scheduler) {}

void Fiber::Schedule() noexcept {
  scheduler_->Submit(this);
}

void Fiber::Resume() noexcept {
  Run();
}

void Fiber::Suspend(IAwaiter *awaiter) noexcept {
  awaiter_ = awaiter;
  Stop();
}

void Fiber::TeleportTo(IScheduler *scheduler) noexcept {
  scheduler_ = scheduler;
  self::Yield();
}

void Fiber::ReleaseResources() noexcept {
  DeallocateStack(::std::move(stack_));
}

void Fiber::Step() noexcept {
  auto const pred = current;
  current = this;
  coroutine_.Resume();
  current = pred;
}

Fiber *Fiber::DoRun() noexcept {
  Step();

  if (coroutine_.IsCompleted()) [[unlikely]] {
    UTILS_ASSERT(!awaiter_, "Awaiter instead of nullptr");
    DestroySelf();
    return nullptr;
  }

  auto const awaiter = ::std::exchange(awaiter_, nullptr);
  UTILS_ASSUME(awaiter, "nullptr instead of awaiter");
  auto next = awaiter->AwaitSymmetricSuspend(FiberHandle(this));
  return next.Release();
}

/* virtual */ void Fiber::Run() noexcept {
  auto next = this;
  while ((next = next->DoRun()));
}

void Fiber::DestroySelf() noexcept {
  ReleaseResources();
  delete this;
}

void Fiber::Stop() noexcept {
  coroutine_.Suspend();
}

/* static */ FiberId Fiber::GetNextId() noexcept {
  static ::std::atomic<FiberId> next_id = kInvalidFiberId + 1;
  return next_id.fetch_add(1, ::std::memory_order_relaxed);
}

Fiber *CreateFiber(Body &&body, IScheduler *scheduler) {
  return ::new Fiber(::std::move(body), AllocateStack(), scheduler);
}


////////////////////////////////////////////////////////////////////////////////


/* API */

namespace self {

FiberId GetId() noexcept {
  return Fiber::Self().GetId();
}

IScheduler &GetScheduler() noexcept {
  return *Fiber::Self().GetScheduler();
}

void Suspend(IAwaiter &awaiter) noexcept {
  Fiber::Self().Suspend(&awaiter);
}

void Yield() noexcept {
  YieldAwaiter awaiter;
  Suspend(awaiter);
}

void SwitchTo(FiberHandle &&next) noexcept {
  SwitchAwaiter awaiter(next);
  Suspend(awaiter);
}

void TeleportTo(IScheduler &scheduler) noexcept {
  Fiber::Self().TeleportTo(&scheduler);
}

}  // namespace self

void Go(IScheduler &scheduler, Body &&body) {
  UTILS_ASSERT(body, "Empty body for fiber");
  auto fiber = CreateFiber(::std::move(body), &scheduler);
  fiber->Schedule();
}

void Go(Body &&body) {
  Go(self::GetScheduler(), ::std::move(body));
}

}  // namespace exe::fiber
