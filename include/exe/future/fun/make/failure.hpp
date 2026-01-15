//
// failure.hpp
// ~~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_MAKE_FAILURE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_MAKE_FAILURE_HPP_INCLUDED_ 1

#include <exe/future/fun/concept/future_value.hpp>
#include <exe/future/fun/core/contract.hpp>
#include <exe/future/fun/result/error.hpp>
#include <exe/future/fun/type/future_fwd.hpp>

#include <utility>

namespace exe::future {

template <concepts::FutureValue T>
inline SemiFuture<T> Failure(Error &&e) {
  auto [f, p] = core::Contract<T>();

  ::std::move(p).SetError(::std::move(e));

  return ::std::move(f);
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_MAKE_FAILURE_HPP_INCLUDED_ */
