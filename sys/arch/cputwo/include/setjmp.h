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

#ifndef _CPUTWO_SETJMP_H_
#define _CPUTWO_SETJMP_H_

/*
 * CPUTwo jmp_buf layout.
 *
 * We save: magic, sp, lr, r4-r11, plus signal mask and spare.
 * _JBLEN=32 is generous to allow for future growth.
 */

#define _JB_MAGIC_CPUTWO__SETJMP	0x63703201	/* cp2\x01 */
#define _JB_MAGIC_CPUTWO_SETJMP	0x43503202	/* CP2\x02 */

#define _JBLEN		32
#define _JB_MAGIC	0
#define _JB_SP		1
#define _JB_LR		2
#define _JB_R4		3
#define _JB_R5		4
#define _JB_R6		5
#define _JB_R7		6
#define _JB_R8		7
#define _JB_R9		8
#define _JB_R10		9
#define _JB_R11		10

#define _JB_SIGMASK	14

#endif /* _CPUTWO_SETJMP_H_ */
