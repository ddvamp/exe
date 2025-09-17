//
// event.hpp
// ~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FIBER_SYNC_EVENT_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_SYNC_EVENT_HPP_INCLUDED_ 1

#include <exe/fiber/api.hpp>
#include <exe/fiber/core/awaiter.hpp>
#include <exe/fiber/core/handle.hpp>

#include <concurrency/intrusive/forward_list.hpp>
#include <util/debug/assert.hpp>

#include <atomic>
#include <utility>

namespace exe::fiber {

/**
 *  A synchronization primitive for waiting for a notification
 *  and sharing data through it
 */
class Event {
 private:
  struct FiberNode : ::concurrency::IntrusiveForwardListNode<> {
    FiberHandle handle;
  };

  struct Waiter final : IAwaiter, FiberNode {
    Event &event;

    explicit Waiter(Event &event) noexcept : event(event) {}

    FiberHandle AwaitSymmetricSuspend(FiberHandle &&self) noexcept override {
      handle = ::std::move(self);
      event.WaitImpl(this);
      return FiberHandle::Invalid();
    }
  };

  using Node = FiberNode::Node;

 private:
  Node dummy_;
  ::std::atomic<Node *> tail_ = &dummy_;

  // To guarantee the expected implementation
  static_assert(::std::atomic<Node *>::is_always_lock_free);

 public:
  ~Event() {
    UTIL_ASSERT(tail_.load(::std::memory_order_relaxed) == &dummy_,
                 "Event is destroyed during use");
  }

  Event(Event const &) = delete;
  void operator= (Event const &) = delete;

  Event(Event &&) = delete;
  void operator= (Event &&) = delete;

 public:
  Event() = default;

  // In case of an instance reuse
  void Reset() noexcept {
    dummy_.Link(nullptr);
  }

  void Wait() noexcept {
    if (IsFired()) {
      // Fast path
      return;
    }

    Waiter awaiter(*this);
    self::Suspend(awaiter);
  }

  void Fire() noexcept {
    auto const waiters = GetWaiters();
    if (!waiters) {
      return;
    }

    Seal();
    ScheduleWaiters(waiters);
  }

 private:
  [[nodiscard]] bool IsFired() const noexcept {
    return dummy_.next_.load(::std::memory_order_acquire) == &dummy_;
  }

  [[nodiscard]] Node *GetWaiters() noexcept {
    return dummy_.next_.exchange(&dummy_, ::std::memory_order_release);
  }

  void WaitImpl(FiberNode *self) noexcept {
    Node *expected = nullptr;
    auto node = tail_.exchange(self, ::std::memory_order_acq_rel);
    if (node->next_.compare_exchange_strong(expected, self,
            ::std::memory_order_relaxed,
            ::std::memory_order_acquire)) [[likely]] {
      return;
    }

    if (node != &dummy_) [[likely]] {
      node->Link(self);
    } else {
      node = self;
      Seal();
    }

    ScheduleWaiters(node);
  }

  void Seal() noexcept {
    tail_.exchange(&dummy_, ::std::memory_order_acquire)->Link(&dummy_);
  }

  void ScheduleWaiters(Node *waiter) const noexcept {
    do {
      auto next = waiter->Next();
      if (!next) [[unlikely]] {
        if (waiter->next_.compare_exchange_strong(next, waiter,
                ::std::memory_order_release,
                ::std::memory_order_relaxed)) [[likely]] {
          return;
        }
      }

      static_cast<Waiter &&>(*waiter).handle.Schedule();
      waiter = next;
    } while (waiter != &dummy_);
  }
};

} // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_SYNC_EVENT_HPP_INCLUDED_ */
