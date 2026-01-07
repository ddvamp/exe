//
// value_of.hpp
// ~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_TRAIT_VALUE_OF_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_TRAIT_VALUE_OF_HPP_INCLUDED_ 1

#include <exe/future/fun/type/future_fwd.hpp>

#include <type_traits>

namespace exe::future {

namespace detail {

template <typename>
struct ValueOfImpl {};

template <typename T>
struct ValueOfImpl<SemiFuture<T>> : ::std::type_identity<T> {};

template <typename T>
struct ValueOfImpl<Future<T>> : ::std::type_identity<T> {};

} // namespace detail

template <typename T>
using ValueOf = detail::ValueOfImpl<T>::type;

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_TRAIT_VALUE_OF_HPP_INCLUDED_ */
