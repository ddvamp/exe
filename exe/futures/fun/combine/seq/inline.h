// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_COMBINE_SEQ_INLINE_H_
#define DDV_EXE_FUTURES_FUN_COMBINE_SEQ_INLINE_H_ 1

#include <utility>

#include "exe/executors/inline.h"
#include "exe/futures/fun/combine/seq/via.h"
#include "exe/futures/fun/mutator/mutator.h"
#include "exe/futures/fun/syntax/pipe.h"

namespace exe::futures {

namespace pipe {

struct [[nodiscard]] InLine : Mutator {
	template <typename T>
	auto mutate(SemiFuture<T> f) noexcept
	{
		return ::std::move(f) | futures::via(executors::getInlineExecutor());
	}
};

} // namespace pipe

inline auto inLine() noexcept
{
	return pipe::InLine{};
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_COMBINE_SEQ_INLINE_H_ */
