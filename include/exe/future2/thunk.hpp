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

template <typename T, typename InputType, typename Consumer>
using Continuation = T::template Continuation<InputType, Consumer>;

template <typename T, typename Consumer>
using Computation = T::template Computation<Consumer>;

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

namespace concepts {

template <typename M>
concept Maker =
    FutureValue<trait::ValueOf<M>> &&
    Computation<trait::Computation<M, Demand<trait::ValueOf<M>>>>;

template <typename C, typename InputType>
concept Combinator =
    ValidInput<InputType, C> &&
    FutureValue<trait::ValueType<C, InputType>> &&
    Continuation<trait::Continuation<C, InputType,
                                     Demand<trait::ValueType<C, InputType>>>,
                 InputType>;

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

// [TODO]: Add size_t parameter for ValueType/Step O(1) access
template <typename Combinator, typename ...Head>
struct TraitsImpl {
  using Error = InvalidCombinator<Combinator,
                                  trait::ValueOf<TraitsImpl<Head...>>,
                                  sizeof...(Head)>;
};

template <typename Maker>
struct TraitsImpl<Maker> {
  using Error = InvalidMaker<Maker>;
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
  using Computation = trait::Computation<Maker, Consumer>;

  template <::std::size_t>
  using Type = ValueType;

  template <::std::size_t, typename Consumer>
  using Step = Computation<Consumer>;
};

template <typename T, ::std::size_t I>
using Type = T::template Type<I>;

template <typename T, ::std::size_t I, typename Consumer>
using Step = T::template Step<I, Consumer>;

template <typename Combinator, typename ...Head>
requires (concepts::Combinator<Combinator, trait::ValueOf<TraitsImpl<Head...>>>)
struct TraitsImpl<Combinator, Head...> {
  using InputType = trait::ValueOf<TraitsImpl<Head...>>;
  using OutputType = trait::ValueType<Combinator, InputType>;

  template <typename Consumer>
  using Continuation = trait::Continuation<Combinator, InputType, Consumer>;

  using ValueType = OutputType;

  template <typename Consumer>
  using Computation = trait::Computation<TraitsImpl<Head...>,
                                         Continuation<Consumer>>;

  template <::std::size_t I>
  using Type =
      ::std::conditional_t<I == 0,
                           ValueType,
                           typename TraitsImpl<Head...>::template Type<I - 1>>;

  template <::std::size_t I, typename Consumer>
  using Step =
      ::std::conditional_t<I == 0,
                           Continuation<Consumer>,
                           Step<TraitsImpl<Head...>, I - 1, Consumer>>;
};

////////////////////////////////////////////////////////////////////////////////

// [TODO]: Inheritance for non-final class types
template <typename V, ::std::size_t>
struct ThunkVal {
  [[no_unique_address]] V val;
};

template <typename ...Ts>
struct ThunkData : Ts... {
  template <::std::size_t I>
  inline auto &get() & noexcept {
    return static_cast<Ts...[I] &>(*this).val;
  }
};

////////////////////////////////////////////////////////////////////////////////

template <typename ...>
struct ThunkHelperImpl;

template <::std::size_t ...Is, typename ...Ts>
struct ThunkHelperImpl<::std::index_sequence<Is...>, Ts...> {
  using Data = ThunkData<ThunkVal<Ts, Is>...>;
  using Traits = TraitsImpl<Ts...[sizeof...(Is) - 1 - Is]...>;
};

template <typename ...Ts>
using ThunkHelper = ThunkHelperImpl<::std::index_sequence_for<Ts...>, Ts...>;

template <typename ...Ts>
using Data = ThunkHelper<Ts...>::Data;

} // namespace detail

template <typename Maker, typename ...Combinators>
using Traits = detail::ThunkHelper<Maker, Combinators...>::Traits;

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
class [[nodiscard]] Thunk : private detail::Data<Maker, Combinators...> {
 private:
  using Data = detail::Data<Maker, Combinators...>;
  using Traits = Traits<Maker, Combinators...>;

 public:
  ~Thunk() = default;

  Thunk(Thunk const &) = delete;
  void operator= (Thunk const &) = delete;

  Thunk(Thunk &&) = default;
  void operator= (Thunk &&) = delete;

 public:
  using ValueType = trait::ValueOf<Traits>;

 private:
  // // [TODO]: ?ResourceDropper
  // template <concepts::Continuation<ValueType> Consumer>
  // struct Continuation {
  //   Thunk thunk_;
  //   Consumer cons_;

  //   void Continue(ValueType &&val, State st) && noexcept {
  //     auto _ = ::std::move(thunk_);
  //     ::std::move(cons_).Continue(::std::move(val), st);
  //   }

  //   void Cancel(State st) && noexcept {
  //     auto _ = ::std::move(thunk_);
  //     ::std::move(cons_).Cancel(st);
  //   }

