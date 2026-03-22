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
 * Emit calls to __do_global_ctors_aux and __do_global_dtors_aux
 * in the .init and .fini sections respectively.
 *
 * CPUTwo call instruction: JMP lr, target
 */

__asm(	"\n\t"
	".pushsection .init, \"ax\", @progbits"		"\n\t"
	"jmp	lr, __do_global_ctors_aux"		"\n\t"
	".popsection");

__asm(	"\n\t"
	".pushsection .fini, \"ax\", @progbits"		"\n\t"
	"jmp	lr, __do_global_dtors_aux"		"\n\t"
	".popsection");
