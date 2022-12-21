#ifndef DDV_UTILS_ABORT_H_
#define DDV_UTILS_ABORT_H_ 1

#include <source_location>
#include <string_view>

namespace utils {

// abnormal program termination with error message
[[noreturn]] void abort(::std::string_view message,
	::std::source_location location =
	::std::source_location::current()) noexcept;

} // namespace utils

#endif /* DDV_UTILS_ABORT_H_ */
