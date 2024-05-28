// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_FIBER_SYNC_MUTEX_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_SYNC_MUTEX_HPP_INCLUDED_ 1

#include <atomic>
#include <mutex>  // std::lock_guard, std::unique_lock
#include <new>

#include <concurrency/intrusive/forward_list.hpp>
#include <util/debug.hpp>
#include <util/utility.hpp>

#include <exe/fiber/api.hpp>
#include <exe/fiber/core/awaiter.hpp>
#include <exe/fiber/core/handle.hpp>

namespace exe::fiber {

class Condvar;

class alignas (::std::hardware_destructive_interference_size) Mutex {
  friend class Condvar;

 private:
  struct FiberInfo : ::concurrency::IntrusiveForwardListNode<> {
    FiberHandle handle_;
  };

  struct LockAwaiter final : IAwaiter, FiberInfo {
    Mutex *m_;

    explicit LockAwaiter(Mutex *m) noexcept : m_(m) {}

    FiberHandle AwaitSymmetricSuspend(FiberHandle &&self) noexcept override {
      handle_ = ::std::move(self);
      return m_->LockImpl(this);
    }
  };

  using Node = FiberInfo::Node;

  Node dummy_{.next_ = &dummy_};
  Node *head_ = &dummy_;
  ::std::atomic<Node *> tail_ = &dummy_;
  ::std::atomic<FiberId> owner_ = kInvalidFiberId;
  
  // To guarantee the expected implementation
  static_assert(::std::atomic<Node *>::is_always_lock_free);
  static_assert(::std::atomic<FiberId>::is_always_lock_free);

 public:
  ~Mutex() {
    UTIL_ASSERT(!IsLocked(), "Mutex is destroyed during use");
  }

  Mutex(Mutex const &) = delete;
  void operator= (Mutex const &) = delete;

  Mutex(Mutex &&) = delete;
  void operator= (Mutex &&) = delete;

 public:
  constexpr Mutex() = default;

  [[nodiscard]] bool TryLock() noexcept {
    UTIL_DEBUG_RUN(CheckRecursive);
    auto const success = TryLockImpl();
    UTIL_DEBUG_RUN(CheckLock, success);
    return success;
  }

  void Lock() noexcept {
    UTIL_DEBUG_RUN(CheckRecursive);
    if (!TryLockImpl()) [[unlikely]] {
      LockAwaiter awaiter(this);
      self::Suspend(awaiter);
    }
    UTIL_DEBUG_RUN(CheckLock, true);
  }

  void Unlock() noexcept {
    UTIL_DEBUG_RUN(CheckUnlock);
    UnlockImpl();
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

 private:
  [[nodiscard]] bool IsLocked() const noexcept {
    return dummy_.next_.load(::std::memory_order_relaxed) != &dummy_;
  }

  [[nodiscard]] bool TryLockImpl() noexcept {
    if (IsLocked()) [[unlikely]] {
      return false;
    }

    return dummy_.next_.compare_exchange_weak(::util::temporary(&dummy_),
                                              nullptr,
                                              ::std::memory_order_acquire,
                                              ::std::memory_order_relaxed);
  }

  FiberHandle LockImpl(FiberInfo *self) noexcept {
    auto const owner = tail_.exchange(self, ::std::memory_order_acq_rel)->
                       next_.exchange(self, ::std::memory_order_relaxed);
    if (!owner) [[likely]] {
      return FiberHandle::Invalid();
    }

    UTIL_IGNORE(owner->next_.load(::std::memory_order_acquire));  // sync
    return Acquire(owner, self, true);
  }

  void UnlockImpl() noexcept {
    auto const owner = head_;
    auto const next = TryTakeNext(owner);
    if (!next) [[likely]] {
      return;
    }

    UTIL_IGNORE(Acquire(owner, next, false));
  }

  static [[nodiscard]] FiberHandle &&GetHandle(Node *node) noexcept {
    [[assume(node)]];
    return ::std::move(*static_cast<FiberInfo *>(node)).handle_;
  }

  static [[nodiscard]] Node *TryTakeNext(Node *node) noexcept {
    auto next = node->next_.load(::std::memory_order_relaxed);
    if (next) {
      return next;
    }

    node->next_.compare_exchange_strong(next, node, ::std::memory_order_release,
                                        ::std::memory_order_relaxed);
    return next;
  }

  FiberHandle Acquire(Node *owner, Node *next, bool resume) noexcept {
    if (owner == &dummy_) {
      dummy_.Link(nullptr);
      tail_.exchange(&dummy_, ::std::memory_order_acq_rel)->Link(&dummy_);

      owner = next;
      next = TryTakeNext(owner);
      if (!next) [[unlikely]] {
        return FiberHandle::Invalid();
      }

      if (resume) [[unlikely]] {
        head_ = next;
        return GetHandle(owner);
      }
    }

    head_ = next;
    GetHandle(owner).Schedule();
    return FiberHandle::Invalid();
  }

 private:
  [[nodiscard, maybe_unused]] auto Owner() noexcept {
    struct _ {
      Mutex *m_;

      operator FiberId () noexcept {
        return m_->owner_.load(::std::memory_order_relaxed);
      }

      void operator= (FiberId id) noexcept {
        m_->owner_.store(id, ::std::memory_order_relaxed);
      }
    };

    return _{this};
  }

  [[maybe_unused]] void CheckOwner() noexcept {
    UTIL_CHECK(Owner() == self::GetId(), "Fiber does not own condvar mutex");
  }

  [[maybe_unused]] void CheckRecursive() noexcept {
    UTIL_CHECK(Owner() != self::GetId(), "Fiber already owns this mutex");
  }

  [[maybe_unused]] void CheckLock(bool success) noexcept {
    if (success) [[likely]] {
      UTIL_CHECK(Owner() == kInvalidFiberId,
                  "Fiber has locked an already locked mutex");
      Owner() = self::GetId();
    }
  }

  [[maybe_unused]] void CheckUnlock() noexcept {
    UTIL_CHECK(Owner() == self::GetId(),
                "Fiber unlocks a mutex that it does not own");
    Owner() = kInvalidFiberId;
  }
};

}  // namespace exe::fiber

#endif  /* DDVAMP_EXE_FIBER_SYNC_MUTEX_HPP_INCLUDED_ */
