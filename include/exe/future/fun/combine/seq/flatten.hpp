//
// flatten.hpp
// ~~~~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_FLATTEN_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_FLATTEN_HPP_INCLUDED_ 1

#include <exe/future/fun/combine/seq/flat_map.hpp>
#include <exe/future/fun/concept/future.hpp>
#include <exe/future/fun/core/operator.hpp>
#include <exe/future/fun/syntax/pipe.hpp> // IWYU pragma: export
#include <exe/future/fun/trait/value_of.hpp>
#include <exe/future/fun/type/future_fwd.hpp>
#include <exe/runtime/inline.hpp>

#include <utility>

namespace exe::future {

namespace pipe {

class [[nodiscard]] Flatten : private core::Operator {
 public:
  template <concepts::Future F>
  SemiFuture<trait::ValueOf<F>> Apply(SemiFuture<F> f) && {
    return ::std::move(*this).Apply(
        SetScheduler(::std::move(f), runtime::GetInline()));
  }

  template <concepts::Future F>
  Future<trait::ValueOf<F>> Apply(Future<F> f) && {
    return ::std::move(f) |
        future::FlatMap([](F &&v) noexcept { return ::std::move(v); });
  }
};

} // namespace pipe

inline auto Flatten() {
  return pipe::Flatten();
}

} // namespace exe::future

#endif /* DDVAMP_EXE_FUTURE_FUN_COMBINE_SEQ_FLATTEN_HPP_INCLUDED_ */
