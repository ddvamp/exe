//
// future_fwd.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_TYPE_FUTURE_FWD_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_TYPE_FUTURE_FWD_HPP_INCLUDED_ 1

#include <exe/future/fun/concept/future_value.hpp>

namespace exe::future {

template <concepts::FutureValue T>
class SemiFuture;

template <concepts::FutureValue T>
class Future;

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_TYPE_FUTURE_FWD_HPP_INCLUDED_ */
