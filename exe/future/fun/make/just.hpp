// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_FUTURE_FUN_MAKE_JUST_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_MAKE_JUST_HPP_INCLUDED_ 1

#include <utility>

#include "exe/future/fun/make/contract/contract.hpp"

namespace exe::future {

inline auto just()
{
	auto [future, promise] = Contract<void>();

	::std::move(promise).setResult({});

	return ::std::move(future);
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_MAKE_JUST_HPP_INCLUDED_ */
