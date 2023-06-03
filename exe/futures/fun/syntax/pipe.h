// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_SYNTAX_PIPE_H_
#define DDV_EXE_FUTURES_FUN_SYNTAX_PIPE_H_ 1

#include <utility>

#include "exe/futures/fun/types/future.h"

namespace exe::futures::pipe {

// TODO:
//		trait -> is_combinator,
//		trait -> is_nothrow_combinator

template <concepts::Future F, typename C>
auto operator| (F &&f, C c)
{
	return c.mutate(::std::move(f));
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_SYNTAX_PIPE_H_ */
