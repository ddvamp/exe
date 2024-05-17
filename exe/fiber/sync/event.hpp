// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_FIBER_SYNC_EVENT_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_SYNC_EVENT_HPP_INCLUDED_ 1

#include <atomic>
#include <utility>

#include <concurrency/intrusive/forward_list.hpp>
#include <utils/debug/assert.hpp>

#include <exe/fiber/api.hpp>
#include <exe/fiber/core/awaiter.hpp>
#include <exe/fiber/core/handle.hpp>

namespace exe::fiber {

class Event {
 private:
  struct Waiter final : ISuspendingAwaiter,
                        ::concurrency::IntrusiveForwardListNode<> {
    Event *event_;
    FiberHandle handle_;

    explicit Waiter(Event *event) : event_(event) {}

    void AwaitSuspend(FiberHandle &&current) noexcept override {
      handle_ = ::std::move(current);
      event_->WaitImpl(this);
    }
  };

  using Node = Waiter::Node;

 private:
  Node dummy_;
  ::std::atomic<Node *> tail_ = &dummy_;

  // To guarantee the expected implementation
  static_assert(::std::atomic<Node *>::is_always_lock_free);

 public:
  ~Event() {
    UTILS_ASSERT(tail_.load(::std::memory_order_relaxed) == &dummy_,
                 "Event is destroyed during use");
  }

  Event(Event const &) = delete;
  void operator= (Event const &) = delete;

  Event(Event &&) = delete;
  void operator= (Event &&) = delete;

 public:
  Event() = default;

  // In case of instance reuse
  void Reset() noexcept {
    dummy_.Link(nullptr);
  }

  void Wait() noexcept {
    if (IsFired()) {
      return;
    }

    Waiter awaiter(this);
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
    return dummy_.next_.exchange(&dummy_, ::std::memory_order_acq_rel);  // ?release
  }

  void WaitImpl(Waiter *self) noexcept {
    Node *expected = nullptr;
    auto node = tail_.exchange(self, ::std::memory_order_acq_rel);
    if (node->next_.compare_exchange_strong(expected, self,
            ::std::memory_order_release,  // ?relaxed
            ::std::memory_order_acquire)) [[likely]] {
      return;
    }

    if (node == &dummy_) [[unlikely]] {
      node = self;
      Seal();
    } else {
      node->Link(self);
    }

    ScheduleWaiters(node);
  }

  void Seal() noexcept {
    tail_.exchange(&dummy_, ::std::memory_order_acq_rel)->Link(&dummy_);  // ?acquire
  }

  void ScheduleWaiters(Node *waiter) const noexcept {
    do {
      auto next = waiter->next_.load(::std::memory_order_relaxed);
      if (!next && !(next = waiter->next_.exchange(waiter, 
                         ::std::memory_order_release))) {
        return;
      }

      ::std::move(static_cast<Waiter *>(waiter)->handle_).Schedule();
      waiter = next;
    } while (waiter != &dummy_);
  }
};

}  // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_SYNC_EVENT_HPP_INCLUDED_ */