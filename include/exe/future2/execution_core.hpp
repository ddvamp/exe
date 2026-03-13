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
#include <exe/future2/model/continuation.hpp>
#include <exe/future2/model/state.hpp>
#include <exe/future2/model/thunk.hpp>
#include <exe/future2/model/thunk_resource.hpp>
#include <exe/future2/trait/value_of.hpp>

#include <util/type_traits.hpp>

#include <cstddef>
#include <new> // ::std::launder
#include <utility>

namespace exe::future {

namespace detail {

template <::std::size_t, typename ...>
struct ExecutionCoreRedirect {
  void *core_;

  void Continue(auto &&, State) && noexcept;
  void Cancel(State) && noexcept;
  cancel::CancelSource &CancelSource() & noexcept;
};

template <typename ...>
inline constexpr bool CorrectCore = false;

template <::std::size_t I, typename ...Ts>
using Resource = Ts...[I];

} // namespace detail

////////////////////////////////////////////////////////////////////////////////

// [TODO]: Variant-like storage
template <typename ...Ts>
struct VariadicStorage;

namespace concepts {

template <typename Consumer, typename Maker, typename ...Combinators>
concept CorrectCore
    = detail::CorrectCore<::std::index_sequence<1 + sizeof...(Combinators)>,
                          Consumer, Maker, Combinators...>;

} // namespace concepts

template <typename Consumer, typename Maker, typename ...Combinators>
requires (concepts::Continuation<
              Consumer,
              trait::ValueOf<Traits<Maker, Combinators...>>> &&
          concepts::CorrectCore<Consumer, Maker, Combinators...>)
class ExecutionCore {
 private:
  using Data = ThunkData<Maker, Combinators...>;
  using Traits = Traits<Maker, Combinators...>;

  template <::std::size_t I>
  using InputType = Type<Traits, I>;

  template <::std::size_t I>
  using Redirect
      = detail::ExecutionCoreRedirect<I, Consumer, Maker, Combinators...>;

  template <::std::size_t I>
  using Resource = detail::Resource<I, Maker, Combinators...>;

  template <::std::size_t I>
  using Step = Step<Traits, I, Redirect<I>>;

  static constexpr auto MaxSizeAlign() {
    return []<::std::size_t ...Is>(::std::index_sequence<Is...>) {
      return ::std::pair{::util::max_size_of_v<Step<Is>...>,
                         ::util::max_alignment_of_v<Step<Is>...>};
    }(::std::make_index_sequence<1 + sizeof...(Combinators)>{});
  }

  alignas (MaxSizeAlign().second) unsigned char buffer_[MaxSizeAlign().first];

  Consumer cons_;
  Data &data_;

 public:
  ExecutionCore(Consumer &&c, Data &d)
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
    if constexpr (I == 1 + sizeof...(Combinators)) {
      return cons_;
    } else {
      return CreateStep<I>();
    }
  }

  // [TODO]: ?Better trait for fast get of next cancel aware step
  template <::std::size_t I>
  auto &GetCancelStep() noexcept {
    if constexpr (I == 1 + sizeof...(Combinators)) {
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

    // [TODO]: Better
    // static_assert(::std::is_nothrow_constructible_v<
    //                   Step, Redirect<I>, Resource<I> &>);
    // static_assert(::std::is_nothrow_destructible_v<Step>);

    return *::new (buffer_) Step({this}, data_.template Get<I>());
  }

  template <::std::size_t I>
  void DestroyStep() noexcept {
    using Step = Step<I>;

    ::std::launder(reinterpret_cast<Step *>(buffer_))->~Step();
  }
};

////////////////////////////////////////////////////////////////////////////////

namespace detail {

template <::std::size_t I, typename ...Ts>
void ExecutionCoreRedirect<I, Ts...>::Continue(auto &&v, State s) && noexcept {
  auto &core = *static_cast<ExecutionCore<Ts...> *>(core_);
  ::std::move(core).template Continue<I>(::std::move(v), s);
}

template <::std::size_t I, typename ...Ts>
void ExecutionCoreRedirect<I, Ts...>::Cancel(State s) && noexcept {
  auto &core = *static_cast<ExecutionCore<Ts...> *>(core_);
  ::std::move(core).template Cancel<I>(s);
}

template <::std::size_t I, typename ...Ts>
cancel::CancelSource &ExecutionCoreRedirect<I, Ts...>::CancelSource() &
    noexcept {
  auto &core = *static_cast<ExecutionCore<Ts...> *>(core_);
  return core.CancelSource();
}

////////////////////////////////////////////////////////////////////////////////

template <::std::size_t I,
          typename Consumer, typename Maker, typename ...Combinators>
using ExecutionCoreStep
    = Step<Traits<Maker, Combinators...>,
           I,
           ExecutionCoreRedirect<I, Consumer, Maker, Combinators...>>;

template <::std::size_t I,
          typename Consumer, typename Maker, typename ...Combinators>
inline constexpr bool CorrectStep
    = ::std::is_nothrow_constructible_v<
          ExecutionCoreStep<I, Consumer, Maker, Combinators...>,
          ExecutionCoreRedirect<I, Consumer, Maker, Combinators...>,
          Resource<I> &> &&
      ::std::is_nothrow_destructible_v<
          ExecutionCoreStep<I, Consumer, Maker, Combinators...>>;

template <::std::size_t ...Is,
          typename Consumer, typename Maker, typename ...Combinators>
inline constexpr bool CorrectCore<::std::index_sequence<Is...>,
                                  Consumer, Maker, Combinators...>
    = (... && CorrectStep<Is, Consumer, Maker, Combinators...>);

} // namespace detail

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_EXECUTION_CORE_HPP_INCLUDED_ */
