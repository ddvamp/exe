// to disable, set a macro UTILS_DISABLE_DEBUG

#ifndef DDV_UTILS_DEBUG_ASSUME_H_
#define DDV_UTILS_DEBUG_ASSUME_H_ 1

#include <source_location>
#include <string_view>

namespace utils::detail {

[[noreturn]] void do_assume(::std::string_view assumption_expression,
	::std::string_view assumption_message,
	::std::source_location assumption_location =
	::std::source_location::current()) noexcept;

} // namespace utils::detail

// debug assume with passing an error message and location
#ifdef UTILS_DISABLE_DEBUG
#	define UTILS_ASSUME(expression, ...) [[assume(expression)]]
#else
#	define UTILS_ASSUME(expression, ...)								\
		do {															\
			if (!(expression)) [[unlikely]] {							\
				::utils::detail::do_assume(#expression, __VA_ARGS__);	\
			}															\
		} while (false)
#endif

#endif /* DDV_UTILS_DEBUG_ASSUME_H_ */
