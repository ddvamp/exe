// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_FIBER_SYNC_CONDVAR_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_SYNC_CONDVAR_HPP_INCLUDED_ 1

#include <cstdint>
#include <utility>

#include <exe/fiber/api.hpp>
#include <exe/fiber/core/awaiter.hpp>
#include "mutex.hpp"

namespace exe::fiber {

class CondVar {
 private:
  struct Waiter final : ISuspendingAwaiter, Mutex::FiberInfo {
    CondVar *cv_;

    explicit Waiter(CondVar *cv) noexcept : cv_(cv) {}

    void AwaitSuspend(FiberHandle &&current) noexcept override {
      handle_ = ::std::move(current);
      cv_->WaitImpl(this);
    }
  };

  using Node = Waiter::Node;

  Mutex &m_;
  Node *head_ = nullptr;
  Node *tail_;

 public:
  ~CondVar() = default;

  CondVar(CondVar const &) = delete;
  void operator= (CondVar const &) = delete;

  CondVar(CondVar &&) = delete;
  void operator= (CondVar &&) = delete;

 public:
  explicit CondVar(Mutex &m) noexcept : m_(m) {}

  /* ATTENTION: All methods below require a locked m_! */

  void Wait() noexcept {
    UTILS_DEBUG_RUN(m_.CheckOwner);

    Waiter awaiter(this);
    self::Suspend(awaiter);
  }

  template <typename Predicate>
  void Wait(Predicate &&stop_waiting) noexcept {
    UTILS_DEBUG_RUN(m_.CheckOwner);

    while (!stop_waiting()) {
      Wait();
    }
  }

  void NotifyOne() noexcept {
    NotifyImpl(false);
  }

  void NotifyAll() noexcept {
    NotifyImpl(true);
  }

 private:
  void WaitImpl(Mutex::FiberInfo *self) noexcept {
    if (head_) {
      tail_->next_.store(self, ::std::memory_order_relaxed);
      tail_ = self;
    } else {
      tail_ = head_ = self;
    }

    m_.Unlock();
  }

  void NotifyImpl(bool const all) noexcept {
    UTILS_DEBUG_RUN(m_.CheckOwner);

    if (!head_) {
      return;
    }

    auto const head = head_;
    auto const tail = all ? tail_ : head_;
    head_ = tail->next_.load(::std::memory_order_relaxed);
    Enqueue(head, tail);
  }

  void Enqueue(Node *head, Node *tail) const noexcept {
    tail->Link(m_.head_);
    m_.head_ = head;
  }
};

}  // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_SYNC_CONDVAR_HPP_INCLUDED_ */
