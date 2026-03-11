//
// unify_invoke.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_CORE_UNIFY_INVOKE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_CORE_UNIFY_INVOKE_HPP_INCLUDED_ 1

#include <exe/unit.hpp>
#include <exe/future2/core/trait/unify_invoke.hpp>

#include <functional>
#include <utility>

namespace exe::future::core {

struct UnifyArgInvoker {
  template <typename Fn, typename ...Args>
  inline static constexpr trait::UnifyArgInvokeResult<Fn, Args...>
      operator() (Fn &&fn, Args &&...args)
      noexcept (trait::UnifyArgNothrowInvocable<Fn, Args...>) {
    if constexpr (trait::UnifyArg<Fn, Args...>) {
      return ::std::invoke(::std::forward<Fn>(fn));
    } else {
      return ::std::invoke(::std::forward<Fn>(fn),
                           ::std::forward<Args>(args)...);
    }
  }
};

inline constexpr UnifyArgInvoker UnifyArgInvoke{};

////////////////////////////////////////////////////////////////////////////////

struct UnifyReturnInvoker {
  template <typename Fn, typename ...Args>
  inline static constexpr trait::UnifyReturnInvokeResult<Fn, Args...>
      operator() (Fn &&fn, Args &&...args)
      noexcept (trait::UnifyReturnNothrowInvocable<Fn, Args...>) {
    if constexpr (trait::UnifyReturn<Fn, Args...>) {
      ::std::invoke(::std::forward<Fn>(fn), ::std::forward<Args>(args)...);
      return Unit{};
    } else {
      return ::std::invoke(::std::forward<Fn>(fn),
                           ::std::forward<Args>(args)...);
    }
  }
};

inline constexpr UnifyReturnInvoker UnifyReturnInvoke{};

////////////////////////////////////////////////////////////////////////////////

struct UnifyInvoker {
  template <typename Fn, typename ...Args>
  inline static constexpr trait::UnifyInvokeResult<Fn, Args...>
      operator() (Fn &&fn, Args &&...args)
      noexcept (trait::UnifyNothrowInvocable<Fn, Args...>) {
    if constexpr (trait::UnifyArg<Fn, Args...>) {
      return UnifyReturnInvoke(::std::forward<Fn>(fn));
    } else {
      return UnifyReturnInvoke(::std::forward<Fn>(fn),
                               ::std::forward<Args>(args)...);
    }
  }
};

inline constexpr UnifyInvoker UnifyInvoke{};

} // namespace exe::future::core

#endif /* DDVAMP_EXE_FUTURE_CORE_UNIFY_INVOKE_HPP_INCLUDED_ */
