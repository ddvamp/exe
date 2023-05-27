// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_UTILS_TYPE_TRAITS_H_
#define DDV_UTILS_TYPE_TRAITS_H_ 1

#include <functional>
#include <type_traits>
#include <utility>

namespace utils {

template <::std::size_t I, typename Head, typename ...Tail>
struct pack_element_impl : pack_element_impl<I - 1, Tail...> {};

template <typename Head, typename ...Tail>
struct pack_element_impl<0, Head, Tail...> {
	using type = Head;
};

template <::std::size_t, typename ...>
struct pack_element {};

template <::std::size_t I, typename ...Ts>
	requires (I < sizeof...(Ts))
struct pack_element<I, Ts...> : pack_element_impl<I, Ts...> {};

template <::std::size_t I, typename ...Ts>
using pack_element_t = pack_element<I, Ts...>::type;


////////////////////////////////////////////////////////////////////////////////


template <bool ...>
struct all_helper;

template <bool ...Vs>
inline constexpr bool all_true_v =
	::std::is_same_v<all_helper<true, Vs...>, all_helper<Vs..., true>>;

template <bool ...Vs>
struct all_true : ::std::bool_constant<all_true_v<Vs...>> {};


////////////////////////////////////////////////////////////////////////////////


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

template <typename T, template <typename ...> typename ...Templates>
inline constexpr bool is_specialization_any_of_v =
	!all_true_v<!is_specialization_v<T, Templates>...>;

template <typename T, template <typename ...> typename ...Templates>
struct is_specialization_any_of
	: ::std::bool_constant<is_specialization_any_of_v<T, Templates...>> {};


////////////////////////////////////////////////////////////////////////////////


template <typename T, typename ...Ts>
inline constexpr bool is_any_of_v = !all_true_v<!::std::is_same_v<T, Ts>...>;

template <typename T, typename ...Ts>
struct is_any_of : ::std::bool_constant<is_any_of_v<T, Ts...>> {};


////////////////////////////////////////////////////////////////////////////////


template <typename ...>
struct are_all_same_helper;

template <typename ...Ts>
inline constexpr bool are_all_same_v =
	::std::is_same_v<
		are_all_same_helper<pack_element_t<0, Ts...>, Ts...>,
		are_all_same_helper<Ts..., pack_element_t<0, Ts...>>
	>;

template <>
inline constexpr bool are_all_same_v<> = true;

template <typename ...Ts>
struct are_all_same : ::std::bool_constant<are_all_same_v<Ts...>> {};


////////////////////////////////////////////////////////////////////////////////


// determine whether T can be direct-initialized with
// result of applying INVOKE operation to F and Args...
template <typename T, typename Fn, typename ...Args>
inline constexpr bool is_constructible_with_v = requires {
	T(::std::invoke(::std::declval<Fn>(), ::std::declval<Args>()...));
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
	{T(::std::invoke(::std::declval<Fn>(), ::std::declval<Args>()...))} noexcept;
};

template <typename T, typename Fn, typename ...Args>
struct is_nothrow_constructible_with
	: ::std::bool_constant<is_nothrow_constructible_with_v<T, Fn, Args...>>
{};

} // namespace utils

#endif /* DDV_UTILS_TYPE_TRAITS_H_ */
