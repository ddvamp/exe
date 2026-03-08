//
// just.hpp
// ~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_MAKE_JUST_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_MAKE_JUST_HPP_INCLUDED_ 1

#include <exe/unit.hpp>
#include <exe/future2/make/ready.hpp>
#include <exe/future2/type/future.hpp>

namespace exe::future {

inline concepts::Future<Unit> auto Just() {
  return Ready(Unit{});
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_MAKE_JUST_HPP_INCLUDED_ */
