//
// internal/strand.hpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_INTERNAL_STRAND_HPP_INCLUDED_
#define DDVAMP_EXE_INTERNAL_STRAND_HPP_INCLUDED_ 1

#include <exe/runtime/strand.hpp>
#include <exe/runtime/task/scheduler.hpp>
#include <exe/runtime/task/task.hpp>

#include <util/debug/assert.hpp>
#include <util/mm/release_sequence.hpp>
#include <util/refer/ref_count.hpp>

#include <atomic>
#include <new>
#include <utility>

namespace exe::runtime {

class Strand::Impl final : private task::TaskBase,
                           public ::util::ref_count<Impl> {
 private:
  struct DummyTask : TaskBase {
    Impl &impl_;

    explicit DummyTask(Impl &impl) noexcept : impl_(impl) {
      Link(this);
    }

    void Run() && noexcept override {
      impl_.RunDummy();
    }
  };

  task::ISafeScheduler &underlying_;
  DummyTask dummy_;
  TaskBase *head_ = &dummy_;
  alignas (::std::hardware_destructive_interference_size)
      ::std::atomic<TaskBase *> tail_ = &dummy_;

  // To guarantee the expected implementation
  static_assert(::std::atomic<TaskBase *>::is_always_lock_free);

 private:
  explicit Impl(task::ISafeScheduler &underlying) noexcept
      : underlying_(underlying),
        dummy_(*this) {
    static_assert(
        sizeof(Impl) == 2 * ::std::hardware_destructive_interference_size,
        "Unexpected size of Strand implementation");
  }

 public:
  [[nodiscard]] static Impl *Create(task::ISafeScheduler &underlying) {
    return ::new Impl(underlying);
  }

  // util::ref_count
  void destroy_self() const noexcept {
    UTIL_ASSERT(dummy_.Next() == &dummy_,
                "Strand`s Run() cycle is destroyed during use");
    delete this;
  }

  [[nodiscard]] task::ISafeScheduler &GetUnderlying() const noexcept {
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

    ::util::SyncWithReleaseSequences(node->next_);

    inc_ref();
    underlying_.Submit(this);
  }

 private:
  void RunDummy() noexcept {
    dummy_.Link(nullptr);
    tail_.exchange(&dummy_, ::std::memory_order_acq_rel)->Link(&dummy_);
  }

  void Run() && noexcept override {
    auto node = head_;
    auto next = head_->Next();

    do {
      ::std::move(*node).Run();
      node = next;

      next = node->Next();
      if (!next) [[unlikely]] {
        if (node->next_.compare_exchange_strong(next, head_ = node,
                ::std::memory_order_release,
                ::std::memory_order_relaxed)) [[likely]] {
          dec_ref();
          return;
        }
      }
    } while (node != &dummy_);

    head_ = node;
    underlying_.Submit(this);
  }
};

} // namespace exe::runtime

#endif /* DDVAMP_EXE_INTERNAL_STRAND_HPP_INCLUDED_ */
