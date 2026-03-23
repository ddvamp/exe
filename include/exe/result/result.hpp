//
// result.hpp
// ~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_RESULT_RESULT_HPP_INCLUDED_
#define DDVAMP_EXE_RESULT_RESULT_HPP_INCLUDED_ 1

#include <exe/result/trait/result.hpp>

#include <expected>
#include <type_traits>
#include <utility>

namespace exe {

template <typename Value, typename Error>
using Result = ::std::expected<Value, Error>;

template <typename V, typename E>
inline constexpr bool trait::Result<Result<V, E>> = true;

namespace result {

template <typename V, typename E>
[[nodiscard]] inline constexpr Result<V, E> Ok(V v)
    noexcept (::std::is_nothrow_move_constructible_v<V>) {
  return Result<V, E>(::std::move(v));
}

template <typename V, typename E>
[[nodiscard]] inline constexpr Result<V, E> Err(E e)
    noexcept (::std::is_nothrow_move_constructible_v<E>) {
  return Result<V, E>(::std::unexpect_t{}, ::std::move(e));
}

} // namespace result

} // namespace exe

#endif /* DDVAMP_EXE_RESULT_RESULT_HPP_INCLUDED_ */
