//
// promise.hpp
// ~~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_TYPE_PROMISE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_TYPE_PROMISE_HPP_INCLUDED_ 1

#include <exe/future/fun/concept/future_value.hpp>
#include <exe/future/fun/core/contract_fwd.hpp>
#include <exe/future/fun/core/hold_state.hpp>
#include <exe/future/fun/detail/promise_state.hpp>
#include <exe/future/fun/result/error.hpp>
#include <exe/future/fun/result/result.hpp>
#include <exe/future/fun/type/promise_fwd.hpp>

#include <exception>
#include <utility>

namespace exe::future {

class BrokenPromise : public ::std::exception {
 public:
  [[nodiscard]] char const *what() const noexcept override {
    return "exe::future::Promise was not used at destruction time";
  }
};

template <concepts::FutureValue T>
class Promise : protected core::HoldState<detail::PromiseState<T>> {
  friend core::Contract<T>;

 private:
  using Base = core::HoldState<detail::PromiseState<T>>;
  using Base::Base;

 public:
  ~Promise() {
    if (IsValid()) [[unlikely]] {
      auto const state = this->Release();
      state->SetError(MakeError(BrokenPromise()));
    }
  }

  Promise(Promise const &) = delete;
  void operator= (Promise const &) = delete;

  Promise(Promise &&) = default;
  void operator= (Promise &&) = delete;

 public:
  [[nodiscard("Pure")]] bool IsValid() const noexcept {
    return this->HasState();
  }

  void SetResult(Result<T> &&r) && noexcept {
    this->ReleaseChecked()->SetResult(::std::move(r));
  }

  void SetValue(T &&v) && noexcept {
    this->ReleaseChecked()->SetValue(::std::move(v));
  }

  void SetError(Error &&e) && noexcept {
    this->ReleaseChecked()->SetError(::std::move(e));
  }
};

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_TYPE_PROMISE_HPP_INCLUDED_ */
