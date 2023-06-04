// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_COMBINE_PAR_FIRST_H_
#define DDV_EXE_FUTURES_FUN_COMBINE_PAR_FIRST_H_ 1

#include <type_traits>
#include <utility>

#include "exe/futures/fun/combine/par/detail/use_count.h"
#include "exe/futures/fun/combine/seq/inline.h"
#include "exe/futures/fun/make/contract/contract.h"
#include "exe/futures/fun/mutator/mutator.h"

#include "utils/type_traits.h"

namespace exe::futures {

namespace detail {

template <typename T>
class FirstState : private UseCount {
private:
	Promise<T> promise_;

public:
	FirstState(count_t count, Promise<T> p)
		: UseCount(count)
		, promise_(::std::move(p))
	{}

	void send(::utils::result<T> &&res) noexcept
	{
		auto order = ::std::memory_order_acquire;

		if (res) {
			if (auto first = use()) {
				order = ::std::memory_order_release;
				setResult(res);
			}

			auto [_, done] = release(order);

			if (done) {
				destroySelf();
			}
		} else {
			auto [clean, done] = release(order);

			if (done) {
				if (clean) {
					setResult(res);
				}

				destroySelf();
			}
		}
	}

private:
	void setResult(::utils::result<T> &res) noexcept
	{
		::std::move(promise_).setResult(::std::move(res));
	}

	void destroySelf() noexcept
	{
		delete this;
	}
};

struct [[nodiscard]] First : Mutator {
	template <typename ...Fs>
	auto mutate(Fs &&...fs)
	{
		using T = ::utils::pack_element_t<0, Fs...>::value_type;

		auto contract = Contract<T>();

		auto state = ::new FirstState(sizeof...(Fs), ::std::move(contract).p);

		(setCallback(
			::std::move(fs) | futures::inLineIfNeeded(),
			[state](::utils::result<T> &&res) noexcept {
				state->send(::std::move(res));
			}
		), ...);

		return ::std::move(contract).f;
	}
};

} // namespace detail

template <concepts::Future ...Fs>
auto first(Fs &&...fs)
	requires (
		sizeof...(Fs) > 1 &&
		::utils::are_all_same_v<typename Fs::value_type...>
	)
{
	return detail::First{}.mutate(::std::move(fs)...);
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_COMBINE_PAR_FIRST_H_ */
