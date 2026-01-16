//
// promise_state.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_DETAIL_PROMISE_STATE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_DETAIL_PROMISE_STATE_HPP_INCLUDED_ 1

#include <exe/future/fun/core/contract_fwd.hpp>
#include <exe/future/fun/core/future_state.hpp>
#include <exe/future/fun/result/error.hpp>
#include <exe/future/fun/result/result.hpp>

#include <util/debug/assert.hpp>

#include <optional>
#include <utility>

namespace exe::future::detail {

template <typename T>
class PromiseState final : private core::FutureState<T> {
  friend core::Contract<T>;

 private:
  ::std::optional<Result<T>> result_;

 public:
  void SetResult(Result<T> &&result) noexcept {
    result_.emplace(::std::move(result));
    this->TrySchedule();
  }

  void SetValue(T &&t) noexcept {
    SetResult(result::Ok(::std::move(t)));
  }

  void SetError(Error &&e) noexcept {
    UTIL_ASSERT(e, "Empty error for promise");
    SetResult(result::Err<T>(::std::move(e)));
  }

 private:
  // TaskBase
  void Run() && noexcept override {
    (*::std::move(this->callback_))(*::std::move(result_));
    delete this;
  }
};

} // namespace exe::future::detail

#endif /* DDVAMP_EXE_FUTURE_FUN_DETAIL_PROMISE_STATE_HPP_INCLUDED_ */
