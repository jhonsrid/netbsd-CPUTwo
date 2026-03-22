/*	$NetBSD$	*/

/*
 * Copyright (c) 2024 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.
 */

/*
 * CPUTwo signal trampoline.
 *
 * The kernel invokes the signal handler directly.  This trampoline is
 * used only for returning from the signal handler.
 *
 * On entry, the stack looks like:
 *
 *		ucontext structure	sp + sizeof(siginfo_t)
 *	sp ->	siginfo structure
 *
 * DWARF CFI annotations allow the unwinder to trace through signals.
 */

#include "SYS.h"
#include "assym.h"

/*
 * CPUTwo DWARF register numbers match the architectural register numbers
 * (r0=0, r1=1, ..., r14=14, pc=15).
 */
#if defined(__LIBGCC_DWARF_ALT_FRAME_RETURN_COLUMN__)
#define DWARF_SIGRETURN_REG	__LIBGCC_DWARF_ALT_FRAME_RETURN_COLUMN__
#else
#define DWARF_SIGRETURN_REG	16
#endif

#define CFI_OFFSET_REG(r)	.cfi_offset r, r * 4

	.text
	.cfi_startproc
	.cfi_signal_frame
	.cfi_def_cfa _REG_SP, SIGINFO_SIZE + _UC_GREGS
	CFI_OFFSET_REG(_REG_R0)
	CFI_OFFSET_REG(_REG_R1)
	CFI_OFFSET_REG(_REG_R2)
	CFI_OFFSET_REG(_REG_R3)
	CFI_OFFSET_REG(_REG_R4)
	CFI_OFFSET_REG(_REG_R5)
	CFI_OFFSET_REG(_REG_R6)
	CFI_OFFSET_REG(_REG_R7)
	CFI_OFFSET_REG(_REG_R8)
	CFI_OFFSET_REG(_REG_R9)
	CFI_OFFSET_REG(_REG_R10)
	CFI_OFFSET_REG(_REG_R11)
	CFI_OFFSET_REG(_REG_R12)
	CFI_OFFSET_REG(_REG_SP)
	CFI_OFFSET_REG(_REG_LR)
	.cfi_return_column DWARF_SIGRETURN_REG
	.cfi_offset DWARF_SIGRETURN_REG, _REG_PC * 4

	/*
	 * One instruction slot before the trampoline entry point,
	 * because the unwinder looks at (return PC - 1 insn).
	 */
	mov	r0, r0		/* nop: padding for unwinder */

ENTRY_NP(__sigtramp_siginfo_2)
	/* Compute pointer to ucontext: sp + sizeof(siginfo_t) */
	addi	r0, sp, SIGINFO_SIZE

	/* setcontext(ucp) */
	mov	r1, r0
	movi	r0, SYS_setcontext
	syscall

	/* If setcontext returns, exit with the error */
	mov	r1, r0
	movi	r0, SYS_exit
	syscall
	.cfi_endproc
END(__sigtramp_siginfo_2)
