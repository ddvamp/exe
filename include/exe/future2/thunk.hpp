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
#include <exe/future2/core/proxy.hpp>
#include <exe/future2/model/combinator.hpp>
#include <exe/future2/model/continuation.hpp>
#include <exe/future2/model/maker.hpp>
#include <exe/future2/model/state.hpp>
#include <exe/future2/model/thunk.hpp>
#include <exe/future2/model/thunk_resource.hpp>
#include <exe/future2/trait/combine_step.hpp>
#include <exe/future2/trait/make_step.hpp>
#include <exe/future2/trait/value_of.hpp>
#include <exe/future2/trait/value_type.hpp>

#include <util/type_traits.hpp>

#include <cstddef>
#include <new> // ::std::launder
#include <utility>

namespace exe::future {

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
using TraitsList = T::template List<>::template Type<I>;

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
using Type = detail::TraitsList<Traits, I>::ValueType;

template <typename Traits, ::std::size_t I, typename Consumer>
using Step = detail::TraitsList<Traits, I>::template Step<Consumer>;

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
  explicit Thunk(Maker &&m, Combinators &&...cs) noexcept
      : data_{{::std::move(m)}, {::std::move(cs)}...} {}

  using ValueType = trait::ValueOf<Traits>;

  template <concepts::Consumer<ValueType> Consumer>
  class ExecutionCore;

  template <concepts::Consumer<ValueType> Consumer>
  class Computation;

  template <typename Consumer>
  inline Computation<Consumer> Materialize(Consumer &&c) && noexcept {
    return Computation<Consumer>(::std::forward<Consumer>(c),
                                 ::std::move(*this));
  }

  template <typename C>
  using MakeStep = ExecutionCore<C>;

  template <typename ...Cs>
  inline ExtendedThunk<Cs...> Extend(Cs &&...c) && noexcept {
    return ::std::move(data_).Extend(::std::move(c)...);
  }

 private:
  Thunk(Data &&d) noexcept : data_(::std::move(d)) {}
};

template <typename ...Ts>
inline constexpr bool trait::Thunk<Thunk<Ts...>> = true;

////////////////////////////////////////////////////////////////////////////////

// [TODO]: Variant-like storage
template <typename ...Ts>
struct VariadicStorage;

// [TODO]: ?Out of Thunk
template <concepts::ThunkResource Maker,
          concepts::ThunkResource ...Combinators>
requires (concepts::CorrectPipeline<Maker, Combinators...>)
template <concepts::Consumer<
              trait::ValueOf<Thunk<Maker, Combinators...>>> Consumer>
class Thunk<Maker, Combinators...>::ExecutionCore {
 private:
  template <::std::size_t I>
  using InputType = Type<Traits, I>;

  template <::std::size_t I>
  using Redirect = core::Redirect<ExecutionCore, I>;

  template <::std::size_t I>
  using Step = Step<Traits, I, Redirect<I>>;

  Consumer cons_;
  Data &data_;

  static constexpr auto MaxSizeAlign() {
    return []<::std::size_t ...Is>(::std::index_sequence<Is...>) {
      return ::std::pair{::util::max_size_of_v<Step<Is>...>,
                         ::util::max_alignment_of_v<Step<Is>...>};
    }(::std::make_index_sequence<1 + sizeof...(Combinators)>{});
  }

  alignas (MaxSizeAlign().second) unsigned char buffer_[MaxSizeAlign().first];

 public:
  ExecutionCore(Consumer &&c, Thunk &t) noexcept
      : cons_(::std::forward<Consumer>(c))
      , data_(t.data_) {}

  void Start(Scheduler &s) && noexcept {
    auto &step = CreateStep<0>();
    ::std::move(step).Start(s);
  }

  template <::std::size_t I>
  void Continue(InputType<I> &&v, State s) && noexcept {
    // [TODO]: ?v by value (possibly dangling reference after GetStep())
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

    Redirect<I> consumer(*this);
    auto &resources = data_.template Get<I>();

    // [TODO]: Better
    static_assert(::std::is_nothrow_constructible_v<
                      Step, Redirect<I>, decltype(resources) &>);

    return *::new (buffer_) Step(::std::move(consumer), resources);
  }

  template <::std::size_t I>
  void DestroyStep() noexcept {
    using Step = Step<I>;

    // [TODO]: Better
    static_assert(::std::is_nothrow_destructible_v<Step>);

    ::std::launder(reinterpret_cast<Step *>(buffer_))->~Step();
  }
};

////////////////////////////////////////////////////////////////////////////////

template <concepts::ThunkResource Maker,
          concepts::ThunkResource ...Combinators>
requires (concepts::CorrectPipeline<Maker, Combinators...>)
template <concepts::Consumer<
              trait::ValueOf<Thunk<Maker, Combinators...>>> Consumer>
class Thunk<Maker, Combinators...>::Computation
    : private ExecutionCore<Consumer> {
 private:
  using Core = ExecutionCore<Consumer>;

  Data data_;

 public:
  ~Computation() = default;

  Computation(Computation const &) = delete;
  void operator= (Computation const &) = delete;

  Computation(Computation &&) = delete;
  void operator= (Computation &&) = delete;

 public:
  Computation(Consumer &&c, Thunk &&t) noexcept
      : Core(::std::forward<Consumer>(c), t)
      , data_(::std::move(t).data_) {}

  using Core::Start;
};

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_THUNK_HPP_INCLUDED_ */
