// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURE_FUN_TERMINATE_GET_H_
#define DDV_EXE_FUTURE_FUN_TERMINATE_GET_H_ 1

#include <optional>
#include <utility>

#include "concurrency/one_shot_event.hpp"

#include "exe/future/fun/combine/seq/inline.hpp"
#include "exe/future/fun/mutator/mutator.hpp"
#include "exe/future/fun/syntax/pipe.hpp"

#include "exe/future/fun/result/result.hpp"

#include "util/abort.hpp"

namespace exe::future {

namespace pipe {

class [[nodiscard]] Get : public detail::Mutator {
	template <concepts::Future F, concepts::Mutator M>
	friend auto operator| (F &&, M) noexcept (M::template mutates_nothrow<F>);

public:
	template <typename>
	inline static constexpr bool mutates_nothrow = false;

	Get() = default;

private:
	template <concepts::Future F>
	auto mutate(F &&f)
	{
		using T = F::value_type;

		::std::optional<::util::result<T>> result;

		::concurrency::OneShotEvent result_is_ready;

		setCallback(
			makeHolder(f) | future::inLineIfNeeded(),
			[&](auto &&res) noexcept {
				result.emplace(::std::move(res));

				try {
					result_is_ready.notify();
				} catch (...) {
					UTIL_ABORT("exception in async callback future::get");
				}
			}
		);

		try {
			result_is_ready.wait();
		} catch (...) {
			UTIL_ABORT("exception while sync wait of future");
		}

		return *::std::move(result);
	}
};

} // namespace pipe

inline auto get() noexcept
{
	return pipe::Get();
}

} // namespace exe::future

#endif /* DDV_EXE_FUTURE_FUN_TERMINATE_GET_H_ */
