//
// wait_group.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_CONCURRENCY_WAIT_GROUP_HPP_INCLUDED_
#define DDVAMP_CONCURRENCY_WAIT_GROUP_HPP_INCLUDED_ 1

#include <util/utility.hpp>
#include <util/debug/assert.hpp>

#include <atomic>
#include <condition_variable>
#include <cstdint> // std::uint64_t
#include <mutex>

namespace concurrency {

/**
 *  Synchronization primitive for waiting for the completion of tasks,
 *  which are expressed as a 32-bit counter. Formally, the wait ends when
 *  the counter drops to zero. It can be used multiple times, but
 *  waiting sessions must happen before one another
 */
class WaitGroup {
 private:
  using State = ::std::uint64_t;

  enum Constants : State {
    kMaxCount = 0xFFFF'FFFF,
    kOneWaiter
  };

  // [ 32 bits | 32 bits ]
  // [ waiters | counter ]
  ::std::atomic<State> state_;
  ::std::mutex m_;
  ::std::condition_variable count_is_zero_;

  // To guarantee the expected implementation
  static_assert(::std::atomic<State>::is_always_lock_free);

 public:
  ~WaitGroup() {
    UTIL_ASSERT(GetCount(state_.load(::std::memory_order_relaxed)) == 0,
                "WaitGroup is destroyed during waiting session");
  }

  WaitGroup(WaitGroup const &) = delete;
  void operator= (WaitGroup const &) = delete;

  WaitGroup(WaitGroup &&) = delete;
  void operator= (WaitGroup &&) = delete;

 public:
  // Throws: std::system_error
  // Error conditions:
  // - resource_unavailable_try_again (condvar)
  explicit WaitGroup(State const init = 0) {
    Reset(init);
  }

  // Sets the value of the counter at the beginning of the waiting session.
  // This call must happen after any previous session calls and
  // happen before any current session calls
  void Reset(State const init = 0) noexcept {
    UTIL_ASSERT(init <= kMaxCount,
                "The init value does not fit into the 32-bit counter");
    state_.store(init, ::std::memory_order_relaxed);
  }

  // Returns true if the counter is already zero
  [[nodiscard]] bool IsReady() const noexcept {
    return GetCount(state_.load(::std::memory_order_acquire)) == 0;
  }

  // Increases the counter by delta
  void Add(State const delta = 1) noexcept {
    [[maybe_unused]] auto const state = state_.fetch_add(
        delta, ::std::memory_order_relaxed);
    UTIL_ASSERT(
        IsAddCorrect(state, delta),
        "The delta must be non-zero and not overflow the 32-bit counter");
  }

  // Decreases the counter by delta and if the counter drops to zero,
  // unlocks all currently waiting threads, if any.
  // This call must happen after corresponding Add() call
  //
  // Throws: std::system_error
  // Error conditions:
  // - operation_not_permitted (mutex)
  // - resource_deadlock_would_occur (mutex)
  void Done(State const delta = 1) {
    auto const state = state_.fetch_sub(delta, ::std::memory_order_release);
    UTIL_ASSERT(
        IsDoneCorrect(state, delta),
        "The delta must be non-zero and not underflow the 32-bit counter");
    if (IsNotifyNeeded(state - delta)) [[unlikely]] {
      { ::std::lock_guard guard(m_); }  // lock-unlock
      count_is_zero_.notify_all();
    }
  }

  // Blocks the current thread until the counter drops to zero.
  // For a single waiting session this call must happen after
  // all Reset() calls and all Add() calls on the zero counter
  //
  // Throws: std::system_error
  // Error conditions:
  // - operation_not_permitted (mutex)
  // - resource_deadlock_would_occur (mutex)
  void Wait() {
    if (IsReady()) [[likely]] {
      // Fast path
      return;
    }

    auto const state = state_.fetch_add(kOneWaiter,
                                        ::std::memory_order_acquire);
    if (GetCount(state) == 0) [[likely]] {
      // Fast path
      return;
    }

    ::std::unique_lock lock(m_);
    while (!IsReady()) {
      count_is_zero_.wait(lock);
    }
  }

 private:
  // Checks if delta is correct (fits into the counter and is not zero), and
  // that the counter is not overflowing after increasing by delta
  [[maybe_unused, nodiscard]] inline static bool IsAddCorrect(
      State const state, State const delta) noexcept {
    // cnt + delta <= MAX_CNT && delta != 0
    return delta - 1 < GetCount(~state);
  }

  // Checks if delta is correct (fits into the counter and is not zero), and
  // that the counter is not underflowing after decreasing by delta
  [[maybe_unused, nodiscard]] inline static bool IsDoneCorrect(
      State const state, State const delta) noexcept {
    // cnt - delta >= 0 && delta != 0
    return delta - 1 < GetCount(state);
  }

  [[nodiscard]] inline static bool IsNotifyNeeded(State const state) noexcept {
    return ::util::all_of(GetCount(state) == 0, GetWaitersCount(state) != 0);
  }

  [[nodiscard]] inline static ::std::uint32_t GetCount(State const state)
      noexcept {
    return static_cast<::std::uint32_t>(state);
  }

  [[nodiscard]] inline static ::std::uint32_t GetWaitersCount(State const state)
      noexcept {
    return static_cast<::std::uint32_t>(state >> 32);
  }
};

} // namespace concurrency

#endif /* DDVAMP_CONCURRENCY_WAIT_GROUP_HPP_INCLUDED_ */
