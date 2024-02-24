// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_UTILS_DEBUG_ASSERT_HPP_INCLUDED_
#define DDVAMP_UTILS_DEBUG_ASSERT_HPP_INCLUDED_ 1

// To disable, set macro UTILS_DISABLE_DEBUG

#include <source_location>
#include <string_view>

#include <utils/macro.hpp>

namespace utils::detail {

[[noreturn]] void do_assert(::std::string_view const expression,
    ::std::string_view const message, ::std::source_location const location =
    ::std::source_location::current()) noexcept;

} // namespace utils::detail

// Runtime check with passing an error message and location
#ifndef UTILS_CHECK
#   define UTILS_CHECK(expr, ...)                               \
        do {                                                    \
            if (!(expr)) [[unlikely]] {                         \
                ::utils::detail::do_assert(#expr, __VA_ARGS__); \
            }                                                   \
        } while (false)
#else
#   error "UTILS_CHECK macro is already defined somewhere else"
#endif

// Debug assert with passing an error message and location
#ifndef UTILS_ASSERT
#   ifndef UTILS_DISABLE_DEBUG
#	    define UTILS_ASSERT(expr, ...) UTILS_CHECK(expr, __VA_ARGS__)
#   else
#	    define UTILS_ASSERT(expr, ...) UTILS_NOTHING
#   endif
#else
#   error "UTILS_ASSERT macro is already defined somewhere else"
#endif

// Similar to UTILS_ASSERT, but anyway calculates expr
#ifndef UTILS_VERIFY
#   ifndef UTILS_DISABLE_DEBUG
#	    define UTILS_VERIFY(expr, ...) UTILS_CHECK(expr, __VA_ARGS__)
#   else
#	    define UTILS_VERIFY(expr, ...) UTILS_IGNORE(expr)
#   endif
#else
#   error "UTILS_VERIFY macro is already defined somewhere else"
#endif

#endif /* DDVAMP_UTILS_DEBUG_ASSERT_HPP_INCLUDED_ */
