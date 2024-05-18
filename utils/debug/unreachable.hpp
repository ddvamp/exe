// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_UTILS_DEBUG_UNREACHABLE_HPP_INCLUDED_
#define DDVAMP_UTILS_DEBUG_UNREACHABLE_HPP_INCLUDED_ 1

// To disable, set macro UTILS_DISABLE_DEBUG

#include <source_location>
#include <string_view>
#include <utility>

namespace utils::detail {

[[noreturn]] void do_unreachable(
    ::std::string_view const message,
    ::std::source_location const location =
        ::std::source_location::current()) noexcept;

}  // namespace utils::detail

// Debug unreachable with passing an error message and location
#ifndef UTILS_UNREACHABLE
#	ifndef UTILS_DISABLE_DEBUG
#		define UTILS_UNREACHABLE(...) ::utils::detail::do_unreachable(__VA_ARGS__)
#	else
#		define UTILS_UNREACHABLE(...) ::std::unreachable()
#	endif
#else
#	error "UTILS_UNREACHABLE macro is already defined somewhere else"
#endif

#endif  /* DDVAMP_UTILS_DEBUG_UNREACHABLE_HPP_INCLUDED_ */
