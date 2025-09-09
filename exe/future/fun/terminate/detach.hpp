// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURE_FUN_TERMINATE_DETACH_H_
#define DDV_EXE_FUTURE_FUN_TERMINATE_DETACH_H_ 1

#include <utility>

#include "exe/future/fun/combine/seq/inline.hpp"
#include "exe/future/fun/mutator/mutator.hpp"
#include "exe/future/fun/syntax/pipe.hpp"

#include "exe/future/fun/result/result.hpp"

namespace exe::future {

namespace pipe {

class [[nodiscard]] Detach : public detail::Mutator {
	template <concepts::Future F, concepts::Mutator M>
	friend auto operator| (F &&, M) noexcept (M::template mutates_nothrow<F>);

public:
	template <typename>
	inline static constexpr bool mutates_nothrow = true;

	Detach() = default;

private:
	template <concepts::Future F>
	void mutate(F f) noexcept
	{
		setCallback(
			makeHolder(f) | future::inLineIfNeeded(),
			[](auto &&) noexcept {}
		);
	}
};

} // namespace pipe

inline auto detach() noexcept
{
	return pipe::Detach();
}

} // namespace exe::future

#endif /* DDV_EXE_FUTURE_FUN_TERMINATE_DETACH_H_ */
