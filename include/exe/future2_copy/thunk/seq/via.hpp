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

#include <type_traits>
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

  template <typename InputType>
  inline static constexpr bool ValidInput = concepts::FutureValue<InputType>;

  template <concepts::ValidInput<Via> InputType>
  using ValueType = InputType;

  template <concepts::ValidInput<Via> InputType,
            concepts::Continuation<ValueType<InputType>> Consumer>
  requires (::std::is_nothrow_destructible_v<Consumer>)
  class [[nodiscard]] Continuation : private Consumer {
   private:
    Scheduler &sched_;

   public:
    ~Continuation() = default;

    Continuation(Continuation const &) = delete;
    void operator= (Continuation const &) = delete;

    Continuation(Continuation &&) = delete;
    void operator= (Continuation &&) = delete;

   public:
    template <typename ...Args>
    requires (::std::is_nothrow_constructible_v<Consumer, Args...>)
    explicit Continuation(Via &&v, Args &&...args) noexcept
        : Consumer(::std::forward<Args>(args)...)
        , sched_(v.sched_) {}

    /* Continuation */

    void Continue(InputType &&v, State) && noexcept {
      static_cast<Consumer &&>(*this).Continue(::std::move(v),
                                               {.sched = sched_});
    }

    void Cancel(State) && noexcept {
      static_cast<Consumer &&>(*this).Cancel({.sched = sched_});
    }

    using Consumer::CancelSource;
  };
};

} // namespace exe::future::thunk

#endif /* DDVAMP_EXE_FUTURE_THUNK_SEQ_VIA_HPP_INCLUDED_ */
