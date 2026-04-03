//
// promise_state.hpp
// ~~~~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_DETAIL_EAGER_PROMISE_STATE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_DETAIL_EAGER_PROMISE_STATE_HPP_INCLUDED_ 1

#include <exe/future2/scheduler.hpp>
#include <exe/future2/detail/eager/eager_state_impl.hpp>
#include <exe/future2/model/consumer.hpp>
#include <exe/future2/model/state.hpp>

namespace exe::future::detail {

template <typename T>
class PromiseState final : public EagerStateImpl<T, PromiseState<T>> {
 private:
  using Base = PromiseState::EagerStateImpl;

 public:
  using Base::SetValue;
  using Base::SetCancel;

 private:
  // IBoxedThunk
  void Start(IConsumer<T> &c, Scheduler &s) && noexcept override {
    Base::SetState(State{s});
    Base::SetConsumer(c);
  }
};

} // namespace exe::future::detail

#endif /* DDVAMP_EXE_FUTURE_DETAIL_EAGER_PROMISE_STATE_HPP_INCLUDED_ */
