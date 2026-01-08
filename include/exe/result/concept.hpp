//
// concept.hpp
// ~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_RESULT_CONCEPT_HPP_INCLUDED_
#define DDVAMP_EXE_RESULT_CONCEPT_HPP_INCLUDED_ 1

#include <util/type_traits.hpp>

#include <expected>

namespace exe::concepts {

namespace detail {

template <typename>
inline constexpr bool IsUnexpectedImpl = false;

template <typename T>
inline constexpr bool IsUnexpectedImpl<::std::unexpected<T>> = true;

template <typename T>
concept ResultValueErrorCommon =
    !::std::is_reference_v<T> &&
    !::std::is_function_v<T> &&
    !::std::is_array_v<T> &&
    !IsUnexpectedImpl<T> &&
    ::std::is_destructible_v<T>; // implies !std::is_void_v<T>

} // namespace detail

template <typename T>
concept ResultValue =
    (detail::ResultValueErrorCommon<T> || ::std::is_void_v<T>) &&
    !::std::is_same_v<::std::remove_cv_t<T>, ::std::in_place_t> &&
    !::std::is_same_v<::std::remove_cv_t<T>, ::std::unexpect_t>;

template <typename T>
concept ResultError =
    detail::ResultValueErrorCommon<T> && !::util::is_qualified_v<T>;

} // namespace exe::concepts

#endif /* DDVAMP_EXE_RESULT_CONCEPT_HPP_INCLUDED_ */
