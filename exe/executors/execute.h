// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_EXECUTORS_EXECUTE_H_
#define DDV_EXE_EXECUTORS_EXECUTE_H_ 1

#include <type_traits>
#include <utility>

#include "exe/executors/executor.h"
#include "exe/executors/task.h"

#include "utils/abort.h"

namespace exe::executors {

namespace detail {

template <typename F>
requires (
	::std::is_class_v<F> &&
	::std::is_nothrow_destructible_v<F> &&
	::std::is_nothrow_invocable_v<F &> &&
	::std::is_void_v<::std::invoke_result_t<F &>>
)
class Task : public TaskBase {
protected:
	[[no_unique_address]] F fn_;

public:
	Task(F const &f)
		noexcept (::std::is_nothrow_copy_constructible_v<F>)
		requires (::std::is_copy_constructible_v<F>)
		: fn_(f)
	{}

	Task(F &&f)
		noexcept (::std::is_nothrow_move_constructible_v<F>)
		requires (::std::is_move_constructible_v<F>)
		: fn_(::std::move(f))
	{}

	void run() noexcept override
	{
		fn_();
		delete this;
	}
};

} // namespace detail

template <typename F>
void execute(IExecutor &where, F &&f)
{
	auto task = ::new detail::Task<F>(::std::forward<F>(f));

	// TODO: exception handling
	try {
		where.execute(task);
	} catch (...) {
		UTILS_ABORT("exception during executors::execute");
	}
}

} // namespace exe::executors

#endif /* DDV_EXE_EXECUTORS_EXECUTE_H_ */
