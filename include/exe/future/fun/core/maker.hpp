//
// maker.hpp
// ~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_CORE_MAKER_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_CORE_MAKER_HPP_INCLUDED_ 1

#include <exe/future/fun/core/maker_fwd.hpp>
#include <exe/future/fun/core/concept/maker_value.hpp>
#include <exe/future/fun/result/unit.hpp>

#include <type_traits>
#include <utility>

namespace exe::future::core {

template <concepts::MakerValue Fn>
class [[nodiscard]] Maker {
 private:
  Fn fn_;

 public:
  ~Maker() = default;

  Maker(Maker const &) = delete;
  void operator= (Maker const &) = delete;

  Maker(Maker &&) = default;
  Maker &operator= (Maker &&) = default;

 public:
  explicit Maker(Fn &&fn) noexcept : fn_(::std::move(fn)) {}

  [[nodiscard]] decltype(auto) operator() () &&
      noexcept (::std::is_nothrow_invocable_v<Fn &&>) {
    if constexpr (::std::is_void_v<::std::invoke_result_t<Fn &&>>) {
      ::std::move(fn_)();
      return Unit{};
    } else {
      return ::std::move(fn_)();
    }
  }
};

} // namespace exe::future::core

#endif /* DDVAMP_EXE_FUTURE_FUN_CORE_MAKER_HPP_INCLUDED_ */
