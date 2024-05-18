// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_FIBER_SYNC_MUTEX_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_SYNC_MUTEX_HPP_INCLUDED_ 1

#include <atomic>
#include <mutex>
#include <new>

#include <concurrency/intrusive/forward_list.hpp>
#include <utils/debug.hpp>
#include <utils/utility.hpp>

#include <exe/fiber/api.hpp>
#include <exe/fiber/core/awaiter.hpp>
#include <exe/fiber/core/handle.hpp>

namespace exe::fiber {

class CondVar;

class alignas (::std::hardware_destructive_interference_size) Mutex {
  friend class CondVar;

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
  Node *head_ = &dummy_;  // known next owner or dummy
  ::std::atomic<Node *> tail_ = &dummy_;  // last added or dummy
  ::std::atomic<FiberId> owner_ = kInvalidFiberId;
  
  // To guarantee the expected implementation
  static_assert(::std::atomic<Node *>::is_always_lock_free);
  static_assert(::std::atomic<FiberId>::is_always_lock_free);

 public:
  ~Mutex() {
    UTILS_ASSERT(!IsLocked(), "Mutex is destroyed during use");
  }

  Mutex(Mutex const &) = delete;
  void operator= (Mutex const &) = delete;

  Mutex(Mutex &&) = delete;
  void operator= (Mutex &&) = delete;

 public:
  Mutex() = default;

  [[nodiscard]] bool TryLock() noexcept {
    UTILS_DEBUG_RUN(CheckRecursive);

    if (IsLocked()) [[unlikely]] {
      return false;
    }

    return dummy_.next_.compare_exchange_weak(::utils::temporary(&dummy_),
                                              nullptr,
                                              ::std::memory_order_acquire,
                                              ::std::memory_order_relaxed);
  }

  void Lock() noexcept {
    if (!TryLock()) [[unlikely]] {
      LockCold();
    }

    UTILS_DEBUG_RUN(CheckLock);
  }

  void Unlock() noexcept {
    UTILS_DEBUG_RUN(CheckUnlock);

    auto next = UnlockImpl();
    if (next.IsValid()) {
      ::std::move(next).Schedule();
    }
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

  void LockCold() noexcept {
    LockAwaiter awaiter(this);
    self::Suspend(awaiter);
  }

  FiberHandle LockImpl(FiberInfo *self) noexcept {
    auto const owner = tail_.exchange(self, ::std::memory_order_acq_rel)->
                       next_.exchange(self, ::std::memory_order_acq_rel);
    if (!owner) [[likely]] {
      return FiberHandle::Invalid();
    }

    if (IsActualFiber(owner)) [[unlikely]] {
      head_ = self;
      GetHandle(owner).Schedule();
      return FiberHandle::Invalid();
    }

    dummy_.Link(nullptr);
    return TakeLastFiber(self);
  }

  FiberHandle UnlockImpl() noexcept {
    auto succ = head_;
    if (IsActualFiber(succ)) [[unlikely]] {
      return TakeFiber(succ);
    }

    if ((succ = TryTakeNext(succ))) [[unlikely]] {
      dummy_.Link(nullptr);
      return TakeFiber(succ);
    }

    return FiberHandle::Invalid();
  }

  FiberHandle TakeFiber(Node *owner) noexcept {
    auto const succ = owner->next_.load(::std::memory_order_acquire);
    if (succ) {
      head_ = succ;
      return GetHandle(owner);
    }

    return TakeLastFiber(owner);
  }

  FiberHandle TakeLastFiber(Node *owner) noexcept {
    auto succ = &dummy_;
    if (tail_.compare_exchange_strong(::utils::temporary(+owner), &dummy_,
                                      ::std::memory_order_release,
                                      ::std::memory_order_relaxed) ||
        (succ = TryTakeNext(owner))) {
      head_ = succ;
      return GetHandle(owner);
    }

    return FiberHandle::Invalid();
  }

  [[nodiscard]] bool IsActualFiber(Node *node) const noexcept {
    return node != &dummy_;
  }

  static FiberHandle &&GetHandle(Node *node) noexcept {
    [[assume(node)]];
    return ::std::move(*static_cast<FiberInfo *>(node)).handle_;
  }

  [[nodiscard]] Node *TryTakeNext(Node *node) noexcept {
    auto next = node->next_.load(::std::memory_order_acquire);
    if (next) {
      return next;
    }

    node->next_.compare_exchange_strong(next, head_ = node,
                                        ::std::memory_order_release,
                                        ::std::memory_order_acquire);
    return next;
  }

 private:
  [[maybe_unused]] FiberId GetOwner() const noexcept {
    return owner_.load(::std::memory_order_relaxed);
  }

  [[maybe_unused]] void SetOwner(FiberId const id) noexcept {
    owner_.store(id, ::std::memory_order_relaxed);
  }

  [[maybe_unused]] void CheckOwner() const noexcept {
    UTILS_CHECK(GetOwner() == self::GetId(), "The mutex is not locked");
  }

  [[maybe_unused]] void CheckRecursive() const noexcept {
    UTILS_CHECK(GetOwner() != self::GetId(),
                "Attempt to recursively lock a mutex");
  }

  [[maybe_unused]] void CheckLock() noexcept {
    UTILS_CHECK(GetOwner() == kInvalidFiberId,
                "Attempt to lock a already locked mutex");
    SetOwner(self::GetId());
  }

  [[maybe_unused]] void CheckUnlock() noexcept {
    UTILS_CHECK(GetOwner() == self::GetId(),
                "Attempt to unlock a mutex without ownership");
    SetOwner(kInvalidFiberId);
  }
};

}  // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_SYNC_MUTEX_HPP_INCLUDED_ */
