//
// wait_group.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FIBER_SYNC_WAIT_GROUP_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_SYNC_WAIT_GROUP_HPP_INCLUDED_ 1

#include <exe/fiber/sync/event.hpp>

#include <util/debug/assert.hpp>
#include <util/mm/release_sequence.hpp>

#include <atomic>
#include <cstdint>

namespace exe::fiber {

// [TODO]: Add IsReady, improve description (Reset on edge of session using),
// make all (thread and fiber) primitives consistent semantically

/**
 *  Synchronization primitive for waiting for the completion of tasks,
 *  which are expressed as a 64-bit counter. Formally, the wait ends when
 *  the counter drops to zero
 *  The Add() calls should happens before the corresponding Done() and
 *  Wait() calls, otherwise the behavior is undefined
 *  WaitGroup can be reused. In this case, all current Wait() calls should
 *  ends (happens) before subsequent Add() calls
 */
class WaitGroup {
 private:
  using Count = ::std::uint64_t;

  ::std::atomic<Count> count_;
  Event count_is_zero_;

  // To guarantee the expected implementation
  static_assert(::std::atomic<Count>::is_always_lock_free);

 public:
  ~WaitGroup() {
    UTIL_ASSERT(count_.load(::std::memory_order_relaxed) == 0,
                 "WaitGroup is destroyed during waiting session");
  }

  WaitGroup(WaitGroup const &) = delete;
  void operator= (WaitGroup const &) = delete;

  WaitGroup(WaitGroup &&) = delete;
  void operator= (WaitGroup &&) = delete;

 public:
  explicit WaitGroup(Count const init = 0) noexcept : count_(init) {}

  // In case of an instance reuse
  void Reset(Count const init = 0) noexcept {
    count_.store(init, ::std::memory_order_relaxed);
    count_is_zero_.Reset();
  }

  // Increases the counter by delta
  void Add(Count const delta = 1) noexcept {
    [[maybe_unused]] auto const old_count =
        count_.fetch_add(delta, ::std::memory_order_relaxed);
    UTIL_ASSERT(old_count < old_count + delta,
                "Increment must be non-zero and not overflow the counter");
  }

  // Reduces the counter by delta, performs synchronization, and
  // if the counter drops to zero, unblocks all fibers currently waiting for
  void Done(Count const delta = 1) noexcept {
    auto const old_count = count_.fetch_sub(delta, ::std::memory_order_release);
    UTIL_ASSERT(old_count - delta < old_count,
                "Decrement must be non-zero and not underflow the counter");
    if (old_count != delta) [[likely]] {
      // Fast path
      return;
    }

    ::util::SyncWithReleaseSequences(count_);
    count_is_zero_.Fire();
  }

  // Blocks the current fiber until the counter drops to zero, and
  // synchronizes with fibers that performed synchronization when calling Done()
  void Wait() noexcept {
    count_is_zero_.Wait();
  }
};

} // namespace exe::fiber

#endif /* DDVAMP_EXE_FIBER_SYNC_WAIT_GROUP_HPP_INCLUDED_ */
