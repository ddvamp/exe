// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_COMBINE_SEQ_MAP_H_
#define DDV_EXE_FUTURES_FUN_COMBINE_SEQ_MAP_H_ 1

#include <type_traits>
#include <utility>

#include "exe/futures/fun/combine/seq/via.h"
#include "exe/futures/fun/make/contract/contract.h"
#include "exe/futures/fun/mutator/mutator.h"
#include "exe/futures/fun/syntax/pipe.h"
#include "exe/futures/fun/traits/map.h"

#include "result/result.h"

namespace exe::futures {

namespace pipe {

template <typename Fn>
struct [[nodiscard]] Map : detail::Mutator {
	[[no_unique_address]] Fn fn;

	template <concepts::Future F>
	auto mutate(F &&f)
		requires (
			hasExecutor<F> &&
			traits::is_invocable_v<Fn &, typename F::value_type>
		)
	{
		using T = F::value_type;
		using U = traits::map_result_t<Fn &, T>;

		auto contract = Contract<U>();

		auto &where = getExecutor(f);

		setCallback(
			::std::move(f),
			futures::makeCallback<T>(
				[](auto &res, auto &fn, auto &p) noexcept {
					::std::move(p).setResult(
						::utils::map_safely(
							[&]() noexcept (
								traits::is_nothrow_invocable_v<Fn &, T>
							) {
								return ::std::move(res).transform(fn);
							}
						)
					);
				},
				::std::move(fn),
				::std::move(contract).p
			)
		);

		return ::std::move(contract).f | futures::via(where);
	}
};

} // namespace pipe

template <typename Fn>
auto map(Fn fn) noexcept
	requires (::std::is_nothrow_destructible_v<Fn>)
{
	return pipe::Map{{}, ::std::move(fn)};
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_COMBINE_SEQ_MAP_H_ */
