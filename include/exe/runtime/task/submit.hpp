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

namespace concepts {

template <typename T>
concept SuitableForSubmit =
    ::std::is_move_constructible_v<T> &&
    ::std::is_nothrow_destructible_v<T> &&
    ::std::is_nothrow_invocable_v<T &&> &&
    ::std::is_void_v<::std::invoke_result_t<T &&>>;

} // namespace concepts

namespace detail {

template <concepts::SuitableForSubmit Fn>
class Task final : public TaskBase {
 private:
  UTIL_NO_UNIQUE_ADDRESS Fn fn_;

 public:
  explicit Task(Fn &&fn) noexcept(::std::is_nothrow_move_constructible_v<Fn>)
      : fn_(::std::forward<Fn>(fn)) {}

  void Run() noexcept override {
    ::std::forward<Fn>(fn_)();
    DestroySelf();
  }

 private:
  void DestroySelf() const noexcept {
    delete this;
  }
};

} // namespace detail

/* fn is destroyed in case of an exception from scheduler */

template <concepts::SuitableForSubmit Fn, concepts::Scheduler S>
void Submit(S &scheduler, Fn fn) {
  auto task = ::std::make_unique<detail::Task<Fn>>(::std::forward<Fn>(fn));
  scheduler.Submit(task.get());
  task.release();
}

template <concepts::SuitableForSubmit Fn, concepts::SafeScheduler S>
void Submit(S &scheduler, Fn fn) {
  auto const task = ::new detail::Task<Fn>(::std::forward<Fn>(fn));
  scheduler.Submit(task);
}

} // namespace exe::runtime::task

#endif /* DDVAMP_EXE_RUNTIME_TASK_SUBMIT_HPP_INCLUDED_ */
