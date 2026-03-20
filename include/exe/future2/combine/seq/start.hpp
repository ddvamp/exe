//
// start.hpp
// ~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_COMBINE_SEQ_START_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_COMBINE_SEQ_START_HPP_INCLUDED_ 1

#include <exe/future2/cancel.hpp>
#include <exe/future2/scheduler.hpp>
#include <exe/future2/thunk.hpp>
#include <exe/future2/core/proxy.hpp>
#include <exe/future2/detail/eager/eager_state_impl.hpp>
#include <exe/future2/model/consumer.hpp>
#include <exe/future2/model/state.hpp>
#include <exe/future2/thunk/seq/box.hpp>
#include <exe/future2/trait/value_of.hpp>
#include <exe/future2/trait/computation.hpp>
#include <exe/future2/type/future.hpp>

#include <utility>

namespace exe::future {

namespace detail {

template <concepts::SomeFuture F>
class EagerState final
    : public EagerStateImpl<trait::ValueOf<F>, EagerState<F>> {
 private:
  using Base = EagerState::EagerStateImpl;
  using Comp = trait::Computation<F, core::Proxy<EagerState>>;

  using InputType = trait::ValueOf<F>;

  Comp comp_;

 public:
  explicit EagerState(F &&f) noexcept : comp_({*this}, ::std::move(f)) {}

  void Run(Scheduler &s) noexcept {
    ::std::move(comp_).Start(s);
  }

  /* Continuation */

  void Continue(InputType &&i, State s) && noexcept {
    Base::SetState(s);
    Base::SetValue(::std::move(i));
  }

  void Cancel(State s) && noexcept {
    Base::SetState(s);
    Base::SetCancel();
  }

  cancel::CancelSource &CancelSource() & noexcept {
    return *this;
  }

 private:
  // IBoxedThunk
  void Start(IConsumer<InputType> &c, Scheduler &) && noexcept override {
    Base::SetConsumer(c);
  }
};

} // namespace detail

namespace pipe {

struct [[nodiscard]] Start {
  Scheduler &s;

  template <concepts::SomeFuture F>
  concepts::Future<trait::ValueOf<F>> auto Pipe(F &&f) {
    auto state = ::new detail::EagerState(::std::move(f));
    state->Run(s);
    return Thunk(thunk::Box(state));
  }
};

} // namespace pipe

inline auto Start(Scheduler &s) {
  return pipe::Start{s};
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_COMBINE_SEQ_START_HPP_INCLUDED_ */
