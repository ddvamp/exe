// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_COMBINE_PAR_ALL_H_
#define DDV_EXE_FUTURES_FUN_COMBINE_PAR_ALL_H_ 1

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

#include "exe/futures/fun/combine/par/detail/use_count.h"
#include "exe/futures/fun/combine/seq/inline.h"
#include "exe/futures/fun/make/contract/contract.h"
#include "exe/futures/fun/mutator/mutator.h"

#include "utils/type_traits.h"

namespace exe::futures {

namespace detail {

template <typename ...Ts>
class AllState : private UseCount {
private:
	using U = ::std::tuple<Ts...>;

	Promise<U> promise_;
	::std::tuple<::std::optional<Ts>...> results_;

public:
	AllState(count_t count, Promise<U> p)
		: UseCount(count)
		, promise_(::std::move(p))
	{}

	template <::std::size_t I, typename T>
	void send(::utils::result<T> &&res) noexcept
	{
		auto order = ::std::memory_order_acq_rel;

		if (res) {
			if constexpr (::std::is_void_v<T>) {
				::std::get<I>(results_).emplace();
			} else {
				::std::get<I>(results_).emplace(*::std::move(res));
			}

			auto [clean, done] = release(order);

			if (done) {
				if (clean) {
					setResult();
				}

				destroySelf();
			}
		} else {
			if (auto first = use()) {
				::std::move(promise_).setError(::std::move(res).error());
			} else {
				order = ::std::memory_order_acquire;
			}

			auto [_, done] = release(order);

			if (done) {
				destroySelf();
			}
		}
	}

private:
	void setResult() noexcept
	{
		[this]<::std::size_t ...Is>
		(::std::index_sequence<Is...>) noexcept {
			::std::move(promise_).setResult(::std::forward_as_tuple(
				*::std::get<Is>(::std::move(results_))...
			));
		}(::std::index_sequence_for<Ts...>{});
	}

	void destroySelf() noexcept
	{
		delete this;
	}
};

////////////////////////////////////////////////////////////////////////////////

struct [[nodiscard]] All : Mutator {
	template <typename T>
	struct void_to_dummy {
		using type = T;
	};

	template <>
	struct void_to_dummy<void> {
		struct dummy {};

		using type = dummy;
	};

	template <typename T>
	using void_to_dummy_t = void_to_dummy<T>::type;



	template <typename ...Fs>
	auto mutate(Fs &&...fs)
	{
		using T = ::std::tuple<void_to_dummy_t<typename Fs::value_type>...>;

		auto contract = Contract<T>();

		auto state = ::new AllState(sizeof...(Fs), ::std::move(contract).p);

		[&]<::std::size_t ...Is>(::std::index_sequence<Is...>)
		{
			(setCallback(
				::std::move(fs) | futures::inLineIfNeeded(),
				[state](::utils::result<Fs::value_type> &&res) noexcept {
					state->send<Is>(::std::move(res));
				}
			), ...);
		}(::std::index_sequence_for<Fs...>{});

		return ::std::move(contract).f;
	}
};

} // namespace detail

template <concepts::Future ...Fs>
auto all(Fs &&...fs)
	requires (sizeof...(Fs) > 1)
{
	return detail::All{}.mutate(::std::move(fs)...);
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_COMBINE_PAR_ALL_H_ */
