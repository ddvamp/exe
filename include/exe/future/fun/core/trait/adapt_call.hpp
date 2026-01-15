//
// adapt_call.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_CORE_TRAIT_ADAPT_CALL_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_CORE_TRAIT_ADAPT_CALL_HPP_INCLUDED_ 1

#include <exe/future/fun/core/adapt_call.hpp>

#include <utility>

namespace exe::future::core::trait {

template <typename Fn, typename ...Args>
using AdaptInputResult = decltype(core::AdaptInput(::std::declval<Fn>(),
                                                   ::std::declval<Args>()...));

////////////////////////////////////////////////////////////////////////////////

template <typename Fn, typename ...Args>
using AdaptOutputResult = decltype(core::AdaptOutput(::std::declval<Fn>(),
                                       ::std::declval<Args>()...));

////////////////////////////////////////////////////////////////////////////////

template <typename Fn, typename ...Args>
using AdaptCallResult = decltype(core::AdaptCall(::std::declval<Fn>(),
                                                 ::std::declval<Args>()...));

} // namespace exe::future::core::trait

#endif /* DDVAMP_EXE_FUTURE_FUN_CORE_TRAIT_ADAPT_CALL_HPP_INCLUDED_ */
