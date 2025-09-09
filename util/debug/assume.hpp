// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

// to disable, set a macro UTIL_DISABLE_DEBUG

#ifndef DDVAMP_UTIL_DEBUG_ASSUME_HPP_INCLUDED_
#define DDVAMP_UTIL_DEBUG_ASSUME_HPP_INCLUDED_ 1

#include <source_location>
#include <string_view>

namespace util::detail {

[[noreturn]] void do_assume(::std::string_view assumption_expression,
	::std::string_view assumption_message,
	::std::source_location assumption_location =
	::std::source_location::current()) noexcept;

} // namespace util::detail

// debug assume with passing an error message and location
#ifdef UTIL_DISABLE_DEBUG
#	define UTIL_ASSUME(expression, ...) [[assume(expression)]]
#else
#	define UTIL_ASSUME(expression, ...)								\
		do {															\
			if (!(expression)) [[unlikely]] {							\
				::util::detail::do_assume(#expression, __VA_ARGS__);	\
			}															\
		} while (false)
#endif

#endif /* DDVAMP_UTIL_DEBUG_ASSUME_HPP_INCLUDED_ */
