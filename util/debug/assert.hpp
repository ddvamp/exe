// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_UTIL_DEBUG_ASSERT_HPP_INCLUDED_
#define DDVAMP_UTIL_DEBUG_ASSERT_HPP_INCLUDED_ 1

// To disable, set macro UTIL_DISABLE_DEBUG

#include <source_location>
#include <string_view>

#include <util/macro.hpp>

namespace util::detail {

[[noreturn]] void do_assert(::std::string_view const expression,
                            ::std::string_view const message,
                            ::std::source_location const location =
                                ::std::source_location::current()) noexcept;

}  // namespace util::detail

// Runtime check with passing an error message and location
#ifndef UTIL_CHECK
# define UTIL_CHECK(expr, ...)                          \
      do {                                              \
        if (expr) [[likely]] {                          \
          break;                                        \
        }                                               \
        ::util::detail::do_assert(#expr, __VA_ARGS__);  \
      } while (false)
#else
# error "UTIL_CHECK macro is already defined somewhere else"
#endif

// Debug assert with passing an error message and location
#ifndef UTIL_ASSERT
# ifndef UTIL_DISABLE_DEBUG
#	  define UTIL_ASSERT(expr, ...) UTIL_CHECK(expr, __VA_ARGS__)
# else
#	  define UTIL_ASSERT(expr, ...) UTIL_NOTHING
# endif
#else
# error "UTIL_ASSERT macro is already defined somewhere else"
#endif

// Similar to UTIL_ASSERT, but anyway calculates expr
#ifndef UTIL_VERIFY
# ifndef UTIL_DISABLE_DEBUG
#	  define UTIL_VERIFY(expr, ...) UTIL_CHECK(expr, __VA_ARGS__)
# else
#	  define UTIL_VERIFY(expr, ...) UTIL_IGNORE(expr)
# endif
#else
# error "UTIL_VERIFY macro is already defined somewhere else"
#endif

#endif  /* DDVAMP_UTIL_DEBUG_ASSERT_HPP_INCLUDED_ */
