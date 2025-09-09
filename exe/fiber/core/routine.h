// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FIBER_CORE_ROUTINE_H_
#define DDV_EXE_FIBER_CORE_ROUTINE_H_ 1

#include <functional>

namespace exe::fiber {

using FiberRoutine = ::std::move_only_function<void() noexcept>;

} // namespace exe::fiber

#endif /* DDV_EXE_FIBER_CORE_ROUTINE_H_ */
