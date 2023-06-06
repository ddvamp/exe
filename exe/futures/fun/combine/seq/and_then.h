// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_COMBINE_SEQ_AND_THEN_H_
#define DDV_EXE_FUTURES_FUN_COMBINE_SEQ_AND_THEN_H_ 1

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
struct [[nodiscard]] AndThen : detail::Mutator {
	[[no_unique_address]] Fn fn;

	template <concepts::Future F>
	auto mutate(F &&f)
		requires (
			has_executor_v<F> &&
			traits::is_invocable_v<Fn &, typename F::value_type> &&
			::utils::is_result_v<
				traits::map_result_t<Fn &, typename F::value_type>
			>
		)
	{
		using T = F::value_type;
		using U = traits::map_result_t<Fn &, T>::value_type;

		auto contract = Contract<U>();

		auto &where = getExecutor(f);

		setCallback(
			::std::move(f),
			futures::makeCallback<T>(
				[](auto &res, auto &fn, auto &p) noexcept {
					::std::move(p).setResult(
						::utils::map_safely([&]() noexcept (
							traits::is_nothrow_invocable_v<Fn &, T>
						) {
							return ::std::move(res).and_then(fn);
						})
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
auto andThen(Fn fn)
	requires (::std::is_nothrow_destructible_v<Fn>)
{
	return pipe::AndThen{{}, ::std::move(fn)};
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_COMBINE_SEQ_AND_THEN_H_ */
