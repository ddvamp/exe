// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_MAKE_JUST_H_
#define DDV_EXE_FUTURES_FUN_MAKE_JUST_H_ 1

#include <utility>

#include "exe/futures/fun/make/contract.h"

namespace exe::futures {

inline auto just()
{
	auto [future, promise] = Contract::open<void>();

	::std::move(promise).setResult({});

	return ::std::move(future);
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_MAKE_JUST_H_ */
