// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_MUTATOR_MUTATOR_H_
#define DDV_EXE_FUTURES_FUN_MUTATOR_MUTATOR_H_ 1

#include <utility>

#include "exe/executors/executor.h"
#include "exe/futures/fun/mutator/fwd.h"
#include "exe/futures/fun/types/future.h"

class exe::futures::detail::Mutator {
protected:
	template <concepts::Future>
	inline static constexpr bool hasExecutor = false;

	template <typename T>
	inline static constexpr bool hasExecutor<Future<T>> = true;

	template <typename T>
	[[nodiscard]] static executors::IExecutor &getExecutor(
		Future<T> const &f) noexcept
	{
		return f.getState().getExecutor();
	}

	template <typename T>
	static Future<T> setExecutor(SemiFuture<T> f,
		executors::IExecutor &where) noexcept
	{
		f.getState().setExecutor(where);
		return Future<T>(f.release());
	}

	template <typename T>
	static void setCallback(Future<T> f, Future<T>::Callback &&cb) noexcept
	{
		f.release()->setCallback(::std::move(cb));
	}
};

#endif /* DDV_EXE_FUTURES_FUN_MUTATOR_MUTATOR_H_ */
