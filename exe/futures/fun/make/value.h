// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_MAKE_VALUE_H_
#define DDV_EXE_FUTURES_FUN_MAKE_VALUE_H_ 1

#include <utility>

#include "exe/futures/fun/make/contract/contract.h"

namespace exe::futures {

template <typename T>
auto value(T v)
{
	auto [future, promise] = Contract<T>();

	::std::move(promise).setResult(::std::move(v));

	return ::std::move(future);
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_MAKE_VALUE_H_ */
