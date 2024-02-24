// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_UTILS_DEBUG_ASSUME_HPP_INCLUDED_
#define DDVAMP_UTILS_DEBUG_ASSUME_HPP_INCLUDED_ 1

// To disable, set macro UTILS_DISABLE_DEBUG

#include <source_location>
#include <string_view>

namespace utils::detail {

[[noreturn]] void do_assume(::std::string_view const expression,
    ::std::string_view const message, ::std::source_location const location =
    ::std::source_location::current()) noexcept;

} // namespace utils::detail

// Debug assume with passing an error message and location
#ifndef UTILS_ASSUME
#   ifndef UTILS_DISABLE_DEBUG
#	    define UTILS_ASSUME(expr, ...)                              \
            do {                                                    \
                if (!(expr)) [[unlikely]] {                         \
                    ::utils::detail::do_assume(#expr, __VA_ARGS__); \
                }                                                   \
            } while (false)
#   else
#	    define UTILS_ASSUME(expr, ...) [[assume(expr)]]
#   endif
#else
#   error "UTILS_ASSUME macro is already defined somewhere else"
#endif

#endif /* DDVAMP_UTILS_DEBUG_ASSUME_HPP_INCLUDED_ */
