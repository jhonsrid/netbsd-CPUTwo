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
 * C runtime startup for CPUTwo.
 *
 * The kernel (or ld_elf.so) passes:
 *   r0 = cleanup function (or 0)
 *   r1 = obj_main (or 0)
 *   r2 = ps_strings
 *   sp = user stack (with argc, argv, envp on it)
 *
 * We call ___start(cleanup, ps_strings) from crt0-common.c.
 */

#include <machine/asm.h>

STRONG_ALIAS(_start, __start)

ENTRY(__start)
	/* r0 = cleanup, r2 = ps_strings.
	 * ___start(cleanup, ps_strings) */
	mov	r1, r2		/* r1 = ps_strings */
	/* r0 = cleanup (already in r0) */
	b	_C_LABEL(___start)
END(__start)
