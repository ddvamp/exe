// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_COMBINE_SEQ_INLINE_H_
#define DDV_EXE_FUTURES_FUN_COMBINE_SEQ_INLINE_H_ 1

#include <utility>

#include "exe/executors/inline.h"
#include "exe/future/fun/combine/seq/via.h"
#include "exe/future/fun/mutator/mutator.h"
#include "exe/future/fun/syntax/pipe.h"

namespace exe::future {

namespace pipe {

class [[nodiscard]] InLine : public detail::Mutator {
	template <concepts::Future F, concepts::Mutator M>
	friend auto operator| (F &&, M) noexcept (M::template mutates_nothrow<F>);

public:
	template <typename>
	inline static constexpr bool mutates_nothrow = true;

	InLine() = default;

private:
	template <concepts::Future F>
	auto mutate(F f) noexcept
	{
		return ::std::move(f) | future::via(executors::getInlineExecutor());
	}
};

} // namespace pipe

inline auto inLine() noexcept
{
	return pipe::InLine();
}

////////////////////////////////////////////////////////////////////////////////

namespace pipe {

class [[nodiscard]] InLineIfNeeded : public detail::Mutator {
	template <concepts::Future F, concepts::Mutator M>
	friend auto operator| (F &&, M) noexcept (M::template mutates_nothrow<F>);

public:
	template <typename>
	inline static constexpr bool mutates_nothrow = true;

	InLineIfNeeded() = default;

private:
	template <concepts::Future F>
	auto mutate(F f) noexcept
	{
		if constexpr (has_executor_v<F>) {
			return ::std::move(f);
		} else {
			return ::std::move(f) | future::inLine();
		}
	}
};

} // namespace pipe

inline auto inLineIfNeeded() noexcept
{
	return pipe::InLineIfNeeded();
}

} // namespace exe::future

#endif /* DDV_EXE_FUTURES_FUN_COMBINE_SEQ_INLINE_H_ */
