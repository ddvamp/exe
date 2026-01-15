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

#include <exe/future/fun/core/adapt_call.hpp>
#include <exe/future/fun/core/concept/adapt_call.hpp>
#include <exe/future/fun/core/maker_fwd.hpp>
#include <exe/future/fun/core/concept/maker_value.hpp>

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
      noexcept (concepts::NothrowAdaptOutputInvocable<Fn &&>) {
    return core::AdaptOutput(::std::move(fn_));
  }
};

} // namespace exe::future::core

#endif /* DDVAMP_EXE_FUTURE_FUN_CORE_MAKER_HPP_INCLUDED_ */
