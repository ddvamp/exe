#include <cstring>

#include "context/exceptions_context.h"

namespace __cxxabiv1 {

extern "C" struct __cxa_eh_globals *__cxa_get_globals() noexcept;

} // namespace __cxxabiv1

namespace context {

void ExceptionsContext::switchTo(ExceptionsContext &target) noexcept
{
	constexpr auto kStateSize = sizeof(ExceptionsContext);

	auto *this_thread_exceptions = ::__cxxabiv1::__cxa_get_globals();
	::std::memcpy(
		exceptions_state_buf_,
		this_thread_exceptions,
		kStateSize
	);
	::std::memcpy(
		this_thread_exceptions,
		target.exceptions_state_buf_,
		kStateSize
	);
}

} // namespace context
