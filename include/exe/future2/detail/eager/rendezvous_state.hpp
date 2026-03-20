//
// rendezvous_state.hpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_DETAIL_EAGER_RENDEZVOUS_STATE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_DETAIL_EAGER_RENDEZVOUS_STATE_HPP_INCLUDED_ 1

#include <exe/future2/model/consumer.hpp>
#include <exe/future2/model/state.hpp>

#include <concurrency/rendezvous.hpp>

#include <optional>
#include <utility>

namespace exe::future::detail {

template <typename T>
class RendezvousState {
 private:
  ::std::optional<T> result_;
  IConsumer<T> *consumer_;
  ::std::optional<State> state_;
  ::concurrency::Rendezvous rendezvous_;

 protected:
  void SetState(State s) {
    state_.emplace(s);
  }

  /* Producer */

  void SetValue(T &&t) {
    result_.emplace(::std::move(t));
    TryContinue();
  }

  void SetCancel() {
    // result_ is empty
    TryCancel();
  }

  /* Consumer */

  bool SetConsumer(IConsumer<T> &c) {
    consumer_ = &c;
    return TryContinue();
  }

 private:
  bool TryContinue() {
    if (rendezvous_.Arrive()) {
      ::std::move(*consumer_).Continue(*::std::move(result_), *state_);
      return true;
    }

    return false;
  }

  void TryCancel() {
    if (rendezvous_.Arrive()) {
      ::std::move(*consumer_).Cancel(*state_);
    }
  }
};

} // namespace exe::future::detail

#endif /* DDVAMP_EXE_FUTURE_DETAIL_EAGER_RENDEZVOUS_STATE_HPP_INCLUDED_ */
