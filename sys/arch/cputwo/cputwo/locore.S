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
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <machine/asm.h>
#include "assym.h"

/*
 * CPUTwo locore: exception vectors, context switch, setjmp/longjmp.
 *
 * Register conventions:
 *   r0-r3:  args/return (caller-saved)
 *   r4-r11: callee-saved
 *   r12:    scratch (caller-saved)
 *   r13:    sp
 *   r14:    lr
 *   r15:    pc (not directly writable except via mov pc, lr / kret / sysret)
 *
 * Supervisor registers (memory-mapped at 0x03FFF000):
 *   +0x00 EPC, +0x04 EFLAGS, +0x08 EVEC, +0x0C CAUSE,
 *   +0x10 STATUS, +0x14 ESTATUS, +0x18 SATP, +0x1C BADADDR
 */

#define REG_EPC		0x03FFF000
#define REG_EFLAGS	0x03FFF004
#define REG_EVEC	0x03FFF008
#define REG_CAUSE	0x03FFF00C
#define REG_STATUS	0x03FFF010
#define REG_ESTATUS	0x03FFF014
#define REG_SATP	0x03FFF018
#define REG_BADADDR	0x03FFF01C

/*
 * exception_entry: hardware dispatches here via EVEC table.
 *
 * On entry: supervisor mode, IE=0, all GPRs preserved by hardware.
 * EPC/EFLAGS/ESTATUS/CAUSE set by hardware.
 *
 * If the trap came from user mode (ESTATUS bit 0 = 0), we must switch
 * to the LWP's kernel stack before saving the trapframe.  We use r12
 * (caller-saved scratch) as a temporary.
 *
 * Kernel stack pointer is stored in the LWP's PCB.  We find it via:
 *   cpu_info_store → ci_curlwp → l_addr (PCB) → pcb_tf (kernel SP)
 * But that's too many loads before we have a scratch register.
 *
 * Simpler approach: store the kernel SP in a global variable
 * (cputwo_kern_sp) which is updated on every context switch and
 * return-to-user.  This is the approach used by simple single-CPU ports.
 */

	.data
	.globl	cputwo_kern_sp
cputwo_kern_sp:
	.word	0			/* kernel SP for current LWP */

	.text
/*
 * exception_entry: all exceptions dispatch here via the EVEC table.
 *
 * On entry (hardware guarantees):
 *   - Supervisor mode, IE=0
 *   - All GPRs (r0-r14) have their pre-trap values
 *   - EPC, EFLAGS, ESTATUS, CAUSE written by hardware
 *
 * r12 is caller-saved in the C ABI, so we use it as scratch here.
 * Its pre-trap value is lost; the trapframe stores 0 for tf_r[12].
 * This is acceptable: r12 is not used for syscall args or return
 * values, and kernel code treats it as caller-saved.
 */
ENTRY_NP(exception_entry)
	/* Check ESTATUS bit 0: 1=supervisor, 0=user */
	lui	r12, 0x03FF
	ori	r12, r12, 0xF014	/* r12 = &ESTATUS (0x03FFF014) */
	lw	r12, r12, 0		/* r12 = ESTATUS */
	andi	r12, r12, 0x01		/* sets Z if user mode */
	bne	.Lkern_trap

	/*
	 * User-mode trap: switch to kernel stack.
	 */
	mov	r12, sp			/* r12 = user SP */
	lui	sp, %hi(cputwo_kern_sp)
	ori	sp, sp, %lo(cputwo_kern_sp)
	lw	sp, sp, 0		/* sp = kernel stack top */
	addi	sp, sp, -TF_SIZE
	sw	r12, sp, TF_R0+52	/* tf_r[13] = user SP */
	movi	r12, 0
	sw	r12, sp, TF_R0+48	/* tf_r[12] = 0 (clobbered) */
	ba	.Lsave_regs

