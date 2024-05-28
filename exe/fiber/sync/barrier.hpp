// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_FIBER_SYNC_BARRIER_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_SYNC_BARRIER_HPP_INCLUDED_ 1

#include <atomic>
#include <cstdint>
#include <utility>

#include <util/debug/assert.hpp>

#include <exe/fiber/api.hpp>
#include <exe/fiber/core/awaiter.hpp>
#include <exe/fiber/core/handle.hpp>

namespace exe::fiber {

class Barrier {
 private:
  struct Node {
    Node *next_ = nullptr;
  };

  struct Waiter final : IAwaiter, Node {
    Barrier *barrier_;
    FiberHandle handle_;

    explicit Waiter(Barrier *barrier) noexcept : barrier_(barrier) {}

    FiberHandle AwaitSymmetricSuspend(FiberHandle &&self) noexcept override {
      handle_ = ::std::move(self);
      return barrier_->ArriveImpl(this);
    }
  };

  using Count = ::std::uint64_t;

 private:
  Node dummy_;
  ::std::atomic<Node *> tail_ = &dummy_;
  Count const expected_;
  ::std::atomic<Count> count_ = expected_;

  // To guarantee the expected implementation
  static_assert(::std::atomic<Node *>::is_always_lock_free);
  static_assert(::std::atomic<Count>::is_always_lock_free);

 public:
  ~Barrier() {
    UTIL_ASSERT(tail_.load(::std::memory_order_relaxed) == &dummy_,
                "Barrier is destroyed during use");
  }

  Barrier(Barrier const &) = delete;
  void operator= (Barrier const &) = delete;

  Barrier(Barrier &&) = delete;
  void operator= (Barrier &&) = delete;

 public:
  explicit Barrier(Count const expected) noexcept : expected_(expected) {
    UTIL_ASSERT(expected < 2, "Too few participants");
  }

  void Arrive() noexcept {
    if (count_.load(::std::memory_order_acquire) == 1) [[unlikely]] {
      // Fast path
      ScheduleWaiters();
      return;
    }

    Waiter awaiter(this);
    self::Suspend(awaiter);
  }

 private:
  FiberHandle ArriveImpl(Waiter *self) noexcept {
    auto const pred = tail_.exchange(self, ::std::memory_order_acq_rel);
    pred->next_ = self;
    if (count_.fetch_sub(1, ::std::memory_order_acq_rel) != 1) [[likely]] {
      return FiberHandle::Invalid();
    }

    pred->next_ = self->next_;
    ScheduleWaiters();
    return ::std::move(self->handle_);
  }

  void ScheduleWaiters() noexcept {
    tail_.store(&dummy_, ::std::memory_order_relaxed);
    count_.store(expected_, ::std::memory_order_relaxed);

    auto waiter = dummy_.next_;
    do {
      auto const next = waiter->next_;
      ::std::move(*static_cast<Waiter *>(waiter)).handle_.Schedule();
      waiter = next;
    } while (waiter);
  }
};

}  // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_SYNC_BARRIER_HPP_INCLUDED_ */
