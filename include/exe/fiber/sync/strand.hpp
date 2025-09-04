//
// strand.hpp
// ~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FIBER_SYNC_STRAND_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_SYNC_STRAND_HPP_INCLUDED_ 1

#include <exe/fiber/api.hpp>
#include <exe/fiber/core/awaiter.hpp>
#include <exe/fiber/core/handle.hpp>

#include <concurrency/intrusive/forward_list.hpp>
#include <util/macro.hpp>
#include <util/utility.hpp>
#include <util/debug/assert.hpp>

#include <atomic>
#include <new>
#include <type_traits>

namespace exe::fiber {

class alignas (::std::hardware_destructive_interference_size) Strand {
 private:
  struct CombineTask {
    using Fn = void (void *) noexcept;

    void *cs_;
    Fn *ramp_;

    CombineTask(CombineTask &) = default;

    template <typename CS>
    CombineTask(CS &&cs) noexcept : cs_(::util::voidify(cs)), ramp_(Task<CS>) {}

    void RunCS() noexcept {
      ramp_(cs_);
    }

    template <typename CS>
    static void Task(void *cs) noexcept {
      ::std::forward<CS>(*static_cast<::std::remove_reference_t<CS> *>(cs))();
    }
  };

  struct CombineAwaiter : IAwaiter, CombineTask,
                          ::concurrency::IntrusiveForwardListNode<> {
    Strand *strand_;
    FiberHandle handle_;
    bool leader_ = false;

    CombineAwaiter(Strand *strand, CombineTask task) noexcept
        : CombineTask(task)
        , strand_(strand) {}

    ~CombineAwaiter() {
      if (leader_) [[unlikely]] {
        strand_->RunCombine(this);
      }
    }

    FiberHandle AwaitSymmetricSuspend(FiberHandle &&self) noexcept override {
      handle_ = ::std::move(self);
      strand_->AddTask(this);
      return FiberHandle::Invalid();
    }

    void Schedule() noexcept {
      leader_ = true;
      ::std::move(handle_).Schedule();
    }

    void operator() () noexcept {
      RunCS();
      ::std::move(handle_).Schedule();
    }
  };

  using Node = CombineAwaiter::Node;

  Node dummy_{.next_ = &dummy_};
  ::std::atomic<Node *> tail_ = &dummy_;

  // To guarantee the expected implementation
  static_assert(::std::atomic<Node *>::is_always_lock_free);

 public:
  ~Strand() {
    UTIL_ASSERT(dummy_.Next() == &dummy_, "Strand is destroyed during use");
  }

  Strand(Strand const &) = delete;
  void operator= (Strand const &) = delete;

  Strand(Strand &&) = delete;
  void operator= (Strand &&) = delete;

 public:
  constexpr Strand() = default;

  template <typename Fn>
  void Combine(Fn fn) noexcept {
    Combine(::std::forward<Fn>(fn));
  }

 private:
  void Combine(CombineTask task) noexcept {
    CombineAwaiter awaiter(this, task);
    self::Suspend(awaiter);
  }

  void AddTask(CombineAwaiter *node) noexcept {
    auto const tail = tail_.exchange(node, ::std::memory_order_acq_rel);
    auto const head = tail->next_.exchange(node, ::std::memory_order_relaxed);
    if (!head) [[likely]] {
      return;
    }

    UTIL_IGNORE(tail->next_.load(::std::memory_order_acquire)); // sync

    if (tail != &dummy_) [[likely]] {
      TrySchedule(head, node);
      return;
    }

    Acquire(node);
  }

  void RunCombine(CombineAwaiter *self) noexcept {
    NoSwitchContextGuard guard;

    self->RunCS();
    auto node = self->Next();

    while (auto next = TryTakeNext(node, node)) {
      if (node == &dummy_) {
        Acquire(next);
        return;
      }

      static_cast<CombineAwaiter &>(*node)();
      node = next;
    }
  }

  void Acquire(Node *node) noexcept {
    dummy_.Link(nullptr);
    tail_.exchange(&dummy_, ::std::memory_order_acq_rel)->Link(&dummy_);
    TrySchedule(node, node);
  }

  void TrySchedule(Node *head, Node *tail) noexcept {
    if (TryTakeNext(tail, head)) [[likely]] {
      static_cast<CombineAwaiter *>(head)->Schedule();
    }
  }

  [[nodiscard]] static Node *TryTakeNext(Node *node, Node *head) noexcept {
    auto next = node->Next();
    if (next) [[likely]] {
      return next;
    }

    node->next_.compare_exchange_strong(next, head, ::std::memory_order_release,
                                        ::std::memory_order_relaxed);
    return next;
  }
};

} // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_SYNC_STRAND_HPP_INCLUDED_ */
