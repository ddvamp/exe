//
// future_value.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_CONCEPT_FUTURE_VALUE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_CONCEPT_FUTURE_VALUE_HPP_INCLUDED_ 1

#include <exe/result/concept.hpp>

#include <util/type_traits.hpp>

namespace exe::future::concepts {

template <typename T>
concept FutureValue =
    ::std::is_object_v<T> &&
    !::util::is_qualified_v<T> &&
    ::std::is_nothrow_destructible_v<T> &&
    ::std::is_nothrow_move_constructible_v<T> && // implies !std::is_array_v<T>
    exe::concepts::ResultValue<T>;

} // namespace exe::future::concepts

#endif /* DDVAMP_EXE_FUTURE_FUN_CONCEPT_FUTURE_VALUE_HPP_INCLUDED_ */
