//
// map.hpp
// ~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_THUNK_SEQ_MAP_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_THUNK_SEQ_MAP_HPP_INCLUDED_ 1

#include <exe/future2/scheduler.hpp>
#include <exe/future2/concept/valid_input.hpp>
#include <exe/future2/model/continuation.hpp>
#include <exe/future2/model/future_value.hpp>
#include <exe/future2/model/state.hpp>

#include <type_traits>
#include <utility>

namespace exe::future::thunk {

template <typename Mapper>
class [[nodiscard]] Map {
 private:
  Mapper mapper_;

 public:
  ~Map() = default;

  Map(Map const &) = delete;
  void operator= (Map const &) = delete;

  // Move-out only
  Map(Map &&) = default;
  void operator= (Map &&) = delete;

 public:
  explicit Map(Mapper m) : mapper_(::std::move(m)) {}

  /* Combinator */

  template <concepts::FutureValue InputType>
  inline static constexpr bool ValidInput =
      ::std::is_invocable_v<Mapper, InputType>;

  template <concepts::ValidInput<Map> InputType>
  using ValueType = ::std::invoke_result_t<Mapper, InputType>;

  template <concepts::ValidInput<Map> InputType,
            concepts::Continuation<ValueType<InputType>> Consumer>
  class [[nodiscard]] Continuation {
   private:
    Via &data_;
    Consumer cons_;

   public:
    template <typename ...Args>
    requires (::std::is_nothrow_constructible_v<Consumer, Args &...>)
    Continuation(Via &r, Args &...args) noexcept : data_(r) , cons_(args...) {}

    void Continue(InputType &&val, State) && noexcept {
      ::std::move(cons_).Continue(::std::move(val), State{data_.sched_});
    }

    void Cancel(State) && noexcept {
      ::std::move(cons_).Cancel(State{data_.sched_});
    }

    /**/

    auto &CancelSource() & noexcept {
      return cons_.CancelSource();
    }
  };
};

} // namespace exe::future::thunk

#endif /* DDVAMP_EXE_FUTURE_THUNK_SEQ_MAP_HPP_INCLUDED_ */
