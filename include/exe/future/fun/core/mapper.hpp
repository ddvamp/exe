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

#include <exe/future/fun/core/adapt_call.hpp>
#include <exe/future/fun/core/concept/adapt_call.hpp>
#include <exe/future/fun/core/maker.hpp>
#include <exe/future/fun/core/mapper_fwd.hpp>
#include <exe/future/fun/core/concept/mapper_value.hpp>

#include <util/concepts.hpp>

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

  template <::util::rvalue_deduced T>
  [[nodiscard]] decltype(auto) operator() (T &&t) &&
      noexcept (concepts::NothrowAdaptCallInvocable<Fn &&, T &&>)
      requires (concepts::AdaptCallInvocable<Fn &&, T &&>) {
    return core::AdaptCall(::std::move(fn_), ::std::move(t));
  }
};

} // namespace exe::future::core

#endif /* DDVAMP_EXE_FUTURE_FUN_CORE_MAPPER_HPP_INCLUDED_ */
