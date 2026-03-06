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
#include <exe/future2/model/continuation.hpp>
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

  template <typename InputType>
  inline static constexpr bool ValidInput = concepts::FutureValue<InputType>;

  template <concepts::ValidInput<Via> InputType>
  using ValueType = InputType;

  template <concepts::ValidInput<Via> InputType,
            concepts::Consumer<ValueType<InputType>> Consumer>
  struct CombineStep {
    Consumer cons_;
    Via &data_;

    CombineStep(Consumer &&c, Via &v) noexcept
        : cons_(::std::forward<Consumer>(c))
        , data_(v) {}

    void Continue(InputType &&v, State) && noexcept {
      ::std::move(cons_).Continue(::std::move(v), {.sched = data_.sched_});
    }

    void Cancel(State) && noexcept {
      ::std::move(cons_).Cancel({.sched = data_.sched_});
    }
  };
};

} // namespace exe::future::thunk

#endif /* DDVAMP_EXE_FUTURE_THUNK_SEQ_VIA_HPP_INCLUDED_ */
