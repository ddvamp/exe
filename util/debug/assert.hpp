// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

// to disable, set a macro UTIL_DISABLE_DEBUG

#ifndef DDV_UTIL_DEBUG_ASSERT_H_
#define DDV_UTIL_DEBUG_ASSERT_H_ 1

#include <source_location>
#include <string_view>

#include "util/macro.hpp"

namespace util::detail {

[[noreturn]] void do_assert(::std::string_view assertion_expression,
	::std::string_view assertion_message,
	::std::source_location assertion_location =
	::std::source_location::current()) noexcept;

} // namespace util::detail

// runtime check with passing an error message and location
#define UTIL_CHECK(expression, ...)								\
	do {															\
		if (!(expression)) [[unlikely]] {							\
			::util::detail::do_assert(#expression, __VA_ARGS__);	\
		}															\
	} while (false)

// debug assert with passing an error message and location
#ifdef UTIL_DISABLE_DEBUG
#	define UTIL_ASSERT(expression, ...) UTIL_NOTHING
#else
#	define UTIL_ASSERT(expression, ...) UTIL_CHECK(expression, __VA_ARGS__)
#endif

// similar to UTIL_ASSERT, but anyway calculates the expression
#ifdef UTIL_DISABLE_DEBUG
#	define UTIL_VERIFY(expression, ...) UTIL_IGNORE(expression)
#else
#	define UTIL_VERIFY(expression, ...) UTIL_CHECK(expression, __VA_ARGS__)
#endif

#endif /* DDV_UTIL_DEBUG_ASSERT_H_ */
