//
// unify_invoke.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_CORE_TRAIT_UNIFY_INVOKE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_CORE_TRAIT_UNIFY_INVOKE_HPP_INCLUDED_ 1

#include <exe/unify_void.hpp>
#include <exe/unit.hpp>

#include <type_traits>

namespace exe::future::core::trait {

namespace detail {

template <typename, typename ...>
struct UnifyReturnInvoke;

template <typename, typename ...>
struct UnifyArgInvoke;

template <typename, typename ...>
struct UnifyInvoke;

} // namespace detail

////////////////////////////////////////////////////////////////////////////////

template <typename Fn, typename ...Args>
inline constexpr bool UnifyArgInvocable
    = detail::UnifyArgInvoke<Fn, Args...>::Invocable::value;

template <typename Fn, typename ...Args>
inline constexpr bool UnifyArgNothrowInvocable
    = detail::UnifyArgInvoke<Fn, Args...>::NothrowInvocable::value;

template <typename Fn, typename ...Args>
using UnifyArgInvokeResult
    = detail::UnifyArgInvoke<Fn, Args...>::InvokeResult;

////////////////////////////////////////////////////////////////////////////////

template <typename Fn, typename ...Args>
inline constexpr bool UnifyReturnInvocable
    = detail::UnifyReturnInvoke<Fn, Args...>::Invocable::value;

template <typename Fn, typename ...Args>
inline constexpr bool UnifyReturnNothrowInvocable
    = detail::UnifyReturnInvoke<Fn, Args...>::NothrowInvocable::value;

template <typename Fn, typename ...Args>
using UnifyReturnInvokeResult
    = detail::UnifyReturnInvoke<Fn, Args...>::InvokeResult;

////////////////////////////////////////////////////////////////////////////////

template <typename Fn, typename ...Args>
inline constexpr bool UnifyInvocable
    = detail::UnifyInvoke<Fn, Args...>::Invocable::value;

template <typename Fn, typename ...Args>
inline constexpr bool UnifyNothrowInvocable
    = detail::UnifyInvoke<Fn, Args...>::NothrowInvocable::value;

template <typename Fn, typename ...Args>
using UnifyInvokeResult
    = detail::UnifyInvoke<Fn, Args...>::InvokeResult;

////////////////////////////////////////////////////////////////////////////////

template <typename Fn, typename ...Args>
inline constexpr bool UnifyArg
    = detail::UnifyArgInvoke<Fn, Args...>::Unify::value;

template <typename Fn, typename ...Args>
inline constexpr bool UnifyReturn
    = detail::UnifyReturnInvoke<Fn, Args...>::Unify::value;

////////////////////////////////////////////////////////////////////////////////

namespace detail {

template <typename, typename ...>
struct UnifyReturnInvoke {
  using Invocable = ::std::false_type;
  using NothrowInvocable = ::std::false_type;
};

template <typename Fn, typename ...Args>
requires (::std::is_invocable_v<Fn, Args...>)
struct UnifyReturnInvoke<Fn, Args...> {
  using R = ::std::invoke_result_t<Fn, Args...>;

  using Invocable = ::std::true_type;
  using NothrowInvocable = ::std::is_nothrow_invocable<Fn, Args...>;
  using InvokeResult = exe::trait::UnifyVoid<R>;
  using Unify = ::std::is_void<R>;
};

////////////////////////////////////////////////////////////////////////////////

template <typename, typename ...>
struct UnifyArgInvoke {
  using Invocable = ::std::false_type;
  using NothrowInvocable = ::std::false_type;
};

template <typename Fn, typename ...Args>
requires (::std::is_invocable_v<Fn, Args...>)
struct UnifyArgInvoke<Fn, Args...> {
  using Invocable = ::std::true_type;
  using NothrowInvocable = ::std::is_nothrow_invocable<Fn, Args...>;
  using InvokeResult = ::std::invoke_result_t<Fn, Args...>;
  using Unify = ::std::false_type;
};

template <typename Fn, typename Arg>
requires (::std::is_same_v<Unit, ::std::decay_t<Arg>> &&
          !::std::is_invocable_v<Fn, Unit> && ::std::is_invocable_v<Fn>)
struct UnifyArgInvoke<Fn, Arg> {
  using Invocable = ::std::true_type;
  using NothrowInvocable = ::std::is_nothrow_invocable<Fn>;
  using InvokeResult = ::std::invoke_result_t<Fn>;
  using Unify = ::std::true_type;
};

////////////////////////////////////////////////////////////////////////////////

template <typename, typename ...>
struct UnifyInvoke {
  using Invocable = ::std::false_type;
  using NothrowInvocable = ::std::false_type;
};

template <typename Fn, typename ...Args>
requires (UnifyArgInvoke<Fn, Args...>::Unify::value)
struct UnifyInvoke<Fn, Args...> : UnifyReturnInvoke<Fn> {};

template <typename Fn, typename ...Args>
requires (!UnifyArgInvoke<Fn, Args...>::Unify::value)
struct UnifyInvoke<Fn, Args...> : UnifyReturnInvoke<Fn, Args...> {};

} // namespace detail

} // namespace exe::future::core::trait

#endif /* DDVAMP_EXE_FUTURE_CORE_TRAIT_UNIFY_INVOKE_HPP_INCLUDED_ */
