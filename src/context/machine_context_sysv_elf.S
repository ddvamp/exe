#
# machine_context_sysv_elf.S
# ~~~~~~~~~~~~~~~~~~~~~~~~~~
#
# Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
#
# Licensed under GNU GPL-3.0-or-later.
# See file LICENSE or <https://www.gnu.org/licenses/> for details.
#

#ifdef CONTEXT_USE_FPENV
# define USE_FPENV 1
#else
# define USE_FPENV 0
#endif

  .file "machine_context_sysv_elf.S"
  .text

  .align  16
  .global SetupMachineContext
  .type   SetupMachineContext, @function
SetupMachineContext:
  # SetupMachineContext(%rdi: stack, %rsi: trampoline, %rdx: arg)

  # set return value (address of stack)
  movq %rdi, %rax

  # 16-bit frame border low alignment
  andq $-16, %rax

  # reserve space for return address, argument and callee-saved registers
  leaq -0x50(%rax), %rax

  # set return address for SwitchMachineContext (address of trampoline)
  movq %rsi, 0x30(%rax)

  # return address for trampoline (0x38(%rax)) isn't set

  # set 7-th argument for trampoline
  movq %rdx, 0x40(%rax)

#if USE_FPENV
  # set the floating-point environment (copy of current)
  stmxcsr -0x08(%rax) # set the MXCSR register state
  fnstenv -0x28(%rax) # set x87 FPU environment
  fldenv  -0x28(%rax) # restore discarded x87 FPU state
#endif

  retq
  .size SetupMachineContext, .-SetupMachineContext

  .align  16
  .global SwitchMachineContext
  .type   SwitchMachineContext, @function
SwitchMachineContext:
  # SwitchMachineContext(%rdi: from, %rsi: to)

  # save callee-saved registers
  pushq %r15
  pushq %r14
  pushq %r13
  pushq %r12
  pushq %rbx
  pushq %rbp

#if USE_FPENV
  # save the floating-point environment
  stmxcsr -0x08(%rsp) # save the MXCSR register state
  fnstenv -0x28(%rsp) # save x87 FPU environment
#endif

  # switch stack (with aliasing prevention)
  movq (%rsi), %r11 # load target stack pointer
  movq %rsp, (%rdi) # save current stack pointer
  movq %r11, %rsp   # restore target stack pointer

#if USE_FPENV
  # restore the floating-point environment
  fldenv  -0x28(%rsp) # restore x87 FPU environment
  ldmxcsr -0x08(%rsp) # restore the MXCSR register state
#endif

  # restore callee-saved registers
  popq %rbp
  popq %rbx
  popq %r12
  popq %r13
  popq %r14
  popq %r15

  retq
  .size SwitchMachineContext, .-SwitchMachineContext

  .section .note.GNU-stack,"",@progbits
