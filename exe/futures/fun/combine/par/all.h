// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_COMBINE_PAR_ALL_H_
#define DDV_EXE_FUTURES_FUN_COMBINE_PAR_ALL_H_ 1

#include <atomic>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <utility>

#include "exe/futures/fun/combine/seq/inline.h"
#include "exe/futures/fun/make/contract/contract.h"
#include "exe/futures/fun/mutator/mutator.h"

#include "utils/defer.h"
#include "utils/type_traits.h"

namespace exe::futures {

namespace detail {

struct [[nodiscard]] All : Mutator {
	template <typename ...Ts>
	class State {
	private:
		using state_t = ::std::uint64_t;
		using U = ::std::tuple<Ts...>;

		::std::atomic<state_t> state_;
		Promise<U> promise_;
		::std::tuple<::std::optional<Ts>...> results_;

	public:
		State(state_t count, Promise<U> p)
			: state_(count)
			, promise_(::std::move(p))
		{}

		template <::std::size_t I>
		void send(
			::utils::result<::utils::pack_element_t<I, Ts...>> &&res) noexcept
		{
			if (res) {
				::std::get<I>(results_).emplace(*::std::move(res));

				auto [success, last] = done(::std::memory_order_acq_rel);

				if (last) {
					if (success) {
						set();
					}

					destroySelf();
				}
			} else {
				auto order = ::std::memory_order_acquire;

				if (auto first = failure()) {
					order = ::std::memory_order_acq_rel;
					::std::move(promise_).setError(::std::move(res).error());
				}

				auto [_, last] = done(order);

				if (last) {
					destroySelf();
				}
			}
		}

	private:
		bool failure() noexcept
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

		void set() noexcept
		{
			[this]<::std::size_t ...Is>
			(::std::index_sequence<Is...>) noexcept {
				::std::move(promise_).setResult(::std::forward_as_tuple(
					*::std::get<Is>(::std::move(results_))...
				));
			}(::std::make_index_sequence<sizeof...(Ts)>{});
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
		using T = ::std::tuple<typename Ts::value_type...>;

		auto contract = Contract<T>();

		auto rollback = ::utils::scope_guard(
			[&]() noexcept {
				::std::move(contract).cancel();
			}
		);

		auto state = ::new State(sizeof...(Ts), ::std::move(contract).p);

		rollback.disable();

		[&]<::std::size_t ...Is>(::std::index_sequence<Is...>)
		{
			(setCallback(
				::std::move(fs) | futures::inLineIfNeeded(),
				[state](::utils::result<Ts::value_type> &&res) noexcept {
					state->send<Is>(::std::move(res));
				}
			), ...);
		}(::std::make_index_sequence<sizeof...(Ts)>{});

		return ::std::move(contract).f;
	}
};

} // namespace detail

template <typename ...Ts>
auto all(Ts &&...fs)
	requires (
		sizeof...(Ts) > 1 &&
		::utils::all_true_v<is_future_v<Ts>...> &&
		::utils::all_true_v<!::utils::is_cv_v<Ts>...>
	)
{
	return detail::All{}.mutate(::std::move(fs)...);
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_COMBINE_PAR_ALL_H_ */
