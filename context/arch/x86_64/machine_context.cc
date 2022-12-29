#include "context/arch/x86_64/machine_context.h"

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

void MachineContext::setup(::utils::memory_view stack,
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
