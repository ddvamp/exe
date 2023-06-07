// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_TERMINATE_APPLY_H_
#define DDV_EXE_FUTURES_FUN_TERMINATE_APPLY_H_ 1

#include <type_traits>
#include <utility>

#include "exe/futures/fun/mutator/mutator.h"
#include "exe/futures/fun/syntax/pipe.h"
#include "exe/futures/fun/types/future.h"

namespace exe::futures {

namespace pipe {

template <typename Fn>
struct [[nodiscard]] Apply : detail::Mutator {
	[[no_unique_address]] Fn fn;

	template <concepts::Future F>
	auto mutate(F &&f)
		noexcept (::std::is_nothrow_constructible_v<typename F::Callback, Fn>)
		requires (
			has_executor_v<F> &&
			::std::is_constructible_v<typename F::Callback, Fn>
		)
	{
		setCallback(::std::move(f), ::std::move(fn));
	}
};

} // namespace pipe

template <typename Fn>
auto apply(Fn fn)
	requires (::std::is_nothrow_destructible_v<Fn>)
{
	return pipe::Apply{{}, ::std::move(fn)};
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_TERMINATE_APPLY_H_ */
