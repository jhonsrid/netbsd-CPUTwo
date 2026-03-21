/* $NetBSD$ */

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
 * CPUTwo machine dependent routines for kvm.
 *
 * Memory layout:
 *   0x00000000 - 0x7FFFFFFF  User space
 *   0x80000000 - 0xBFFFFFFF  Kernel direct-mapped (KSEG): VA = PA + 0x80000000
 *   0xC0000000 - 0xFFFFFFFF  Kernel virtual (dynamic mappings)
 */

#include <sys/cdefs.h>
__RCSID("$NetBSD$");

#include <sys/param.h>
#include <sys/proc.h>
#include <sys/stat.h>
#include <sys/kcore.h>
#include <sys/types.h>

#include <stdlib.h>
#include <unistd.h>
#include <nlist.h>
#include <kvm.h>

#include <uvm/uvm_extern.h>

#include <limits.h>
#include <db.h>

#include "kvm_private.h"

#define CPUTWO_KERNBASE		0x80000000UL
#define CPUTWO_KSEG_END		0xC0000000UL

void
_kvm_freevtop(kvm_t *kd)
{

	if (kd->vmst != 0)
		free(kd->vmst);
}

int
_kvm_initvtop(kvm_t *kd)
{

	return (0);
}

/*
 * Translate a kernel virtual address to a physical address.
 */
int
_kvm_kvatop(kvm_t *kd, vaddr_t va, paddr_t *pa)
{
	int page_off;

	if (ISALIVE(kd)) {
		_kvm_err(kd, 0, "vatop called in live kernel!");
		return (0);
	}

	page_off = va & PGOFSET;

	if (va < CPUTWO_KERNBASE) {
		_kvm_err(kd, 0, "invalid kernel virtual address");
		goto lose;
	}

	if (va < CPUTWO_KSEG_END) {
		/*
		 * Direct-mapped KSEG address: PA = VA - KERNBASE.
		 */
		*pa = va - CPUTWO_KERNBASE;
		return (NBPG - page_off);
	}

	/*
	 * Kernel virtual address >= 0xC0000000.
	 * Would need page table walk — not yet implemented.
	 */
	_kvm_err(kd, 0, "kernel virtual address translation not implemented");

 lose:
	*pa = (paddr_t)-1;
	return (0);
}

/*
 * Translate a physical address to a file-offset in the crash dump.
 */
off_t
_kvm_pa2off(kvm_t *kd, paddr_t pa)
{

	return (kd->dump_off + (off_t)pa);
}

int
_kvm_mdopen(kvm_t *kd)
{

	kd->min_uva = 0x04000000;	/* VM_MIN_ADDRESS */
	kd->max_uva = 0x80000000UL - 4096;	/* VM_MAXUSER_ADDRESS */

	return (0);
}
