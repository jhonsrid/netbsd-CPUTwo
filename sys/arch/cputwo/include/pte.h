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

#ifndef _CPUTWO_PTE_H_
#define _CPUTWO_PTE_H_

typedef uint32_t pt_entry_t;

/*
 * CPUTwo Sv32 page table entry bits.
 * Matches xv6-CPUTwo cputwo.h definitions.
 *
 * PTE layout: [31:12]=PPN, [11:10]=RSW(software), [9]=G, [8]=U,
 *             [7]=X, [6]=W, [5]=R, [4:2]=reserved, [1]=D, [0]=V
 */
#define PTE_V	(1u << 0)	/* Valid */
#define PTE_D	(1u << 1)	/* Dirty (set by hardware on store) */
#define PTE_R	(1u << 5)	/* Readable */
#define PTE_W	(1u << 6)	/* Writable */
#define PTE_X	(1u << 7)	/* Executable */
#define PTE_U	(1u << 8)	/* User-accessible */
#define PTE_G	(1u << 9)	/* Global (not flushed by SFENCE/SATP write) */

/*
 * PPN is in bits [31:12].  PA and PPN share the same position,
 * so PA2PTE just masks off the page offset, and PTE2PA extracts the PPN.
 */
#define PA2PTE(pa)	((((uint32_t)(pa)) >> 12) << 12)
#define PTE2PA(pte)	(((pte) >> 12) << 12)
#define PTE_FLAGS(pte)	((pte) & 0xFFF)

/* Sv32: 10-bit VPN fields */
#define PXMASK		0x3FF
#define PXSHIFT(level)	(12 + (10 * (level)))
#define PX(level, va)	((((uint32_t)(va)) >> PXSHIFT(level)) & PXMASK)

/* Number of PTEs per page table page */
#define NPTEPG		(PAGE_SIZE / sizeof(pt_entry_t))	/* 1024 */

#endif /* _CPUTWO_PTE_H_ */
