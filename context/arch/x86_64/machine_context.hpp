// Copyright (C) 2023 Artyom Kolpakov <ddvamp007@gmail.com>
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDV_CONTEXT_ARCH_X86_64_MACHINE_CONTEXT_H_
#define DDV_CONTEXT_ARCH_X86_64_MACHINE_CONTEXT_H_ 1

#include "context/trampoline.hpp"

#include "util/memory/view.hpp"

namespace context {

class MachineContext {
private:
	void *rsp_;

public:
	// set initial context
	void setup(::util::memory_view stack, ITrampoline *trampoline) noexcept;

	// save current context in this and reset target context
	// (this and target are allowed to be aliased)
	void switchTo(MachineContext &target) noexcept;
};

} // namespace context

#endif /* DDV_CONTEXT_ARCH_X86_64_MACHINE_CONTEXT_H_ */
