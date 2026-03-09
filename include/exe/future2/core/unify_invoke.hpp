//
// adapt_call.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_CORE_ADAPT_CALL_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_CORE_ADAPT_CALL_HPP_INCLUDED_ 1

#include <exe/unit.hpp>
#include <exe/future2/core/trait/unify_invoke.hpp>

#include <functional>
#include <utility>

namespace exe::future::core {

template <typename Fn, typename ...Args>
inline constexpr trait::UnifyArgInvokeResult<Fn, Args...> UnifyArgInvoke(
    Fn &&fn, Args &&...args)
    noexcept (trait::UnifyArgNothrowInvocable<Fn, Args...>) {
  if constexpr (trait::UnifyArg<Fn, Args...>) {
    return ::std::invoke(::std::forward<Fn>(fn));
  } else {
    return ::std::invoke(::std::forward<Fn>(fn), ::std::forward<Args>(args)...);
  }
}

////////////////////////////////////////////////////////////////////////////////

template <typename Fn, typename ...Args>
inline constexpr trait::UnifyReturnInvokeResult<Fn, Args...> UnifyReturnInvoke(
    Fn &&fn, Args &&...args)
    noexcept (trait::UnifyReturnNothrowInvocable<Fn, Args...>) {
  if constexpr (trait::UnifyReturn<Fn, Args...>) {
    ::std::invoke(::std::forward<Fn>(fn), ::std::forward<Args>(args)...);
    return Unit{};
  } else {
    return ::std::invoke(::std::forward<Fn>(fn), ::std::forward<Args>(args)...);
  }
}

////////////////////////////////////////////////////////////////////////////////

template <typename Fn, typename ...Args>
inline constexpr trait::UnifyInvokeResult<Fn, Args...> UnifyInvoke(
    Fn &&fn, Args &&...args)
    noexcept (trait::UnifyNothrowInvocable<Fn, Args...>) {
  if constexpr (trait::UnifyArg<Fn, Args...>) {
    return core::UnifyReturnInvoke(::std::forward<Fn>(fn));
  } else {
    return core::UnifyReturnInvoke(::std::forward<Fn>(fn),
                                   ::std::forward<Args>(args)...);
  }
}

} // namespace exe::future::core

#endif /* DDVAMP_EXE_FUTURE_CORE_ADAPT_CALL_HPP_INCLUDED_ */
