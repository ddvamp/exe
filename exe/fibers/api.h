#ifndef DDV_EXE_FIBERS_API_H_
#define DDV_EXE_FIBERS_API_H_ 1

#include "exe/executors/executor.h"
#include "exe/fibers/core/awaiter.h"
#include "exe/fibers/core/routine.h"

namespace exe::fibers {

using executors::IExecutor;

// start fiber on the executor
void go(IExecutor &where, FiberRoutine);

// start fiber on the executor of current fiber
void go(FiberRoutine);

namespace self {

// For synchronization primitives
// Do not use directly
void suspend(IAwaiter *) noexcept;

void yield();

// reschedule current fiber and activate the next
void switchTo(FiberHandle next) noexcept;

} // namespace self

} // namespace exe::fibers

#endif /* DDV_EXE_FIBERS_API_H_ */
