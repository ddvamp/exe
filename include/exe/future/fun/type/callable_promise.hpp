//
// callable_promise.hpp
// ~~~~~~~~~~~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_TYPE_CALLABLE_PROMISE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_TYPE_CALLABLE_PROMISE_HPP_INCLUDED_ 1

#include <exe/future/fun/concept/future_value.hpp>
#include <exe/future/fun/result/error.hpp>
#include <exe/future/fun/result/result.hpp>
#include <exe/future/fun/type/promise.hpp>

#include <utility>

namespace exe::future {

/**
 *  Functor-like promise
 */
template <concepts::FutureValue T>
class CallablePromise : protected Promise<T> {
 public:
  explicit CallablePromise(Promise<T> &&p) noexcept
      : Promise<T>(::std::move(p)) {}

  [[nodiscard("Pure")]] explicit operator bool () const noexcept {
    return this->IsValid();
  }

  void operator() (Result<T> &&r) && noexcept {
    ::std::move(*this).SetResult(::std::move(r));
  }

  void operator() (T &&v) && noexcept {
    ::std::move(*this).SetValue(::std::move(v));
  }

  void operator() (Error &&e) && noexcept {
    ::std::move(*this).SetError(::std::move(e));
  }
};

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_TYPE_CALLABLE_PROMISE_HPP_INCLUDED_ */
