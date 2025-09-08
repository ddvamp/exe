// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_TRAITS_MAP_H_
#define DDV_EXE_FUTURES_FUN_TRAITS_MAP_H_ 1

#include <type_traits>

namespace exe::future::traits {

template <typename ...Ts>
struct InvokeTraitsImpl {
	using is_invocable			= ::std::is_invocable<Ts...>;
	using is_nothrow_invocable	= ::std::is_nothrow_invocable<Ts...>;
	using invoke_result			= ::std::invoke_result<Ts...>;
};

template <typename ...Ts>
struct InvokeTraits : InvokeTraitsImpl<Ts...> {};

template <typename T>
struct InvokeTraits<T, void> : InvokeTraitsImpl<T> {};

////////////////////////////////////////////////////////////////////////////////

template <typename Fn, typename ...Args>
inline constexpr bool is_invocable_v =
	InvokeTraits<Fn, Args...>::is_invocable::value;

template <typename Fn, typename ...Args>
inline constexpr bool is_nothrow_invocable_v =
	InvokeTraits<Fn, Args...>::is_nothrow_invocable::value;

template <typename Fn, typename ...Args>
using invoke_result_t = InvokeTraits<Fn, Args...>::invoke_result::type;


////////////////////////////////////////////////////////////////////////////////


template <typename T>
inline constexpr bool is_nothrow_move_constructible_v =
	::std::is_nothrow_move_constructible_v<T>;

template <>
inline constexpr bool is_nothrow_move_constructible_v<void> = true;


////////////////////////////////////////////////////////////////////////////////


template <typename Fn, typename ...Args>
using map_result_t = ::std::remove_cvref_t<invoke_result_t<Fn, Args...>>;

} // namespace exe::future::traits

#endif /* DDV_EXE_FUTURES_FUN_TRAITS_MAP_H_ */
