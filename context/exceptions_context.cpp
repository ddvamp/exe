// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#include <cstring>

#include "context/exceptions_context.hpp"

namespace __cxxabiv1 {

extern "C" struct __cxa_eh_globals *__cxa_get_globals() noexcept;

} // namespace __cxxabiv1

namespace context {

void ExceptionsContext::switchTo(ExceptionsContext &target) noexcept
{
	constexpr auto kStateSize = sizeof(exceptions_state_buf_);

	auto this_thread_exceptions = ::__cxxabiv1::__cxa_get_globals();

	decltype(exceptions_state_buf_) tmp;

	// prevent aliasing
	::std::memcpy(tmp, target.exceptions_state_buf_, kStateSize);
	::std::memcpy(exceptions_state_buf_, this_thread_exceptions, kStateSize);
	::std::memcpy(this_thread_exceptions, tmp, kStateSize);
}

} // namespace context
