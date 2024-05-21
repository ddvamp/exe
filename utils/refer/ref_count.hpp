// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_UTILS_REFER_REF_COUNT_HPP_INCLUDED_
#define DDVAMP_UTILS_REFER_REF_COUNT_HPP_INCLUDED_ 1

#include <atomic>
#include <cstddef>

#include <utils/debug/assert.hpp>
#include <utils/macro.hpp>

#include "ref.hpp"

namespace utils {

template <typename Derived>
class ref_count {
 private:
  mutable ::std::atomic<::std::size_t> cnt_;
  UTILS_NO_UNIQUE_ADDRESS detail::ref_validator<Derived> v_;

  // To guarantee the expected implementation
  static_assert(::std::atomic<::std::size_t>::is_always_lock_free);

 public:
  constexpr explicit ref_count(::std::size_t init = 1) noexcept : cnt_(init) {}

  [[nodiscard]] ::std::size_t use_count() const noexcept {
    return cnt_.load(::std::memory_order_relaxed);
  }

  void inc_ref() const noexcept {
    UTILS_IGNORE(cnt_.fetch_add(1, ::std::memory_order_relaxed));
  }

  void dec_ref() const noexcept {
    auto const before = cnt_.fetch_sub(1, ::std::memory_order_acq_rel);
    if (before > 1) {
      return;
    }

    UTILS_ASSERT(before != 0, "Accessing an ref_counted object "
                              "with zero ref count");
    static_cast<Derived const *>(this)->destroy_self();
  }
};

}  // namespace utils

#endif /* DDVAMP_UTILS_REFER_REF_COUNT_HPP_INCLUDED_ */
