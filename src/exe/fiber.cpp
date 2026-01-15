//
// fiber.cpp
// ~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#include <exe/fiber/api.hpp>
#include <exe/fiber/core/awaiter.hpp>
#include <exe/fiber/core/body.hpp>
#include <exe/fiber/core/fiber.hpp>
#include <exe/fiber/core/handle.hpp>
#include <exe/fiber/core/id.hpp>
#include <exe/fiber/core/scheduler.hpp>
#include <exe/fiber/core/stack.hpp>

#include <util/abort.hpp>
#include <util/debug.hpp>

#include <atomic>
#include <utility>

namespace exe::fiber {

namespace {

thread_local Fiber *current = nullptr;

struct ContextGuard {
  Fiber *from;

  ContextGuard(Fiber *to, Fiber *from) noexcept : from(from) {
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

  explicit SwitchAwaiter(FiberHandle &&target) noexcept : target_(target) {}

  FiberHandle AwaitSymmetricSuspend(FiberHandle &&self) noexcept override {
    // Prevents race condition
    auto target = ::std::move(target_);
    ::std::move(self).Schedule();
    return ::std::move(target);
  }
};

} // namespace

////////////////////////////////////////////////////////////////////////////////

/* static */ Fiber *Fiber::Create(Body &&body, Scheduler &scheduler) {
  return ::new Fiber(::std::move(body), AllocateStack(), scheduler);
}

/* static */ Fiber &Fiber::Self() noexcept {
  UTIL_ASSERT(AmIFiber(), "Not in the fiber context");
  return *current;
}

void Fiber::Schedule() noexcept {
  scheduler_.get().Submit(this);
}

void Fiber::Resume() noexcept {
  ::std::move(*this).Run();
}

void Fiber::Suspend(IAwaiter &awaiter) noexcept {
  awaiter_ = &awaiter;
  Stop();
}

void Fiber::TeleportTo(Scheduler &scheduler) noexcept {
  scheduler_ = scheduler;
  self::Yield();
}

Fiber::Fiber(Body &&body, Stack &&stack, Scheduler &scheduler) noexcept
    : stack_(::std::move(stack))
    , coroutine_(::std::move(body), stack_.View())
    , scheduler_(scheduler) {}

/* virtual */ void Fiber::Run() && noexcept {
  ContextGuard guard(nullptr, current);
  auto fiber = this;
  while ((fiber = fiber->DoRun()));
}

Fiber *Fiber::DoRun() noexcept {
  if (auto awaiter = Step()) [[likely]] {
    return awaiter->AwaitSymmetricSuspend(FiberHandle(*this)).Release();
  }

  DestroySelf();
  return nullptr;
}

IAwaiter *Fiber::Step() noexcept {
  ContextGuard guard(this, nullptr);

  coroutine_.Resume();
  UTIL_ASSERT(coroutine_.IsCompleted() == !awaiter_,
              "Internal error! Awaiter is either lost or "
              "provided for a completed fiber");

  return ::std::exchange(awaiter_, nullptr);
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

Scheduler &GetScheduler() noexcept {
  return Fiber::Self().GetScheduler();
}

void Suspend(IAwaiter &awaiter) noexcept {
  Fiber::Self().Suspend(awaiter);
}

void Yield() noexcept {
  YieldAwaiter awaiter;
  Suspend(awaiter);
}

void SwitchTo(FiberHandle &&next) noexcept {
  SwitchAwaiter awaiter(::std::move(next));
  Suspend(awaiter);
}

void TeleportTo(Scheduler &scheduler) noexcept {
  Fiber::Self().TeleportTo(scheduler);
}

} // namespace self

////////////////////////////////////////////////////////////////////////////////

void Go(Scheduler &scheduler, Body &&body) {
  UTIL_ASSERT(body, "Empty body for fiber");
  Fiber::Create(::std::move(body), scheduler)->Schedule();
}

void Go(Body &&body) {
  Go(self::GetScheduler(), ::std::move(body));
}

////////////////////////////////////////////////////////////////////////////////

NoSwitchContextGuard::NoSwitchContextGuard() noexcept : self(current) {
  current = nullptr;
}

NoSwitchContextGuard::~NoSwitchContextGuard() {
  current = self;
}

} // namespace exe::fiber
