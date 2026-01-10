//
// mapper.hpp
// ~~~~~~~~~~
//
// Copyright (C) 2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_CORE_MAPPER_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_CORE_MAPPER_HPP_INCLUDED_ 1

#include <exe/future/fun/core/maker.hpp>
#include <exe/future/fun/result/unit.hpp>

#include <util/concepts.hpp>
#include <util/type_traits.hpp>

#include <functional>
#include <utility>

namespace exe::future::core {

template <typename Fn>
requires (::std::is_object_v<Fn> &&
          !::util::is_qualified_v<Fn> &&
          ::std::is_nothrow_destructible_v<Fn> &&
          ::std::is_nothrow_move_constructible_v<Fn>)
class Mapper {
 private:
  Fn fn_;

 public:
  ~Mapper() = default;

  Mapper(Mapper const &) = delete;
  void operator= (Mapper const &) = delete;

  Mapper(Mapper &&) = default;
  Mapper &operator= (Mapper &&) = default;

 public:
  explicit Mapper(Fn &&fn) noexcept : fn_(::std::move(fn)) {}

  [[nodiscard]] decltype(auto) operator() (Unit &&) &&
      noexcept (::std::is_nothrow_invocable_v<Fn &&>)
      requires (::std::is_invocable_v<Fn &&> &&
                !::std::is_invocable_v<Fn &&, Unit &&>) {
    return Maker(::std::move(fn_))();
  }

  template <::util::rvalue_deduced T>
  [[nodiscard]] decltype(auto) operator() (T &&t) &&
      noexcept (::std::is_nothrow_invocable_v<Fn &&, T &&>)
      requires (::std::is_invocable_v<Fn &&, T &&>) {
    return Maker([&] {
      return ::std::invoke(::std::move(fn_), ::std::move(t));
    })();
  }
};

namespace concepts {

namespace detail {

template <typename>
inline constexpr bool IsMapperImpl = false;

template <typename T>
inline constexpr bool IsMapperImpl<Mapper<T>> = true;

} // namespace detail

template <typename T>
concept Mapper = detail::IsMapperImpl<T>;

} // namespace concepts

} // namespace exe::future::core

#endif /* DDVAMP_EXE_FUTURE_FUN_CORE_MAPPER_HPP_INCLUDED_ */
