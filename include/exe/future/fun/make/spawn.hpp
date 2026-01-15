//
// spawn.hpp
// ~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_MAKE_SPAWN_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_MAKE_SPAWN_HPP_INCLUDED_ 1

#include <exe/future/fun/combine/seq/via.hpp>
#include <exe/future/fun/concept/future_value.hpp>
#include <exe/future/fun/core/adapt_call.hpp>
#include <exe/future/fun/core/contract.hpp>
#include <exe/future/fun/core/concept/adapt_call.hpp>
#include <exe/future/fun/core/trait/adapt_call.hpp>
#include <exe/future/fun/result/error.hpp>
#include <exe/future/fun/type/future_fwd.hpp>
#include <exe/future/fun/type/scheduler.hpp>
#include <exe/runtime/task/submit.hpp>

#include <type_traits>
#include <utility>

namespace exe::future {

template <typename Fn>
requires (core::concepts::AdaptOutputInvocable<::std::decay_t<Fn>> &&
          concepts::FutureValue<
              core::trait::AdaptOutputResult<::std::decay_t<Fn>>>)
Future<core::trait::AdaptOutputResult<::std::decay_t<Fn>>>
Spawn(Scheduler &where, Fn &&fn) {
  using U = core::trait::AdaptOutputResult<::std::decay_t<Fn>>;

  auto [f, p] = core::Contract<U>();

  auto task = [p = ::std::move(p), fn = ::std::forward<Fn>(fn)]
              mutable noexcept {
    try {
      ::std::move(p).SetValue(core::AdaptOutput(::std::move(fn)));
    } catch (...) {
      ::std::move(p).SetError(CurrentError());
    }
  };

  runtime::task::Submit(where, ::std::move(task));

  return ::std::move(f) | Via(where);
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_MAKE_SPAWN_HPP_INCLUDED_ */
