// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_UTILS_REFER_REF_COUNTED_H_
#define DDV_UTILS_REFER_REF_COUNTED_H_ 1

#include <atomic>
#include <concepts>
#include <type_traits>

namespace utils {

template <typename Derived>
class RefCounted;

////////////////////////////////////////////////////////////////////////////////

template <typename T>
inline constexpr bool is_ref_counted_v =
	::std::derived_from<T, RefCounted<::std::remove_cv_t<T>>> &&
	requires (::std::remove_cv_t<T> const &t) {
		{ t.destroySelf() } noexcept;
	};

template <typename T>
struct is_ref_counted : ::std::bool_constant<is_ref_counted_v<T>> {};

namespace concepts {

template <typename T>
concept RefCounted = is_ref_counted_v<T>;

} // namespace concepts

////////////////////////////////////////////////////////////////////////////////

namespace detail {

template <typename T>
struct RefValidator {
	constexpr RefValidator() noexcept
	{
		static_assert(
			concepts::RefCounted<T>,
			"Class does not inherit RefCounted"
		);
	}
};

} // namespace detail

////////////////////////////////////////////////////////////////////////////////

template <typename Derived>
class RefCounted {
private:
	mutable ::std::atomic_size_t ref_cnt_ = 1;

	[[no_unique_address]] detail::RefValidator<Derived> v_;

public:
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

} // namespace utils

#endif /* DDV_UTILS_REFER_REF_COUNTED_H_ */
