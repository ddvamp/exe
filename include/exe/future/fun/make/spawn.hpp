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
#include <exe/future/fun/core/contract.hpp>
#include <exe/future/fun/core/maker.hpp>
#include <exe/future/fun/core/concept/maker.hpp>
#include <exe/future/fun/result/error.hpp>
#include <exe/future/fun/type/future.hpp>
#include <exe/future/fun/type/scheduler.hpp>
#include <exe/runtime/task/submit.hpp>

#include <type_traits>
#include <utility>

namespace exe::future {

namespace detail {

template <core::concepts::Maker Fn>
Future<::std::invoke_result_t<Fn &&>> SpawnImpl(Scheduler &where, Fn &&fn) {
  auto [f, p] = core::Contract<::std::invoke_result_t<Fn &&>>();

  auto task = [p = ::std::move(p), fn = ::std::move(fn)] mutable noexcept {
    try {
      ::std::move(p).SetValue(::std::move(fn)());
    } catch (...) {
      ::std::move(p).SetError(CurrentError());
    }
  };

  runtime::task::Submit(where, ::std::move(task));

  return ::std::move(f) | Via(where);
}

} // namespace detail

template <typename Fn>
auto Spawn(Scheduler &where, Fn fn) {
  return detail::SpawnImpl(where, core::Maker(::std::move(fn)));
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_MAKE_SPAWN_HPP_INCLUDED_ */
