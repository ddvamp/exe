// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_UTILS_TYPE_TRAITS_HPP_INCLUDED_
#define DDVAMP_UTILS_TYPE_TRAITS_HPP_INCLUDED_ 1

#include <type_traits>

namespace utils {

namespace detail {

template <bool ...>
struct all_helper;

} // namespace detail

template <bool ...Vs>
inline constexpr bool is_all_of_v = ::std::is_same_v<
    detail::all_helper<true, Vs...>, detail::all_helper<Vs..., true>>;

template <bool ...Vs>
struct is_all_of : ::std::bool_constant<is_all_of_v<Vs...>> {};

template <bool ...Vs>
inline constexpr bool is_none_of_v = ::std::is_same_v<
    detail::all_helper<false, Vs...>, detail::all_helper<Vs..., false>>;

template <bool ...Vs>
struct is_none_of : ::std::bool_constant<is_none_of_v<Vs...>> {};

template <bool ...Vs>
inline constexpr bool is_any_of_v = !is_none_of_v<Vs...>;

template <bool ...Vs>
struct is_any_of : ::std::bool_constant<is_any_of_v<Vs...>> {};

} // namespace utils

#endif /* DDVAMP_UTILS_TYPE_TRAITS_HPP_INCLUDED_ */
