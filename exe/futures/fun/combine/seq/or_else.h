// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_COMBINE_SEQ_OR_ELSE_H_
#define DDV_EXE_FUTURES_FUN_COMBINE_SEQ_OR_ELSE_H_ 1

#include <exception>
#include <type_traits>
#include <utility>

#include "exe/futures/fun/make/contract.h"
#include "exe/futures/fun/mutator/mutator.h"
#include "exe/futures/fun/syntax/pipe.h"

#include "result/result.h"

namespace exe::futures {

namespace pipe {

// TODO: harmful exceptions
template <typename F>
struct [[nodiscard]] OrElse : Mutator {
	[[no_unique_address]] F fun;

	using map_result_t =
		::std::remove_cvref_t<::std::invoke_result_t<F &, ::utils::error>>;

	template <typename T>
	Future<T> mutate(Future<T> f)
		requires (::std::is_same_v<map_result_t, ::utils::result<T>>)
	{
		// loss future at exception
		auto [future, promise] = Contract::open<T>();

		// loss future at exception
		setCallback<T>(
			::std::move(f),
			[fn = ::std::move(fun), p = ::std::move(promise)]
			(::utils::result<T> &&res) mutable noexcept {
				if constexpr (
					::std::is_nothrow_invocable_v<F &, ::utils::error> &&
					::std::is_nothrow_move_constructible_v<T>
				) {
					::std::move(p).setResult(::std::move(res).or_else(fn));
				} else try {
					::std::move(p).setResult(::std::move(res).or_else(fn));
				} catch (...) {
					::std::move(p).setError(::std::current_exception());
				}
			}
		);

		return future;
	}
};

} // namespace pipe

template <typename F>
auto orElse(F fun) noexcept
	requires (
		::std::is_nothrow_destructible_v<F> &&
		::std::is_invocable_v<F &, ::utils::error>
	)
{
	return pipe::OrElse{::std::move(fun)};
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_COMBINE_SEQ_OR_ELSE_H_ */