.Lkern_trap:
	/* Kernel-mode trap: sp is already a kernel stack */
	addi	sp, sp, -TF_SIZE
	addi	r12, sp, TF_SIZE
	sw	r12, sp, TF_R0+52	/* tf_r[13] = pre-trap kernel SP */
	movi	r12, 0
	sw	r12, sp, TF_R0+48	/* tf_r[12] = 0 (clobbered) */

.Lsave_regs:
	/* Save all general registers to trapframe */
	sw	r0,  sp, TF_R0
	sw	r1,  sp, TF_R0+4
	sw	r2,  sp, TF_R0+8
	sw	r3,  sp, TF_R0+12
	sw	r4,  sp, TF_R0+16
	sw	r5,  sp, TF_R0+20
	sw	r6,  sp, TF_R0+24
	sw	r7,  sp, TF_R0+28
	sw	r8,  sp, TF_R0+32
	sw	r9,  sp, TF_R0+36
	sw	r10, sp, TF_R0+40
	sw	r11, sp, TF_R0+44
	/* r12 and sp already saved above */
	sw	r14, sp, TF_R0+56	/* lr */

	/* Save supervisor state from memory-mapped registers */
	lui	r0, 0x03FF
	ori	r0, r0, 0xF000		/* r0 = 0x03FFF000 */
	lw	r1, r0, 0x00		/* EPC */
	sw	r1, sp, TF_PC
	lw	r1, r0, 0x14		/* ESTATUS (pre-trap STATUS, saved by hw) */
	sw	r1, sp, TF_STATUS
	lw	r1, r0, 0x04		/* EFLAGS (pre-trap flags, saved by hw) */
	sw	r1, sp, TF_EFLAGS
	lw	r1, r0, 0x0C		/* CAUSE */
	sw	r1, sp, TF_CAUSE

	/* Call trap(struct trapframe *) with sp as argument */
	mov	r0, sp
	jmp	lr, trap

	/* trap() returns here -- restore state */
	lui	r0, 0x03FF
	ori	r0, r0, 0xF000

	/* Restore supervisor state */
	lw	r1, sp, TF_PC
	sw	r1, r0, 0x00		/* EPC */
	lw	r1, sp, TF_EFLAGS
	sw	r1, r0, 0x04		/* EFLAGS */
	lw	r1, sp, TF_STATUS
	sw	r1, r0, 0x14		/* ESTATUS (restored by kret/sysret) */

	/* Restore general registers */
	lw	r1,  sp, TF_R0+4
	lw	r2,  sp, TF_R0+8
	lw	r3,  sp, TF_R0+12
	lw	r4,  sp, TF_R0+16
	lw	r5,  sp, TF_R0+20
	lw	r6,  sp, TF_R0+24
	lw	r7,  sp, TF_R0+28
	lw	r8,  sp, TF_R0+32
	lw	r9,  sp, TF_R0+36
	lw	r10, sp, TF_R0+40
	lw	r11, sp, TF_R0+44
	lw	r12, sp, TF_R0+48
	lw	r14, sp, TF_R0+56	/* lr */

	/*
	 * Restore SP: if returning to user, load user SP from trapframe.
	 * If returning to kernel, pop the frame.
	 * Check ESTATUS bit 0 to determine.
	 */
	lui	r0, 0x03FF
	ori	r0, r0, 0xF014		/* &ESTATUS */
	lw	r0, r0, 0
	andi	r0, r0, 0x01
	lw	r0, sp, TF_R0		/* restore r0 early */
	bne	.Lkret_kern

	/* User return: restore user SP from trapframe, then SYSRET */
	lw	sp, sp, TF_R0+52
	.word	0x11000000		/* SYSRET */

.Lkret_kern:
	/* Kernel return: pop trapframe, then KRET */
	addi	sp, sp, TF_SIZE
	.word	0x3F000000		/* KRET */
END(exception_entry)

