// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_UTILS_UTILITY_HPP_INCLUDED_
#define DDVAMP_UTILS_UTILITY_HPP_INCLUDED_ 1

#include <utility>

#include "type_traits.hpp"

namespace utils {

// To check bool values without laziness/unnecessary branches

template <typename ...Ts>
[[nodiscard]] inline constexpr bool all_of(Ts &&...ts)
    noexcept (is_all_of_v<::std::is_nothrow_constructible_v<bool, Ts>...>)
    requires (is_all_of_v<::std::is_constructible_v<bool, Ts>...>) {
    return (1 & ... & static_cast<int>(
        static_cast<bool>(::std::forward<Ts>(ts)))) != 0;
}

template <typename ...Ts>
[[nodiscard]] inline constexpr bool any_of(Ts &&...ts)
    noexcept (is_all_of_v<::std::is_nothrow_constructible_v<bool, Ts>...>)
    requires (is_all_of_v<::std::is_constructible_v<bool, Ts>...>) {
    return (0 | ... | static_cast<int>(
        static_cast<bool>(::std::forward<Ts>(ts)))) != 0;
}

template <typename ...Ts>
[[nodiscard]] inline constexpr bool none_of(Ts &&...ts)
    noexcept (is_all_of_v<::std::is_nothrow_constructible_v<bool, Ts>...>)
    requires (is_all_of_v<::std::is_constructible_v<bool, Ts>...>) {
    return (0 | ... | static_cast<int>(
        static_cast<bool>(::std::forward<Ts>(ts)))) == 0;
}

} // namespace utils

#endif /* DDVAMP_UTILS_UTILITY_HPP_INCLUDED_ */
