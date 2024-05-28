// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_UTIL_DEBUG_ASSUME_HPP_INCLUDED_
#define DDVAMP_UTIL_DEBUG_ASSUME_HPP_INCLUDED_ 1

// To disable, set macro UTIL_DISABLE_DEBUG

#include <source_location>
#include <string_view>

namespace util::detail {

[[noreturn]] void do_assume(::std::string_view const expression,
                            ::std::string_view const message,
                            ::std::source_location const location =
                                ::std::source_location::current()) noexcept;

}  // namespace util::detail

// Debug assume with passing an error message and location
#ifndef UTIL_ASSUME
# ifndef UTIL_DISABLE_DEBUG
#	  define UTIL_ASSUME(expr, ...)                         \
        do {                                              \
          if (expr) [[likely]] {                          \
            break;                                        \
          }                                               \
          ::util::detail::do_assume(#expr, __VA_ARGS__);  \
        } while (false)
# else
#	  define UTIL_ASSUME(expr, ...) [[assume(expr)]]
# endif
#else
# error "UTIL_ASSUME macro is already defined somewhere else"
#endif

#endif  /* DDVAMP_UTIL_DEBUG_ASSUME_HPP_INCLUDED_ */
