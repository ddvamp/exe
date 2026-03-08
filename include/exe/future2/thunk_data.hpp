//
// thunk_data.hpp
// ~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_THUNK_DATA_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_THUNK_DATA_HPP_INCLUDED_ 1

#include <cstddef>
#include <utility>

namespace exe::future {

namespace detail {

// [TODO]: ?Inheritance for non-final class types
template <typename T, ::std::size_t>
struct ThunkVal {
  [[no_unique_address]] T val;
};

template <typename ...>
struct ThunkDataImpl;

} // namespace detail

template <typename ...Ts>
using ThunkData =
    detail::ThunkDataImpl<::std::index_sequence_for<Ts...>, Ts...>;

namespace detail {

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
    using D = ThunkData<Us...>;
    return {static_cast<Leaf<Is> &&>(*this)...,
            {static_cast<D::template Leaf<Js> &&>(d).val}...};
  }
};

template <typename ...Ts>
ThunkDataImpl(Ts...) -> ThunkDataImpl<::std::index_sequence_for<Ts...>, Ts...>;

} // namespace detail

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_THUNK_DATA_HPP_INCLUDED_ */
