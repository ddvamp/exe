//
// pipe.hpp
// ~~~~~~~~
//
// Copyright (C) 2023-2026 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_SYNTAX_PIPE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_SYNTAX_PIPE_HPP_INCLUDED_ 1

#include <exe/future/fun/concept/future.hpp>

#include <utility>

namespace exe::future::pipe {

template <concepts::Future F, typename Op>
[[nodiscard]] inline decltype(auto) operator| (F f, Op op) {
  return ::std::move(op).Apply(::std::move(f));
}

} // namespace exe::future::pipe

#endif /* DDVAMP_EXE_FUTURE_FUN_SYNTAX_PIPE_HPP_INCLUDED_ */