/*
 * cpu_switchto(struct lwp *oldlwp, struct lwp *newlwp, struct lwp *returning)
 *
 * Save callee-saved registers of oldlwp into its switchframe (in PCB).
 * Restore callee-saved registers of newlwp from its switchframe.
 * Return oldlwp (in r0 for the caller of cpu_switchto in newlwp's context).
 *
 * r0 = oldlwp (may be NULL)
 * r1 = newlwp
 * r2 = returning (not used here)
 */
ENTRY_NP(cpu_switchto)
	/* Save callee-saved regs of old LWP if non-NULL */
	cmpi	r0, 0
	beq	.Lswitch_restore

	/* oldlwp->l_addr points to PCB; switchframe is at offset PCB_SF */
	lw	r3, r0, L_PCB		/* r3 = oldlwp->l_addr (PCB pointer) */
	addi	r3, r3, PCB_SF		/* r3 = &pcb->pcb_sf */
	sw	r4,  r3, SF_R4
	sw	r5,  r3, SF_R5
	sw	r6,  r3, SF_R6
	sw	r7,  r3, SF_R7
	sw	r8,  r3, SF_R8
	sw	r9,  r3, SF_R9
	sw	r10, r3, SF_R10
	sw	r11, r3, SF_R11
	sw	sp,  r3, SF_SP
	sw	lr,  r3, SF_LR

.Lswitch_restore:
	/* Save oldlwp pointer for return value */
	mov	r12, r0

	/* Restore callee-saved regs of new LWP */
	lw	r3, r1, L_PCB		/* r3 = newlwp->l_addr */
	addi	r3, r3, PCB_SF		/* r3 = &pcb->pcb_sf */
	lw	r4,  r3, SF_R4
	lw	r5,  r3, SF_R5
	lw	r6,  r3, SF_R6
	lw	r7,  r3, SF_R7
	lw	r8,  r3, SF_R8
	lw	r9,  r3, SF_R9
	lw	r10, r3, SF_R10
	lw	r11, r3, SF_R11
	lw	sp,  r3, SF_SP
	lw	lr,  r3, SF_LR

	/*
	 * Update curlwp and kernel SP for trap entry.
	 * r1 still holds newlwp.
	 */
	lui	r3, %hi(cpu_info_store)
	ori	r3, r3, %lo(cpu_info_store)
	sw	r1, r3, CI_CURLWP

	/* Update cputwo_kern_sp = new kernel stack top (sp after switch) */
	lui	r3, %hi(cputwo_kern_sp)
	ori	r3, r3, %lo(cputwo_kern_sp)
	sw	sp, r3, 0

	/* Return oldlwp */
	mov	r0, r12
	mov	pc, lr
END(cpu_switchto)

/*
 * setjmp(label_t *jb)
 * Save callee-saved registers to jb, return 0.
 */
ENTRY_NP(setjmp)
	/* r0 = jb pointer; save callee-saved regs at known offsets */
	sw	r4,  r0, 0
	sw	r5,  r0, 4
	sw	r6,  r0, 8
	sw	r7,  r0, 12
	sw	r8,  r0, 16
	sw	r9,  r0, 20
	sw	r10, r0, 24
	sw	r11, r0, 28
	sw	sp,  r0, 32
	sw	lr,  r0, 36
	movi	r0, 0
	mov	pc, lr
END(setjmp)

/*
 * longjmp(label_t *jb)
 * Restore callee-saved registers from jb, return 1.
 */
ENTRY_NP(longjmp)
	/* r0 = jb pointer */
	lw	r4,  r0, 0
	lw	r5,  r0, 4
	lw	r6,  r0, 8
	lw	r7,  r0, 12
	lw	r8,  r0, 16
	lw	r9,  r0, 20
	lw	r10, r0, 24
	lw	r11, r0, 28
	lw	sp,  r0, 32
	lw	lr,  r0, 36
	movi	r0, 1
	mov	pc, lr
END(longjmp)
