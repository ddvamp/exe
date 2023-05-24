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

template <typename F>
struct [[nodiscard]] FlatMap : Mutator {
	[[no_unique_address]] F fun;

	template <typename T>
	auto mutate(Future<T> f)
	{
		return
			::std::move(f) |
			futures::map(::std::move(fun)) |
			futures::flatten();
	}
};

} // namespace pipe

template <typename F>
auto flatMap(F fun) noexcept
{
	return pipe::FlatMap{{}, ::std::move(fun)};
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLAT_MAP_H_ */
