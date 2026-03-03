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
          concepts::Continuation<InputType> Consumer>
class [[nodiscard]] Proxy {
 private:
  Consumer &cons_;

 public:
  explicit Proxy(Consumer &c) noexcept : cons_(c) {}

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

// [TODO]: = core::Proxy<IConsumer<T>>;
template <typename T>
using Demand = IConsumer<T> &;

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
};

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

static_assert(CorrectPipeline<int>);

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
  // [TODO]: ?ResourceDropper
  template <concepts::Continuation<ValueType> Consumer>
  struct Continuation {
    Thunk thunk_;
    Consumer cons_;

    void Continue(ValueType &&val, State st) && noexcept {
      auto _ = ::std::move(thunk_);
      ::std::move(cons_).Continue(::std::move(val), st);
    }

    void Cancel(State st) && noexcept {
      auto _ = ::std::move(thunk_);
      ::std::move(cons_).Cancel(st);
    }

    /**/

    cancel::CancelSource &CancelSource() & noexcept {
      return cons_.CancelSource();
    }
  };

 public:
  template <concepts::Continuation<ValueType> Consumer>
  class Computation : private Continuation<Consumer> {
   private:
    using Cont = Continuation<Consumer>;
    using Comp = Traits::template Computation<Cont &>;

    // [TODO]: ?lazy
    Comp comp;

   public:
    ~Computation() = default;

    Computation(Computation const &) = delete;
    void operator= (Computation const &) = delete;

    Computation(Computation &&) = delete;
    void operator= (Computation &&) = delete;

   public:
    Computation(Thunk &&t, Consumer &&c)
        : Computation(::std::make_index_sequence<sizeof...(Combinators) + 1>{},
                      t, c) {}

    void Start(Scheduler &sched) && noexcept {
      ::std::move(comp).Start(sched);
    }

   private:
    template <::std::size_t ...Is>
    Computation(::std::index_sequence<Is...>, Thunk &t, Consumer &c)
        : Cont{.thunk_ = ::std::move(t),
               .cons_ = ::std::forward<Consumer>(c)}
        , comp(Cont::thunk_.template get<Is>()...,
               static_cast<Cont &>(*this)) {}
  };

 public:
  explicit Thunk(Maker &&m, Combinators &&...cs) noexcept
      : Data{{::std::move(m)}, {::std::move(cs)}...} {}

  template <concepts::Combinator<ValueType> Combinator>
  concepts::Thunk auto Extend(Combinator c) && noexcept {
    return ExtendImpl(::std::make_index_sequence<sizeof...(Combinators) + 1>{},
                      c);
  }

  template <concepts::Continuation<ValueType> Consumer>
  concepts::Computation auto Materialize(
      ::std::type_identity_t<Consumer> &&c) && noexcept {
    return Computation<Consumer>(::std::move(*this),
                                 ::std::forward<Consumer>(c));
  }

 private:
  template <::std::size_t ...Is, typename C>
  auto ExtendImpl(::std::index_sequence<Is...>, C &c) noexcept {
    return Thunk(::std::move(this->template get<Is>())..., ::std::move(c));
  }
};

template <typename ...Ts>
inline constexpr bool trait::Thunk<Thunk<Ts...>> = true;

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_THUNK_HPP_INCLUDED_ */
