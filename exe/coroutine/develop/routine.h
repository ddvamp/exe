#ifndef DDV_EXE_COROUTINE_DEVELOP_ROUTINE_H_
#define DDV_EXE_COROUTINE_DEVELOP_ROUTINE_H_ 1

#include <functional>

namespace exe::coroutine {

using Routine = ::std::move_only_function<void() noexcept>;

} // namespace exe::coroutine

#endif /* DDV_EXE_COROUTINE_DEVELOP_ROUTINE_H_ */
