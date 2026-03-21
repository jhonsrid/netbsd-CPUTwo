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
 * Terminator entries for .ctors, .dtors, .eh_frame, and .jcr sections.
 */

#include <machine/asm.h>

	.section	.ctors, "aw", @progbits
	.p2align	2
	.global		__CTOR_LIST_END__
	.hidden		__CTOR_LIST_END__
__CTOR_LIST_END__:
	.word		0

	.section	.dtors, "aw", @progbits
	.p2align	2
	.global		__DTOR_LIST_END__
	.hidden		__DTOR_LIST_END__
__DTOR_LIST_END__:
	.word		0

	.section	.eh_frame, "a", @progbits
	.p2align	2
	.space		4

	.section	.jcr, "aw", @progbits
	.p2align	2
	.space		4
