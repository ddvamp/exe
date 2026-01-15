//
// adapt_call.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_CORE_CONCEPT_ADAPT_CALL_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_CORE_CONCEPT_ADAPT_CALL_HPP_INCLUDED_ 1

#include <exe/future/fun/core/adapt_call.hpp>

namespace exe::future::core::concepts {

template <typename Fn, typename ...Args>
concept AdaptInputInvocable = requires {
  { core::AdaptInput(::std::declval<Fn>(), ::std::declval<Args>()...) };
};

template <typename Fn, typename ...Args>
concept NothrowAdaptInputInvocable = requires {
  { core::AdaptInput(::std::declval<Fn>(),
                     ::std::declval<Args>()...) } noexcept;
};

////////////////////////////////////////////////////////////////////////////////

template <typename Fn, typename ...Args>
concept AdaptOutputInvocable = requires {
  { core::AdaptOutput(::std::declval<Fn>(), ::std::declval<Args>()...) };
};

template <typename Fn, typename ...Args>
concept NothrowAdaptOutputInvocable = requires {
  { core::AdaptOutput(::std::declval<Fn>(),
                      ::std::declval<Args>()...) } noexcept;
};

////////////////////////////////////////////////////////////////////////////////

template <typename Fn, typename ...Args>
concept AdaptCallInvocable = requires {
  { core::AdaptCall(::std::declval<Fn>(), ::std::declval<Args>()...) };
};

template <typename Fn, typename ...Args>
concept NothrowAdaptCallInvocable = requires {
  { core::AdaptCall(::std::declval<Fn>(), ::std::declval<Args>()...) } noexcept;
};

} // namespace exe::future::core::concepts

#endif /* DDVAMP_EXE_FUTURE_FUN_CORE_CONCEPT_ADAPT_CALL_HPP_INCLUDED_ */
