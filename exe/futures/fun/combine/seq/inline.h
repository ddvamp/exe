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

struct [[nodiscard]] InLine : detail::Mutator {
	template <concepts::Future F>
	auto mutate(F f) noexcept
	{
		return ::std::move(f) | futures::via(executors::getInlineExecutor());
	}
};

} // namespace pipe

inline auto inLine() noexcept
{
	return pipe::InLine{};
}

////////////////////////////////////////////////////////////////////////////////

namespace pipe {

struct [[nodiscard]] InLineIfNeeded : detail::Mutator {
	template <concepts::Future F>
	auto mutate(F f) noexcept
	{
		if constexpr (has_executor_v<F>) {
			return ::std::move(f);
		} else {
			return ::std::move(f) | futures::inLine();
		}
	}
};

} // namespace pipe

inline auto inLineIfNeeded() noexcept
{
	return pipe::InLineIfNeeded{};
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_COMBINE_SEQ_INLINE_H_ */
