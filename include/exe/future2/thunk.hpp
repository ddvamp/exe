//
// thunk.hpp
// ~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_THUNK_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_THUNK_HPP_INCLUDED_ 1

#include <exe/future2/cancel.hpp>
#include <exe/future2/scheduler.hpp>
#include <exe/future2/concept/async_safe.hpp>
#include <exe/future2/concept/valid_input.hpp>
#include <exe/future2/model/computation.hpp>
#include <exe/future2/model/continuation.hpp>
#include <exe/future2/model/future_value.hpp>
#include <exe/future2/model/state.hpp>
#include <exe/future2/model/thunk.hpp>
#include <exe/future2/model/thunk_resource.hpp>

#include <util/type_traits.hpp>

#include <cstddef>
#include <utility>

namespace exe::future {

namespace trait {

template <typename T>
using ValueOf = T::ValueType;

template <typename T, typename InputType>
using ValueType = T::template ValueType<InputType>;

template <typename T, typename Consumer>
using MakeStep = T::template MakeStep<Consumer>;

template <typename T, typename InputType, typename Consumer>
using CombineStep = T::template CombineStep<InputType, Consumer>;

// template <typename T, typename InputType, typename Consumer>
// using Continuation = T::template Continuation<InputType, Consumer>;

// template <typename T, typename Consumer>
// using Computation = T::template Computation<Consumer>;

} // namespace trait

template <concepts::FutureValue V>
struct IConsumer {
 protected:
  // Lifetime cannot be controlled via IConsumer<V> &
  ~IConsumer() = default;

 public:
  virtual void Continue(V &&, State) && noexcept = 0;

  virtual void Cancel(State) && noexcept = 0;

  virtual cancel::CancelSource &CancelSource() & noexcept = 0;
};

template <concepts::FutureValue InputType,
          concepts::Continuation<InputType> Proxied>
class [[nodiscard]] Proxy {
 private:
  Proxied &cons_;

 public:
  explicit Proxy(Proxied &c) noexcept : cons_(c) {}

  /* Continuation */

  void Continue(InputType &&i, State s) && noexcept {
    ::std::move(cons_).Continue(::std::move(i), s);
  }

  void Cancel(State s) && noexcept {
    ::std::move(cons_).Cancel(s);
  }

  auto &CancelSource() & noexcept {
    return cons_.CancelSource();
  }
};

template <typename InputType>
using Demand = Proxy<InputType, IConsumer<InputType>>;

////////////////////////////////////////////////////////////////////////////////

namespace detail {

template <typename Maker>
using MakeStepProbe = trait::MakeStep<Maker, Demand<trait::ValueOf<Maker>>>;

template <typename Combinator, typename InputType>
using CombineStepProbe =
    trait::CombineStep<Combinator,
                       InputType,
                       Demand<trait::ValueType<Combinator, InputType>>>;

} // namespace detail

namespace concepts {

template <typename M>
concept Maker = requires {
  typename trait::ValueOf<M>;
  requires FutureValue<trait::ValueOf<M>>;

  typename detail::MakeStepProbe<M>;
  // requires Computation<detail::MakeStepProbe<M>>;
};

template <typename C, typename InputType>
concept Combinator = requires {
  requires ValidInput<InputType, C>;

  typename trait::ValueType<C, InputType>;
  requires FutureValue<trait::ValueType<C, InputType>>;

  typename detail::CombineStepProbe<C, InputType>;
  // requires Continuation<detail::CombineStepProbe<C, InputType>>;
};

} // namespace concepts

////////////////////////////////////////////////////////////////////////////////

namespace detail {

template <typename Maker>
struct InvalidMaker {};

template <typename Combinator, typename InputType, ::std::size_t Step>
struct InvalidCombinator {};

template <typename T>
concept ErroneousTraits = requires {
  typename T::Error;
};

////////////////////////////////////////////////////////////////////////////////

// [TODO]: ?Adapt util::type_list
template <typename ...Ts>
struct TypeList {
  template <::std::size_t I>
  using Type = Ts...[I];
};

template <typename Combinator, typename ...Head>
struct TraitsImpl;

template <typename Maker>
struct TraitsImpl<Maker> {
  using Error = InvalidMaker<Maker>;
};

template <typename Combinator, typename ...Head>
struct TraitsImpl {
  using Error = InvalidCombinator<Combinator,
                                  trait::ValueOf<TraitsImpl<Head...>>,
                                  sizeof...(Head)>;
};

template <typename Combinator, typename ...Head>
requires (ErroneousTraits<TraitsImpl<Head...>>)
struct TraitsImpl<Combinator, Head...> {
  using Error = TraitsImpl<Head...>::Error;
};

template <typename Maker>
requires (concepts::Maker<Maker>)
struct TraitsImpl<Maker> {
  using ValueType = trait::ValueOf<Maker>;

