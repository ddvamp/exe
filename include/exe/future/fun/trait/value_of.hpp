//
// value_of.hpp
// ~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_TRAIT_VALUE_OF_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_TRAIT_VALUE_OF_HPP_INCLUDED_ 1

#include <exe/future/fun/concept/future.hpp>

namespace exe::future::trait {

template <concepts::Future F>
using ValueOf = F::ValueType;

} // namespace exe::future::trait

#endif /* DDVAMP_EXE_FUTURE_FUN_TRAIT_VALUE_OF_HPP_INCLUDED_ */
