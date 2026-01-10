//
// contract.hpp
// ~~~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_MAKE_CONTRACT_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_MAKE_CONTRACT_HPP_INCLUDED_ 1

#include <exe/future/fun/detail/shared_state.hpp>
#include <exe/future/fun/make/contract_fwd.hpp>
#include <exe/future/fun/type/future.hpp>
#include <exe/future/fun/type/promise.hpp>

namespace exe::future {

template <typename T>
struct [[nodiscard]] Contract {
  SemiFuture<T> future;
  Promise<T> promise;

  Contract() : Contract(detail::SharedState<T>::Create()) {}

 private:
  Contract(detail::SharedState<T> *state) noexcept
      : future(state)
      , promise(state) {}
};

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_MAKE_CONTRACT_HPP_INCLUDED_ */
