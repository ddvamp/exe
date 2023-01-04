#ifndef DDV_EXE_FIBERS_CORE_AWAITER_H_
#define DDV_EXE_FIBERS_CORE_AWAITER_H_ 1

#include <utility>

#include "exe/fibers/core/handle.h"

namespace exe::fibers {

class IAwaiter {
public:
	virtual ~IAwaiter() = default;

	virtual void awaitSuspend(FiberHandle &&) = 0;
	virtual FiberHandle awaitSymmetricSuspend(FiberHandle &&) = 0;
};

class ISuspendingAwaiter : public IAwaiter {
public:
	FiberHandle awaitSymmetricSuspend(FiberHandle &&h) override
	{
		awaitSuspend(::std::move(h));
		return FiberHandle::invalid();
	}
};

} // namespace exe::fibers

#endif /* DDV_EXE_FIBERS_CORE_AWAITER_H_ */
