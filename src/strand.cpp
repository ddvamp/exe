// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#include "exe/sched/strand.hpp"

#include <util/macro.hpp>
#include <util/debug/assert.hpp>
#include <util/refer/ref_count.hpp>

#include <atomic>
#include <new>

namespace exe::sched {

class Strand::Impl final : private task::TaskBase,
                           public ::util::ref_count<Impl> {
 private:
  struct DummyTask : TaskBase {
    Impl &impl_;

    explicit DummyTask(Impl &impl) noexcept : impl_(impl) {
      Link(this);
    }

    void Run() noexcept override {
      impl_.DummyRun();
    }
  };

  IScheduler &underlying_;
  DummyTask dummy_;
  TaskBase *head_ = &dummy_;
  alignas (::std::hardware_destructive_interference_size)
      ::std::atomic<TaskBase *> tail_ = &dummy_;

  // To guarantee the expected implementation
  static_assert(::std::atomic<TaskBase *>::is_always_lock_free);

 private:
  explicit Impl(IScheduler &underlying) noexcept
      : underlying_(underlying),
        dummy_(*this) {
    static_assert(
        sizeof(Impl) == 2 * ::std::hardware_destructive_interference_size,
        "Unexpected size of Strand implementation");
  }

 public:
  [[nodiscard]] static Impl *Create(IScheduler &underlying) {
    return ::new Impl(underlying);
  }

  void destroy_self() const noexcept {
    UTIL_ASSERT(dummy_.next_.load(::std::memory_order_relaxed) == &dummy_,
                "Strand`s Run() cycle is destroyed during use");
    delete this;
  }

  [[nodiscard]] IScheduler &GetUnderlying() const noexcept {
    return underlying_;
  }

  void Submit(TaskBase *task) noexcept {
    UTIL_ASSERT(task, "nullptr instead of task");
    task->Link(nullptr);
    auto const node = tail_.exchange(task, ::std::memory_order_acq_rel)->
                      next_.exchange(task, ::std::memory_order_relaxed);
    if (!node) [[likely]] {
      return;
    }

    inc_ref();
    UTIL_IGNORE(node->next_.load(::std::memory_order_acquire));  // sync
    underlying_.Submit(this);
  }

 private:
  void DummyRun() noexcept {
    dummy_.Link(nullptr);
    tail_.exchange(&dummy_, ::std::memory_order_acq_rel)->Link(&dummy_);
  }

  void Run() noexcept override {
    auto node = head_;
    auto next = node->next_.load(::std::memory_order_relaxed);

    while (true) {
      node->Run();
      node = next;

      next = node->next_.load(::std::memory_order_relaxed);
      if (!next) [[unlikely]] {
        if (node->next_.compare_exchange_strong(next, head_ = node,
                ::std::memory_order_release,
                ::std::memory_order_relaxed)) [[likely]] {
          dec_ref();
          return;
        }
      }

      if (node == &dummy_) [[unlikely]] {
        head_ = node;
        underlying_.Submit(this);
        return;
      }
    }
  }
};

Strand::~Strand() = default;

Strand::Strand(IScheduler &underlying) : impl_(Impl::Create(underlying)) {}

/* virtual */ void Strand::Submit(task::TaskBase *task) noexcept {
  impl_->Submit(task);
}

task::ISafeScheduler &Strand::GetUnderlying() const noexcept {
  return impl_->GetUnderlying();
}

} // namespace exe::sched
