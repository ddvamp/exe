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

#include <exe/future/fun/result/unit.hpp>

#include <util/type_traits.hpp>

#include <utility>

namespace exe::future::core {

template <typename Fn>
requires (::std::is_object_v<Fn> &&
          !::util::is_qualified_v<Fn> &&
          ::std::is_nothrow_destructible_v<Fn> &&
          ::std::is_nothrow_move_constructible_v<Fn> &&
          ::std::is_invocable_v<Fn &&>)
class Maker {
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

namespace concepts {

namespace detail {

template <typename>
inline constexpr bool IsMakerImpl = false;

template <typename T>
inline constexpr bool IsMakerImpl<Maker<T>> = true;

} // namespace detail

template <typename T>
concept Maker = detail::IsMakerImpl<T>;

} // namespace concepts

} // namespace exe::future::core

#endif /* DDVAMP_EXE_FUTURE_FUN_CORE_MAKER_HPP_INCLUDED_ */
