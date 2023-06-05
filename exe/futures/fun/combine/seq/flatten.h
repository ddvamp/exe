// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLATTEN_H_
#define DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLATTEN_H_ 1

#include <exception>

#include "exe/futures/fun/combine/seq/via.h"
#include "exe/futures/fun/make/contract/contract.h"
#include "exe/futures/fun/mutator/mutator.h"
#include "exe/futures/fun/syntax/pipe.h"

#include "result/result.h"

#include "utils/utility.h"

namespace exe::futures {

namespace pipe {

struct [[nodiscard]] Flatten : detail::Mutator {
	template <typename T>
	auto mutate(Future<Future<T>> &&f)
	{
		auto contract = Contract<T>();

		// TODO: Future<SemiFuture<T>>
		auto &where = getExecutor(f);

		setCallback(
			::std::move(f),
			futures::makeCallback<Future<T>>(
				::utils::types_list<::utils::deduce_type_t, Callback<T>>,

				[](auto &res, auto &cb) noexcept {
					if (res.has_error()) {
						cb(::std::move(res).error());
					} else {
						setCallback(*::std::move(res), ::std::move(cb));
					}
				},

				::utils::builder([&]() { 
					return futures::makeCallback<T>(
						[] (auto &res, auto &p) noexcept {
							::std::move(p).setResult(::std::move(res));
						},
						::std::move(contract).p
					);
				})
			)
		);

		return ::std::move(contract).f | futures::via(where);
	}
};

} // namespace pipe

inline auto flatten() noexcept
{
	return pipe::Flatten{};
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLATTEN_H_ */
