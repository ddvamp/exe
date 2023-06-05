// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_MAKE_SUBMIT_H_
#define DDV_EXE_FUTURES_FUN_MAKE_SUBMIT_H_ 1

#include <exception>
#include <type_traits>
#include <utility>

#include "exe/executors/executor.h"
#include "exe/futures/fun/make/contract/contract.h"
#include "exe/futures/fun/combine/seq/via.h"
#include "exe/futures/fun/traits/map.h"

#include "result/result.h"

#include "utils/abort.h"

namespace exe::futures {

namespace detail {

template <typename F>
class Task : public executors::TaskBase {
private:
	using T = traits::map_result_t<F &>::value_type;

	[[no_unique_address]] F fn_;
	Promise<T> p_;

public:
	Task(F fun, Promise<T> p) noexcept
		: fn_(::std::move(fun))
		, p_(::std::move(p))
	{}

	void run() noexcept override
	{
		::std::move(p_).setResult(::utils::map_safely(fn_));
		delete this;
	}
};

} // namespace detail

template <typename Fn>
auto submit(executors::IExecutor &where, Fn fn)
	requires (
		::std::is_nothrow_destructible_v<Fn> &&
		::std::is_invocable_v<Fn &> &&
		::utils::is_result_v<traits::map_result_t<Fn &>>
	)
{
	using T = traits::map_result_t<Fn &>::value_type;

	auto contract = Contract<T>();

	auto task = ::new detail::Task(::std::move(fn), ::std::move(contract).p);

	try {
		where.execute(task);
	} catch (...) {
		UTILS_ABORT("exception while submitting a task in executor");
	}

	return ::std::move(contract).f | futures::via(where);
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_MAKE_SUBMIT_H_ */
