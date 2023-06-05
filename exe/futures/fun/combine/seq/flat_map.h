// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLAT_MAP_H_
#define DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLAT_MAP_H_ 1

#include <utility>

#include "exe/futures/fun/combine/seq/flatten.h"
#include "exe/futures/fun/combine/seq/map.h"
#include "exe/futures/fun/mutator/mutator.h"

namespace exe::futures {

namespace pipe {

template <typename Fn>
struct [[nodiscard]] FlatMap : detail::Mutator {
	[[no_unique_address]] Fn fn;

	template <concepts::Future F>
	auto mutate(F &&f)
		requires (hasExecutor<F>)
	{
		return
			::std::move(f) |
			futures::map(::std::move(fn)) |
			futures::flatten();
	}
};

} // namespace pipe

template <typename Fn>
auto flatMap(Fn fn) noexcept
{
	return pipe::FlatMap{{}, ::std::move(fn)};
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLAT_MAP_H_ */
