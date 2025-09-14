//
// submit.hpp
// ~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_RUNTIME_TASK_SUBMIT_HPP_INCLUDED_
#define DDVAMP_EXE_RUNTIME_TASK_SUBMIT_HPP_INCLUDED_ 1

#include <exe/runtime/task/scheduler.hpp>
#include <exe/runtime/task/task.hpp>

#include <util/macro.hpp>
#include <util/type_traits.hpp>

#include <memory>
#include <utility>

namespace exe::runtime::task {

namespace detail {

template <typename T>
concept SuitableForTask =
		::std::is_object_v<T> && !::util::is_qualified_v<T> &&
    ::std::is_nothrow_destructible_v<T> &&
    ::std::is_nothrow_invocable_v<T &&> &&
    ::std::is_void_v<::std::invoke_result_t<T &&>>;

template <SuitableForTask Fn>
class Task final : public TaskBase {
 private:
  UTIL_NO_UNIQUE_ADDRESS Fn fn_;

 public:
  explicit Task(Fn fn) noexcept(::std::is_nothrow_move_constructible_v<Fn>)
      : fn_(::std::move(fn)) {}

  void Run() noexcept override {
    ::std::move(fn_)();
    DestroySelf();
  }

 private:
  void DestroySelf() const noexcept {
    delete this;
  }
};

} // namespace detail

/* fn is destroyed in case of an exception from scheduler */

template <concepts::Scheduler S, typename Fn>
void Submit(S &scheduler, Fn &&fn) {
	using Task = detail::Task<::std::remove_cvref_t<Fn>>;
  auto task = ::std::make_unique<Task>(::std::forward<Fn>(fn));
  scheduler.Submit(task.get());
  task.release();
}

template <concepts::SafeScheduler S, typename Fn>
void Submit(S &scheduler, Fn &&fn) {
  auto const task = ::new detail::Task(::std::forward<Fn>(fn));
  scheduler.Submit(task);
}

} // namespace exe::runtime::task

#endif /* DDVAMP_EXE_RUNTIME_TASK_SUBMIT_HPP_INCLUDED_ */
