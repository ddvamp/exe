// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_MAKE_VALUE_H_
#define DDV_EXE_FUTURES_FUN_MAKE_VALUE_H_ 1

#include <type_traits>
#include <utility>

#include "exe/futures/fun/make/contract/contract.h"

#include "utils/defer.h"

namespace exe::futures {

template <typename T>
auto value(T v)
{
	auto contract = Contract<T>();

	if constexpr (::std::is_nothrow_move_constructible_v<T>) {
		::std::move(contract).p.setResult(::std::move(v));
	} else {
		auto rollback = ::utils::scope_guard(
			[&]() noexcept {
				::std::move(contract).cancel();
			}
		);
		::std::move(contract).p.setResult(::std::move(v));
		rollback.disable();
	}

	return ::std::move(contract).f;
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_MAKE_VALUE_H_ */
