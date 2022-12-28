#ifndef DDV_UTILS_UNREACHABLE_H_
#define DDV_UTILS_UNREACHABLE_H_ 1

#include <source_location>
#include <string_view>
#include <utility>

namespace utils {

[[noreturn]] void unreachable(::std::string_view message,
	::std::source_location location =
	::std::source_location::current()) noexcept;

} // namespace utils

#ifdef UTILS_DISABLE_ASSERT
#	define UTILS_UNREACHABLE(message) ::std::unreachable()
#else
#	define UTILS_UNREACHABLE(message) \
		::utils::unreachable(message)
#endif

#ifdef UTILS_DISABLE_ASSERT
#	define UTILS_LOCATED_UNREACHABLE(message, location) ::std::unreachable()
#else
#	define UTILS_LOCATED_UNREACHABLE(message, location) \
		::utils::unreachable(message, location)
#endif

#endif /* DDV_UTILS_UNREACHABLE_H_ */
