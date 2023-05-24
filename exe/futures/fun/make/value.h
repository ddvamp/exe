// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_MAKE_VALUE_H_
#define DDV_EXE_FUTURES_FUN_MAKE_VALUE_H_ 1

#include <type_traits>
#include <utility>

#include "exe/futures/fun/make/contract.h"

namespace exe::futures {

template <typename V>
auto value(V &&v)
{
	using T = ::std::remove_cvref_t<V>;

	auto contract = Contract::open<T>();

	if constexpr (::std::is_nothrow_constructible_v<T, V>) {
		::std::move(contract).promise.setResult(::std::forward<V>(v));
	} else {
		auto rollback = ::utils::scope_guard(
			[&contract]() noexcept {
				Contract::close(::std::move(contract));
			}
		);
		::std::move(contract).promise.setResult(::std::forward<V>(v));
		rollback.disable();
	}

	return ::std::move(contract).future;
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_MAKE_VALUE_H_ */