//
// just.hpp
// ~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_MAKE_JUST_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_MAKE_JUST_HPP_INCLUDED_ 1

#include <exe/future/fun/core/contract.hpp>
#include <exe/future/fun/result/unit.hpp>
#include <exe/future/fun/type/future_fwd.hpp>

#include <utility>

namespace exe::future {

inline SemiFuture<Unit> Just() {
  auto [f, p] = core::Contract<Unit>();

  ::std::move(p).SetValue(Unit{});

  return ::std::move(f);
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_MAKE_JUST_HPP_INCLUDED_ */