  template <typename Consumer>
  using Step = trait::MakeStep<Maker, Consumer>;

  template <typename ...Ts>
  using List = TypeList<TraitsImpl, Ts...>;
};

template <typename Combinator, typename ...Head>
requires (concepts::Combinator<Combinator, trait::ValueOf<TraitsImpl<Head...>>>)
struct TraitsImpl<Combinator, Head...> {
  using Prev = TraitsImpl<Head...>;
  using InputType = trait::ValueOf<Prev>;
  using OutputType = trait::ValueType<Combinator, InputType>;

  using ValueType = OutputType;

  template <typename Consumer>
  using Step = trait::CombineStep<Combinator, InputType, Consumer>;

  template <typename ...Ts>
  using List = Prev::template List<TraitsImpl, Ts...>;
};

template <typename T, ::std::size_t I>
using Traits = T::template List<>::template Type<I>;

////////////////////////////////////////////////////////////////////////////////

template <typename ...>
struct TraitsHelper;

template <::std::size_t ...Is, typename ...Ts>
struct TraitsHelper<::std::index_sequence<Is...>, Ts...> {
  using Traits = TraitsImpl<Ts...[sizeof...(Is) - 1 - Is]...>;
};

} // namespace detail

template <typename ...Ts>
using Traits =
    detail::TraitsHelper<::std::index_sequence_for<Ts...>, Ts...>::Traits;

template <typename Traits, ::std::size_t I>
using Type = detail::Traits<Traits, I>::ValueType;

template <typename Traits, ::std::size_t I, typename Consumer>
using Step = detail::Traits<Traits, I>::template Step<Consumer>;

////////////////////////////////////////////////////////////////////////////////

namespace detail {

// [TODO]: ?Inheritance for non-final class types
template <typename T, ::std::size_t>
struct ThunkVal {
  [[no_unique_address]] T val;
};

template <typename ...>
struct ThunkDataImpl;

template <typename ...Ts>
using ThunkData = ThunkDataImpl<::std::index_sequence_for<Ts...>, Ts...>;

template <typename ...Ts, ::std::size_t ...Is>
struct ThunkDataImpl<::std::index_sequence<Is...>, Ts...>
    : ThunkVal<Ts, Is>... {
  template <typename ...Us>
  using ExtendedData = ThunkData<Ts..., Us...>;

  template <::std::size_t I>
  using Leaf = ThunkVal<Ts...[I], I>;

  template <::std::size_t I>
  inline Ts...[I] &Get() & noexcept {
    return static_cast<Leaf<I> &>(*this).val;
  }

  template <typename ...Us>
  inline ExtendedData<Us...> Extend(Us &&...us) && {
    return {static_cast<Leaf<Is> &&>(*this)..., {::std::forward<Us>(us)}...};
  }

  template <typename ...Us, ::std::size_t ...Js>
  inline ExtendedData<Us...> Extend(
      ThunkDataImpl<::std::index_sequence<Js...>, Us...> &&d) && {
    using D = ThunkDataImpl<::std::index_sequence<Js...>, Us...>;
    return {static_cast<Leaf<Is> &&>(*this)...,
            {static_cast<D::template Leaf<Js> &&>(d).val}...};
  }
};

template <typename ...Ts>
ThunkDataImpl(Ts...) -> ThunkDataImpl<::std::index_sequence_for<Ts...>, Ts...>;

} // namespace detail

////////////////////////////////////////////////////////////////////////////////

namespace concepts {

template <typename Maker, typename ...Combinators>
concept CorrectPipeline =
    !detail::ErroneousTraits<Traits<Maker, Combinators...>>;

} // namespace concepts

////////////////////////////////////////////////////////////////////////////////

template <concepts::ThunkResource Maker,
          concepts::ThunkResource ...Combinators>
requires (concepts::CorrectPipeline<Maker, Combinators...>)
class [[nodiscard]] Thunk {
 private:
  template <concepts::ThunkResource M, concepts::ThunkResource ...Cs>
  requires (concepts::CorrectPipeline<M, Cs...>)
  friend class Thunk;

  using Data = detail::ThunkData<Maker, Combinators...>;
  using Traits = Traits<Maker, Combinators...>;

  template <typename ...Cs>
  using ExtendedThunk = Thunk<Maker, Combinators..., Cs...>;

  Data data_;

 public:
  ~Thunk() = default;

  Thunk(Thunk const &) = delete;
  void operator= (Thunk const &) = delete;

  // Move-out only
  Thunk(Thunk &&) = default;
  void operator= (Thunk &&) = delete;

 public:
  inline explicit Thunk(Maker &&m, Combinators &&...cs) noexcept
      : data_{{::std::move(m)}, {::std::move(cs)}...} {}

