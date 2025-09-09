// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_SYNTAX_PIPE_H_
#define DDV_EXE_FUTURES_FUN_SYNTAX_PIPE_H_ 1

#include <utility>

#include "exe/future/fun/mutator/fwd.h"
#include "exe/future/fun/types/future.h"

namespace exe::future::pipe {

template <concepts::Future F, concepts::Mutator M>
auto operator| (F &&f, M m) noexcept (M::template mutates_nothrow<F>)
{
	return m.mutate(::std::move(f));
}

} // namespace exe::future::pipe

#endif /* DDV_EXE_FUTURES_FUN_SYNTAX_PIPE_H_ */
