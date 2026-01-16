//
// future_state.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_CORE_FUTURE_STATE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_CORE_FUTURE_STATE_HPP_INCLUDED_ 1

#include <exe/future/fun/detail/callback.hpp>
#include <exe/future/fun/type/scheduler.hpp>
#include <exe/runtime/task/task.hpp>

#include <concurrency/rendezvous.hpp>
#include <util/debug/assert.hpp>

#include <optional>
#include <utility>

namespace exe::future::core {

template <typename T>
class FutureState : private runtime::task::TaskBase {
  static_assert(::std::is_nothrow_destructible_v<T>);
  static_assert(::std::is_nothrow_move_constructible_v<T>);

 private:
  ::concurrency::Rendezvous rendezvous_;
  Scheduler *scheduler_ = nullptr;

 protected:
  ::std::optional<detail::Callback<T>> callback_;

 public:
  [[nodiscard("Pure")]] Scheduler *GetScheduler() const noexcept {
    return scheduler_;
  }

  void SetScheduler(Scheduler &where) noexcept {
    scheduler_ = &where;
  }

  void SetCallback(detail::Callback<T> &&callback) noexcept {
    UTIL_ASSERT(callback, "SetCallback got an empty callback for future");
    callback_.emplace(::std::move(callback));
    TrySchedule();
  }

 protected:
  void TrySchedule() noexcept {
    if (rendezvous_.Arrive()) {
      scheduler_->Submit(this);
    }
  }
};

} // namespace exe::future::core

#endif /* DDVAMP_EXE_FUTURE_FUN_CORE_FUTURE_STATE_HPP_INCLUDED_ */
