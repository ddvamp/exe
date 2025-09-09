//
//
//
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FUTURE_FUN_SYNTAX_PIPE_HPP_INCLUDED_
#define DDVAMP_EXE_FUTURE_FUN_SYNTAX_PIPE_HPP_INCLUDED_ 1

#include <utility>

#include "exe/future/fun/mutator/fwd.hpp"
#include "exe/future/fun/types/future.hpp"

namespace exe::future::pipe {

template <concepts::Future F, concepts::Mutator M>
auto operator| (F &&f, M m) noexcept (M::template mutates_nothrow<F>)
{
	return m.mutate(::std::move(f));
}

} // namespace exe::future::pipe

#endif /* DDVAMP_EXE_FUTURE_FUN_SYNTAX_PIPE_HPP_INCLUDED_ */
