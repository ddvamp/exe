// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FIBER_CORE_STACK_H_
#define DDV_EXE_FIBER_CORE_STACK_H_ 1

#include "context/stack.hpp"

namespace exe::fiber {

using Stack = ::context::Stack;

[[nodiscard]] Stack allocateStack();

void deallocateStack(Stack &&stack) noexcept;

} // namespace exe::fiber

#endif /* DDV_EXE_FIBER_CORE_STACK_H_ */
