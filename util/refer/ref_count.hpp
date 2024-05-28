// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_UTIL_REFER_REF_COUNT_HPP_INCLUDED_
#define DDVAMP_UTIL_REFER_REF_COUNT_HPP_INCLUDED_ 1

#include <atomic>
#include <cstddef>

#include "ref.hpp"
#include <util/debug/assert.hpp>
#include <util/macro.hpp>

namespace util {

template <typename Derived>
class ref_count {
 private:
  mutable ::std::atomic<::std::size_t> cnt_;

  // To guarantee the expected implementation
  static_assert(::std::atomic<::std::size_t>::is_always_lock_free);

 public:
  constexpr explicit ref_count(::std::size_t init = 1) noexcept : cnt_(init) {
    UTIL_REF_VALIDATE_TYPE(Derived);
  }

  [[nodiscard]] ::std::size_t use_count() const noexcept {
    return cnt_.load(::std::memory_order_relaxed);
  }

  void inc_ref() const noexcept {
    UTIL_IGNORE(cnt_.fetch_add(1, ::std::memory_order_relaxed));
  }

  void dec_ref() const noexcept {
    UTIL_REF_VALIDATE_TYPE(Derived);

    auto const before = cnt_.fetch_sub(1, ::std::memory_order_acq_rel);
    if (before > 1) {
      return;
    }

    UTIL_ASSERT(before != 0, "Accessing an ref_counted object "
                              "with zero ref count");
    static_cast<Derived const *>(this)->destroy_self();
  }
};

}  // namespace util

#endif  /* DDVAMP_UTIL_REFER_REF_COUNT_HPP_INCLUDED_ */
