#ifndef DDV_CONTEXT_ARCH_X86_64_MACHINE_CONTEXT_H_
#define DDV_CONTEXT_ARCH_X86_64_MACHINE_CONTEXT_H_ 1

#include "context/trampoline.h"

namespace context {

namespace detail {

extern "C" void switchMachineContext(void *from, void *to) noexcept;

} // namespace detail

class MachineContext {
private:
	void *rsp_;

public:
	void setup(void *stack, ITrampoline *trampoline) noexcept;

	void switchTo(MachineContext &target) noexcept
	{
		detail::switchMachineContext(&rsp_, &target.rsp_);
	}
};

} // namespace context

#endif /* DDV_CONTEXT_ARCH_X86_64_MACHINE_CONTEXT_H_ */
