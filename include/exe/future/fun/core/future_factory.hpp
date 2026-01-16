//
// future_factory.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_CORE_FUTURE_FACTORY_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_CORE_FUTURE_FACTORY_HPP_INCLUDED_ 1

#include <exe/future/fun/core/future_factory_fwd.hpp>
#include <exe/future/fun/core/future_state.hpp>
#include <exe/future/fun/type/future.hpp>

namespace exe::future::core {

class FutureFactory {
 protected:
  template <typename T>
  static SemiFuture<T> MakeSemiFuture(FutureState<T> *state) noexcept {
    return SemiFuture<T>(state);
  }
};

} // namespace exe::future::core

#endif /* DDVAMP_EXE_FUTURE_FUN_CORE_FUTURE_FACTORY_HPP_INCLUDED_ */
