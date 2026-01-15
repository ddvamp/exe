//
// adapt_call.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_CORE_ADAPT_CALL_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_CORE_ADAPT_CALL_HPP_INCLUDED_ 1

#include <exe/future/fun/result/unit.hpp>

#include <functional>
#include <type_traits>
#include <utility>

namespace exe::future::core {

template <typename Fn, typename ...Args>
inline constexpr decltype(auto) AdaptInput(Fn &&fn, Args &&...args)
    noexcept (::std::is_nothrow_invocable_v<Fn, Args...>)
    requires (::std::is_invocable_v<Fn, Args...>) {
  return ::std::invoke(::std::forward<Fn>(fn), ::std::forward<Args>(args)...);
}

template <typename Fn>
inline constexpr decltype(auto) AdaptInput(Fn &&fn, Unit)
    noexcept (::std::is_nothrow_invocable_v<Fn>)
    requires (!::std::is_invocable_v<Fn, Unit> && ::std::is_invocable_v<Fn>) {
  return ::std::invoke(::std::forward<Fn>(fn));
}

////////////////////////////////////////////////////////////////////////////////

template <typename Fn, typename ...Args>
inline constexpr decltype(auto) AdaptOutput(Fn &&fn, Args &&...args)
    noexcept (::std::is_nothrow_invocable_v<Fn, Args...>)
    requires (::std::is_invocable_v<Fn, Args...>) {
  if constexpr (::std::is_void_v<::std::invoke_result_t<Fn, Args...>>) {
    ::std::invoke(::std::forward<Fn>(fn), ::std::forward<Args>(args)...);
    return Unit{};
  } else {
    return ::std::invoke(::std::forward<Fn>(fn), ::std::forward<Args>(args)...);
  }
}

////////////////////////////////////////////////////////////////////////////////

template <typename Fn, typename ...Args>
inline constexpr decltype(auto) AdaptCall(Fn &&fn, Args &&...args)
    noexcept (::std::is_nothrow_invocable_v<Fn, Args...>)
    requires (::std::is_invocable_v<Fn, Args...>) {
  return core::AdaptOutput(::std::forward<Fn>(fn),
                           ::std::forward<Args>(args)...);
}

template <typename Fn>
inline constexpr decltype(auto) AdaptCall(Fn &&fn, Unit)
    noexcept (::std::is_nothrow_invocable_v<Fn>)
    requires (!::std::is_invocable_v<Fn, Unit> && ::std::is_invocable_v<Fn>) {
  return core::AdaptOutput(::std::forward<Fn>(fn));
}

} // namespace exe::future::core

#endif /* DDVAMP_EXE_FUTURE_FUN_CORE_ADAPT_CALL_HPP_INCLUDED_ */
