// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_TERMINATE_GET_H_
#define DDV_EXE_FUTURES_FUN_TERMINATE_GET_H_ 1

#include <exception>
#include <type_traits>
#include <utility>

#include "concurrency/one_time_notification.h"

#include "exe/futures/fun/combine/seq/inline.h"
#include "exe/futures/fun/mutator/mutator.h"
#include "exe/futures/fun/syntax/pipe.h"
#include "exe/futures/fun/traits/map.h"

#include "result/result.h"

#include "utils/abort.h"

namespace exe::futures {

namespace pipe {

struct [[nodiscard]] Get : Mutator {
	template <typename T>
	auto mutate(SemiFuture<T> f)
	{
		return mutate(::std::move(f) | futures::inLine());
	}

	// TODO: my eyes hurt
	// how to think about exceptions, please tell me
	template <typename T>
	auto mutate(Future<T> f)
	{
		::utils::result<T> result;

		// loss future at exception
		::concurrency::OneTimeNotification result_is_ready;

		// loss future at exception
		setCallback(
			::std::move(f),
			[&](::utils::result<T> &&res) noexcept {
				if constexpr (traits::is_nothrow_move_constructible_v<T>) {
					result = ::std::move(res);
				} else try {
					result = ::std::move(res);
				} catch (...) {
					result = ::std::current_exception();
				}

				try {
					result_is_ready.notify();
				} catch (...) {
					UTILS_ABORT("exception in async callback futures::get");
				}
			}
		);

		try {
			result_is_ready.wait();
		} catch (...) {
			UTILS_ABORT("exception while sync wait of future");
		}

		if constexpr (traits::is_nothrow_move_constructible_v<T>) {
			return result;
		} else try {
			return result;
		} catch (...) {
			return ::std::current_exception();
		}
	}
};

} // namespace pipe

inline auto get() noexcept
{
	return pipe::Get{};
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_TERMINATE_GET_H_ */
