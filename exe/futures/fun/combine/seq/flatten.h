// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLATTEN_H_
#define DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLATTEN_H_ 1

#include <utility>

#include "exe/futures/fun/combine/seq/flat_map.h"
#include "exe/futures/fun/combine/seq/inline.h"
#include "exe/futures/fun/mutator/mutator.h"
#include "exe/futures/fun/syntax/pipe.h"

namespace exe::futures {

namespace pipe {

class [[nodiscard]] Flatten : public detail::Mutator {
	template <concepts::Future F, concepts::Mutator M>
	friend auto operator| (F &&, M) noexcept (M::template mutates_nothrow<F>);

public:
	template <typename>
	inline static constexpr bool mutates_nothrow = false;

	Flatten() = default;

private:
	template <concepts::Future F>
	auto mutate(F &&f)
	{
		auto mapper = [](auto f) noexcept { return ::std::move(f); };

		if constexpr (has_executor_v<F>) {
			return ::std::move(f) | futures::flatMap(mapper);
		} else {
			return unsetExecutor(
				makeHolder(f) | futures::inLine() | futures::flatMap(mapper)
			);
		}
	}
};

} // namespace pipe

inline auto flatten() noexcept
{
	return pipe::Flatten();
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLATTEN_H_ */
