// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLATTEN_H_
#define DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLATTEN_H_ 1

#include "exe/futures/fun/combine/seq/via.h"
#include "exe/futures/fun/make/contract/contract.h"
#include "exe/futures/fun/mutator/mutator.h"
#include "exe/futures/fun/syntax/pipe.h"

#include "result/result.h"

#include "utils/utility.h"

namespace exe::futures {

namespace pipe {

struct [[nodiscard]] Flatten : detail::Mutator {
	template <concepts::Future F>
	auto mutate(F &&f)
		requires (concepts::Future<typename F::value_type>)
	{
		using T = F::value_type;
		using U = T::value_type;

		auto contract = Contract<U>();

		auto move_future = [&]() noexcept {
			if constexpr (hasExecutor<F>) {
				return [&, &where = getExecutor(f)]() noexcept {
					return ::std::move(contract).f | futures::via(where);
				};
			} else {
				return [&]() noexcept {
					return ::std::move(contract).f;
				};
			}
		}();

		setCallback(
			::std::move(f),
			futures::makeCallback<T>(
				::utils::types_list<::utils::deduce_type_t, Callback<U>>,

				[](auto &res, auto &cb) noexcept {
					if (res.has_error()) {
						cb(::std::move(res).error());
					} else {
						setCallback(*::std::move(res), ::std::move(cb));
					}
				},

				::utils::builder([&]() { 
					return futures::makeCallback<U>(
						[] (auto &res, auto &p) noexcept {
							::std::move(p).setResult(::std::move(res));
						},
						::std::move(contract).p
					);
				})
			)
		);

		return move_future();
	}
};

} // namespace pipe

inline auto flatten() noexcept
{
	return pipe::Flatten{};
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLATTEN_H_ */
