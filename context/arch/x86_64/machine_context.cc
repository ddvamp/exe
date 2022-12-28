#include "context/arch/x86_64/machine_context.h"

#include "utils/unreachable.h"

namespace context {

namespace {

[[noreturn]] void machineContextTrampoline(
	void *, void *, void *, void *, void *, void *, void *arg7) noexcept
{
	auto t = static_cast<ITrampoline *>(arg7);
	t->run();

	UTILS_UNREACHABLE("machineContextTrampoline out of bounds");
}

extern "C" void *setupMachineContext(void *stack,
	decltype(machineContextTrampoline) trampoline, void *arg) noexcept;

} // namespace

void MachineContext::setup(void *stack, ITrampoline *trampoline) noexcept
{
	rsp_ = setupMachineContext(stack, machineContextTrampoline, trampoline);
}

} // namespace context
