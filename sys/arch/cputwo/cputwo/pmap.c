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

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/pool.h>

#include <uvm/uvm.h>
#include <uvm/uvm_extern.h>

#include <machine/pmap.h>
#include <machine/pte.h>
#include <machine/vmparam.h>

struct pmap kernel_pmap_store;

void
pmap_bootstrap(void)
{

	panic("pmap_bootstrap: not implemented");
}

void
pmap_init(void)
{

	panic("pmap_init: not implemented");
}

void
pmap_virtual_space(vaddr_t *vstartp, vaddr_t *vendp)
{

	panic("pmap_virtual_space: not implemented");
}

pmap_t
pmap_create(void)
{

	panic("pmap_create: not implemented");
}

void
pmap_destroy(pmap_t pm)
{

	panic("pmap_destroy: not implemented");
}

void
pmap_reference(pmap_t pm)
{

	panic("pmap_reference: not implemented");
}

int
pmap_enter(pmap_t pm, vaddr_t va, paddr_t pa, vm_prot_t prot, u_int flags)
{

	panic("pmap_enter: not implemented");
}

void
pmap_remove(pmap_t pm, vaddr_t sva, vaddr_t eva)
{

	panic("pmap_remove: not implemented");
}

void
pmap_protect(pmap_t pm, vaddr_t sva, vaddr_t eva, vm_prot_t prot)
{

	panic("pmap_protect: not implemented");
}

void
pmap_unwire(pmap_t pm, vaddr_t va)
{

	panic("pmap_unwire: not implemented");
}

bool
pmap_extract(pmap_t pm, vaddr_t va, paddr_t *pap)
{

	panic("pmap_extract: not implemented");
}

void
pmap_kenter_pa(vaddr_t va, paddr_t pa, vm_prot_t prot, u_int flags)
{

	panic("pmap_kenter_pa: not implemented");
}

void
pmap_kremove(vaddr_t va, vsize_t size)
{

	panic("pmap_kremove: not implemented");
}

void
pmap_copy(pmap_t dst, pmap_t src, vaddr_t dstva, vsize_t size, vaddr_t srcva)
{

	/* nothing */
}

void
pmap_update(pmap_t pm)
{

	/* nothing */
}

void
pmap_activate(struct lwp *l)
{

	panic("pmap_activate: not implemented");
}

void
pmap_deactivate(struct lwp *l)
{

	/* nothing */
}

void
pmap_zero_page(paddr_t pa)
{

	panic("pmap_zero_page: not implemented");
}

void
pmap_copy_page(paddr_t src, paddr_t dst)
{

	panic("pmap_copy_page: not implemented");
}

void
pmap_page_protect(struct vm_page *pg, vm_prot_t prot)
{

	panic("pmap_page_protect: not implemented");
}

bool
pmap_clear_modify(struct vm_page *pg)
{

	panic("pmap_clear_modify: not implemented");
}

bool
pmap_clear_reference(struct vm_page *pg)
{

	panic("pmap_clear_reference: not implemented");
}

bool
pmap_is_modified(struct vm_page *pg)
{

	panic("pmap_is_modified: not implemented");
}

bool
pmap_is_referenced(struct vm_page *pg)
{

	panic("pmap_is_referenced: not implemented");
}

paddr_t
pmap_phys_address(paddr_t cookie)
{

	return cookie;
}

vaddr_t
pmap_steal_memory(vsize_t size, vaddr_t *vstartp, vaddr_t *vendp)
{

	panic("pmap_steal_memory: not implemented");
}
