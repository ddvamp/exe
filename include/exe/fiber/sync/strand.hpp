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
    void *cs;
    void (*ramp)(void *);

    CombineTask(CombineTask &) = default;

    template <typename CS>
    explicit CombineTask(CS &cs) noexcept
        : cs(::util::voidify(cs))
        , ramp(Task<CS>) {}

    void Run() noexcept {
      ramp(cs);
    }

    template <typename CS>
    static void Task(void *cs) noexcept {
      ::std::move(*static_cast<CS *>(cs))();
    }
  };

  struct FiberInfo : CombineTask, ::concurrency::IntrusiveForwardListNode<> {
    FiberHandle handle;
    bool leader = false;
  };

  struct CombineAwaiter final : IAwaiter, FiberInfo {
    Strand &strand;

    CombineAwaiter(Strand &strand, CombineTask task) noexcept
        : FiberInfo(task)
        , strand(strand) {}

    ~CombineAwaiter() {
      if (leader) [[unlikely]] {
        strand.RunCombine(this);
      }
    }

    FiberHandle AwaitSymmetricSuspend(FiberHandle &&self) noexcept override {
      handle = ::std::move(self);
      strand.AddTask(this);
      return FiberHandle::Invalid();
    }
  };

  using Node = FiberInfo::Node;

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
  void Combine(Fn fn) noexcept
      requires (::std::is_nothrow_destructible_v<Fn> &&
                ::std::is_nothrow_invocable_v<Fn> &&
                ::std::is_void_v<::std::invoke_result_t<Fn>>) {
    Combine(CombineTask(fn));
  }

 private:
  void Combine(CombineTask task) noexcept {
    CombineAwaiter awaiter(*this, task);
    self::Suspend(awaiter);
  }

  void AddTask(FiberInfo *self) noexcept {
    auto const pred = tail_.exchange(self, ::std::memory_order_acq_rel)->
                      next_.exchange(self, ::std::memory_order_relaxed);
    if (!pred) [[likely]] {
      return;
    }

    // Synchronization
    UTIL_IGNORE(pred->next_.load(::std::memory_order_acquire));

    if (pred != &dummy_) [[likely]] {
      return Schedule(pred);
    }

    Seal(self);
  }

  void RunCombine(FiberInfo *self) noexcept {
    NoSwitchContextGuard guard;

    auto node = self->Next();
    self->Run();

    while (auto next = TryTakeNext(node)) {
      if (node == &dummy_) [[unlikely]] {
        return Seal(next);
      }

      Run(node);
      node = next;
    }
  }

  void Seal(Node *node) noexcept {
    dummy_.Link(nullptr);
    tail_.exchange(&dummy_, ::std::memory_order_acq_rel)->Link(&dummy_);

    if (TryTakeNext(node)) [[likely]] {
      Schedule(node);
    }
  }

  [[nodiscard]] static Node *TryTakeNext(Node *node) noexcept {
    auto next = node->Next();
    if (next) [[likely]] {
      return next;
    }

    node->next_.compare_exchange_strong(next, node, ::std::memory_order_release,
                                        ::std::memory_order_relaxed);
    return next;
  }

  static void Schedule(Node *node) noexcept {
    [[assume(node)]];
    auto &a = *static_cast<FiberInfo *>(node);
    a.leader = true;
    ::std::move(a).handle.Schedule();
  }

  static void Run(Node *node) noexcept {
    [[assume(node)]];
    auto &a = *static_cast<FiberInfo *>(node);
    a.Run();
    ::std::move(a).handle.Schedule();
  }
};

} // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_SYNC_STRAND_HPP_INCLUDED_ */
