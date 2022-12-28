#ifndef DDV_EXE_COROUTINE_ROUTINE_H_
#define DDV_EXE_COROUTINE_ROUTINE_H_ 1

#include "utils/function.h"

namespace exe::coroutine {

using Routine = ::utils::function<void()>;

} // namespace exe::coroutine

#endif /* DDV_EXE_COROUTINE_ROUTINE_H_ */
