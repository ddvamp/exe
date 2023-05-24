// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_TRAITS_INVOKE_H_
#define DDV_EXE_FUTURES_FUN_TRAITS_INVOKE_H_ 1

#include <type_traits>

namespace exe::futures::traits {

template <typename F, typename T>
inline constexpr bool is_invocable_v =
	::std::is_void_v<T> ?
	::std::is_invocable_v<F> :
	::std::is_invocable_v<F, T>;

template <typename F, typename T>
inline constexpr bool is_nothrow_invocable_v =
	::std::is_void_v<T> ?
	::std::is_nothrow_invocable_v<F> :
	::std::is_nothrow_invocable_v<F, T>;

template <typename F, typename T>
using invoke_result_t =
	::std::conditional_t<
		::std::is_void_v<T>,
		::std::invoke_result_t<F>,
		::std::invoke_result_t<F, T>
	>;

} // namespace exe::futures::traits

#endif /* DDV_EXE_FUTURES_FUN_TRAITS_INVOKE_H_ */
