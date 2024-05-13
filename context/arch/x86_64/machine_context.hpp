// Copyright (C) 2023-2024 Artyom Kolpakov <ddvamp007@gmail.com>
// 
// Licensed under GNU GPL-3.0-or-later.
// See file LICENSE or <https://www.gnu.org/licenses/> for details.

#ifndef DDVAMP_CONTEXT_ARCH_X86_64_MACHINE_CONTEXT_HPP_INCLUDED_
#define DDVAMP_CONTEXT_ARCH_X86_64_MACHINE_CONTEXT_HPP_INCLUDED_ 1

#include <utils/memory/view.hpp>

#include <context/trampoline.hpp>

namespace context {

class MachineContext {
 private:
  void *rsp_;

 public:
  // Set initial context
  void Setup(::utils::memory_view stack, ITrampoline *trampoline) noexcept;

  // Save current context in this and reset target context
  // (this and target are allowed to be aliased)
  void SwitchTo(MachineContext &target) noexcept;
};

}  // namespace context

#endif /* DDVAMP_CONTEXT_ARCH_X86_64_MACHINE_CONTEXT_HPP_INCLUDED_ */