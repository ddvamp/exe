// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_COMBINE_PAR_ALL_H_
#define DDV_EXE_FUTURES_FUN_COMBINE_PAR_ALL_H_ 1

#include <cstddef>
#include <memory>
#include <optional>
#include <tuple>

#include "exe/futures/fun/combine/par/detail/use_count.h"
#include "exe/futures/fun/combine/seq/inline.h"
#include "exe/futures/fun/make/contract/contract.h"
#include "exe/futures/fun/mutator/mutator.h"

#include "utils/type_traits.h"
#include "utils/utility.h"

namespace exe::futures {

namespace detail {

template <typename>
class AllState;

template <typename ...Ts>
class AllState<::std::tuple<Ts...>> : private UseCount {
private:
	using U = ::std::tuple<Ts...>;

	::std::optional<Promise<U>> promise_;
	::std::tuple<::std::optional<Ts>...> results_;

public:
	explicit AllState(count_t count) noexcept
		: UseCount(count)
	{}

	void setPromise(Promise<U> p) noexcept
	{
		promise_.emplace(::std::move(p));
	}

	template <::std::size_t I, typename T>
	void setResult(::utils::result<T> &&res) noexcept
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
				::std::move(*promise_).setError(::std::move(res).error());
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
			::std::move(*promise_).setResult(
				::std::forward_as_tuple(
					*::std::get<Is>(::std::move(results_))...
				)
			);
		}(::std::index_sequence_for<Ts...>{});
	}

	void destroySelf() noexcept
	{
		delete this;
	}
};

////////////////////////////////////////////////////////////////////////////////

class [[nodiscard]] All : public Mutator {
public:
	All() = default;

	template <typename ...Fs>
	auto mutate(Fs &&...fs)
	{
		using T = ::std::tuple<
			::utils::change_if_same_t<
				void,
				typename Fs::value_type,
				::utils::unit_t
			>...
		>;

		auto contract = Contract<T>();

		auto state = ::std::make_unique<AllState<T>>(sizeof...(Fs));

		[&]<::std::size_t ...Is>(::std::index_sequence<Is...>) {
			// strong exception guarantee
			auto callback_list = ::std::tuple{
				Fs::Callback(
					[state = state.get()](auto &&res) noexcept {
						state->setResult<Is>(::std::move(res));
					}
				)...
			};

			state.release()->setPromise(::std::move(contract).p);

			(..., setCallback(
				::std::move(fs) | futures::inLineIfNeeded(),
				::std::get<Is>(::std::move(callback_list))
			));
		}(::std::index_sequence_for<Fs...>{});

		return ::std::move(contract).f;
	}
};

} // namespace detail

template <concepts::Future ...Fs>
auto all(Fs &&...fs)
	requires (sizeof...(Fs) > 1)
{
	return detail::All().mutate(::std::move(fs)...);
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_COMBINE_PAR_ALL_H_ */
