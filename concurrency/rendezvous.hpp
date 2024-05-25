// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_CONCURRENCY_RENDEZVOUS_HPP_INCLUDED_
#define DDVAMP_CONCURRENCY_RENDEZVOUS_HPP_INCLUDED_ 1

#include <atomic>

namespace concurrency {

class Rendezvous {
 private:
  ::std::atomic<bool> both_ = false;

  // To guarantee the expected implementation
  static_assert(::std::atomic<bool>::is_always_lock_free);

 public:
  constexpr Rendezvous() = default;

  // In case of instance reuse
  void Reset() noexcept {
    both_.store(false, ::std::memory_order_relaxed);
  }

  // May be false negative
  [[nodiscard]] bool IsReady() const noexcept {
    return both_.load(::std::memory_order_acquire);
  }

  [[nodiscard]] bool Arrive() noexcept {
    auto expected = false;
    both_.compare_exchange_strong(expected, true, ::std::memory_order_release,
                                  ::std::memory_order_acquire);
    return expected;
  }
};

}  // namespace concurrency

#endif /* DDVAMP_CONCURRENCY_RENDEZVOUS_HPP_INCLUDED_ */
