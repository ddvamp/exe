// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_UTILS_ABORT_H_
#define DDV_UTILS_ABORT_H_ 1

#include <source_location>
#include <string_view>

namespace util {

// abnormal program termination with passing an error message and location
[[noreturn]] void abort(::std::string_view message,
	::std::source_location location =
	::std::source_location::current()) noexcept;

} // namespace util

#define UTILS_ABORT(...) ::util::abort(__VA_ARGS__)

#endif /* DDV_UTILS_ABORT_H_ */
