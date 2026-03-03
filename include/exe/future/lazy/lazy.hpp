#pragma once

#include <exe/future/fun/core/adapt_call.hpp>
#include <exe/future/fun/core/concept/adapt_call.hpp>
#include <exe/future/fun/core/trait/adapt_call.hpp>
#include <exe/future/fun/result/result.hpp>
#include <exe/future/fun/result/error.hpp>
#include <exe/future/fun/result/unit.hpp>
#include <exe/future/fun/type/scheduler.hpp>
#include <exe/runtime/task/task.hpp>

#include <util/macro.hpp>
#include <util/debug/assert.hpp>
#include <util/mm/release_sequence.hpp>

#include <concepts>
#include <cstddef>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>

#include "consumer.hpp"

namespace exe::future::lazy {

template <typename ...Ts>
using Tail = Ts...[sizeof...(Ts) - 1];

namespace trait {

template <typename Thunk, typename Demand>
using Materialize =
    decltype(::std::declval<Thunk>().Materialize(::std::declval<Demand>()));

template <typename Thunk>
using ValueOf = Thunk::ValueType;

} // namespace trait

template <typename T>
using Demand = core::IConsumer<T> &;

namespace concepts {

template <typename T>
concept AsyncSafe =
    ::std::is_nothrow_destructible_v<T> &&       // destroy context
    ::std::is_nothrow_move_constructible_v<T> && // direct-init context
    ::std::is_nothrow_convertible_v<T &&, T>;    // copy-init context

template <typename T>
concept FutureValue =
    AsyncSafe<T> && // implies !::std::is_array_v<T>
    ::std::is_object_v<T> &&
    !::util::is_qualified_v<T>;


// [TODO]: ?AsyncSafe
template <typename C, typename V>
concept Continuation = requires (C c, V v, State s) {
  { ::std::move(c).Continue(::std::move(v), s) } noexcept
      -> ::std::same_as<void>;
  { ::std::move(c).Cancel(s) } noexcept -> ::std::same_as<void>;
  { c.CancelSource() } noexcept -> ::std::same_as<cancel::CancelSource &>;
};

// [TODO]: ?AsyncSafe
template <typename C>
concept Computation = requires (C c, Scheduler &s) {
  { ::std::move(c).Start(s) } noexcept -> ::std::same_as<void>;
};

// [TODO]: ?AsyncSafe
template <typename T>
concept Thunk = requires {
  requires FutureValue<typename T::ValueType>;
  requires Computation<
               typename T::template Computation<Demand<trait::ValueOf<T>>>>;
  requires ::std::is_nothrow_constructible_v<
               typename T::template Computation<Demand<trait::ValueOf<T>>>,
               T &&>;
};

} // namespace concepts

namespace core {

template <concepts::FutureValue InputType,
          concepts::Continuation<InputType> Consumer>
class Proxy {
 private:
  Consumer &cons_;

 public:
  explicit Proxy(Consumer &c) noexcept : cons_(c) {}

  /* Continuation */

  void Continue(InputType &&v, State s) && noexcept {
    ::std::move(cons_).Continue(::std::move(v), s);
  }

  void Cancel(State s) && noexcept {
    ::std::move(cons_).Cancel(s);
  }

  auto &CancelSource() & noexcept {
    return cons_.CancelSource();
  }
};

} // namespace core

} // namespace exe::future::lazy
