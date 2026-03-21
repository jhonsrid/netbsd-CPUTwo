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

#include <sys/types.h>
#include <ucontext.h>
#include <lwp.h>
#include <stdlib.h>

#include <machine/mcontext.h>

void
_lwp_makecontext(ucontext_t *ucp, void (*start)(void *),
    void *arg, void *private, caddr_t stk, size_t stksize)
{
	__greg_t *gr = ucp->uc_mcontext.__gregs;

	/* Set up the stack */
	uintptr_t sp = ((uintptr_t)stk + stksize) & ~7;

	gr[_REG_SP] = (__greg_t)sp;
	gr[_REG_PC] = (__greg_t)(uintptr_t)start;
	gr[_REG_R0] = (__greg_t)(uintptr_t)arg;

	/* When start returns, call _lwp_exit */
	extern void _lwp_exit(void);
	gr[_REG_LR] = (__greg_t)(uintptr_t)_lwp_exit;
}
