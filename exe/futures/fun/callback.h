// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_EXE_FUTURES_CORE_CALLBACK_H_
#define DDV_EXE_FUTURES_CORE_CALLBACK_H_ 1

#include <functional>

#include "result/result.h"

namespace exe::futures {

// Instead of synchronously waiting for future value
//
// User must take care of passing arguments, returning values,
// and handling exceptions himself
template <typename T>
using Callback =
	::std::move_only_function<void(::utils::result<T> &&) noexcept>;

} // namespace exe::futures

#endif /* DDV_EXE_FUTURES_CORE_CALLBACK_H_ */
