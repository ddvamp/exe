// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#include "fiber.hpp"

#include <utility>

#include <utils/abort.hpp>
#include <utils/debug.hpp>
#include <utils/defer.hpp>

#include <exe/fiber/api.hpp>

namespace exe::fiber {

namespace {

thread_local Fiber *current = nullptr;

struct ContextGuard {
  Fiber *from;

  explicit ContextGuard(Fiber *to, Fiber *from) noexcept : from(from) {
    current = to;
  }

  ~ContextGuard() {
    current = from;
  }
};

[[nodiscard]] bool AmIFiber() noexcept {
  return current;
}

struct YieldAwaiter final : IAwaiter {
  FiberHandle AwaitSymmetricSuspend(FiberHandle &&self) noexcept override {
    ::std::move(self).Schedule();
    return FiberHandle::Invalid();
  }
};

struct SwitchAwaiter final : IAwaiter {
  FiberHandle &target_;

  explicit SwitchAwaiter(FiberHandle &target) noexcept : target_(target) {}

  FiberHandle AwaitSymmetricSuspend(FiberHandle &&self) noexcept override {
    // Prevents race condition
    ::utils::defer cleanup([&self]() noexcept {
        ::std::move(self).Schedule(); 
    });
    return ::std::move(target_);
  }
};

}  // namespace


////////////////////////////////////////////////////////////////////////////////


/* static */ Fiber *Fiber::Create(Body &&body, IScheduler *scheduler) {
  return ::new Fiber(::std::move(body), AllocateStack(), scheduler);
}

/* static */ Fiber &Fiber::Self() noexcept {
  UTILS_ASSERT(AmIFiber(), "Not in the fiber context");
  return *current;
}

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

Fiber::Fiber(Body &&body, Stack &&stack, IScheduler *scheduler) noexcept
    : stack_(::std::move(stack))
    , coroutine_(::std::move(body), stack_.View())
    , scheduler_(scheduler) {}

/* virtual */ void Fiber::Run() noexcept {
  ContextGuard guard(nullptr, current);
  auto fiber = this;
  while ((fiber = fiber->DoRun()));
}

Fiber *Fiber::DoRun() noexcept {
  Step();

  if (coroutine_.IsCompleted()) [[unlikely]] {
    UTILS_ASSERT(!awaiter_, "Awaiter instead of nullptr");
    DestroySelf();
    return nullptr;
  }

  UTILS_ASSUME(awaiter_, "nullptr instead of awaiter");
  return awaiter_->AwaitSymmetricSuspend(FiberHandle(this)).Release();
}

void Fiber::Step() noexcept {
  ContextGuard guard(this, nullptr);
  awaiter_ = nullptr;
  coroutine_.Resume();
}

void Fiber::Stop() noexcept {
  coroutine_.Suspend();
}

void Fiber::DestroySelf() noexcept {
  ReleaseResources();
  delete this;
}

void Fiber::ReleaseResources() noexcept {
  DeallocateStack(::std::move(stack_));
}

/* static */ FiberId Fiber::GetNextId() noexcept {
  return next_id_.fetch_add(1, ::std::memory_order_relaxed);
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
  Fiber::Create(::std::move(body), &scheduler)->Schedule();
}

void Go(Body &&body) {
  Go(self::GetScheduler(), ::std::move(body));
}


NoSwitchContextGuard::NoSwitchContextGuard() noexcept : self(current) {
  current = nullptr;
}

NoSwitchContextGuard::~NoSwitchContextGuard() {
  current = self;
}

}  // namespace exe::fiber
