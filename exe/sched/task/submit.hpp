// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_EXE_SCHED_TASK_SUBMIT_HPP_INCLUDED_
#define DDVAMP_EXE_SCHED_TASK_SUBMIT_HPP_INCLUDED_ 1

#include <memory>
#include <utility>

#include <util/macro.hpp>
#include <util/type_traits.hpp>

#include "scheduler.hpp"
#include "task.hpp"

namespace exe::sched::task {

namespace detail {

template <typename Fn>
requires (::util::is_all_of_v<::std::is_class_v<Fn>,
                              ::std::is_nothrow_destructible_v<Fn>,
                              ::std::is_nothrow_move_constructible_v<Fn>,
                              ::std::is_nothrow_invocable_v<Fn &&> &&
                              ::std::is_void_v<::std::invoke_result_t<Fn &&>>>)
class Task final : public TaskBase {
 private:
  UTIL_NO_UNIQUE_ADDRESS Fn fn_;

 public:
  Task(Fn fn) noexcept : fn_(::std::move(fn)) {}

  void Run() noexcept override {
    ::std::move(fn_)();
    DestroySelf();
  }

 private:
  void DestroySelf() noexcept {
    delete this;
  }
};

}  // namespace detail

// TODO: What to do with the loss of fn on exception?
template <concepts::UnsafeScheduler S, typename Fn>
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

}  // namespace exe::sched::task

#endif  /* DDVAMP_EXE_SCHED_TASK_SUBMIT_HPP_INCLUDED_ */
