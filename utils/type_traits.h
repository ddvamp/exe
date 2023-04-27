// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_UTILS_TYPE_TRAITS_H_
#define DDV_UTILS_TYPE_TRAITS_H_ 1

#include <functional>
#include <type_traits>
#include <utility>

namespace utils {

// true if and only if T is a specialization of Template
template <typename T, template <typename ...> typename Template>
inline constexpr bool is_specialization_v = false;

template <template <typename ...> typename Template, typename ...Ts>
inline constexpr bool is_specialization_v<Template<Ts...>, Template> = true;

template <typename T, template <typename ...> typename Template>
struct is_specialization
	: ::std::bool_constant<is_specialization_v<T, Template>>
{};

////////////////////////////////////////////////////////////////////////////////

template <typename T, typename ...Ts>
inline constexpr bool is_any_of_v = (::std::is_same_v<T, Ts> || ...);

template <typename T, typename ...Ts>
struct is_any_of : ::std::bool_constant<is_any_of_v<T, Ts...>> {};

////////////////////////////////////////////////////////////////////////////////

namespace detail {

template <typename ...>
struct head_type_or_void {
	using type = void;
};

template <typename Head, typename ...Tail>
struct head_type_or_void<Head, Tail...> {
	using type = Head;
};

template <typename ...Ts>
using head_type_or_void_t = typename head_type_or_void<Ts...>::type;

} // namespace detail

template <typename ...Ts>
inline constexpr bool are_all_same_v =
	::std::is_same_v<
		::std::common_type<detail::head_type_or_void<Ts...>, Ts...>,
		::std::common_type<Ts..., detail::head_type_or_void<Ts...>>
	>;

template <typename ...Ts>
struct are_all_same : ::std::bool_constant<are_all_same_v<Ts...>> {};

////////////////////////////////////////////////////////////////////////////////

// determine whether T can be direct-initialized with
// result of applying INVOKE operation to F and Args...
template <typename T, typename Fn, typename ...Args>
inline constexpr bool is_constructible_with_v = requires {
	T(::std::invoke(::std::declval<F>(), ::std::declval<Args>()...));
};

template <typename T, typename Fn, typename ...Args>
struct is_constructible_with
	: ::std::bool_constant<is_constructible_with_v<T, Fn, Args...>>
{};

////////////////////////////////////////////////////////////////////////////////

// determine whether direct-initialization of T from
// result of applying INVOKE operation to F and Args...
// is both valid and not potentially-throwing
template <typename T, typename Fn, typename ...Args>
inline constexpr bool is_nothrow_constructible_with_v = requires {
	{T(::std::invoke(::std::declval<F>(), ::std::declval<Args>()...))} noexcept;
};

template <typename T, typename Fn, typename ...Args>
struct is_nothrow_constructible_with
	: ::std::bool_constant<is_nothrow_constructible_with_v<T, Fn, Args...>>
{};

} // namespace utils

#endif /* DDV_UTILS_TYPE_TRAITS_H_ */
