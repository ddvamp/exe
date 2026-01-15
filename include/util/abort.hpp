//
// abort.hpp
// ~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#ifndef DDVAMP_UTIL_ABORT_HPP_INCLUDED_
#define DDVAMP_UTIL_ABORT_HPP_INCLUDED_ 1

#include <source_location>
#include <string_view>

namespace util {

/* Abnormal program termination with passing an error message and location */
[[noreturn]] void abort(::std::string_view const message,
                        ::std::source_location const location =
                            ::std::source_location::current()) noexcept;

} // namespace util

#ifdef UTIL_ABORT
# error "UTIL_ABORT macro could not be defined because it is already defined somewhere else"
#else
# define UTIL_ABORT(...) ::util::abort(__VA_ARGS__)
#endif

#endif /* DDVAMP_UTIL_ABORT_HPP_INCLUDED_ */
