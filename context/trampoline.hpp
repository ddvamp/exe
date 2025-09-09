// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_CONTEXT_TRAMPOLINE_H_
#define DDV_CONTEXT_TRAMPOLINE_H_ 1

#include "util/debug.hpp"

namespace context {

// entry point for context
class ITrampoline {
public:
	virtual ~ITrampoline() = default;

	[[noreturn]] void run() noexcept
	{
		doRun();
		UTIL_UNREACHABLE("ITrampoline::run out of bounds");
	}

private:
	[[noreturn]] virtual void doRun() noexcept = 0;
};

} // namespace context

#endif /* DDV_CONTEXT_TRAMPOLINE_H_ */
