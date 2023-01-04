#ifndef DDV_EXE_FIBERS_CORE_STACK_H_
#define DDV_EXE_FIBERS_CORE_STACK_H_ 1

#include "context/stack.h"

namespace exe::fibers {

using Stack = ::context::Stack;

[[nodiscard]] Stack allocateStack();

void deallocateStack(Stack &&stack) noexcept;

} // namespace exe::fibers

#endif /* DDV_EXE_FIBERS_CORE_STACK_H_ */
