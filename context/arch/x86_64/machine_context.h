#ifndef DDV_CONTEXT_ARCH_X86_64_MACHINE_CONTEXT_H_
#define DDV_CONTEXT_ARCH_X86_64_MACHINE_CONTEXT_H_ 1

#include "context/trampoline.h"

#include "utils/memory/view.h"

namespace context {

class MachineContext {
private:
	void *rsp_;

public:
	void setup(::utils::memory_view stack, ITrampoline *trampoline) noexcept;

	void switchTo(MachineContext &target) noexcept;
};

} // namespace context

#endif /* DDV_CONTEXT_ARCH_X86_64_MACHINE_CONTEXT_H_ */
