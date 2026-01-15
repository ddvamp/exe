//
// error.hpp
// ~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_RESULT_ERROR_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_RESULT_ERROR_HPP_INCLUDED_ 1

#include <util/debug/assert.hpp>

#include <exception>
#include <utility>

namespace exe::future {

using Error = ::std::exception_ptr;

template <typename T>
[[nodiscard]] inline constexpr Error MakeError(T &&t) noexcept {
  return ::std::make_exception_ptr(::std::forward<T>(t));
}

[[nodiscard]] inline constexpr Error CurrentError() noexcept {
  return ::std::current_exception();
}

[[noreturn]] inline constexpr void ThrowError(Error &&e) {
  UTIL_ASSERT(e, "Empty error in ThrowError()");
  ::std::rethrow_exception(::std::move(e));
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_RESULT_ERROR_HPP_INCLUDED_ */
