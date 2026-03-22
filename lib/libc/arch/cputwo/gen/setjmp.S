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
 * C library -- setjmp, longjmp (signal-saving versions)
 *
 *	longjmp(a, v)
 * will generate a "return(v)" from the last call to
 *	setjmp(a)
 * by restoring registers and the signal mask.
 */

#include <sys/syscall.h>
#include <machine/asm.h>
#include <machine/setjmp.h>

/*
 * __setjmp14(jmp_buf env)
 *
 * Save callee-saved registers plus signal mask.
 */
ENTRY(__setjmp14)
	/* Allocate stack frame to save lr and r4 across sigprocmask call */
	subi	sp, sp, 8
	sw	lr,  [sp + 4]
	sw	r4,  [sp + 0]

	mov	r4, r0			/* r4 = jmpbuf pointer */

	/*
	 * Get current signal mask: __sigprocmask14(0, NULL, &env->sigmask)
	 * Kernel syscall ABI: r0=sysnum, r1=how, r2=set, r3=oset
	 */
	addi	r3, r4, (_JB_SIGMASK * 4)	/* r3 = oset = &env[_JB_SIGMASK] */
	movi	r2, 0			/* r2 = set = NULL */
	movi	r1, 0			/* r1 = how = 0 (SIG_BLOCK with null = query) */
	movi	r0, SYS___sigprocmask14
	syscall

	mov	r0, r4			/* restore jmpbuf pointer */

	/* Store magic number */
	movi	r12, _JB_MAGIC_CPUTWO_SETJMP
	sw	r12, [r0 + (_JB_MAGIC * 4)]

	/* Save callee-saved registers into jmpbuf.
	 * We need to save the ORIGINAL lr and sp (before our frame). */
	lw	r12, [sp + 4]		/* original lr */
	sw	r12, [r0 + (_JB_LR * 4)]
	addi	r12, sp, 8		/* original sp */
	sw	r12, [r0 + (_JB_SP * 4)]

	/* Save original r4 (before we clobbered it with jmpbuf pointer) */
	lw	r12, [sp + 0]
	sw	r12, [r0 + (_JB_R4 * 4)]

	sw	r5,  [r0 + (_JB_R5 * 4)]
	sw	r6,  [r0 + (_JB_R6 * 4)]
	sw	r7,  [r0 + (_JB_R7 * 4)]
	sw	r8,  [r0 + (_JB_R8 * 4)]
	sw	r9,  [r0 + (_JB_R9 * 4)]
	sw	r10, [r0 + (_JB_R10 * 4)]
	sw	r11, [r0 + (_JB_R11 * 4)]

	/* Restore our frame and return 0 */
	lw	r4,  [sp + 0]
	lw	lr,  [sp + 4]
	addi	sp, sp, 8

	movi	r0, 0
	mov	pc, lr
END(__setjmp14)

/*
 * __longjmp14(jmp_buf env, int val)
 *
 * Restore signal mask and callee-saved registers, then return val.
 */
ENTRY(__longjmp14)
	/*
	 * Stack frame: save jmpbuf pointer and return value across
	 * the sigprocmask syscall.  Use r4=jmpbuf, r5=val.
	 */
	subi	sp, sp, 8
	sw	lr,  [sp + 4]
	sw	r4,  [sp + 0]

	mov	r4, r0			/* r4 = jmpbuf */
	mov	r5, r1			/* r5 = val */

	/* Verify magic */
	lw	r12, [r4 + (_JB_MAGIC * 4)]
	movi	r3, _JB_MAGIC_CPUTWO_SETJMP
	cmp	r12, r3
	bne	.Lbotch14

	/*
	 * Restore signal mask: __sigprocmask14(SIG_SETMASK, &env->sigmask, NULL)
	 * Kernel ABI: r0=sysnum, r1=how, r2=set, r3=oset
	 */
	movi	r3, 0			/* r3 = oset = NULL */
	addi	r2, r4, (_JB_SIGMASK * 4)	/* r2 = set = &env[_JB_SIGMASK] */
	movi	r1, 3			/* r1 = how = SIG_SETMASK */
	movi	r0, SYS___sigprocmask14
	syscall

	/*
	 * Prepare return value before restoring everything.
	 * r5 holds the original val argument.
	 */
	mov	r0, r5
	cmpi	r0, 0
	bne	1f
	movi	r0, 1			/* longjmp must not return 0 */
1:
	/* r0 = return value, r4 = jmpbuf.  Now restore all regs from jmpbuf.
	 * Use r12 as scratch. Do sp and lr last. */
	mov	r12, r4			/* r12 = jmpbuf (r4 about to be restored) */
	lw	r4,  [r12 + (_JB_R4 * 4)]
	lw	r5,  [r12 + (_JB_R5 * 4)]
	lw	r6,  [r12 + (_JB_R6 * 4)]
	lw	r7,  [r12 + (_JB_R7 * 4)]
	lw	r8,  [r12 + (_JB_R8 * 4)]
	lw	r9,  [r12 + (_JB_R9 * 4)]
	lw	r10, [r12 + (_JB_R10 * 4)]
	lw	r11, [r12 + (_JB_R11 * 4)]
	lw	sp,  [r12 + (_JB_SP * 4)]
	lw	lr,  [r12 + (_JB_LR * 4)]

	mov	pc, lr

.Lbotch14:
	ba	_C_LABEL(longjmperror)
	/* NOTREACHED */
END(__longjmp14)
