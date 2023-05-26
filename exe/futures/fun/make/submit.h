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
#include "exe/futures/fun/make/contract/contract.h"
#include "exe/futures/fun/combine/seq/via.h"
#include "exe/futures/fun/traits/map.h"

#include "result/result.h"

#include "utils/defer.h"

namespace exe::futures {

namespace detail {

template <typename F>
class Task : executors::TaskBase {
private:
	using T = traits::map_result_t<F &>::value_type;

	[[no_unique_address]] F fn_;
	Promise<T> p_;

public:
	Task(F fun, Promise<T> p) noexcept
		: fn_(::std::move(fun))
		, p_(::std::move(p))
	{}

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
		if constexpr (::std::is_nothrow_invocable_v<F &>) {
			invoke();
		} else try {
			invoke();
		} catch (...) {
			fail();
		}
	}

private:
	void invoke()
	{
		::std::move(p_).setResult(::std::invoke(fn_));
		destroySelf();
	}

	void fail() noexcept
	{
		::std::move(p_).setError(::std::current_exception());
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

	auto contract = Contract<T>();

	auto rollback = ::utils::scope_guard(
		[&]() noexcept {
			::std::move(contract).cancel();
		}
	);

	auto task =
		::new detail::Task(::std::move(fun), ::std::move(contract).p);

	rollback.disable();

	task->submit(where);

	return ::std::move(contract).f | futures::via(where);
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_MAKE_SUBMIT_H_ */
