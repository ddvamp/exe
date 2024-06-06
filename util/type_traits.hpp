// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_UTIL_TYPE_TRAITS_HPP_INCLUDED_
#define DDVAMP_UTIL_TYPE_TRAITS_HPP_INCLUDED_ 1

#include <type_traits>

namespace util {

template <typename>
struct type {};

template <typename ...>
struct types {};

template <auto>
struct value {};

template <auto ...>
struct values {};


template <bool ...Vs>
inline constexpr bool is_all_of_v = ::std::is_same_v<values<true, Vs...>,
                                                     values<Vs..., true>>;

template <bool ...Vs>
struct is_all_of : ::std::bool_constant<is_all_of_v<Vs...>> {};

template <bool ...Vs>
inline constexpr bool is_none_of_v = ::std::is_same_v<values<false, Vs...>,
                                                      values<Vs..., false>>;

template <bool ...Vs>
struct is_none_of : ::std::bool_constant<is_none_of_v<Vs...>> {};

template <bool ...Vs>
inline constexpr bool is_any_of_v = !is_none_of_v<Vs...>;

template <bool ...Vs>
struct is_any_of : ::std::bool_constant<is_any_of_v<Vs...>> {};


template <typename ...Traits>
inline constexpr bool conjunction_v = is_all_of_v<Traits::value...>;

template <typename ...Traits>
struct conjunction : ::std::bool_constant<conjunction_v<Traits...>> {};

template <typename ...Traits>
inline constexpr bool disjunction_v = is_any_of_v<Traits::value...>;

template <typename ...Traits>
struct disjunction : ::std::bool_constant<disjunction_v<Traits...>> {};

template <typename Trait>
inline constexpr bool negation_v = !static_cast<bool>(Trait::value);

template <typename Trait>
struct negation : ::std::bool_constant<negation_v<Trait>> {};


template <typename ...>
inline constexpr bool is_all_same_v = true;

template <typename T, typename ...Ts>
inline constexpr bool is_all_same_v<T, Ts...> = ::std::is_same_v<
    types<T, Ts...>, types<Ts..., T>>;

template <typename ...Ts>
struct is_all_same : ::std::bool_constant<is_all_same_v<Ts...>> {};


namespace detail {

template <typename T, ::std::size_t>
struct proxy : type<T> {};

template <typename ...>
struct proxies;

template <typename ...Ts, ::std::size_t ...Is>
struct proxies<::std::index_sequence<Is...>, Ts...> : proxy<Ts, Is>... {};

template <typename ...Ts>
using proxies_t = proxies<::std::index_sequence_for<Ts...>, Ts...>;

}  // namespace detail


template <typename ...Ts>
inline constexpr bool is_all_unique_v = requires (detail::proxies_t<Ts...> *p,
                                                  void (*use)(...)) {
  use(static_cast<type<Ts> *>(p)...);
};

template <typename ...Ts>
struct is_all_unique : ::std::bool_constant<is_all_unique_v<Ts...>> {};

}  // namespace util

#endif  /* DDVAMP_UTIL_TYPE_TRAITS_HPP_INCLUDED_ */