  using ValueType = trait::ValueOf<Traits>;

  template <typename ...Cs>
  inline ExtendedThunk<Cs...> Extend(Cs &&...c) && noexcept {
    return ::std::move(data_).Extend(::std::move(c)...);
  }

 private:
  inline Thunk(Data &&d) noexcept : data_(::std::move(d)) {}

//  private:
//   template <typename Consumer>
//   using Comp = trait::Computation<Traits, Proxy<ValueType, Consumer>>;

//  public:
//   template <concepts::Continuation<ValueType> Consumer>
//   class Computation : private Comp<Consumer> {
//    private:
//     using Base = Comp<Consumer>;

//    public:
//     ~Computation() = default;

//     Computation(Computation const &) = delete;
//     void operator= (Computation const &) = delete;

//     Computation(Computation &&) = delete;
//     void operator= (Computation &&) = delete;

//    public:
//     Computation(Thunk &&t, Consumer &c) noexcept : Computation(t, c) {}

//     /* Computation */

//     using Base::Start;

//    private:
//     template <typename ...Ts>
//     Computation(detail::ThunkData<Ts...> &d, Consumer &c) noexcept
//         : Base(static_cast<Ts &&>(d).val..., c) {}
//   };

//   template <concepts::Continuation<ValueType> Consumer>
//   concepts::Computation auto Materialize(Consumer &c) && noexcept {
//     using Cons = Proxy<ValueType, Consumer>;
//     return Computation<Cons>(::std::move(*this), Cons(c));
//   }
};

template <typename ...Ts>
inline constexpr bool trait::Thunk<Thunk<Ts...>> = true;

template <typename Raw, ::std::size_t I>
struct Redirect {
  Raw &raw_;

  // explicit Redirect(Raw &r) noexcept : raw_(r) {}

  void Continue(auto &&v, State s) && noexcept {
    ::std::move(raw_).template Continue<I>(::std::move(v), s);
  }

  void Cancel(State s) && noexcept {
    ::std::move(raw_).template Cancel<I>(s);
  }

  auto &CancelSource() & noexcept {
    return raw_.CancelSource();
  }
};

// [TODO]: Variant-like storage
template <typename ...Ts>
struct VariadicStorage;

template <typename Consumer, typename Maker, typename ...Combinators>
requires (concepts::CorrectPipeline<Maker, Combinators...>) // [TODO]: Up
struct Core {
  using Traits = Traits<Maker, Combinators...>;

  Consumer cons_;

  ::std::tuple<Maker, Combinators...> specs_;

  template <::std::size_t I>
  using Type = Type<Traits, I>;

  template <::std::size_t I>
  using Step = Step<Traits, I, Redirect<Core, I>>;

  static constexpr auto MaxSizeAlign() {
    return []<::std::size_t ...Is>(::std::index_sequence<Is...>) {
      return ::std::pair{::util::max_size_of_v<Step<Is>...>,
                         ::util::max_alignment_of_v<Step<Is>...>};
    }(::std::make_index_sequence<1 + sizeof...(Combinators)>{});
  }

  alignas (MaxSizeAlign().second) unsigned char buffer_[MaxSizeAlign().first];

  Core(Consumer &&c, Maker &&m, Combinators &&...cs)
      : cons_(::std::forward<Consumer>(c))
      , specs_{::std::move(m), ::std::move(cs)...} {}

  void Start(Scheduler &s) && noexcept {
    auto &stage = Stage<0>();
    ::std::move(stage).Start(s);
  }

  template <::std::size_t I>
  void Continue(Type<I> &&v, State s) && noexcept { // [TODO]: ?by value
    Destroy<I>();

    auto &stage = Stage<I + 1>();
    ::std::move(stage).Continue(::std::move(v), s);
  }

  template <::std::size_t I>
  void Cancel(State s) && noexcept {
    Destroy<I>();

    auto &stage = Stage<I + 1>();
    ::std::move(stage).Cancel(s);
  }

  auto &CancelSource() & noexcept {
    return cons_.CancelSource();
  }

 private:
  template <::std::size_t I>
  auto &Stage() {
    if constexpr (I == 1 + sizeof...(Combinators)) {
      return cons_;
    } else {
      return Create<I>();
    }
  }

  template <::std::size_t I>
  auto &Create() {
    using Cons = Redirect<Core, I>;

    return *::new (buffer_) Step<I>(Cons(*this), ::std::get<I>(specs_));
  }

  template <::std::size_t I>
  void Destroy() {
    using Step = Step<I>;

    ::std::launder(reinterpret_cast<Step *>(buffer_))->~Step();
  }
};

template <typename Consumer, typename ...Ts>
Core(Consumer &&, Ts...) -> Core<Consumer, Ts...>;

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_THUNK_HPP_INCLUDED_ */
