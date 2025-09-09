//
//
//
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_COMBINE_PAR_FIRST_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_COMBINE_PAR_FIRST_HPP_INCLUDED_ 1

#include <array>
#include <memory>
#include <optional>
#include <utility>

#include "exe/future/fun/combine/par/detail/use_count.hpp"
#include "exe/future/fun/combine/seq/inline.hpp"
#include "exe/future/fun/make/contract/contract.hpp"
#include "exe/future/fun/mutator/mutator.hpp"

#include "exe/future/fun/type_traits.hpp"

namespace exe::future {

namespace detail {

template <typename T>
class FirstState : private UseCount {
private:
	::std::optional<Promise<T>> promise_;

public:
	explicit FirstState(count_t count) noexcept
		: UseCount(count)
	{}

	void setPromise(Promise<T> p) noexcept
	{
		promise_.emplace(::std::move(p));
	}

	void setResult(::util::result<T> &&res) noexcept
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
	void setResult(::util::result<T> &res) noexcept
	{
		::std::move(*promise_).setResult(::std::move(res));
	}

	void destroySelf() noexcept
	{
		delete this;
	}
};

////////////////////////////////////////////////////////////////////////////////

class [[nodiscard]] First : public Mutator {
public:
	First() = default;

	template <typename ...Fs>
	auto mutate(Fs &&...fs)
	{
		using T = pack_element_t<0, Fs...>::value_type;

		auto contract = Contract<T>();

		auto state = ::std::make_unique<FirstState<T>>(sizeof...(Fs));

		// strong exception guarantee
		auto callback_list = ::std::array{
			Fs::Callback(
				[state = state.get()](auto &&res) noexcept {
					state->setResult(::std::move(res));
				}
			)...
		};

		state.release()->setPromise(::std::move(contract).p);

		[&]<::std::size_t ...Is>(::std::index_sequence<Is...>) noexcept {
			(..., setCallback(
				::std::move(fs) | future::inLineIfNeeded(),
				::std::move(callback_list[Is])
			));
		}(::std::index_sequence_for<Fs...>{});

		return ::std::move(contract).f;
	}
};

} // namespace detail

template <concepts::Future ...Fs>
auto first(Fs &&...fs)
	requires (
		sizeof...(Fs) > 1 &&
		are_all_same_v<typename Fs::value_type...>
	)
{
	return detail::First().mutate(::std::move(fs)...);
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_COMBINE_PAR_FIRST_HPP_INCLUDED_ */
