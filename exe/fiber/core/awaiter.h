// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FIBERS_CORE_AWAITER_H_
#define DDV_EXE_FIBERS_CORE_AWAITER_H_ 1

#include <utility>

#include "exe/fiber/core/handle.h"

namespace exe::fiber {

class IAwaiter {
public:
	virtual ~IAwaiter() = default;

	virtual void awaitSuspend(FiberHandle &&) = 0;
	[[nodiscard]] virtual FiberHandle awaitSymmetricSuspend(FiberHandle &&) = 0;
};

class ISuspendingAwaiter : public IAwaiter {
public:
	[[nodiscard]] FiberHandle awaitSymmetricSuspend(FiberHandle &&h) override
	{
		awaitSuspend(::std::move(h));
		return FiberHandle::invalid();
	}
};

} // namespace exe::fiber

#endif /* DDV_EXE_FIBERS_CORE_AWAITER_H_ */
