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

#include <exe/future/fun/make/contract.hpp>
#include <exe/future/fun/type/future_fwd.hpp>
#include <util/utility.hpp>

namespace exe::future {

inline SemiFuture<::util::unit_t> Just() {
  auto [f, p] = Contract<::util::unit_t>();

  ::std::move(p).SetValue({});

  return ::std::move(f);
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_MAKE_JUST_HPP_INCLUDED_ */
