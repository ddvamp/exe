#ifndef DDV_EXE_FIBERS_CORE_HANDLE_H_
#define DDV_EXE_FIBERS_CORE_HANDLE_H_ 1

#include "exe/fibers/core/fwd.h"

namespace exe::fibers {

// Class for managing a fiber from user code
// 
// The behavior is undefined if more than one or none
// of all copies of the same handler are used for fiber execution
class [[nodiscard]] FiberHandle {
private:
	friend class Fiber;

	Fiber *fiber_ = nullptr;

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

	// schedule execution on the executor set on fiber
	void schedule() noexcept;

	// execute fiber immediately
	void resume() noexcept;

private:
	explicit FiberHandle(Fiber *fiber) noexcept
		: fiber_(fiber)
	{}

	Fiber *release() noexcept;
};

} // namespace exe::fibers

#endif /* DDV_EXE_FIBERS_CORE_HANDLE_H_ */
