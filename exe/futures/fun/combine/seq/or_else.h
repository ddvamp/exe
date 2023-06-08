// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_COMBINE_SEQ_OR_ELSE_H_
#define DDV_EXE_FUTURES_FUN_COMBINE_SEQ_OR_ELSE_H_ 1

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
class [[nodiscard]] OrElse : public detail::Mutator {
	template <concepts::Future F, concepts::Mutator M>
	friend auto operator| (F &&, M) noexcept (M::template mutates_nothrow<F>);

private:
	Fn &&fn_;

public:
	template <typename>
	inline static constexpr bool mutates_nothrow = false;

	explicit OrElse(Fn &&fn) noexcept
		: fn_(::std::forward<Fn>(fn))
	{}

private:
	template <concepts::Future F>
	auto mutate(F &&f)
		requires (
			has_executor_v<F> &&
			::std::is_same_v<
				traits::map_result_t<Fn &, ::utils::error>,
				::utils::result<typename F::value_type>
			>
		)
	{
		using T = F::value_type;

		auto contract = Contract<T>();

		auto &where = getExecutor(f);

		setCallback(
			::std::move(f),
			futures::makeCallback<T>(
				[](auto &res, auto &fn, auto &p) noexcept {
					::std::move(p).setResult(
						::utils::map_safely([&]() noexcept (
							::std::is_nothrow_invocable_v<Fn &, ::utils::error>
						) {
							return ::std::move(res).or_else(fn);
						})
					);
				},
				::std::forward<Fn>(fn_),
				::std::move(contract).p
			)
		);

		return ::std::move(contract).f | futures::via(where);
	}
};

} // namespace pipe

template <typename Fn>
auto orElse(Fn &&fn) noexcept
	requires (
		::std::is_nothrow_destructible_v<::std::remove_cvref_t<Fn>> &&
		::std::is_invocable_v<Fn &, ::utils::error>
	)
{
	return pipe::OrElse<Fn>(::std::forward<Fn>(fn));
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_COMBINE_SEQ_OR_ELSE_H_ */
