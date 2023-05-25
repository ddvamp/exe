// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_MAKE_SUBMIT_H_
#define DDV_EXE_FUTURES_FUN_MAKE_SUBMIT_H_ 1

#include <exception>
#include <functional>
#include <type_traits>
#include <utility>

#include "exe/executors/executor.h"
#include "exe/futures/fun/make/contract.h"
#include "exe/futures/fun/combine/seq/via.h"
#include "exe/futures/fun/traits/map.h"

#include "result/result.h"

#include "utils/defer.h"

namespace exe::futures {

namespace detail {

template <typename F>
struct Task : executors::TaskBase {
	using T = traits::map_result_t<F &>::value_type;

	[[no_unique_address]] F fn;
	Promise<T> p;

	void submit(executors::IExecutor &where) noexcept
	{
		try {
			where.execute(this);
		} catch (...) {
			fail();
		}
	}

	void run() noexcept override
	{
		if constexpr (
			::std::is_nothrow_invocable_v<F &> &&
			traits::is_nothrow_move_constructible_v<T>
		) {
			invoke();
		} else try {
			invoke();
		} catch (...) {
			fail();
		}
	}

	void invoke()
	{
		::std::move(p).setResult(::std::invoke(fn));
		destroySelf();
	}

	void fail() noexcept
	{
		::std::move(p).setError(::std::current_exception());
		destroySelf();
	}

	void destroySelf() noexcept
	{
		delete this;
	}
};

} // namespace detail

template <typename F>
auto submit(executors::IExecutor &where, F fun)
	requires (
		::std::is_nothrow_destructible_v<F> &&
		::std::is_invocable_v<F &> &&
		::utils::is_result_v<traits::map_result_t<F &>>
	)
{
	using T = traits::map_result_t<F &>::value_type;

	auto contract = Contract::open<T>();

	auto rollback = ::utils::scope_guard(
		[&contract]() noexcept {
			Contract::close(::std::move(contract));
		}
	);

	auto task =
		::new detail::Task{::std::move(fun), ::std::move(contract).promise};

	rollback.disable();

	task->submit(where);

	return ::std::move(contract).future | futures::via(where);
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_MAKE_SUBMIT_H_ */
