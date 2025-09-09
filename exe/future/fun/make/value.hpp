// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_FUTURE_FUN_MAKE_VALUE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_MAKE_VALUE_HPP_INCLUDED_ 1

#include <utility>

#include "exe/future/fun/make/contract/contract.hpp"

namespace exe::future {

template <typename T>
auto value(T v)
{
	auto [future, promise] = Contract<T>();

	::std::move(promise).setResult(::std::move(v));

	return ::std::move(future);
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_MAKE_VALUE_HPP_INCLUDED_ */
