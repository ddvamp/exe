//
// future.hpp
// ~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_TYPE_FUTURE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_TYPE_FUTURE_HPP_INCLUDED_ 1

#include <exe/future/fun/concept/future_value.hpp>
#include <exe/future/fun/core/contract_fwd.hpp>
#include <exe/future/fun/core/operator_fwd.hpp>
#include <exe/future/fun/detail/shared_state.hpp>
#include <exe/future/fun/type/future_fwd.hpp>
#include <exe/runtime/inline.hpp>

namespace exe::future {

// [TODO]: move to util, ?better name
struct Noop {
  constexpr void operator() (auto &&...) const noexcept {}
};

/**
 *  Class for representing future value
 *
 *  Future is moveable value type
 */
template <concepts::FutureValue T>
class [[nodiscard]] SemiFuture : protected detail::HoldState<T> {
  friend core::Contract<T>;
  friend core::Operator;

 protected:
  using Base = detail::HoldState<T>;
  using Base::Base;

 public:
  using ValueType = T;

  ~SemiFuture() {
    if (this->HasState()) [[unlikely]] {
      auto const state = this->Release();
      state->SetScheduler(runtime::GetInline());
      state->SetCallback(Noop{});
    }
  }

  SemiFuture(SemiFuture const &) = delete;
  void operator= (SemiFuture const &) = delete;

  SemiFuture(SemiFuture &&) = default;
  void operator= (SemiFuture &&) = delete;
};

template <concepts::FutureValue T>
class [[nodiscard]] Future : public SemiFuture<T> {
  friend core::Operator;

 private:
  using SemiFuture<T>::SemiFuture;
};

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_TYPE_FUTURE_HPP_INCLUDED_ */
