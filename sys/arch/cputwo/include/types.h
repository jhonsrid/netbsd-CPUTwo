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

#ifndef _CPUTWO_TYPES_H_
#define _CPUTWO_TYPES_H_

#include <sys/cdefs.h>
#include <sys/featuretest.h>
#include <cputwo/int_types.h>

#if defined(_KERNEL) || defined(_KMEMUSER) || defined(_KERNTYPES) || defined(_STANDALONE)
typedef	unsigned long	vm_offset_t;	/* deprecated */
typedef	unsigned long	vm_size_t;	/* deprecated */

typedef __UINTPTR_TYPE__	paddr_t;
typedef __UINTPTR_TYPE__	psize_t;
typedef __UINTPTR_TYPE__	vaddr_t;
typedef __UINTPTR_TYPE__	vsize_t;
#define	PRIxPADDR	PRIxPTR
#define	PRIxPSIZE	PRIxPTR
#define	PRIuPSIZE	PRIuPTR
#define	PRIxVADDR	PRIxPTR
#define	PRIxVSIZE	PRIxPTR
#define	PRIuVSIZE	PRIuPTR

typedef unsigned long int register_t;
#endif

#if defined(_KERNEL)
/*
 * label_t: used by setjmp/longjmp in the kernel.
 * Save callee-saved registers r4-r11, sp (r13), lr (r14), plus spare.
 */
typedef struct label_t {
	register_t lb_reg[16];
} label_t;
#endif

typedef	unsigned int	__cpu_simple_lock_nv_t;
typedef	unsigned long int	__register_t;

#define	__SIMPLELOCK_LOCKED	1
#define	__SIMPLELOCK_UNLOCKED	0

/* No __HAVE_FAST_SOFTINTS — use slow path (LWP-based) for now */
#define	__HAVE_MM_MD_DIRECT_MAPPED_PHYS
#define	__HAVE_SYSCALL_INTERN
#define	__HAVE_CPU_DATA_FIRST
#define	__HAVE_MINIMAL_EMUL

/* No FPU, no TLS for now */

#if defined(_KERNEL)
#define	__HAVE_RAS
#endif

#endif /* _CPUTWO_TYPES_H_ */
