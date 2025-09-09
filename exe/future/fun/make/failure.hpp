// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_FUTURE_FUN_MAKE_FAILURE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_MAKE_FAILURE_HPP_INCLUDED_ 1

#include <utility>

#include "exe/future/fun/make/contract/contract.hpp"

#include "exe/future/fun/result/result.hpp"

namespace exe::future {

template <typename T>
auto failure(::util::error error)
{
	auto [future, promise] = Contract<T>();

	::std::move(promise).setError(::std::move(error));

	return ::std::move(future);
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_MAKE_FAILURE_HPP_INCLUDED_ */
