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
 * .init and .fini section prologues.
 * crti.S provides the opening of _init and _fini functions.
 * crtn.S provides the closing (epilogue).
 */

#include <machine/asm.h>

#include "sysident.S"

	.section ".init", "ax", @progbits
	.p2align 2
	.globl	_init
	.type	_init, _ASM_TYPE_FUNCTION
_init:
	subi	sp, sp, 4
	sw	lr, [sp, #0]

	.section ".fini", "ax", @progbits
	.p2align 2
	.globl	_fini
	.type	_fini, _ASM_TYPE_FUNCTION
_fini:
	subi	sp, sp, 4
	sw	lr, [sp, #0]
