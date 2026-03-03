//
// via.hpp
// ~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_THUNK_SEQ_VIA_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_THUNK_SEQ_VIA_HPP_INCLUDED_ 1

#include <exe/future2/scheduler.hpp>
#include <exe/future2/concept/valid_input.hpp>
#include <exe/future2/model/control.hpp>
#include <exe/future2/model/future_value.hpp>
#include <exe/future2/model/state.hpp>

#include <utility>

namespace exe::future::thunk {

class [[nodiscard]] Via {
 private:
  Scheduler &sched_;

 public:
  ~Via() = default;

  Via(Via const &) = delete;
  void operator= (Via const &) = delete;

  // Move-out only
  Via(Via &&) = default;
  void operator= (Via &&) = delete;

 public:
  explicit Via(Scheduler &s) noexcept : sched_(s) {}

  /* Combinator */

  template <typename V>
  inline static constexpr bool ValidInput = concepts::FutureValue<V>;

  template <concepts::ValidInput<Via> InputType>
  using ValueType = InputType;

  template <concepts::ValidInput<Via> InputType,
            concepts::Control<ValueType<InputType>, Via> Control>
  class [[nodiscard]] Step {
   private:
    Control ctrl_;

   public:
    explicit Step(Control c) noexcept : ctrl_(::std::forward<Control>(c)) {}

    void Execute(InputType &&val, State) && noexcept {
      auto &sched = ctrl_.Data().sched_;
      ::std::move(ctrl_).Return(::std::move(val), State{sched});
    }

    void Execute(State) && noexcept {
      auto &sched = ctrl_.Data().sched_;
      ::std::move(ctrl_).Cancel(State{sched});
    }
  };
};

} // namespace exe::future::thunk

#endif /* DDVAMP_EXE_FUTURE_THUNK_SEQ_VIA_HPP_INCLUDED_ */
