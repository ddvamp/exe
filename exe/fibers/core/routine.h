#ifndef DDV_EXE_FIBERS_CORE_ROUTINE_H_
#define DDV_EXE_FIBERS_CORE_ROUTINE_H_ 1

#include <functional>

namespace exe::fibers {

using FiberRoutine = ::std::move_only_function<void() noexcept>;

} // namespace exe::fibers

#endif /* DDV_EXE_FIBERS_CORE_ROUTINE_H_ */
