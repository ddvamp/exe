// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_UTILS_ABORT_HPP_INCLUDED_
#define DDVAMP_UTILS_ABORT_HPP_INCLUDED_ 1

#include <source_location>
#include <string_view>

namespace utils {

// Abnormal program termination with passing an error message and location
[[noreturn]] void abort(::std::string_view const message,
                        ::std::source_location const location =
                            ::std::source_location::current()) noexcept;

}  // namespace utils

#ifndef UTILS_ABORT
#	define UTILS_ABORT(...) ::utils::abort(__VA_ARGS__) 
#else
# error "UTILS_ABORT macro is already defined somewhere else"
#endif

#endif  /* DDVAMP_UTILS_ABORT_HPP_INCLUDED_ */
