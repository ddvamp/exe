// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FIBER_CORE_HANDLE_H_
#define DDV_EXE_FIBER_CORE_HANDLE_H_ 1

#include "exe/fiber/core/fwd.h"

namespace exe::fiber {

// Class for managing fiber in awaiters
class [[nodiscard]] FiberHandle {
private:
	friend class Fiber;

	Fiber *fiber_ = nullptr;

public:
	// precondition: isValid() == false
	~FiberHandle();

	FiberHandle(FiberHandle const &) = delete;
	void operator= (FiberHandle const &) = delete;

	FiberHandle(FiberHandle &&that) noexcept
		: fiber_(that.release())
	{}
	// precondition: isValid() == false
	FiberHandle &operator= (FiberHandle &&) noexcept;

public:
	FiberHandle() = default;

	static FiberHandle invalid() noexcept
	{
		return FiberHandle{};
	}

	[[nodiscard]] bool isValid() const noexcept
	{
		return fiber_;
	}

	// Schedule execution on executor set on fiber
	//
	// Precondition: isValid() == true
	void schedule() && noexcept;

	// Execute fiber immediately
	//
	// Precondition: isValid() == true
	void resume() && noexcept;

private:
	explicit FiberHandle(Fiber *fiber) noexcept
		: fiber_(fiber)
	{}

	Fiber *release() noexcept;
	Fiber *releaseChecked() noexcept;
};

} // namespace exe::fiber

#endif /* DDV_EXE_FIBER_CORE_HANDLE_H_ */
