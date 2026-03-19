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

#ifndef _CPUTWO_FRAME_H_
#define _CPUTWO_FRAME_H_

#include <cputwo/reg.h>

/*
 * Trap frame saved on exception entry.
 *
 * tf_r[0..14] = r0-r14 (r13=sp, r14=lr)
 * tf_pc       = saved PC (from EPC supervisor register)
 * tf_status   = saved STATUS (from ESTATUS)
 * tf_eflags   = saved EFLAGS
 * tf_cause    = exception cause code
 */
struct trapframe {
	uint32_t tf_r[15];	/* r0 through r14 */
	uint32_t tf_pc;		/* saved program counter */
	uint32_t tf_status;	/* saved STATUS register */
	uint32_t tf_eflags;	/* saved EFLAGS register */
	uint32_t tf_cause;	/* exception cause */
};

/* Convenience aliases */
#define tf_r0	tf_r[0]
#define tf_sp	tf_r[13]
#define tf_lr	tf_r[14]

#endif /* _CPUTWO_FRAME_H_ */