  //   /**/

  //   cancel::CancelSource &CancelSource() & noexcept {
  //     return cons_.CancelSource();
  //   }
  // };

  template <typename Consumer>
  using Comp = trait::Computation<Traits, Proxy<ValueType, Consumer>>;

 public:
  template <concepts::Continuation<ValueType> Consumer>
  class Computation : private Comp<Consumer> {
   private:
    using Base = Comp<Consumer>;

   public:
    ~Computation() = default;

    Computation(Computation const &) = delete;
    void operator= (Computation const &) = delete;

    Computation(Computation &&) = delete;
    void operator= (Computation &&) = delete;

   public:
    Computation(Thunk &&t, Consumer &c) noexcept : Computation(t, c) {}

    /* Computation */

    using Base::Start;

   private:
    template <typename ...Ts>
    Computation(detail::ThunkData<Ts...> &d, Consumer &c) noexcept
        : Base(static_cast<Ts &&>(d).val..., c) {}
  };

 public:
  explicit Thunk(Maker &&m, Combinators &&...cs) noexcept
      : Data{{::std::move(m)}, {::std::move(cs)}...} {}

  template <concepts::Continuation<ValueType> Consumer>
  concepts::Computation auto Materialize(Consumer &c) && noexcept {
    using Cons = Proxy<ValueType, Consumer>;
    return Computation<Cons>(::std::move(*this), Cons(c));
  }

//   template <concepts::Combinator<ValueType> Combinator>
//   concepts::Thunk auto Extend(Combinator c) && noexcept {
//     return ExtendImpl(::std::make_index_sequence<sizeof...(Combinators) + 1>{},
//                       c);
//   }

//  private:
//   template <::std::size_t ...Is, typename C>
//   auto ExtendImpl(::std::index_sequence<Is...>, C &c) noexcept {
//     return Thunk(::std::move(this->template get<Is>())..., ::std::move(c));
//   }
};

template <typename ...Ts>
inline constexpr bool trait::Thunk<Thunk<Ts...>> = true;

template <typename Consumer, typename Maker, typename ...Combinators>
struct Core {
  template <::std::size_t I>
  struct Redirect {
    Core &core_;

    void Continue(auto &&v, State s) && noexcept {
      ::std::move(core_).template Continue<I>(::std::move(v), s);
    }

    void Cancel(State s) && noexcept {
      ::std::move(core_).template Cancel<I>(s);
    }

    auto &CancelSource() & noexcept {
      return core_.CancelSource();
    }
  };

  using Traits = Traits<Maker, Combinators...>;

  Consumer cons_;

  Maker maker_;
  ::std::tuple<Combinators...> combinators_;

  void *buffer_;

  Core(void *ptr, Consumer &&c, Maker &&m, Combinators &&...cs)
      : cons_(::std::forward<Consumer>(c))
      , maker_(::std::move(m))
      , combinators_{::std::move(cs)...}
      , buffer_(ptr) {}

  template <typename T>
  T &As() {
    return *static_cast<T *>(buffer_);
  }

  template <::std::size_t I>
  using Type = detail::Type<Traits, I>;

  template <::std::size_t I>
  using Step = detail::Step<Traits, I, Redirect<I>>;

  void Start(Scheduler &s) && noexcept {
    using Curr = Step<0>;

    ::new (buffer_) Curr(this, ::std::move(maker_));
    ::std::move(As<Curr>()).Start(s);
  }

  template <::std::size_t I>
  void Continue(Type<I> &&v, State s) && noexcept { // [TODO]: ?by value
    if constexpr (I < sizeof...(Combinators)) {
      using Prev = Step<I>;
      using Curr = Step<I + 1>;

      Destroy<Prev>();
      ::new (buffer_) Curr(this, ::std::get<I>(::std::move(combinators_)));
      ::std::move(As<Curr>()).Continue(::std::move(v), s);
    } else {
      using Prev = Step<I>;

      Destroy<Prev>();
      ::std::move(cons_).Continue(::std::move(v), s);
    }
  }

  template <::std::size_t I>
  void Cancel(State s) && noexcept {
    if constexpr (I < sizeof...(Combinators)) {
      using Prev = Step<I>;
      using Curr = Step<I + 1>;

      Destroy<Prev>();
      ::new (buffer_) Curr(this, ::std::get<I>(::std::move(combinators_)));
      ::std::move(As<Curr>()).Cancel(s);
    } else {
      using Prev = Step<I>;

      Destroy<Prev>();
      ::std::move(cons_).Cancel(s);
    }
  }

  auto &CancelSource() & noexcept {
    return cons_.CancelSource();
  }

 private:
  template <typename T>
  void Destroy() {
    As<T>().~T();
  }
};

template <typename Consumer, typename ...Ts>
Core(void *, Consumer &&, Ts...) -> Core<Consumer, Ts...>;

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
