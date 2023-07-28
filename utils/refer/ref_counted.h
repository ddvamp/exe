// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_UTILS_REFER_REF_COUNTED_H_
#define DDV_UTILS_REFER_REF_COUNTED_H_ 1

#include <atomic>
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace utils {

////////////////////////////////////////////////////////////////////////////////

namespace concepts {

template <typename T>
concept RefDestructible =
	::std::destructible<T> &&
	requires (T const &t) {
		{ t.destroySelf() } noexcept;
	};

} // namespace concepts

////////////////////////////////////////////////////////////////////////////////

template <concepts::RefDestructible Derived>
class RefCounted {
private:
	mutable ::std::atomic_size_t ref_cnt_ = 1;

public:
	[[nodiscard]] ::std::size_t useCount() const noexcept
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
			destroySelf();
		}
	}

private:
	auto self() const noexcept
	{
		static_assert(::std::derived_from<Derived, RefCounted>);

		return static_cast<Derived const *>(this);
	}

	void destroySelf() const noexcept
	{
		self()->destroySelf();
	}
};

////////////////////////////////////////////////////////////////////////////////

template <typename T>
inline constexpr bool is_ref_counted_v = ::std::derived_from<
	::std::remove_cv_t<T>,
	RefCounted<::std::remove_cv_t<T>>
>;

template <typename T>
struct is_ref_counted : ::std::bool_constant<is_ref_counted_v<T>> {};


namespace concepts {

template <typename T>
concept RefCounted = is_ref_counted_v<T>;

} // namespace concepts

} // namespace utils

#endif /* DDV_UTILS_REFER_REF_COUNTED_H_ */
