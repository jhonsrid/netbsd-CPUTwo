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
 * C library -- sigsetjmp, siglongjmp
 *
 *	siglongjmp(a, v) will generate a "return(v)" from the last call to
 *	sigsetjmp(a, savemask) by restoring registers from the jmpbuf,
 *	and optionally restoring the signal mask.
 */

#include <machine/asm.h>
#include <machine/setjmp.h>

/*
 * __sigsetjmp14(jmp_buf env, int savemask)
 *
 * Save the "savemask" flag at jmpbuf[_JBLEN], then tail-call either
 * _setjmp (no signal save) or __setjmp14 (with signal save).
 */
ENTRY(__sigsetjmp14)
	/* Store savemask flag at env[_JBLEN] */
	sw	r1, [r0 + (_JBLEN * 4)]

	cmpi	r1, 0
	bne	1f
	/* savemask == 0: tail-call _setjmp */
	ba	_C_LABEL(_setjmp)
1:
	/* savemask != 0: tail-call __setjmp14 */
	ba	_C_LABEL(__setjmp14)
END(__sigsetjmp14)

/*
 * __siglongjmp14(jmp_buf env, int val)
 *
 * Check the saved "savemask" flag and tail-call the appropriate longjmp.
 */
ENTRY(__siglongjmp14)
	lw	r12, [r0 + (_JBLEN * 4)]
	cmpi	r12, 0
	bne	1f
	/* savemask == 0: tail-call _longjmp */
	ba	_C_LABEL(_longjmp)
1:
	/* savemask != 0: tail-call __longjmp14 */
	ba	_C_LABEL(__longjmp14)
END(__siglongjmp14)
