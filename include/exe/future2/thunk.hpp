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

template <::std::size_t I, typename Combinator, typename ...Head>
struct TraitsImpl;

template <::std::size_t I, typename ...Ts>
TraitsImpl<I, Ts...> Select(TraitsImpl<I, Ts...>);

template <typename Traits, ::std::size_t I>
using TraitsBase = decltype(Select<I>(::std::declval<Traits>()));

template <typename ...Ts>
using Traits = TraitsImpl<sizeof...(Ts) - 1, Ts...>;

template <::std::size_t I, typename Combinator, typename ...Head>
struct TraitsImpl : Traits<Head...> {
  using Error = InvalidCombinator<Combinator,
                                  trait::ValueOf<Traits<Head...>>,
                                  I>;
};

template <typename Maker>
struct TraitsImpl<0, Maker> {
  using Error = InvalidMaker<Maker>;
};

template <::std::size_t I, typename Combinator, typename ...Head>
requires (ErroneousTraits<Traits<Head...>>)
struct TraitsImpl<I, Combinator, Head...> : Traits<Head...> {};

template <typename Maker>
requires (concepts::Maker<Maker>)
struct TraitsImpl<0, Maker> {
  using ValueType = trait::ValueOf<Maker>;

  template <typename Consumer>
  using Step = trait::MakeStep<Maker, Consumer>;
};

template <::std::size_t I, typename Combinator, typename ...Head>
requires (concepts::Combinator<Combinator, trait::ValueOf<Traits<Head...>>>)
struct TraitsImpl<I, Combinator, Head...> : Traits<Head...> {
  using InputType = trait::ValueOf<Traits<Head...>>;
  using OutputType = trait::ValueType<Combinator, InputType>;

  using ValueType = OutputType;

  template <typename Consumer>
  using Step = trait::CombineStep<Combinator, InputType, Consumer>;
};

////////////////////////////////////////////////////////////////////////////////

// [TODO]: Inheritance for non-final class types
template <typename V, ::std::size_t>
struct ThunkVal {
  using Type = V;

  [[no_unique_address]] V val;
};

template <typename ...>
struct ThunkData;

////////////////////////////////////////////////////////////////////////////////

template <typename ...>
struct ThunkHelperImpl;

template <::std::size_t ...Is, typename ...Ts>
struct ThunkHelperImpl<::std::index_sequence<Is...>, Ts...> {
  using Data = ThunkData<ThunkVal<Ts, Is>...>;
  using Traits = Traits<Ts...[sizeof...(Is) - 1 - Is]...>;
};

template <typename ...Ts>
using ThunkHelper = ThunkHelperImpl<::std::index_sequence_for<Ts...>, Ts...>;

template <typename ...Ts>
using Data = ThunkHelper<Ts...>::Data;

////////////////////////////////////////////////////////////////////////////////

template <typename ...Ts>
struct ThunkData : Ts... {
  template <typename ...Us>
  using ExtendedData = Data<typename Ts::Type..., Us...>;

  template <::std::size_t I>
  inline auto &Get() & noexcept {
    return static_cast<Ts...[I] &>(*this).val;
  }

  template <typename ...Vs>
  inline ExtendedData<Vs...> Extend(Vs &&...vs) && {
    return {static_cast<Ts &&>(*this)..., {::std::forward<Vs>(vs)}...};
  }
};

////////////////////////////////////////////////////////////////////////////////

} // namespace detail

template <typename Maker, typename ...Combinators>
struct Traits : detail::ThunkHelper<Maker, Combinators...>::Traits {
  template <::std::size_t I>
  using Type = detail::TraitsBase<Traits, I>::ValueType;

  template <::std::size_t I, typename Consumer>
  using Step = detail::TraitsBase<Traits, I>::template Step<Consumer>;
};

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

  using Data = detail::Data<Maker, Combinators...>;
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
  explicit Thunk(Maker &&m, Combinators &&...cs) noexcept
      : data_{{::std::move(m)}, {::std::move(cs)}...} {}

  using ValueType = trait::ValueOf<Traits>;

  template <typename ...Cs>
  inline ExtendedThunk<Cs...> Extend(Cs &&...c) && noexcept {
    return ExtendedThunk<Cs...>(::std::move(data_).Extend(::std::move(c)...));
  }

 private:
  explicit Thunk(Data &&d) noexcept : data_(::std::move(d)) {}

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

//  private:
//   template <::std::size_t ...Is, typename C>
//   auto ExtendImpl(::std::index_sequence<Is...>, C &c) noexcept {
//     return Thunk(::std::move(this->template get<Is>())..., ::std::move(c));
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

// [TODO]: variant-like storage
template <typename ...Ts>
struct VariadicStorage;

template <typename Consumer, typename Maker, typename ...Combinators>
requires (concepts::CorrectPipeline<Maker, Combinators...>) // [TODO]: Up
struct Core {
  using Traits = Traits<Maker, Combinators...>;

  Consumer cons_;

  ::std::tuple<Maker, Combinators...> specs_;

  template <::std::size_t I>
  using Type = Traits::template Type<I>;

  template <::std::size_t I>
  using Step = Traits::template Step<I, Redirect<Core, I>>;

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

template <typename Consumer, typename Maker, typename ...Combinators>
struct Computation {
  Consumer cons;
  Maker maker;
  ::std::tuple<Combinators...> combinators;

  // Storage<Maker, Combinators...>

  Computation(Consumer &&c, Maker &&m, Combinators &&...cs)
      : cons(::std::move(c))
      , maker(::std::move(m))
      , combinators{::std::move(cs)...} {}
};

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_THUNK_HPP_INCLUDED_ */
