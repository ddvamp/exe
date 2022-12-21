// to disable asserts, set a macro UTILS_DISABLE_ASSERT

#ifndef DDV_UTILS_ASSERT_H_
#define DDV_UTILS_ASSERT_H_ 1

#include <source_location>
#include <string_view>

#include "utils/macro.h"

namespace utils::detail {

[[noreturn]] void do_assert(::std::string_view assertion_expression,
	::std::string_view assertion_message,
	::std::source_location assertion_location) noexcept;

} // namespace utils::detail

// runtime cheking with error message and location passing
#define UTILS_LOCATED_CHECK(expression, message, location)				\
	do {																\
		if (!(expression)) [[unlikely]] {								\
			::utils::detail::do_assert(#expression, message, location);	\
		}																\
	} while (false)

#define UTILS_CHECK(expression, message) \
	UTILS_LOCATED_CHECK(expression, message, UTILS_CURRENT_LOCATION)

// DEBUG cheking with error message and location passing
#ifdef UTILS_DISABLE_ASSERT
#	define UTILS_ASSERT(expression, message) UTILS_NOTHING
#else
#	define UTILS_ASSERT(expression, message) UTILS_CHECK(expression, message)
#endif

#ifdef UTILS_DISABLE_ASSERT
#	define UTILS_LOCATED_ASSERT(expression, message, location) UTILS_NOTHING
#else
#	define UTILS_LOCATED_ASSERT(expression, message, location) \
		UTILS_LOCATED_CHECK(expression, message, location)
#endif

// similar to UTILS_ASSERT, but anyway calculates the expression
#ifdef UTILS_DISABLE_ASSERT
#	define UTILS_VERIFY(expression, message) UTILS_IGNORE(expression)
#else
#	define UTILS_VERIFY(expression, message) UTILS_CHECK(expression, message)
#endif

#ifdef UTILS_DISABLE_ASSERT
#	define UTILS_LOCATED_VERIFY(expression, message, location) \
		UTILS_IGNORE(expression)
#else
#	define UTILS_LOCATED_VERIFY(expression, message, location) \
		UTILS_LOCATED_CHECK(expression, message, location)
#endif

#endif /* DDV_UTILS_ASSERT_H_ */
