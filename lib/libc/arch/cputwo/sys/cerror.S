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
 * __cerror — common error handling for system call stubs.
 *
 * On entry, r0 contains the errno value (from the kernel).
 * We store it into errno and return -1.
 */

#include <machine/asm.h>

#ifdef _REENTRANT
	.globl	_C_LABEL(__errno)
ENTRY(__cerror)
	/* Save lr and errno across __errno() call */
	subi	sp, sp, 8
	sw	lr,  [sp, #4]
	sw	r0,  [sp, #0]		/* save errno value */

	bl	_C_LABEL(__errno)	/* r0 = &errno */

	lw	r12, [sp, #0]		/* recover errno value */
	sw	r12, [r0, #0]		/* *__errno() = errno value */

	lw	lr,  [sp, #4]
	addi	sp, sp, 8

	movi	r0, -1			/* return -1 */
	movi	r1, -1
	mov	pc, lr
END(__cerror)
#else
	.globl	_C_LABEL(errno)
ENTRY(__cerror)
	sw	r0, _C_LABEL(errno)
	movi	r0, -1
	movi	r1, -1
	mov	pc, lr
END(__cerror)
#endif /* _REENTRANT */
