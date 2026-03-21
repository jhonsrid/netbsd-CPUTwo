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
 * _resumecontext: called when a context created by makecontext returns.
 * Loads uc_link and calls setcontext, or exits if uc_link is NULL.
 */

#include <machine/asm.h>
#include "assym.h"
#include "SYS.h"

ENTRY(_resumecontext)
	/*
	 * When the function set by makecontext returns, lr was set
	 * to _resumecontext.  We need a ucontext_t to work with.
	 * Allocate space on the stack for a ucontext_t, get the
	 * current context, follow uc_link, and setcontext to it.
	 */
	subi	sp, sp, UCONTEXT_SIZE
	mov	r0, sp
	/* getcontext(ucp) to populate the structure */
	mov	r3, r2
	mov	r2, r1
	mov	r1, r0
	movi	r0, SYS_getcontext
	syscall

	/* Load uc_link */
	lw	r0, [sp, #_UC_LINK]
	cmpi	r0, 0
	beq	1f

	/* setcontext(uc_link) */
	mov	r1, r0
	movi	r0, SYS_setcontext
	syscall

1:
	/* No link context — exit(0) */
	movi	r1, 0
	movi	r0, SYS_exit
	syscall
	/* NOTREACHED */
END(_resumecontext)
