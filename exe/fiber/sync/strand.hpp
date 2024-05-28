// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_FIBER_SYNC_STRAND_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_SYNC_STRAND_HPP_INCLUDED_ 1

#include <atomic>
#include <memory>  // std::addressof
#include <new>
#include <type_traits>
#include <utility>

#include <concurrency/intrusive/forward_list.hpp>
#include <util/debug/assert.hpp>
#include <util/macro.hpp>

#include <exe/fiber/api.hpp>
#include <exe/fiber/core/awaiter.hpp>
#include <exe/fiber/core/handle.hpp>

namespace exe::fiber {

class alignas (::std::hardware_destructive_interference_size) Strand {
 private:
  struct CombineTask {
    using Fn = void (void *&, FiberHandle &) noexcept;

    void *cs_;
    Fn *fn_;
    Fn *master_;

    CombineTask(CombineTask &) = default;

    template <typename CS>
    CombineTask(CS &&cs) noexcept
        : cs_(const_cast<void *>(static_cast<void const volatile *>(
                                     ::std::addressof(cs)))),
          fn_(Slave<CS>),
          master_(Master<CS>) {}

    [[nodiscard]] bool IsUsed() const noexcept {
      return cs_;
    }

    void Switch() noexcept {
      fn_ = master_;
    }

    void Run(FiberHandle &h) noexcept {
      fn_(cs_, h);
    }

    template <typename CS>
    static void Master(void *&cs, FiberHandle &) noexcept {
      ::std::forward<CS>(*static_cast<::std::remove_reference_t<CS> *>(cs))();
    }

    template <typename CS>
    static void Slave(void *&cs, FiberHandle &h) noexcept {
      Master<CS>(cs, h);
      cs = nullptr;
      ::std::move(h).Schedule();
    }
  };

  struct CombineAwaiter : IAwaiter, CombineTask,
                          concurrency::IntrusiveForwardListNode<> {
    Strand *s_;
    FiberHandle handle_;

    CombineAwaiter(CombineTask task) noexcept : CombineTask(task) {}

    FiberHandle AwaitSymmetricSuspend(FiberHandle &&self) noexcept override {
      handle_ = ::std::move(self);
      return s_->CombineImpl(this);
    }

    [[nodiscard]] bool AmICombiner() const noexcept {
      return IsUsed();
    }

    [[nodiscard]] FiberHandle &&GetHandle() noexcept {
      Switch();
      return ::std::move(handle_);
    }

    void operator() () noexcept {
      Run(handle_);
    }
  };

  using Node = CombineAwaiter::Node;

  Node dummy_{.next_ = &dummy_};
  ::std::atomic<Node *> tail_ = &dummy_;

  // To guarantee the expected implementation
  static_assert(::std::atomic<Node *>::is_always_lock_free);

 public:
  ~Strand() {
    UTIL_ASSERT(dummy_.next_.load(::std::memory_order_relaxed) == &dummy_,
                 "Strand is destroyed during use");
  }

  Strand(Strand const &) = delete;
  void operator= (Strand const &) = delete;

  Strand(Strand &&) = delete;
  void operator= (Strand &&) = delete;

 public:
  constexpr Strand() = default;

  void Combine(CombineAwaiter awaiter) noexcept {
    awaiter.s_ = this;
    self::Suspend(awaiter);

    if (!awaiter.AmICombiner()) [[likely]] {
      return;
    }

    CombineCold(&awaiter);
  }

 private:
  FiberHandle CombineImpl(CombineAwaiter *self) noexcept {
    auto const tail = tail_.exchange(self, ::std::memory_order_acq_rel);
    auto const head = tail->next_.exchange(self, ::std::memory_order_relaxed);
    if (!head) [[likely]] {
      return FiberHandle::Invalid();
    }

    return CombineImplCold(head, tail, self);
  }

  void CombineCold(Node *node) noexcept {
    NoSwitchContextGuard guard;

    auto next = node->next_.load(::std::memory_order_relaxed);
    do {
      static_cast<CombineAwaiter &>(*node)();
      node = next;

      next = TryTakeNext(node, node);
      if (!next) [[unlikely]] {
        return;
      }
    } while (node != &dummy_);

    if (Acquire(next)) [[likely]] {
      GetHandle(next).Schedule();
    }
  }

  FiberHandle CombineImplCold(Node *head, Node *tail, Node *self) noexcept {
    UTIL_IGNORE(tail->next_.load(::std::memory_order_acquire));  // sync

    if (tail != &dummy_) [[likely]] {
      auto const next = TryTakeNext(self, head);
      if (next) [[likely]] {
        self->Link(head);
        tail->Link(next);
        return GetHandle(self);
      }

      return FiberHandle::Invalid();
    }

    if (Acquire(self)) [[likely]] {
      return GetHandle(self);
    }

    return FiberHandle::Invalid();
  }

  [[nodiscard]] bool Acquire(Node *node) noexcept {
    dummy_.Link(nullptr);
    tail_.exchange(&dummy_, ::std::memory_order_acq_rel)->Link(&dummy_);
    return TryTakeNext(node, node);
  }

  static [[nodiscard]] FiberHandle &&GetHandle(Node *node) noexcept {
    return static_cast<CombineAwaiter *>(node)->GetHandle();
  }

  static [[nodiscard]] Node *TryTakeNext(Node *node, Node *head) noexcept {
    auto next = node->next_.load(::std::memory_order_relaxed);
    if (next) [[likely]] {
      return next;
    }

    node->next_.compare_exchange_strong(next, head, ::std::memory_order_release,
                                        ::std::memory_order_relaxed);
    return next;
  }
};

}  // namespace exe::fiber

#endif  /* DDVAMP_EXE_FIBER_SYNC_STRAND_HPP_INCLUDED_ */
