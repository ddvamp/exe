//
// mutex.hpp
// ~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FIBER_SYNC_MUTEX_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_SYNC_MUTEX_HPP_INCLUDED_ 1

#include <exe/fiber/api.hpp>
#include <exe/fiber/core/awaiter.hpp>
#include <exe/fiber/core/handle.hpp>

#include <concurrency/intrusive/forward_list.hpp>
#include <util/debug.hpp>
#include <util/macro.hpp>
#include <util/utility.hpp>

#include <atomic>
#include <mutex> // IWYU pragma: export - std::lock_guard, std::unique_lock
#include <new>

namespace exe::fiber {

class Condvar;

class alignas (::std::hardware_destructive_interference_size) Mutex {
  friend class Condvar;

 private:
  struct FiberInfo : ::concurrency::IntrusiveForwardListNode<> {
    FiberHandle handle;
  };

  struct LockAwaiter final : IAwaiter, FiberInfo {
    Mutex &m;

    explicit LockAwaiter(Mutex &m) noexcept : m(m) {}

    FiberHandle AwaitSymmetricSuspend(FiberHandle &&self) noexcept override {
      handle = ::std::move(self);
      return m.LockImpl(this);
    }
  };

  using Node = FiberInfo::Node;

  Node dummy_{.next_ = &dummy_};
  Node *head_ = &dummy_;
  ::std::atomic<Node *> tail_ = &dummy_;

  // To guarantee the expected implementation
  static_assert(::std::atomic<Node *>::is_always_lock_free);

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
    if (IsLocked()) [[unlikely]] {
      return false;
    }

    return dummy_.next_.compare_exchange_weak(::util::temporary(&dummy_),
                                              nullptr,
                                              ::std::memory_order_acquire,
                                              ::std::memory_order_relaxed);
  }

  void Lock() noexcept {
    if (!TryLock()) [[unlikely]] {
      LockAwaiter awaiter(*this);
      self::Suspend(awaiter);
    }
  }

  void Unlock() noexcept {
    auto const owner = head_;
    auto const next = TryTakeNext(owner);
    if (!next) [[likely]] {
      return;
    }

    UTIL_IGNORE(Acquire(owner, next, false));
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
    return dummy_.Next() != &dummy_;
  }

  FiberHandle LockImpl(FiberInfo *self) noexcept {
    auto const owner = tail_.exchange(self, ::std::memory_order_acq_rel)->
                       next_.exchange(self, ::std::memory_order_relaxed);
    if (!owner) [[likely]] {
      return FiberHandle::Invalid();
    }

    UTIL_IGNORE(owner->next_.load(::std::memory_order_acquire)); // sync
    return Acquire(owner, self, true);
  }

  [[nodiscard]] static FiberHandle &&GetHandle(Node *node) noexcept {
    [[assume(node)]];
    return ::std::move(*static_cast<FiberInfo *>(node)).handle;
  }

  [[nodiscard]] static Node *TryTakeNext(Node *node) noexcept {
    auto next = node->Next();
    if (next) {
      return next;
    }

    node->next_.compare_exchange_strong(next, node, ::std::memory_order_release,
                                        ::std::memory_order_relaxed);
    return next;
  }

  FiberHandle Acquire(Node *owner, Node *next, bool resume) noexcept {
    if (owner == &dummy_) {
      Seal();

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

  void Seal() noexcept {
    dummy_.Link(nullptr);
    tail_.exchange(&dummy_, ::std::memory_order_acq_rel)->Link(&dummy_);
  }
};

} // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_SYNC_MUTEX_HPP_INCLUDED_ */
