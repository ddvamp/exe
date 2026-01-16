//
// barrier.hpp
// ~~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FIBER_SYNC_BARRIER_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_SYNC_BARRIER_HPP_INCLUDED_ 1

#include <exe/fiber/api.hpp>
#include <exe/fiber/core/awaiter.hpp>
#include <exe/fiber/core/handle.hpp>

#include <util/debug/assert.hpp>
#include <util/mm/release_sequence.hpp>

#include <atomic>
#include <cstdint>
#include <utility>

namespace exe::fiber {

class Barrier {
 private:
  struct FiberNode {
    FiberNode *next = nullptr;
    FiberHandle handle;
  };

  struct Waiter final : IAwaiter, FiberNode {
    Barrier &barrier;

    explicit Waiter(Barrier &barrier) noexcept : barrier(barrier) {}

    FiberHandle AwaitSymmetricSuspend(FiberHandle &&self) noexcept override {
      handle = ::std::move(self);
      return barrier.ArriveImpl(this);
    }
  };

  using Count = ::std::uint64_t;

 private:
  ::std::atomic<FiberNode *> top_ = nullptr;
  Count const participants_;
  ::std::atomic<Count> remains_ = participants_;

  // To guarantee the expected implementation
  static_assert(::std::atomic<FiberNode *>::is_always_lock_free);
  static_assert(::std::atomic<Count>::is_always_lock_free);

 public:
  ~Barrier() {
    UTIL_ASSERT(!top_.load(::std::memory_order_relaxed),
                "Barrier is destroyed during use");
  }

  Barrier(Barrier const &) = delete;
  void operator= (Barrier const &) = delete;

  Barrier(Barrier &&) = delete;
  void operator= (Barrier &&) = delete;

 public:
  explicit Barrier(Count const participants) noexcept
      : participants_(participants) {
    UTIL_ASSERT(participants >= 2, "Too few participants");
  }

  void Arrive() noexcept {
    if (remains_.load(::std::memory_order_relaxed) == 1) [[unlikely]] {
      // Fast path
      ScheduleWaiters();
      return;
    }

    Waiter awaiter(*this);
    self::Suspend(awaiter);
  }

 private:
  FiberHandle ArriveImpl(FiberNode *self) noexcept {
    auto const next = top_.exchange(self, ::std::memory_order_relaxed);
    self->next = next;

    if (remains_.fetch_sub(1, ::std::memory_order_release) > 1) [[likely]] {
      return FiberHandle::Invalid();
    }

    ScheduleWaiters(self);
    return ::std::move(*self).handle;
  }

  void ScheduleWaiters(FiberNode *self = nullptr) noexcept {
    ::util::SyncWithReleaseSequences(remains_);
    remains_.store(participants_, ::std::memory_order_relaxed);

    auto waiter = top_.load(::std::memory_order_relaxed);
    top_.store(nullptr, ::std::memory_order_relaxed);

    // There is at least 1 other participant
    do {
      auto node = ::std::exchange(waiter, waiter->next);
      if (node != self) [[likely]] {
        ::std::move(*node).handle.Schedule();
      }
    } while (waiter);
  }
};

} // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_SYNC_BARRIER_HPP_INCLUDED_ */
