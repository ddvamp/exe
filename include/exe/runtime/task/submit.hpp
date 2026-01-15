//
// submit.hpp
// ~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_RUNTIME_TASK_SUBMIT_HPP_INCLUDED_
#define DDVAMP_EXE_RUNTIME_TASK_SUBMIT_HPP_INCLUDED_ 1

#include <exe/runtime/task/scheduler.hpp>
#include <exe/runtime/task/task.hpp>

#include <util/macro.hpp>

#include <memory>
#include <type_traits>
#include <utility>

namespace exe::runtime::task {

namespace detail {

template <typename Fn>
class SubmitTask final : public TaskBase {
  static_assert(::std::is_nothrow_destructible_v<Fn>);
  static_assert(::std::is_nothrow_invocable_v<::std::decay_t<Fn>>);

 private:
  UTIL_NO_UNIQUE_ADDRESS Fn fn_;

 public:
  explicit SubmitTask(Fn fn) : fn_(::std::move(fn)) {}

  void Run() && noexcept override {
    ::std::move(fn_)();
    delete this;
  }
};

} // namespace detail

/* fn is destroyed in case of an exception from scheduler */

template <typename Fn>
requires (::std::is_nothrow_destructible_v<::std::decay_t<Fn>> &&
          ::std::is_nothrow_invocable_v<::std::decay_t<Fn>> &&
          ::std::is_void_v<::std::invoke_result_t<::std::decay_t<Fn>>>)
void Submit(IScheduler &where, Fn &&fn) {
  using Task = detail::SubmitTask<::std::decay_t<Fn>>;
  auto task = ::std::make_unique<Task>(::std::forward<Fn>(fn));
  where.Submit(task.get());
  task.release();
}

} // namespace exe::runtime::task

#endif /* DDVAMP_EXE_RUNTIME_TASK_SUBMIT_HPP_INCLUDED_ */
