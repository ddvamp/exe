// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_FUN_COMBINE_SEQ_VIA_H_
#define DDV_EXE_FUTURES_FUN_COMBINE_SEQ_VIA_H_ 1

#include <utility>

#include "exe/executors/executor.h"
#include "exe/futures/fun/mutator/mutator.h"
#include "exe/futures/fun/syntax/pipe.h"

namespace exe::futures {

namespace pipe {

struct [[nodiscard]] Via : detail::Mutator {
	executors::IExecutor &executor;

	template <typename T>
	auto mutate(SemiFuture<T> f) noexcept
	{
		return setExecutor(::std::move(f), executor);
	}
};

} // namespace pipe

inline auto via(executors::IExecutor &where) noexcept
{
	return pipe::Via{{}, where};
}

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_FUN_COMBINE_SEQ_VIA_H_ */
