#ifndef DDV_EXE_FIBERS_API_H_
#define DDV_EXE_FIBERS_API_H_ 1

#include "exe/executors/executor.h"
#include "exe/fibers/core/awaiter.h"
#include "exe/fibers/core/routine.h"

namespace exe::fibers {

using executors::IExecutor;

// Start fiber on the executor
//
// Precondition: bool(routine) == true
void go(IExecutor &where, FiberRoutine routine);

// start fiber on the executor of current fiber
//
// Precondition: bool(routine) == true
void go(FiberRoutine routine);

namespace self {

[[nodiscard]] IExecutor &getExecutor() noexcept;

// For synchronization primitives
// Do not use directly
void suspend(IAwaiter &) noexcept;

// reschedule the current fiber
void yield();

// reschedule the current fiber and activate the next one if it is valid
// otherwise, the call is equivalent to yield
void switchTo(FiberHandle next) noexcept;

// reschedule current fiber on executor
void teleportTo(IExecutor &executor);

} // namespace self

} // namespace exe::fibers

#endif /* DDV_EXE_FIBERS_API_H_ */
