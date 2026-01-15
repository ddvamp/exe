//
// operator.hpp
// ~~~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_CORE_OPERATOR_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_CORE_OPERATOR_HPP_INCLUDED_ 1

#include <exe/future/fun/core/operator_fwd.hpp>
#include <exe/future/fun/detail/callback.hpp>
#include <exe/future/fun/type/future.hpp>
#include <exe/future/fun/type/scheduler.hpp>

#include <type_traits>
#include <utility>

namespace exe::future::core {

class Operator {
 protected:
  ~Operator() = default;

  Operator(Operator const &) = delete;
  void operator= (Operator const &) = delete;

  Operator(Operator &&) = delete;
  void operator= (Operator &&) = delete;

 protected:
  Operator() = default;

  template <typename T>
  [[nodiscard("Pure")]] static bool IsValid(SemiFuture<T> const &f) noexcept {
    return f.HasState();
  }

  template <typename T>
  [[nodiscard("Pure")]] static Scheduler &GetScheduler(Future<T> const &f)
      noexcept {
    return *f.GetStateChecked()->GetScheduler();
  }

  template <typename T>
  static Future<T> SetScheduler(SemiFuture<T> f, Scheduler &where) noexcept {
    auto const state = f.ReleaseChecked();
    state->SetScheduler(where);
    return Future<T>(state);
  }

  template <typename T>
  static void SetCallback(Future<T> f,
                          ::std::type_identity_t<detail::Callback<T>> &&cb)
      noexcept {
    f.ReleaseChecked()->SetCallback(::std::move(cb));
  }
};

} // namespace exe::future::core

#endif /* DDVAMP_EXE_FUTURE_FUN_CORE_OPERATOR_HPP_INCLUDED_ */
