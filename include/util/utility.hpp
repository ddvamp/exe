//
// utility.hpp
// ~~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_UTIL_UTILITY_HPP_INCLUDED_
#define DDVAMP_UTIL_UTILITY_HPP_INCLUDED_ 1

#include <util/concepts.hpp>
#include <util/macro.hpp>
#include <util/type_traits.hpp>

#include <utility> // IWYU pragma: export

namespace util {

/* Type placeholder, void replacement */
struct unit_t {};

inline constexpr unit_t unit{};

////////////////////////////////////////////////////////////////////////////////

/**
 *  @brief    Convert an rvalue to an lvalue.
 *  @param t  A thing of arbitrary type.
 *  @return   The parameter explicitly converted to an lvalue-reference.
 *
 *  This function can be used to convert an prvalue to an lvalue.
 *  In this case temporary materialization occurs.
 */
template <rvalue_deduced T>
[[nodiscard]] inline constexpr T &temporary(T &&t) noexcept {
  return static_cast<T &>(t);
}

////////////////////////////////////////////////////////////////////////////////

/* Simple utility for combining lambdas */
template <typename ...Ts>
struct overloaded : Ts... {
  using Ts::operator()...;
};

////////////////////////////////////////////////////////////////////////////////

/* For eager checking of bool values */

template <typename ...Ts>
[[nodiscard]] inline constexpr bool all_of(Ts &&...ts)
    noexcept (is_all_of_v<::std::is_nothrow_constructible_v<bool, Ts &&>...>)
    requires (is_all_of_v<::std::is_constructible_v<bool, Ts &&>...>) {
  return (1 & ... & static_cast<int>(
              static_cast<bool>(::std::forward<Ts>(ts)))) != 0;
}

template <typename ...Ts>
[[nodiscard]] inline constexpr bool any_of(Ts &&...ts)
    noexcept (is_all_of_v<::std::is_nothrow_constructible_v<bool, Ts &&>...>)
    requires (is_all_of_v<::std::is_constructible_v<bool, Ts &&>...>) {
  return (0 | ... | static_cast<int>(
              static_cast<bool>(::std::forward<Ts>(ts)))) != 0;
}

template <typename ...Ts>
[[nodiscard]] inline constexpr bool none_of(Ts &&...ts)
    noexcept (is_all_of_v<::std::is_nothrow_constructible_v<bool, Ts &&>...>)
    requires (is_all_of_v<::std::is_constructible_v<bool, Ts &&>...>) {
  return (0 | ... | static_cast<int>(
              static_cast<bool>(::std::forward<Ts>(ts)))) == 0;
}

////////////////////////////////////////////////////////////////////////////////

/**
 *  Similar to https://en.cppreference.com/w/cpp/memory/voidify.html
 *  but forcibly removes cv-qualification
 */

template <typename T>
[[nodiscard("Pure")]] inline constexpr void *voidify(T *obj) noexcept {
  return const_cast<void *>(static_cast<void const volatile *>(obj));
}

template <typename T>
[[nodiscard("Pure")]] inline constexpr void *voidify(T &obj) noexcept {
  return voidify(::std::addressof(obj));
}

template <typename T>
void *voidify(T const &&obj) = delete;

} // namespace util

#endif /* DDVAMP_UTIL_UTILITY_HPP_INCLUDED_ */
