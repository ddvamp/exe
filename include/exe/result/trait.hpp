//
// trait.hpp
// ~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_RESULT_TRAIT_HPP_INCLUDED_
#define DDVAMP_EXE_RESULT_TRAIT_HPP_INCLUDED_ 1

#include <expected>
#include <type_traits>

namespace exe::result {

namespace detail {

template <typename T>
inline constexpr bool is_unexpected_v = false;

template <typename T>
inline constexpr bool is_unexpected_v<::std::unexpected<T>> = true;

template <typename T>
concept SutableForResult =
    !::std::is_reference_v<T> &&
    !::std::is_function_v<T> &&
    !::std::is_array_v<T> &&
    !detail::is_unexpected_v<T> &&
    ::std::is_destructible_v<T>; // implies !std::is_void_v<T>

} // namespace detail

template <typename T>
concept SuitableValue =
    (detail::SutableForResult<T> || ::std::is_void_v<T>) &&
    !::std::is_same_v<::std::remove_cv_t<T>, ::std::in_place_t> &&
    !::std::is_same_v<::std::remove_cv_t<T>, ::std::unexpect_t>;

template <typename T>
concept SuitableError =
    detail::SutableForResult<T> &&
    !::std::is_const_v<T> &&
    !::std::is_volatile_v<T>;

} // namespace exe::result

#endif /* DDVAMP_EXE_RESULT_TRAIT_HPP_INCLUDED_ */
