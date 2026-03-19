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
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <machine/asm.h>

/*
 * Board startup for evbcputwo (QEMU cputwo-board).
 *
 * CPU starts at PA 0x00000000 in supervisor mode, IE=0, MMU off.
 * Set up initial stack pointer at MMIO base (full-descending: first
 * push goes to 0x03EFFFFC) and call cputwo_init().
 */

	.text
	.globl	start
start:
	/*
	 * Set stack pointer to 0x03F00000 (MMIO base).
	 * Full-descending stack: first "addi sp, sp, -N" lands below MMIO.
	 */
	lui	sp, 0x03F0
	ori	sp, sp, 0x0000

	/*
	 * Clear BSS.
	 * Linker provides __bss_start and _end symbols.
	 */
	lui	r0, %hi(__bss_start)
	ori	r0, r0, %lo(__bss_start)
	lui	r1, %hi(_end)
	ori	r1, r1, %lo(_end)
	movi	r2, 0
1:
	cmp	r0, r1
	beq	2f
	sw	r2, r0, 0
	addi	r0, r0, 4
	ba	1b
2:
	/* Call board init */
	jmp	lr, cputwo_init

	/* Should not return */
3:	ba	3b
