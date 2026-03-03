#pragma once

#include "cancel.hpp"

#include <exe/future/fun/type/scheduler.hpp>
#include <exe/future/fun/result/error.hpp>

namespace exe::future::lazy {

struct State {
  Scheduler *sched;
};

namespace core {

template <typename Value>
struct IConsumer {
 protected:
  // Lifetime cannot be controlled via IConsumer<> *
  ~IConsumer() = default;

 public:
  virtual void Continue(Value &&, State) && noexcept = 0;

  virtual void Failure(Error &&, State) && noexcept = 0;

  virtual void Cancel(State) && noexcept = 0;

  virtual cancel::CancelSource &CancelSource() & noexcept = 0;

  /**/

  IConsumer &&FailureAware() && noexcept {
    return ::std::move(*this);
  }

  IConsumer &&CancelAware() && noexcept {
    return ::std::move(*this);
  }
};

} // namespace core

} // namespace exe::future::lazy
