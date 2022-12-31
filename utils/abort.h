#ifndef DDV_UTILS_ABORT_H_
#define DDV_UTILS_ABORT_H_ 1

#include <source_location>
#include <string_view>

namespace utils {

// abnormal program termination with passing an error message and location
[[noreturn]] void abort(::std::string_view message,
	::std::source_location location =
	::std::source_location::current()) noexcept;

} // namespace utils

#define UTILS_ABORT(...) ::utils::abort(__VA_ARGS__) 

#endif /* DDV_UTILS_ABORT_H_ */
