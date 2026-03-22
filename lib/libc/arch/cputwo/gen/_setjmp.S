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
 * C library -- _setjmp, _longjmp
 *
 *	_longjmp(a, v)
 * will generate a "return(v)" from the last call to
 *	_setjmp(a)
 * by restoring registers from the jmpbuf.
 * The previous signal state is NOT restored.
 *
 * jmp_buf layout (uint32_t[_JBLEN=32]):
 *   [0]  magic
 *   [1]  sp
 *   [2]  lr
 *   [3]  r4
 *   [4]  r5
 *   [5]  r6
 *   [6]  r7
 *   [7]  r8
 *   [8]  r9
 *   [9]  r10
 *   [10] r11
 */

#include <machine/asm.h>
#include <machine/setjmp.h>

ENTRY(_setjmp)
	/* Store magic number */
	movi	r12, _JB_MAGIC_CPUTWO__SETJMP
	sw	r12, [r0 + (_JB_MAGIC * 4)]

	/* Save callee-saved registers */
	sw	sp,  [r0 + (_JB_SP * 4)]
	sw	lr,  [r0 + (_JB_LR * 4)]
	sw	r4,  [r0 + (_JB_R4 * 4)]
	sw	r5,  [r0 + (_JB_R5 * 4)]
	sw	r6,  [r0 + (_JB_R6 * 4)]
	sw	r7,  [r0 + (_JB_R7 * 4)]
	sw	r8,  [r0 + (_JB_R8 * 4)]
	sw	r9,  [r0 + (_JB_R9 * 4)]
	sw	r10, [r0 + (_JB_R10 * 4)]
	sw	r11, [r0 + (_JB_R11 * 4)]

	/* Return 0 */
	movi	r0, 0
	mov	pc, lr
END(_setjmp)

ENTRY(_longjmp)
	/* Verify magic number */
	lw	r12, [r0 + (_JB_MAGIC * 4)]
	movi	r3, _JB_MAGIC_CPUTWO__SETJMP
	cmp	r12, r3
	bne	.Lbotch

	/* Restore callee-saved registers */
	lw	r4,  [r0 + (_JB_R4 * 4)]
	lw	r5,  [r0 + (_JB_R5 * 4)]
	lw	r6,  [r0 + (_JB_R6 * 4)]
	lw	r7,  [r0 + (_JB_R7 * 4)]
	lw	r8,  [r0 + (_JB_R8 * 4)]
	lw	r9,  [r0 + (_JB_R9 * 4)]
	lw	r10, [r0 + (_JB_R10 * 4)]
	lw	r11, [r0 + (_JB_R11 * 4)]
	lw	sp,  [r0 + (_JB_SP * 4)]
	lw	lr,  [r0 + (_JB_LR * 4)]

	/* Return value: if v == 0, return 1 */
	mov	r0, r1
	cmpi	r0, 0
	bne	1f
	movi	r0, 1
1:
	mov	pc, lr

.Lbotch:
	ba	_C_LABEL(longjmperror)
	/* NOTREACHED */
END(_longjmp)
