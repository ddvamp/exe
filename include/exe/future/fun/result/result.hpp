//
// result.hpp
// ~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_RESULT_RESULT_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_RESULT_RESULT_HPP_INCLUDED_ 1

#include <exe/future/fun/result/error.hpp>
#include <exe/result/result.hpp>

#include <expected>
#include <type_traits>
#include <utility>

namespace exe::future {

template <typename Value>
using Result = exe::Result<Value, Error>;

namespace result {

template <typename V>
[[nodiscard]] inline constexpr Result<V> Ok(V v)
    noexcept(::std::is_nothrow_move_constructible_v<V>) {
  return Result<V>(::std::move(v));
}

template <typename V>
[[nodiscard]] inline constexpr Result<V> Err(Error &&e) noexcept {
  return Result<V>(::std::unexpect_t{}, ::std::move(e));
}

} // namespace result

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_RESULT_RESULT_HPP_INCLUDED_ */
