// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_FIBER_SYNC_CONDVAR_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_SYNC_CONDVAR_HPP_INCLUDED_ 1

#include <utility>

#include <exe/fiber/api.hpp>
#include <exe/fiber/core/awaiter.hpp>
#include "mutex.hpp"

namespace exe::fiber {

class Condvar {
 private:
  struct Waiter final : IAwaiter, Mutex::FiberInfo {
    Condvar *cv_;

    explicit Waiter(Condvar *cv) noexcept : cv_(cv) {}

    FiberHandle AwaitSymmetricSuspend(FiberHandle &&self) noexcept override {
      handle_ = ::std::move(self);
      cv_->WaitImpl(this);
      return FiberHandle::Invalid();
    }
  };

  using Node = Waiter::Node;

  Mutex &m_;
  Node *head_ = nullptr;
  Node *tail_;

 public:
  ~Condvar() = default;

  Condvar(Condvar const &) = delete;
  void operator= (Condvar const &) = delete;

  Condvar(Condvar &&) = delete;
  void operator= (Condvar &&) = delete;

 public:
  constexpr explicit Condvar(Mutex &m) noexcept : m_(m) {}

  /* ATTENTION: All methods below require a locked m_! */

  void Wait() noexcept {
    UTILS_DEBUG_RUN(m_.CheckUnlock);
    Waiter awaiter(this);
    self::Suspend(awaiter);
    UTILS_DEBUG_RUN(m_.CheckLock, true);
  }

  template <typename Predicate>
  void Wait(Predicate &&stop_waiting) noexcept {
    UTILS_DEBUG_RUN(m_.CheckOwner);
    while (!stop_waiting()) {
      Wait();
    }
  }

  void NotifyOne() noexcept {
    UTILS_DEBUG_RUN(m_.CheckOwner);
    NotifyImpl(false);
  }

  void NotifyAll() noexcept {
    UTILS_DEBUG_RUN(m_.CheckOwner);
    NotifyImpl(true);
  }

 private:
  void Enqueue(Node *head, Node *tail) const noexcept {
    tail->Link(m_.head_);
    m_.head_ = head;
  }

  void WaitImpl(Mutex::FiberInfo *self) noexcept {
    if (head_) {
      tail_->next_.store(self, ::std::memory_order_relaxed);
      tail_ = self;
    } else {
      tail_ = head_ = self;
    }

    m_.UnlockImpl();
  }

  void NotifyImpl(bool const all) noexcept {
    if (!head_) [[unlikely]] {
      return;
    }

    auto const head = head_;
    auto const tail = all ? tail_ : head_;
    head_ = tail->next_.load(::std::memory_order_relaxed);
    Enqueue(head, tail);
  }
};

}  // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_SYNC_CONDVAR_HPP_INCLUDED_ */
