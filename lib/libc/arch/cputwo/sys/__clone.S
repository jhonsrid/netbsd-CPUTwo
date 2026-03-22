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
 * int __clone(int (*fn)(void *), void *stack, int flags, void *arg);
 *
 * C ABI: r0=fn, r1=stack, r2=flags, r3=arg
 *
 * The kernel __clone syscall expects: arg0=flags, arg1=stack
 * Kernel ABI: r0=SYS___clone, r1=flags, r2=stack
 */

#include <sys/errno.h>
#include "SYS.h"

#ifdef WEAK_ALIAS
WEAK_ALIAS(clone, __clone)
#endif

ENTRY(__clone)
	/* Validate: fn and stack must not be NULL */
	cmpi	r0, 0
	beq	.Linval
	cmpi	r1, 0
	beq	.Linval

	/*
	 * Push fn and arg onto the child's stack so the child
	 * can retrieve them after the syscall.
	 */
	subi	r1, r1, 8		/* make room on child stack */
	sw	r0, [r1 + 0]		/* child_stack[0] = fn */
	sw	r3, [r1 + 4]		/* child_stack[4] = arg */

	/*
	 * Syscall: __clone(flags, stack)
	 * Kernel ABI: r0=SYS, r1=flags, r2=stack
	 */
	mov	r3, r1			/* save stack in r3 temporarily */
	mov	r1, r2			/* r1 = flags (kernel arg0) */
	mov	r2, r3			/* r2 = stack (kernel arg1) */
	movi	r0, SYS___clone
	syscall

	cmpi	r1, 0
	bne	.Lerr			/* error */

	/* r0 = pid.  If r0 != 0, we're the parent */
	cmpi	r0, 0
	bne	.Lparent

	/* Child: sp is the new stack.  Fetch fn and arg. */
	lw	r12, [sp + 0]		/* r12 = fn */
	lw	r0,  [sp + 4]		/* r0 = arg */

	/* Call fn(arg) */
	callr	lr, r12

	/* fn returned: call _exit(retval) */
	mov	r1, r0			/* r1 = exit status (kernel arg0) */
	movi	r0, SYS_exit
	syscall
	/* NOTREACHED */

.Lparent:
	mov	pc, lr

.Linval:
	movi	r0, EINVAL
	/* fall through to __cerror */
.Lerr:
	ba	_C_LABEL(__cerror)
END(__clone)
