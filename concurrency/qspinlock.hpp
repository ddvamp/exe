// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_CONCURRENCY_QSPINLOCK_HPP_INCLUDED_
#define DDVAMP_CONCURRENCY_QSPINLOCK_HPP_INCLUDED_ 1

#include <atomic>
#include <new>  // std::hardware_destructive_interference_size

#include <util/debug/assert.hpp>
#include <util/utility.hpp>

#include "intrusive/forward_list.hpp"
#include "pause.hpp"

namespace concurrency {

class alignas (::std::hardware_destructive_interference_size) QSpinlock {
 private:
  struct Node : IntrusiveForwardListNode<Node> {
    ::std::atomic_bool locked_ = false;
  };

  Node dummy_;
  ::std::atomic<Node *> tail_ = &dummy_;

  // To guarantee the expected implementation
  static_assert(::std::atomic_bool::is_always_lock_free);
  static_assert(::std::atomic<Node *>::is_always_lock_free);

 public:
  ~QSpinlock() {
    UTIL_ASSERT(!IsLocked(), "QSpinlock is destroyed during use");
  }

  QSpinlock(QSpinlock const &) = delete;
  void operator= (QSpinlock const &) = delete;

  QSpinlock(QSpinlock &&) = delete;
  void operator= (QSpinlock &&) = delete;

 public:
  QSpinlock() = default;

  class Token final {
   private:
    QSpinlock &lock_;
    Node node_;

   public:
    ~Token() = default;

    Token(Token const &) = delete;
    void operator= (Token const &) = delete;

    Token(Token &&) = delete;
    void operator= (Token &&) = delete;

   public:
    explicit Token(QSpinlock &spinlock) noexcept
        : lock_(spinlock), node_{&spinlock.dummy_} {}

    [[nodiscard]] bool TryLock() noexcept {
      return lock_.TryLock();
    }

    void Lock() noexcept {
      lock_.Lock(node_);
    }

    void Unlock() noexcept {
      lock_.Unlock(node_);
    }

    // Cpp17Lockable
    // https://eel.is/c++draft/thread.req.lockable.req

    [[nodiscard]] bool try_lock() noexcept {
      return TryLock();
    }

    void lock() noexcept {
      Lock();
    }

    void unlock() noexcept {
      Unlock();
    }
  };

  [[nodiscard]] Token GetToken() noexcept {
    return Token(*this);
  }

  class Guard final {
   private:
    Token token_;

   public:
    ~Guard() {
      token_.Unlock();
    }

    Guard(Guard const &) = delete;
    void operator= (Guard const &) = delete;

    Guard(Guard &&) = delete;
    void operator= (Guard &&) = delete;

   public:
    explicit Guard(QSpinlock &lock) noexcept : token_(lock) {
      token_.Lock();
    }
  };

  [[nodiscard]] Guard MakeGuard() noexcept {
    return Guard(*this);
  }

 private:
  [[nodiscard]] bool IsLocked() const noexcept {
    return dummy_.locked_.load(::std::memory_order_relaxed);
  }

  [[nodiscard]] bool TryLock() noexcept {
    if (IsLocked()) [[unlikely]] {
      return false;
    }

    return dummy_.locked_.compare_exchange_weak(::util::temporary(false), true,
                                                ::std::memory_order_acquire,
                                                ::std::memory_order_relaxed);
  }

  void Lock(Node &node) noexcept {
    if (!TryLock()) [[unlikely]] {
      LockCold(node);
    }
  }

  void LockCold(Node &node) noexcept {
    node.Link(nullptr);
    node.locked_.store(true, ::std::memory_order_relaxed);

    auto const pred = tail_.exchange(&node, ::std::memory_order_acq_rel);
    if (pred == &dummy_) [[likely]] {
      while (!TryLock()) {
        Pause();
      }

      return;
    }

    pred->next_.store(&node, ::std::memory_order_release);

    while (node.locked_.load(::std::memory_order_acquire)) {
      Pause();
    }
  }

  void Unlock(Node &node) noexcept {
    auto *succ = node.next_.load(::std::memory_order_acquire);
    if (!succ) [[unlikely]] {
      succ = UnlockCold(node);
    }

    succ->locked_.store(false, ::std::memory_order_release);
    node.Link(&dummy_);
  }

  [[nodiscard]] Node *UnlockCold(Node &node) noexcept {
    if (tail_.compare_exchange_strong(::util::temporary(&node), &dummy_,
                                      ::std::memory_order_relaxed,
                                      ::std::memory_order_acquire)) [[likely]] {
      return &dummy_;
    }

    Node *succ;
    while (!(succ = node.next_.load(::std::memory_order_relaxed))) {
      Pause();
    }

    return succ;
  }
};

}  // namespace concurrency

#endif  /* DDVAMP_CONCURRENCY_QSPINLOCK_HPP_INCLUDED_ */
