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

template <typename ...>
struct UnifyInvokeTraits {
  using UnifyArgInvocable = ::std::false_type;
  using UnifyArgNothrowInvocable = ::std::false_type;

  using UnifyReturnInvocable = ::std::false_type;
  using UnifyReturnNothrowInvocable = ::std::false_type;

  using UnifyInvocable = ::std::false_type;
  using UnifyNothrowInvocable = ::std::false_type;
};

// Partial specialization for direct invocation
template <typename Fn, typename ...Args>
requires (::std::is_invocable_v<Fn, Args...>)
struct UnifyInvokeTraits<Fn, Args...> {
  using UnifyArgInvocable = ::std::true_type;
  using UnifyArgNothrowInvocable = ::std::is_nothrow_invocable<Fn, Args...>;
  using UnifyArgInvokeResult = ::std::invoke_result_t<Fn, Args...>;

  using UnifyReturnInvocable = UnifyArgInvocable;
  using UnifyReturnNothrowInvocable = UnifyArgNothrowInvocable;
  using UnifyReturnInvokeResult = exe::trait::UnifyVoid<UnifyArgInvokeResult>;

  using UnifyInvocable = UnifyReturnInvocable;
  using UnifyNothrowInvocable = UnifyReturnNothrowInvocable;
  using UnifyInvokeResult = UnifyReturnInvokeResult;

  using UnifyArg = ::std::false_type;
  using UnifyReturn = ::std::is_void<UnifyArgInvokeResult>;
};

// Partial specialization for argument unification
template <typename Fn, typename Arg>
requires (::std::is_same_v<Unit, ::std::decay_t<Arg>> &&
          !::std::is_invocable_v<Fn, Unit> && ::std::is_invocable_v<Fn>)
struct UnifyInvokeTraits<Fn, Arg> {
  using UnifyArgInvocable = ::std::true_type;
  using UnifyArgNothrowInvocable = ::std::is_nothrow_invocable<Fn>;
  using UnifyArgInvokeResult = ::std::invoke_result_t<Fn>;

  using UnifyReturnInvocable = ::std::false_type;
  using UnifyReturnNothrowInvocable = ::std::false_type;

  using UnifyInvocable = UnifyArgInvocable;
  using UnifyNothrowInvocable = UnifyArgNothrowInvocable;
  using UnifyInvokeResult = exe::trait::UnifyVoid<UnifyArgInvokeResult>;

  using UnifyArg = ::std::true_type;
  using UnifyReturn = ::std::is_void<UnifyArgInvokeResult>;
};

} // namespace detail

////////////////////////////////////////////////////////////////////////////////

template <typename Fn, typename ...Args>
inline constexpr bool UnifyArgInvocable
    = detail::UnifyInvokeTraits<Fn, Args...>::UnifyArgInvocable::value;

template <typename Fn, typename ...Args>
inline constexpr bool UnifyArgNothrowInvocable
    = detail::UnifyInvokeTraits<Fn, Args...>::UnifyArgNothrowInvocable::value;

template <typename Fn, typename ...Args>
using UnifyArgInvokeResult
    = detail::UnifyInvokeTraits<Fn, Args...>::UnifyArgInvokeResult;

template <typename Fn, typename ...Args>
inline constexpr bool UnifyReturnInvocable
    = detail::UnifyInvokeTraits<Fn, Args...>::UnifyReturnInvocable::value;

template <typename Fn, typename ...Args>
inline constexpr bool UnifyReturnNothrowInvocable
    = detail::UnifyInvokeTraits<Fn, Args...>::UnifyReturnNothrowInvocable::value;

template <typename Fn, typename ...Args>
using UnifyReturnInvokeResult
    = detail::UnifyInvokeTraits<Fn, Args...>::UnifyReturnInvokeResult;

template <typename Fn, typename ...Args>
inline constexpr bool UnifyInvocable
    = detail::UnifyInvokeTraits<Fn, Args...>::UnifyInvocable::value;

template <typename Fn, typename ...Args>
inline constexpr bool UnifyNothrowInvocable
    = detail::UnifyInvokeTraits<Fn, Args...>::UnifyNothrowInvocable::value;

template <typename Fn, typename ...Args>
using UnifyInvokeResult
    = detail::UnifyInvokeTraits<Fn, Args...>::UnifyInvokeResult;

template <typename Fn, typename ...Args>
inline constexpr bool UnifyArg
    = detail::UnifyInvokeTraits<Fn, Args...>::UnifyArg::value;

template <typename Fn, typename ...Args>
inline constexpr bool UnifyReturn
    = detail::UnifyInvokeTraits<Fn, Args...>::UnifyReturn::value;

} // namespace exe::future::core::trait

#endif /* DDVAMP_EXE_FUTURE_CORE_TRAIT_UNIFY_INVOKE_HPP_INCLUDED_ */
