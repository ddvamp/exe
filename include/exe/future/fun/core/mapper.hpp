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
#include <exe/future/fun/core/mapper_fwd.hpp>
#include <exe/future/fun/core/concept/mapper_value.hpp>
#include <exe/future/fun/result/unit.hpp>

#include <util/concepts.hpp>

#include <type_traits>
#include <functional>
#include <utility>

namespace exe::future::core {

template <concepts::MapperValue Fn>
class [[nodiscard]] Mapper {
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

} // namespace exe::future::core

#endif /* DDVAMP_EXE_FUTURE_FUN_CORE_MAPPER_HPP_INCLUDED_ */
