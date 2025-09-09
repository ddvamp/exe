#
#
#
#
# Copyright (C) 2023-2025 Artyom Kolpakov <ddvamp007@gmail.com>
#
# Licensed under GNU GPL-3.0-or-later.
# See file LICENSE or <https://www.gnu.org/licenses/> for details.
#

	.file	"machine_context_sysv_elf.S"
	.text

	.align	16
	.global	setupMachineContext
	.type	setupMachineContext, @function
setupMachineContext:
	# setupMachineContext(%rdi: stack, %rsi: trampoline, %rdx: arg)

	# set return value (address of stack)
	movq	%rdi, %rax

	# 16-bit frame border low alignment
	andq	$-16, %rax

	# reserve space for return address, argument and callee-saved registers
	leaq	-0x50(%rax), %rax

	# set return address for switchMachineContext (address of trampoline)
	movq	%rsi, 0x30(%rax)

	# return address for trampoline (0x38(%rax)) isn't set

	# set 7-th argument for trampoline
	movq	%rdx, 0x40(%rax)

	# set the floating-point environment (copy of current)
	stmxcsr	-0x8(%rax)		# set the MXCSR register state
	fnstenv	-0x28(%rax)		# set x87 FPU environment
	fldenv	-0x28(%rax)		# restore discarded x87 FPU state

	retq
	.size	setupMachineContext, .-setupMachineContext

	.align	16
	.global	switchMachineContext
	.type	switchMachineContext, @function
switchMachineContext:
	# switchMachineContext(%rdi: from, %rsi: to)

	# save callee-saved registers
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	pushq	%rbp

	# save the floating-point environment
	stmxcsr	-0x8(%rsp)		# save the MXCSR register state
	fnstenv	-0x28(%rsp)		# save x87 FPU environment

	# switch stack (with aliasing prevention)
	movq	(%rsi), %r11	# load target stack pointer
	movq	%rsp, (%rdi)	# save current stack pointer
	movq	%r11, %rsp		# restore target stack pointer

	# restore the floating-point environment
	fldenv	-0x28(%rsp)		# restore x87 FPU environment
	ldmxcsr	-0x8(%rsp)		# restore the MXCSR register state

	# restore callee-saved registers
	popq	%rbp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15

	retq
	.size	switchMachineContext, .-switchMachineContext

	.section	.note.GNU-stack,"",@progbits
