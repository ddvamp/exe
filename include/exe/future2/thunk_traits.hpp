//
// thunk_traits.hpp
// ~~~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_THUNK_TRAITS_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_THUNK_TRAITS_HPP_INCLUDED_ 1

#include <exe/future2/model/combinator.hpp>
#include <exe/future2/model/maker.hpp>
#include <exe/future2/trait/combine_step.hpp>
#include <exe/future2/trait/make_step.hpp>
#include <exe/future2/trait/value_of.hpp>
#include <exe/future2/trait/value_type.hpp>

#include <cstddef>
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

////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////

template <typename T, ::std::size_t I>
using TraitsList = T::template List<>::template Type<I>;

template <typename ...>
struct TraitsHelper;

template <::std::size_t ...Is, typename ...Ts>
struct TraitsHelper<::std::index_sequence<Is...>, Ts...> {
  using Traits = TraitsImpl<Ts...[sizeof...(Is) - 1 - Is]...>;
};

} // namespace detail

////////////////////////////////////////////////////////////////////////////////

template <typename ...Ts>
using Traits =
    detail::TraitsHelper<::std::index_sequence_for<Ts...>, Ts...>::Traits;

template <typename Traits, ::std::size_t I>
using Type = detail::TraitsList<Traits, I>::ValueType;

template <typename Traits, ::std::size_t I, typename Consumer>
using Step = detail::TraitsList<Traits, I>::template Step<Consumer>;

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_THUNK_TRAITS_HPP_INCLUDED_ */
