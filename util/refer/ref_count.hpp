//
//
//
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_UTIL_REFER_REF_COUNT_HPP_INCLUDED_
#define DDVAMP_UTIL_REFER_REF_COUNT_HPP_INCLUDED_ 1

#include <atomic>
#include <concepts>
#include <type_traits>

#include "util/macro.hpp"

namespace util {

template <typename Derived>
class RefCount;

////////////////////////////////////////////////////////////////////////////////

template <typename T>
inline constexpr bool is_ref_count_v =
	::std::derived_from<T, RefCount<::std::remove_cv_t<T>>> &&
	requires (::std::remove_cv_t<T> const &t) {
		{ t.destroySelf() } noexcept;
	};

template <typename T>
struct is_ref_count : ::std::bool_constant<is_ref_count_v<T>> {};

namespace concepts {

template <typename T>
concept RefCount = is_ref_count_v<T>;

} // namespace concepts

////////////////////////////////////////////////////////////////////////////////

namespace detail {

template <typename T>
struct RefValidator {
	constexpr RefValidator() noexcept
	{
		static_assert(
			concepts::RefCount<T>,
			"Class does not inherit RefCount"
		);
	}
};

} // namespace detail

////////////////////////////////////////////////////////////////////////////////

template <typename Derived>
class RefCount {
private:
	mutable ::std::atomic_size_t ref_cnt_;

	UTIL_NO_UNIQUE_ADDRESS detail::RefValidator<Derived> v_;

public:
	explicit RefCount(::std::size_t count = 1) noexcept
		: ref_cnt_(count)
	{}

	[[nodiscard]] auto useCount() const noexcept
	{
		return ref_cnt_.load(::std::memory_order_relaxed);
	}

	void incRef() const noexcept
	{
		ref_cnt_.fetch_add(1, ::std::memory_order_relaxed);
	}

	void decRef() const noexcept
	{
		if (1 == ref_cnt_.fetch_sub(1, ::std::memory_order_acq_rel)) {
			static_cast<Derived const *>(this)->destroySelf();
		}
	}
};

} // namespace util

#endif /* DDVAMP_UTIL_REFER_REF_COUNT_HPP_INCLUDED_ */
