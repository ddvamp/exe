// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_MAKE_SUBMIT_H_
#define DDV_EXE_FUTURES_FUN_MAKE_SUBMIT_H_ 1

#include <exception>
#include <memory>
#include <type_traits>
#include <utility>

#include "exe/executors/executor.h"
#include "exe/futures/fun/make/contract/contract.h"
#include "exe/futures/fun/combine/seq/via.h"
#include "exe/futures/fun/traits/map.h"

#include "result/result.h"

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
		destroySelf();
	}

	void reset() noexcept
	{
		::std::move(p_).setResult(::utils::error(nullptr));
		destroySelf();
	}

private:
	void destroySelf() noexcept
	{
		delete this;
	}
};

} // namespace detail

template <executors::concepts::Executor E, typename Fn>
auto submit(E &where, Fn fn)
	requires (
		::std::is_nothrow_destructible_v<Fn> &&
		::std::is_invocable_v<Fn &> &&
		::utils::is_result_v<traits::map_result_t<Fn &>>
	)
{
	using T = traits::map_result_t<Fn &>::value_type;

	auto contract = Contract<T>();

	auto task =
		[]<typename Task, typename Deleter>
		(Task *task, Deleter deleter) noexcept {
			return ::std::unique_ptr<Task, Deleter>(task, deleter);
		}(
			::new detail::Task(::std::move(fn), ::std::move(contract).p),
			[](auto *ptr) noexcept { ptr->reset(); }
		);

	where.submit(task.get());

	task.release();

	if constexpr (executors::concepts::NothrowExecutor<E>) {
		return ::std::move(contract).f | futures::via(where);
	} else {
		return ::std::move(contract).f;
	}
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_MAKE_SUBMIT_H_ */
