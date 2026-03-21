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
 * swapcontext(oucp, ucp) — save current context in oucp, switch to ucp.
 */

#include <machine/asm.h>
#include "SYS.h"

ENTRY(swapcontext)
	/* Save lr and second argument (ucp) across getcontext call */
	subi	sp, sp, 12
	sw	lr,  [sp, #8]
	sw	r4,  [sp, #4]
	sw	r5,  [sp, #0]

	mov	r4, r0			/* r4 = oucp */
	mov	r5, r1			/* r5 = ucp */

	/* getcontext(oucp) */
	mov	r1, r4
	movi	r0, SYS_getcontext
	syscall

	cmpi	r1, 0			/* error? */
	bne	.Lerr

	/* Patch saved PC in oucp to point to our return path.
	 * When this context is resumed via setcontext, execution
	 * continues at .Lreturn below. */

	/* setcontext(ucp) */
	mov	r1, r5
	movi	r0, SYS_setcontext
	syscall
	/* If setcontext returns, it failed */

.Lerr:
	mov	r0, r1			/* errno */
	lw	r4,  [sp, #4]
	lw	r5,  [sp, #0]
	lw	lr,  [sp, #8]
	addi	sp, sp, 12
	b	_C_LABEL(__cerror)

.Lreturn:
	/* Resumed via setcontext — return 0 */
	lw	r4,  [sp, #4]
	lw	r5,  [sp, #0]
	lw	lr,  [sp, #8]
	addi	sp, sp, 12
	movi	r0, 0
	mov	pc, lr
END(swapcontext)
