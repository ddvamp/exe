// to disable, set a macro UTILS_DISABLE_DEBUG

#ifndef DDV_UTILS_DEBUG_ASSERT_H_
#define DDV_UTILS_DEBUG_ASSERT_H_ 1

#include <source_location>
#include <string_view>

#include "utils/macro.h"

namespace utils::detail {

[[noreturn]] void do_assert(::std::string_view assertion_expression,
	::std::string_view assertion_message,
	::std::source_location assertion_location =
	::std::source_location::current()) noexcept;

} // namespace utils::detail

// runtime check with passing an error message and location
#define UTILS_CHECK(expression, ...)								\
	do {															\
		if (!(expression)) [[unlikely]] {							\
			::utils::detail::do_assert(#expression, __VA_ARGS__);	\
		}															\
	} while (false)

// debug assert with passing an error message and location
#ifdef UTILS_DISABLE_DEBUG
#	define UTILS_ASSERT(expression, ...) UTILS_NOTHING
#else
#	define UTILS_ASSERT(expression, ...) UTILS_CHECK(expression, __VA_ARGS__)
#endif

// similar to UTILS_ASSERT, but anyway calculates the expression
#ifdef UTILS_DISABLE_DEBUG
#	define UTILS_VERIFY(expression, ...) UTILS_IGNORE(expression)
#else
#	define UTILS_VERIFY(expression, ...) UTILS_CHECK(expression, __VA_ARGS__)
#endif

#endif /* DDV_UTILS_DEBUG_ASSERT_H_ */
