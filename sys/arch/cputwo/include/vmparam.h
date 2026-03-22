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

#ifndef _CPUTWO_VMPARAM_H_
#define _CPUTWO_VMPARAM_H_

/*
 * CPUTwo: 32-bit, Sv32 two-level page tables, 4KB pages.
 */

#define PAGE_SHIFT	12
#define PAGE_SIZE	(1 << PAGE_SHIFT)	/* 4096 */
#define PAGE_MASK	(PAGE_SIZE - 1)

/*
 * Kernel stack size: 2 pages = 8KB.
 */
#define USPACE		8192
#define	UPAGES		(USPACE >> PAGE_SHIFT)

/*
 * USRSTACK is the top (end) of the user stack.
 * User VA space is [0, 0x80000000).  Place stack at the top.
 */
#define USRSTACK	((vaddr_t) 0x80000000U - PAGE_SIZE)

#ifndef MAXTSIZ
#define	MAXTSIZ		(1UL << 26)	/* max text size: 64MB */
#endif

#ifndef MAXDSIZ
#define	MAXDSIZ		(1UL << 26)	/* max data size: 64MB */
#endif

#ifndef MAXSSIZ
#define	MAXSSIZ		(1UL << 26)	/* max stack size: 64MB */
#endif

#ifndef DFLDSIZ
#define	DFLDSIZ		(1UL << 25)	/* default data size: 32MB */
#endif

#ifndef DFLSSIZ
#define	DFLSSIZ		(1UL << 21)	/* default stack size: 2MB */
#endif

/*
 * Virtual address space layout.
 * Lower half [0, 0x80000000) is user; upper half is kernel.
 */
/*
 * User VA starts above the kernel identity map region (0x00000000-0x03FFFFFF).
 * The identity map uses supervisor-only superpages (G=1, U=0) in every
 * L1 table, so user code cannot access them.  Start user space at 0x04000000.
 */
#define	VM_MIN_ADDRESS		((vaddr_t) 0x04000000)
#define	VM_MAXUSER_ADDRESS	((vaddr_t) 0x80000000U - PAGE_SIZE)
#define	VM_MAX_ADDRESS		VM_MAXUSER_ADDRESS

#define VM_MIN_KERNEL_ADDRESS	((vaddr_t) 0x80000000UL)
#define VM_MAX_KERNEL_ADDRESS	((vaddr_t) 0xFFFFFFFFUL - PAGE_SIZE)

/* virtual sizes (bytes) for various kernel submaps */
#define USRIOSIZE		(PAGE_SIZE / 8)
#define VM_PHYS_SIZE		(USRIOSIZE * PAGE_SIZE)

#define VM_PHYSSEG_MAX		4
#define VM_PHYSSEG_STRAT	VM_PSTRAT_BSEARCH

#define VM_NFREELIST		1
#define	VM_FREELIST_DEFAULT	0

#endif /* _CPUTWO_VMPARAM_H_ */
