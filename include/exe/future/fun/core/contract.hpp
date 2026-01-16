//
// contract.hpp
// ~~~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_CORE_CONTRACT_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_CORE_CONTRACT_HPP_INCLUDED_ 1

#include <exe/future/fun/core/contract_fwd.hpp>
#include <exe/future/fun/core/future_factory.hpp>
#include <exe/future/fun/detail/promise_state.hpp>
#include <exe/future/fun/type/future.hpp>
#include <exe/future/fun/type/promise.hpp>

namespace exe::future::core {

template <typename T>
struct [[nodiscard]] Contract : private FutureFactory {
  SemiFuture<T> future;
  Promise<T> promise;

  Contract() : Contract(::new detail::PromiseState<T>) {}

 private:
  Contract(detail::PromiseState<T> *state) noexcept
      : future(MakeSemiFuture(state))
      , promise(state) {}
};

} // namespace exe::future::core

#endif /* DDVAMP_EXE_FUTURE_FUN_CORE_CONTRACT_HPP_INCLUDED_ */
