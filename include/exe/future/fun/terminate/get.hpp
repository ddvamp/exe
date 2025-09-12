//
//
//
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_TERMINATE_GET_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_TERMINATE_GET_HPP_INCLUDED_ 1

#include <atomic>
#include <optional>
#include <utility>

#include "exe/future/fun/combine/seq/inline.hpp"
#include "exe/future/fun/mutator/mutator.hpp"
#include "exe/future/fun/syntax/pipe.hpp"

#include "exe/future/fun/result/result.hpp"

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

		// [TODO]: Write future::Event or concurrency::StrongEvent
		::std::atomic_flag f1 = false;
		::std::atomic_flag f2 = false;

		setCallback(
			makeHolder(f) | future::inLineIfNeeded(),
			[&](auto &&res) noexcept {
				result.emplace(::std::move(res));

				f1.test_and_set(::std::memory_order_relaxed);
				f1.notify_all();
				f2.test_and_set(::std::memory_order_release);
			}
		);

		f1.wait(false, ::std::memory_order_relaxed);
		while (!f2.test(::std::memory_order_acquire)) {
			// Relax();
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

#endif /* DDVAMP_EXE_FUTURE_FUN_TERMINATE_GET_HPP_INCLUDED_ */
