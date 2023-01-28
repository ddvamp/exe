// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

// to disable, set a macro UTILS_DISABLE_DEBUG

#ifndef DDV_UTILS_DEBUG_UNREACHABLE_H_
#define DDV_UTILS_DEBUG_UNREACHABLE_H_ 1

#include <source_location>
#include <string_view>
#include <utility>

namespace utils::detail {

[[noreturn]] void do_unreachable(::std::string_view message,
	::std::source_location location =
	::std::source_location::current()) noexcept;

} // namespace utils::detail

// debug unreachable with passing an error message and location
#ifdef UTILS_DISABLE_DEBUG
#	define UTILS_UNREACHABLE(...) ::std::unreachable()
#else
#	define UTILS_UNREACHABLE(...) ::utils::detail::do_unreachable(__VA_ARGS__)
#endif

#endif /* DDV_UTILS_DEBUG_UNREACHABLE_H_ */
