// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLAT_MAP_H_
#define DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLAT_MAP_H_ 1

#include <type_traits>

#include "exe/futures/fun/combine/seq/via.h"
#include "exe/futures/fun/make/contract/contract.h"
#include "exe/futures/fun/mutator/mutator.h"
#include "exe/futures/fun/syntax/pipe.h"
#include "exe/futures/fun/traits/map.h"

#include "result/result.h"

#include "utils/utility.h"

namespace exe::futures {

namespace pipe {

template <typename Fn>
struct [[nodiscard]] FlatMap : detail::Mutator {
	[[no_unique_address]] Fn fn;

	template <concepts::Future F>
	auto mutate(F &&f)
		requires (
			has_executor_v<F> &&
			traits::is_invocable_v<Fn &, typename F::value_type> &&
			concepts::Future<
				traits::invoke_result_t<Fn &, typename F::value_type>
			>
		)
	{
		using T = F::value_type;
		using U = traits::invoke_result_t<Fn &, T>::value_type;

		auto contract = Contract<U>();

		auto &where = getExecutor(f);

		setCallback(
			::std::move(f),
			futures::makeCallback<T>(
				::utils::types_list<::utils::deduce_type_t, Fn, Callback<U>>,

				[](auto &res, auto &fn, auto &cb) noexcept {
					auto map_res = ::utils::map_safely([&]() noexcept (
						traits::is_nothrow_invocable_v<Fn &, T>
					) {
						return ::std::move(res).transform(fn);
					});

					if (map_res.has_error()) {
						cb(::std::move(map_res).error());
					} else {
						setCallback(*::std::move(map_res), ::std::move(cb));
					}
				},

				::std::move(fn),

				::utils::builder([&]() {
					return futures::makeCallback<U>(
						[](auto &res, auto &p) noexcept {
							::std::move(p).setResult(::std::move(res));
						},
						::std::move(contract).p
					);
				})
			)
		);

		return ::std::move(contract).f | futures::via(where);
	}
};

} // namespace pipe

template <typename Fn>
auto flatMap(Fn fn)
	noexcept (::std::is_nothrow_move_constructible_v<Fn>)
	requires (::std::is_nothrow_destructible_v<Fn>)
{
	return pipe::FlatMap{{}, ::std::move(fn)};
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLAT_MAP_H_ */
