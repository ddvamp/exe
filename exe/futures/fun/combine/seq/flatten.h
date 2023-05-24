// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLATTEN_H_
#define DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLATTEN_H_ 1

#include <exception>
#include <type_traits>
#include <utility>

#include "exe/futures/fun/make/contract.h"
#include "exe/futures/fun/mutator/mutator.h"
#include "exe/futures/fun/syntax/pipe.h"

#include "result/result.h"

namespace exe::futures {

namespace pipe {

// TODO: harmful exceptions
struct [[nodiscard]] Flatten : Mutator {
	template <typename T>
	Future<T> mutate(Future<Future<T>> f)
	{
		// loss future at exception
		auto [future, promise] = Contract::open<T>();

		// loss future at exception
		setCallback<Future<T>>(
			::std::move(f),
			[p = ::std::move(promise)]
			(::utils::result<Future<T>> &&res) mutable noexcept {
				if (res.has_error()) {
					::std::move(p).setError(::std::move(res).error());
					return;
				}

				setCallback<T>(
					*::std::move(res),
					[p = ::std::move(p)]
					(::utils::result<T> &&res) mutable noexcept {
						if constexpr (
							::std::is_nothrow_move_constructible_v<T>
						) {
							::std::move(p).setResult(::std::move(res));
						} else try {
							::std::move(p).setResult(::std::move(res));
						} catch (...) {
							::std::move(p).setError(::std::current_exception());
						}
					}
				);
			}
		);

		return future;
	}
};

} // namespace pipe

inline auto flatten() noexcept
{
	return pipe::Flatten{};
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_COMBINE_SEQ_FLATTEN_H_ */
