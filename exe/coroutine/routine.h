#ifndef DDV_EXE_COROUTINE_ROUTINE_H_
#define DDV_EXE_COROUTINE_ROUTINE_H_ 1

#include <functional>

namespace exe::coroutine {

using Routine = ::std::move_only_function<void()>;

} // namespace exe::coroutine

#endif /* DDV_EXE_COROUTINE_ROUTINE_H_ */
