// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_COMBINE_PAR_FIRST_H_
#define DDV_EXE_FUTURES_FUN_COMBINE_PAR_FIRST_H_ 1

#include <atomic>
#include <cstdint>
#include <type_traits>
#include <utility>

#include "exe/futures/fun/combine/seq/inline.h"
#include "exe/futures/fun/make/contract/contract.h"
#include "exe/futures/fun/mutator/mutator.h"

#include "utils/defer.h"
#include "utils/type_traits.h"

namespace exe::futures {

namespace detail {

struct [[nodiscard]] First : Mutator {
	template <typename T>
	class State {
	private:
		using state_t = ::std::uint64_t;

		::std::atomic<state_t> state_;
		Promise<T> promise_;

	public:
		State(state_t count, Promise<T> p)
			: state_(count)
			, promise_(::std::move(p))
		{}

		void send(::utils::result<T> &&res) noexcept
		{
			auto order = ::std::memory_order_acquire;

			if (res) {
				if (auto first = success()) {
					order = ::std::memory_order_release;
					set(res);
				}

				auto [_, last] = done(order);

				if (last) {
					destroySelf();
				}
			} else {
				auto [failure, last] = done(order);

				if (last) {
					if (failure) {
						set(res);
					}

					destroySelf();
				}
			}
		}

	private:
		bool success() noexcept
		{
			auto state = state_.fetch_add(
				state_t(1) << 32,
				::std::memory_order_relaxed
			);

			return first(state);
		}

		::std::pair<bool, bool> done(::std::memory_order order) noexcept
		{
			auto state = state_.fetch_sub(1, order);

			return {first(state), last(state)};
		}

		void set(::utils::result<T> &res) noexcept
		{
			::std::move(promise_).setResult(::std::move(res));
		}

		void destroySelf() noexcept
		{
			delete this;
		}

		static bool first(state_t const state) noexcept
		{
			return static_cast<::std::uint32_t>(state >> 32) == 0;
		}

		static bool last(state_t const state) noexcept
		{
			return static_cast<::std::uint32_t>(state) == 1;
		}
	};

	template <typename ...Ts>
	auto mutate(Ts &&...fs)
	{
		using T = ::utils::pack_element_t<0, Ts...>::value_type;

		auto contract = Contract<T>();

		auto rollback = ::utils::scope_guard(
			[&]() noexcept {
				::std::move(contract).cancel();
			}
		);

		auto state = ::new State(sizeof...(Ts), ::std::move(contract).p);

		rollback.disable();

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

template <typename ...Ts>
auto first(Ts &&...fs)
	requires (
		sizeof...(Ts) > 1 &&
		::utils::all_true_v<is_future_v<Ts>...> &&
		::utils::all_true_v<!::utils::is_cv_v<Ts>...> &&
		::utils::are_all_same_v<typename Ts::value_type...>
	)
{
	return detail::First{}.mutate(::std::move(fs)...);
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_COMBINE_PAR_FIRST_H_ */
