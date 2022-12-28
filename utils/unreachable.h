#ifndef DDV_UTILS_UNREACHABLE_H_
#define DDV_UTILS_UNREACHABLE_H_ 1

#include <source_location>
#include <string_view>
#include <utility>

namespace utils::detail {

[[noreturn]] void unreachable_debug(::std::string_view message,
	::std::source_location location =
	::std::source_location::current()) noexcept;

} // namespace utils::detail

#ifdef UTILS_DISABLE_ASSERT
#	define UTILS_UNREACHABLE(message) ::std::unreachable()
#else
#	define UTILS_UNREACHABLE(message) \
		::utils::detail::unreachable_debug(message)
#endif

#ifdef UTILS_DISABLE_ASSERT
#	define UTILS_LOCATED_UNREACHABLE(message, location) ::std::unreachable()
#else
#	define UTILS_LOCATED_UNREACHABLE(message, location) \
		::utils::detail::unreachable_debug(message, location)
#endif

#endif /* DDV_UTILS_UNREACHABLE_H_ */
