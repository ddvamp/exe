//
//
//
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_MAP_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_MAP_HPP_INCLUDED_ 1

#include <type_traits>
#include <utility>

#include "exe/future/fun/combine/seq/via.hpp"
#include "exe/future/fun/make/contract/contract.hpp"
#include "exe/future/fun/mutator/mutator.hpp"
#include "exe/future/fun/syntax/pipe.hpp"
#include "exe/future/fun/traits/map.hpp"

#include "exe/future/fun/result/result.hpp"

namespace exe::future {

namespace pipe {

template <typename Fn>
class [[nodiscard]] Map : public detail::Mutator {
	template <concepts::Future F, concepts::Mutator M>
	friend auto operator| (F &&, M) noexcept (M::template mutates_nothrow<F>);

private:
	Fn &&fn_;

public:
	template <typename>
	inline static constexpr bool mutates_nothrow = false;

	explicit Map(Fn &&fn) noexcept
		: fn_(::std::forward<Fn>(fn))
	{}

private:
	template <concepts::Future F>
	auto mutate(F &&f)
		requires (
			has_scheduler_v<F> &&
			traits::is_invocable_v<Fn &, typename F::value_type>
		)
	{
		using T = F::value_type;
		using U = traits::map_result_t<Fn &, T>;

		auto contract = Contract<U>();

		auto &where = getScheduler(f);

		setCallback(
			::std::move(f),
			future::makeCallback<T>(
				[](auto &res, auto &fn, auto &p) noexcept {
					::std::move(p).setResult(
						::util::map_safely([&]() noexcept (
							traits::is_nothrow_invocable_v<Fn &, T>
						) {
							return ::std::move(res).transform(fn);
						})
					);
				},
				::std::forward<Fn>(fn_),
				::std::move(contract).p
			)
		);

		return ::std::move(contract).f | future::via(where);
	}
};

} // namespace pipe

template <typename Fn>
auto map(Fn &&fn) noexcept
	requires (::std::is_nothrow_destructible_v<::std::remove_cvref_t<Fn>>)
{
	return pipe::Map<Fn>(::std::forward<Fn>(fn));
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_MAP_HPP_INCLUDED_ */
