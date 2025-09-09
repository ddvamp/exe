// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURE_FUN_COMBINE_PAR_FIRST_H_
#define DDV_EXE_FUTURE_FUN_COMBINE_PAR_FIRST_H_ 1

#include <array>
#include <memory>
#include <optional>
#include <utility>

#include "exe/future/fun/combine/par/detail/use_count.h"
#include "exe/future/fun/combine/seq/inline.h"
#include "exe/future/fun/make/contract/contract.h"
#include "exe/future/fun/mutator/mutator.h"

#include "util/type_traits.h"

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
		using T = ::util::pack_element_t<0, Fs...>::value_type;

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
		::util::are_all_same_v<typename Fs::value_type...>
	)
{
	return detail::First().mutate(::std::move(fs)...);
}

} // namespace exe::future

#endif /* DDV_EXE_FUTURE_FUN_COMBINE_PAR_FIRST_H_ */
