// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLATTEN_H_
#define DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLATTEN_H_ 1

#include <exception>
#include <utility>

#include "exe/futures/fun/combine/seq/via.h"
#include "exe/futures/fun/make/contract/contract.h"
#include "exe/futures/fun/mutator/mutator.h"
#include "exe/futures/fun/syntax/pipe.h"

#include "result/result.h"

namespace exe::futures {

namespace pipe {

// TODO: harmful exceptions
struct [[nodiscard]] Flatten : detail::Mutator {
	template <typename T>
	auto mutate(Future<Future<T>> f)
	{
		// loss future at exception
		auto [future, promise] = Contract<T>();

		auto &where = getExecutor(f);

		// loss future at exception
		setCallback(
			::std::move(f),
			[p = ::std::move(promise)]
			(::utils::result<Future<T>> &&res) mutable noexcept {
				if (res.has_error()) {
					::std::move(p).setError(::std::move(res).error());
					return;
				}

				setCallback(
					*::std::move(res),
					[p = ::std::move(p)]
					(::utils::result<T> &&res) mutable noexcept {
						::std::move(p).setResult(::std::move(res));
					}
				);
			}
		);

		return ::std::move(future) | futures::via(where);
	}
};

} // namespace pipe

inline auto flatten() noexcept
{
	return pipe::Flatten{};
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLATTEN_H_ */
