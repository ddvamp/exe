//
// lift_void.hpp
// ~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_TRAIT_LIFT_VOID_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_TRAIT_LIFT_VOID_HPP_INCLUDED_ 1

#include <exe/future/fun/result/unit.hpp>

#include <util/type_traits.hpp>

namespace exe::future::trait {

namespace detail {

template <typename T>
struct LiftVoidImpl : ::std::type_identity<T> {};

template <typename T>
requires (::std::is_void_v<T>)
struct LiftVoidImpl<T> : ::std::type_identity<::util::cv_like_t<T, Unit>> {};

} // namespace detail

template <typename T>
using LiftVoid = detail::LiftVoidImpl<T>::type;

} // namespace exe::future::trait

#endif /* DDVAMP_EXE_FUTURE_FUN_TRAIT_LIFT_VOID_HPP_INCLUDED_ */
