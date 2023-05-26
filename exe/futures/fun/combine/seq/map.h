// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_COMBINE_SEQ_MAP_H_
#define DDV_EXE_FUTURES_FUN_COMBINE_SEQ_MAP_H_ 1

#include <exception>
#include <type_traits>
#include <utility>

#include "exe/futures/fun/combine/seq/via.h"
#include "exe/futures/fun/make/contract.h"
#include "exe/futures/fun/mutator/mutator.h"
#include "exe/futures/fun/syntax/pipe.h"
#include "exe/futures/fun/traits/map.h"

#include "result/result.h"

namespace exe::futures {

namespace pipe {

// TODO: harmful exceptions
template <typename F>
struct [[nodiscard]] Map : detail::Mutator {
	[[no_unique_address]] F fun;

	template <typename T>
	auto mutate(Future<T> f)
		requires (traits::is_invocable_v<F &, T>)
	{
		using U = traits::map_result_t<F &, T>;

		// loss future at exception
		auto [future, promise] = Contract<U>();

		auto &where = getExecutor(f);

		// loss future at exception
		setCallback(
			::std::move(f),
			[fn = ::std::move(fun), p = ::std::move(promise)]
			(::utils::result<T> &&res) mutable noexcept {
				if constexpr (
					traits::is_nothrow_invocable_v<F &, T> &&
					traits::is_nothrow_move_constructible_v<U>
				) {
					::std::move(p).setResult(::std::move(res).transform(fn));
				} else try {
					::std::move(p).setResult(::std::move(res).transform(fn));
				} catch (...) {
					::std::move(p).setError(::std::current_exception());
				}
			}
		);

		return ::std::move(future) | futures::via(where);
	}
};

} // namespace pipe

template <typename F>
auto map(F fun) noexcept
	requires (::std::is_nothrow_destructible_v<F>)
{
	return pipe::Map{{}, ::std::move(fun)};
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_COMBINE_SEQ_MAP_H_ */
