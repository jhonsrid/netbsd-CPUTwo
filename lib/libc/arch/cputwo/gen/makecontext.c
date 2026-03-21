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
#include <inttypes.h>
#include <ucontext.h>
#include <stdarg.h>

#include <machine/mcontext.h>

void
makecontext(ucontext_t *ucp, void (*func)(void), int argc, ...)
{
	__greg_t *gr = ucp->uc_mcontext.__gregs;
	uintptr_t *sp;
	va_list ap;
	int i;

	/* Set up the new stack: 8-byte aligned, below ss_sp + ss_size */
	sp = (uintptr_t *)
	    (((uintptr_t)ucp->uc_stack.ss_sp + ucp->uc_stack.ss_size) & ~7);

	/* The function to call */
	gr[_REG_PC] = (__greg_t)(uintptr_t)func;

	/* When func returns, it should call _resumecontext */
	extern void _resumecontext(void);
	gr[_REG_LR] = (__greg_t)(uintptr_t)_resumecontext;

	/* Set stack pointer */
	gr[_REG_SP] = (__greg_t)(uintptr_t)sp;

	/*
	 * Pass arguments: first 3 in r1, r2, r3 (since r0 is used for
	 * the syscall number in the kernel, but in userland function calls
	 * args go in r0-r3).  Actually for makecontext the function is
	 * called normally with the C ABI, so args go in r0-r3, rest on stack.
	 */
	va_start(ap, argc);
	for (i = 0; i < argc && i < 4; i++) {
		gr[_REG_R0 + i] = va_arg(ap, int);
	}
	/* Additional arguments go on the stack */
	if (argc > 4) {
		sp -= (argc - 4);
		sp = (uintptr_t *)((uintptr_t)sp & ~7);	/* re-align */
		for (i = 4; i < argc; i++) {
			sp[i - 4] = va_arg(ap, int);
		}
		gr[_REG_SP] = (__greg_t)(uintptr_t)sp;
	}
	va_end(ap);
}
