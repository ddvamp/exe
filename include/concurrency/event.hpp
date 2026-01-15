//
// event.hpp
// ~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_CONCURRENCY_EVENT_HPP_INCLUDED_
#define DDVAMP_CONCURRENCY_EVENT_HPP_INCLUDED_ 1

#include <atomic>

#include <util/debug/assert.hpp>

namespace concurrency {

/**
 *  Synchronization primitive for waiting for a event and
 *  sharing data through it
 *
 *  The user must ensure that the exit from the last of Wait or Fire call
 *  happens before the primitive is destroyed. For details see
 *  https://open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2616r4.html
 */
class Event {
 private:
  ::std::atomic_flag is_ready_;

 public:
  [[nodiscard]] bool IsReady() const noexcept {
    return is_ready_.test(::std::memory_order_acquire);
  }

  // It may end before the end of the Fire call
  void Wait() const noexcept {
    is_ready_.wait(false, ::std::memory_order_acquire);
  }

  void Fire() noexcept {
    UTIL_VERIFY(!is_ready_.test_and_set(::std::memory_order_release),
                "Event was fired a second time");

    is_ready_.notify_all();
  }

  // In case of instance reuse
  void Reset() noexcept {
    is_ready_.clear(::std::memory_order_relaxed);
  }
};

} // namespace concurrency

#endif /* DDVAMP_CONCURRENCY_EVENT_HPP_INCLUDED_ */
