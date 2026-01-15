//
// meeting.hpp
// ~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_CONCURRENCY_MEETING_HPP_INCLUDED_
#define DDVAMP_CONCURRENCY_MEETING_HPP_INCLUDED_ 1

#include <util/debug/assert.hpp>

#include <atomic>
#include <cstdint>

namespace concurrency {

/**
 *  Synchronization primitive for determining the last participant
 *  and synchronizing it with all the previous ones
 */
class Meeting {
 private:
  using Count = ::std::uint64_t;

  ::std::atomic<Count> seats_;

  // To guarantee the expected implementation
  static_assert(::std::atomic<Count>::is_always_lock_free);

 public:
  explicit Meeting(Count const init) noexcept {
    Reset(init);
  }

  // In case of instance reuse
  void Reset(Count const init) noexcept {
    UTIL_ASSERT(init > 1, "At least two participants are required");
    seats_.store(init, ::std::memory_order_relaxed);
  }

  // Returns true and syncs with previous participants if this one is the last
  // May be false negative
  [[nodiscard]] bool IsReady() const noexcept {
    return seats_.load(::std::memory_order_acquire) == 1;
  }

  // Returns true and syncs with previous participants if this one is the last,
  // otherwise, he announces his arrival
  [[nodiscard]] bool Arrive() noexcept {
    auto const left = seats_.fetch_sub(1, ::std::memory_order_acq_rel);
    UTIL_ASSERT(left > 0, "The amount of participants exceeded expectation");
    return left == 1;
  }
};

} // namespace concurrency

#endif /* DDVAMP_CONCURRENCY_MEETING_HPP_INCLUDED_ */
