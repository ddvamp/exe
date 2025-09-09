// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_EXECUTORS_EXECUTE_H_
#define DDV_EXE_EXECUTORS_EXECUTE_H_ 1

#include <memory>
#include <type_traits>
#include <utility>

#include "exe/runtime/task/scheduler.hpp"
#include "exe/runtime/task/task.hpp"

#include "util/macro.hpp"

namespace exe::runtime {

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
	UTIL_NO_UNIQUE_ADDRESS F fn_;

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
		destroySelf();
	}

private:
	void destroySelf() noexcept
	{
		delete this;
	}
};

} // namespace detail

template <concepts::Scheduler E, typename Fn>
void submit(E &where, Fn &&f)
{
	using Task = detail::Task<::std::remove_cvref_t<Fn>>;

	auto task = ::std::make_unique<Task>(::std::forward<Fn>(f));

	where.submit(task.get());

	task.release();
}

} // namespace exe::runtime

#endif /* DDV_EXE_EXECUTORS_EXECUTE_H_ */
