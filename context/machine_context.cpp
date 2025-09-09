//
//
//
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#include "context/machine_context.hpp"

namespace context {

namespace {

[[noreturn]] void machineContextTrampoline(
	void *, void *, void *, void *, void *, void *, void *arg7) noexcept
{
	auto t = static_cast<ITrampoline *>(arg7);
	t->run();
}

extern "C" void *setupMachineContext(void *stack,
	decltype(machineContextTrampoline) trampoline, void *arg) noexcept;

extern "C" void switchMachineContext(void **from, void **to) noexcept;

} // namespace

void MachineContext::setup(::util::memory_view stack,
	ITrampoline *trampoline) noexcept
{
	rsp_ = setupMachineContext(
		&stack.back(),
		machineContextTrampoline,
		trampoline
	);
}

void MachineContext::switchTo(MachineContext &target) noexcept
{
	switchMachineContext(&rsp_, &target.rsp_);
}

} // namespace context
