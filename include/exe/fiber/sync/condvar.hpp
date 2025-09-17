//
// condvar.hpp
// ~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FIBER_SYNC_CONDVAR_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_SYNC_CONDVAR_HPP_INCLUDED_ 1

#include <exe/fiber/api.hpp>
#include <exe/fiber/core/awaiter.hpp>
#include <exe/fiber/core/handle.hpp>
#include <exe/fiber/sync/mutex.hpp>

#include <utility>

namespace exe::fiber {

class Condvar {
 private:
  struct Waiter final : IAwaiter, Mutex::FiberInfo {
    Condvar &cv;

    explicit Waiter(Condvar &cv) noexcept : cv(cv) {}

    FiberHandle AwaitSymmetricSuspend(FiberHandle &&self) noexcept override {
      handle = ::std::move(self);
      cv.WaitImpl(this);
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
    Waiter awaiter(*this);
    self::Suspend(awaiter);
  }

  template <typename Predicate>
  void Wait(Predicate &&stop_waiting) noexcept {
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
      tail_->Link(self);
      tail_ = self;
    } else {
      tail_ = head_ = self;
    }

    m_.Unlock();
  }

  void NotifyImpl(bool const all) noexcept {
    if (!head_) [[unlikely]] {
      return;
    }

    auto const head = head_;
    auto const tail = all ? tail_ : head_;
    head_ = tail->Next();

    tail->Link(m_.head_);
    m_.head_ = head;
  }
};

} // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_SYNC_CONDVAR_HPP_INCLUDED_ */
