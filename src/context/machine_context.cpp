//
// machine_context.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
//
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.
//

#include <context/machine_context.hpp>

namespace context {

namespace {

[[noreturn]] void MachineContextTrampoline(void *, void *, void *,
                                           void *, void *, void *,
                                           void *arg7) noexcept {
  static_cast<ITrampoline *>(arg7)->Run();
}

extern "C" void *SetupMachineContext(
    void *stack, decltype(MachineContextTrampoline) trampoline,
    void *arg) noexcept;

extern "C" void SwitchMachineContext(void **from, void **to) noexcept;

} // namespace

void MachineContext::Setup(::util::memory_view stack,
                           ITrampoline *trampoline) noexcept {
  rsp_ = SetupMachineContext(&stack.back(), MachineContextTrampoline,
                             trampoline);
}

void MachineContext::SwitchTo(MachineContext &target) noexcept {
  SwitchMachineContext(&rsp_, &target.rsp_);
}

} // namespace context
