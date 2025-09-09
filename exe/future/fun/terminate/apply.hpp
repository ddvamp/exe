// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURE_FUN_TERMINATE_APPLY_H_
#define DDV_EXE_FUTURE_FUN_TERMINATE_APPLY_H_ 1

#include <type_traits>
#include <utility>

#include "exe/future/fun/mutator/mutator.hpp"
#include "exe/future/fun/syntax/pipe.hpp"
#include "exe/future/fun/types/future.hpp"

namespace exe::future {

namespace pipe {

template <typename Fn>
class [[nodiscard]] Apply : public detail::Mutator {
	template <concepts::Future F, concepts::Mutator M>
	friend auto operator| (F &&, M) noexcept (M::template mutates_nothrow<F>);

private:
	Fn &&fn_;

public:
	template <typename>
	inline static constexpr bool mutates_nothrow = false;

	explicit Apply(Fn &&fn) noexcept
		: fn_(::std::forward<Fn>(fn))
	{}

private:
	template <concepts::Future F>
	auto mutate(F &&f)
		noexcept (::std::is_nothrow_constructible_v<typename F::Callback, Fn>)
		requires (
			has_scheduler_v<F> &&
			::std::is_constructible_v<typename F::Callback, Fn>
		)
	{
		setCallback(::std::move(f), ::std::forward<Fn>(fn_));
	}
};

} // namespace pipe

template <typename Fn>
auto apply(Fn &&fn) noexcept
	requires (::std::is_nothrow_destructible_v<::std::remove_cvref_t<Fn>>)
{
	return pipe::Apply<Fn>(::std::forward<Fn>(fn));
}

} // namespace exe::future

#endif /* DDV_EXE_FUTURE_FUN_TERMINATE_APPLY_H_ */
