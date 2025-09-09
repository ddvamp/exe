// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLAT_MAP_H_
#define DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLAT_MAP_H_ 1

#include <type_traits>

#include "exe/future/fun/combine/seq/via.h"
#include "exe/future/fun/make/contract/contract.h"
#include "exe/future/fun/mutator/mutator.h"
#include "exe/future/fun/syntax/pipe.h"
#include "exe/future/fun/traits/map.h"

#include "result/result.h"

#include "util/utility.h"

namespace exe::future {

namespace pipe {

template <typename Fn>
class [[nodiscard]] FlatMap : public detail::Mutator {
	template <concepts::Future F, concepts::Mutator M>
	friend auto operator| (F &&, M) noexcept (M::template mutates_nothrow<F>);

private:
	Fn &&fn_;

public:
	template <typename>
	inline static constexpr bool mutates_nothrow = false;

	explicit FlatMap(Fn &&fn) noexcept
		: fn_(::std::forward<Fn>(fn))
	{}

private:
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
			future::makeCallback<T>(
				::util::types_list<::util::deduce_type_t, Fn, Callback<U>>,

				[](auto &res, auto &fn, auto &cb) noexcept {
					auto map_res = ::util::map_safely([&]() noexcept (
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

				::std::forward<Fn>(fn_),

				::util::builder([&]() {
					return future::makeCallback<U>(
						[](auto &res, auto &p) noexcept {
							::std::move(p).setResult(::std::move(res));
						},
						::std::move(contract).p
					);
				})
			)
		);

		return ::std::move(contract).f | future::via(where);
	}
};

} // namespace pipe

template <typename Fn>
auto flatMap(Fn &&fn) noexcept
	requires (::std::is_nothrow_destructible_v<::std::remove_cvref_t<Fn>>)
{
	return pipe::FlatMap<Fn>(::std::forward<Fn>(fn));
}

} // namespace exe::future

#endif /* DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLAT_MAP_H_ */
