//
// ref_count.hpp
// ~~~~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_UTIL_REFER_REF_COUNT_HPP_INCLUDED_
#define DDVAMP_UTIL_REFER_REF_COUNT_HPP_INCLUDED_ 1

#include <util/macro.hpp>
#include <util/debug/assert.hpp>
#include <util/mm/release_sequence.hpp>

#include <atomic>
#include <concepts>
#include <cstddef>

namespace util {

template <typename Derived>
class ref_count {
 private:
  mutable ::std::atomic<::std::size_t> cnt_;

  // To guarantee the expected implementation
  static_assert(::std::atomic<::std::size_t>::is_always_lock_free);

 public:
  constexpr explicit ref_count(::std::size_t init = 1) noexcept : cnt_(init) {}

  // It may return an stale value
  [[nodiscard]] ::std::size_t use_count() const noexcept {
    return cnt_.load(::std::memory_order_relaxed);
  }

  void inc_ref() const noexcept {
    UTIL_IGNORE(cnt_.fetch_add(1, ::std::memory_order_relaxed));
  }

  void dec_ref() const noexcept
      requires (::std::derived_from<Derived, ref_count> &&
                requires (Derived const &d) {
                  { d.destroy_self() } noexcept -> ::std::same_as<void>;
                }) {
    auto const before = cnt_.fetch_sub(1, ::std::memory_order_release);
    if (before > 1) {
      return;
    }

    UTIL_ASSERT(before != 0, "An attempt to lower a ref_count below zero");
    ::util::SyncWithReleaseSequences(cnt_);
    static_cast<Derived const *>(this)->destroy_self();
  }
};

} // namespace util

#endif /* DDVAMP_UTIL_REFER_REF_COUNT_HPP_INCLUDED_ */
