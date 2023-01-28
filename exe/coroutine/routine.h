// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_COROUTINE_ROUTINE_H_
#define DDV_EXE_COROUTINE_ROUTINE_H_ 1

#include <functional>

namespace exe::coroutine {

using Routine = ::std::move_only_function<void()>;

} // namespace exe::coroutine

#endif /* DDV_EXE_COROUTINE_ROUTINE_H_ */
