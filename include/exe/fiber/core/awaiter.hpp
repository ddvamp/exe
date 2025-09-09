//
//
//
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_EXE_FIBER_CORE_AWAITER_HPP_INCLUDED_
#define DDVAMP_EXE_FIBER_CORE_AWAITER_HPP_INCLUDED_ 1

#include <utility>

#include "exe/fiber/core/handle.hpp"

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

#endif /* DDVAMP_EXE_FIBER_CORE_AWAITER_HPP_INCLUDED_ */
