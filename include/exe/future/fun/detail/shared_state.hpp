//
// shared_state.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_DETAIL_SHARED_STATE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_DETAIL_SHARED_STATE_HPP_INCLUDED_ 1

#include <exe/future/fun/detail/callback.hpp>
#include <exe/future/fun/result/error.hpp>
#include <exe/future/fun/result/result.hpp>
#include <exe/future/fun/type/scheduler.hpp>
#include <exe/runtime/task/task.hpp>

#include <concurrency/rendezvous.hpp>
#include <util/debug.hpp>
#include <util/type_traits.hpp>

#include <optional>
#include <utility>

namespace exe::future::detail {

/**
 *  Shared state for Future and Promise
 *
 *  Allows you to pass result from Promise to Future and callback in
 *  the opposite direction, as well as specify where callback will be called
 */
template <typename T>
class SharedState : public runtime::task::TaskBase {
  static_assert(::std::is_nothrow_destructible_v<T>);
  static_assert(::std::is_nothrow_move_constructible_v<T>);

 private:
  ::std::optional<Result<T>> result_;
  ::std::optional<Callback<T>> callback_;
  ::concurrency::Rendezvous rendezvous_;
  Scheduler *scheduler_ = nullptr;

 public:
  [[nodiscard]] static SharedState *Create() {
    return ::new SharedState();
  }

  static void Destroy(SharedState *state) noexcept {
    state->DestroySelf();
  }

  [[nodiscard("Pure")]] Scheduler *GetScheduler() const noexcept {
    return scheduler_;
  }

  void SetScheduler(Scheduler &where) noexcept {
    scheduler_ = &where;
  }

  void SetResult(Result<T> &&result) noexcept {
    result_.emplace(::std::move(result));
    TrySchedule();
  }

  void SetValue(T &&t) noexcept {
    SetResult(result::Ok(::std::move(t)));
  }

  void SetError(Error &&e) noexcept {
    UTIL_ASSERT(e, "Empty error for promise");
    SetResult(result::Err<T>(::std::move(e)));
  }

  void SetCallback(Callback<T> &&callback) noexcept {
    UTIL_ASSERT(callback, "Empty callback for future");
    callback_.emplace(::std::move(callback));
    TrySchedule();
  }

 private:
  // TaskBase
  void Run() && noexcept override {
    (*::std::move(callback_))(*::std::move(result_));
    DestroySelf();
  }

  void TrySchedule() noexcept {
    if (rendezvous_.Arrive()) {
      scheduler_->Submit(this);
    }
  }

  void DestroySelf() noexcept {
    delete this;
  }
};

template <typename T>
class HoldState {
 private:
  using State = SharedState<T>;

  State *state_;

 protected:
  ~HoldState() {
    UTIL_ASSERT(!HasState(),
                "HoldState is not used at the time of destruction");
  }

  HoldState(HoldState const &) = delete;
  void operator= (HoldState const &) = delete;

  HoldState(HoldState &&that) noexcept : state_(that.Release()) {}
  void operator= (HoldState &&) = delete;

 protected:
  explicit HoldState(State *state) noexcept : state_(state) {}

  [[nodiscard("Pure")]] bool HasState() const noexcept {
    return state_;
  }

  /* Unchecked */

  [[nodiscard("Pure")]] State *GetState() noexcept {
    return state_;
  }

  [[nodiscard("Pure")]] State const *GetState() const noexcept {
    return state_;
  }

  [[nodiscard]] State *Release() noexcept {
    return ::std::exchange(state_, nullptr);
  }

  /* Checked */

  [[nodiscard("Pure")]] State *GetStateChecked() noexcept {
    UTIL_DEBUG_RUN(CheckState);
    return GetState();
  }

  [[nodiscard("Pure")]] State const *GetStateChecked() const noexcept {
    UTIL_DEBUG_RUN(CheckState);
    return GetState();
  }

  [[nodiscard]] State *ReleaseChecked() noexcept {
    UTIL_DEBUG_RUN(CheckState);
    return Release();
  }

 private:
  [[maybe_unused]] void CheckState() const noexcept {
    UTIL_ASSERT(HasState(),
                "HoldState does not hold shared state when it is expected");
  }
};

} // namespace exe::future::detail

#endif /* DDVAMP_EXE_FUTURE_FUN_DETAIL_SHARED_STATE_HPP_INCLUDED_ */
