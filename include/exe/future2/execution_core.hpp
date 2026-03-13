//
// execution_core.hpp
// ~~~~~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_EXECUTION_CORE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_EXECUTION_CORE_HPP_INCLUDED_ 1

#include <exe/future2/cancel.hpp>
#include <exe/future2/scheduler.hpp>
#include <exe/future2/thunk_data.hpp>
#include <exe/future2/thunk_traits.hpp>
#include <exe/future2/core/proxy.hpp>
#include <exe/future2/model/continuation.hpp>
#include <exe/future2/model/state.hpp>
#include <exe/future2/trait/value_of.hpp>

#include <util/type_traits.hpp>

#include <cstddef>
#include <new> // ::std::launder
#include <utility>

namespace exe::future {

namespace detail {

// [TODO]: Variant-like storage
template <typename ...Ts>
struct VariadicStorage;

template <typename Consumer, typename ...Specs>
class ExecutionCoreBase {
 public:
  using Data = ThunkData<Specs...>;
  using Traits = Traits<Specs...>;

  template <::std::size_t I>
  using InputType = Type<Traits, I>;

  template <::std::size_t I>
  using Redirect = core::Redirect<ExecutionCoreBase, I>;

  template <::std::size_t I>
  using Step = Step<Traits, I, Redirect<I>>;

  template <::std::size_t I>
  using Spec = Specs...[I];

 private:
  static constexpr auto MaxSizeAlign() {
    return []<::std::size_t ...Is>(::std::index_sequence<Is...>) {
      return ::std::pair{::util::max_size_of_v<Step<Is>...>,
                         ::util::max_alignment_of_v<Step<Is>...>};
    }(::std::index_sequence_for<Specs...>{});
  }

  alignas (MaxSizeAlign().second) unsigned char buffer_[MaxSizeAlign().first];

  Consumer cons_;
  Data &data_;

 public:
  ExecutionCoreBase(Consumer &&c, Data &d)
      : cons_(::std::forward<Consumer>(c))
      , data_(d) {}

  void Start(Scheduler &s) && noexcept {
    auto &step = CreateStep<0>();
    ::std::move(step).Start(s);
  }

  template <::std::size_t I>
  void Continue(InputType<I> v, State s) && noexcept {
    DestroyStep<I>();

    auto &step = GetContinueStep<I + 1>();
    ::std::move(step).Continue(::std::move(v), s);
  }

  template <::std::size_t I>
  void Cancel(State s) && noexcept {
    DestroyStep<I>();

    auto &step = GetCancelStep<I + 1>();
    ::std::move(step).Cancel(s);
  }

  cancel::CancelSource &CancelSource() & noexcept {
    return cons_.CancelSource();
  }

 private:
  template <::std::size_t I>
  auto &GetContinueStep() noexcept {
    if constexpr (I == sizeof...(Specs)) {
      return cons_;
    } else {
      return CreateStep<I>();
    }
  }

  // [TODO]: ?Better trait for fast get of next cancel aware step
  template <::std::size_t I>
  auto &GetCancelStep() noexcept {
    if constexpr (I == sizeof...(Specs)) {
      return cons_;
    } else if constexpr (detail::HasCancel<Step<I>>) {
      return CreateStep<I>();
    } else {
      return GetCancelStep<I + 1>();
    }
  }

  template <::std::size_t I>
  auto &CreateStep() noexcept {
    using Step = Step<I>;

    return *::new (buffer_) Step({this}, data_.template Get<I>());
  }

  template <::std::size_t I>
  void DestroyStep() noexcept {
    using Step = Step<I>;

    ::std::launder(reinterpret_cast<Step *>(buffer_))->~Step();
  }
};

////////////////////////////////////////////////////////////////////////////////

template <typename Core, ::std::size_t I>
inline constexpr bool CorrectStep
    = ::std::is_nothrow_constructible_v<typename Core::template Step<I>,
                                        typename Core::template Redirect<I>,
                                        typename Core::template Spec<I> &> &&
      ::std::is_nothrow_destructible_v<typename Core::template Step<I>>;

template <typename ...>
inline constexpr bool CorrectSteps = false;

template <typename Core, ::std::size_t ...Is>
inline constexpr bool CorrectSteps<Core, ::std::index_sequence<Is...>>
    = (... && CorrectStep<Core, Is>);

} // namespace detail

namespace concepts {

template <typename Consumer, typename Maker, typename ...Combinators>
concept CorrectCore
    // = CorrectPipeline<Maker, Combinators...> && [TODO]: ?
    = Continuation<Consumer, trait::ValueOf<Traits<Maker, Combinators...>>> &&
      detail::CorrectSteps<
          detail::ExecutionCoreBase<Consumer, Maker, Combinators...>,
          ::std::index_sequence_for<Maker, Combinators...>>;

} // namespace concepts

template <typename Consumer, typename Maker, typename ...Combinators>
requires (concepts::CorrectCore<Consumer, Maker, Combinators...>)
class ExecutionCore
    : private detail::ExecutionCoreBase<Consumer, Maker, Combinators...> {
 private:
  using Base = ExecutionCore::ExecutionCoreBase;
  // detail::ExecutionCoreBase<Consumer, Maker, Combinators...>;

 public:
  using Base::Base;
  using Base::Start;
};

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_EXECUTION_CORE_HPP_INCLUDED_ */
