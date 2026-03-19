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
 * CPUTwo pmap: Sv32 two-level page table management.
 *
 * Virtual address space layout:
 *   0x00000000 - 0x7FFFFFFF   User space (2 GB)
 *   0x80000000 - 0x83FFFFFF   Kernel direct map (PA 0 - 64MB, superpages)
 *   0xC0000000 - 0xFFFFF000   Kernel VM (dynamically mapped)
 *
 * MMIO at VA 0x03F00000-0x03FFFFFF bypasses MMU (hardware feature),
 * so device drivers access MMIO at physical addresses directly.
 *
 * Sv32 page table:
 *   L1: 1024 entries, each covers 4MB (superpage if leaf)
 *   L2: 1024 entries, each covers 4KB
 *   PTE: [31:12]=PPN, [9]=G, [8]=U, [7]=X, [6]=W, [5]=R, [1]=D, [0]=V
 *
 * SATP register: bit31=EN, bits[19:0]=PPN of L1 table
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/pool.h>
#include <sys/kmem.h>
#include <sys/mutex.h>
#include <sys/atomic.h>

#include <uvm/uvm.h>
#include <uvm/uvm_extern.h>

#include <machine/cpu.h>
#include <machine/pmap.h>
#include <machine/pte.h>
#include <machine/vmparam.h>

/*
 * Supervisor registers.
 */
#define SREG_SATP	(*(volatile uint32_t *)0x03FFF018)

/*
 * SFENCE instruction — flush non-global TLB entries.
 * Encoded as opcode 0x3E, all other fields zero.
 */
static inline void
cputwo_sfence(void)
{
	__asm volatile(".word 0x3E000000" ::: "memory");
}

/*
 * Kernel pmap (defined in machdep.c, declared in pmap.h).
 */
extern struct pmap kernel_pmap_store;

/*
 * Physical memory management for early bootstrap (before UVM is up).
 * pmap_steal_memory() carves pages from this pool.
 */
static paddr_t	bootstrap_free_pa;	/* next free physical page */
static paddr_t	bootstrap_end_pa;	/* end of available physical memory */

/*
 * Kernel virtual address space for dynamic mappings (VM_MIN_KERNEL_ADDRESS
 * is the direct map, so we use 0xC0000000+ for the "real" kernel VM).
 */
#define KERNEL_VM_BASE	((vaddr_t)0xC0000000)
static vaddr_t	kernel_vm_next;		/* next free kernel VA for dynamic maps */

/*
 * Kernel L1 page table (must be page-aligned, 4KB = 1024 entries).
 */
static pt_entry_t kernel_l1table[1024] __aligned(PAGE_SIZE);

/*
 * Direct-map base: VA 0x80000000 = PA 0x00000000.
 * For a physical address in the direct-map range, convert PA<->VA.
 */
#define KERNBASE	0x80000000UL
#define PA_TO_KVA(pa)	((vaddr_t)((paddr_t)(pa) + KERNBASE))
#define KVA_TO_PA(va)	((paddr_t)((vaddr_t)(va) - KERNBASE))

static bool mmu_enabled;	/* true after pmap_bootstrap enables MMU */

/*
 * Convert PA to a usable pointer.
 * Before MMU: PA == VA (kernel linked at PA 0).
 * After MMU:  PA accessible via direct map at VA 0x80000000+PA.
 */
static inline void *
pa_to_ptr(paddr_t pa)
{
	if (mmu_enabled)
		return (void *)PA_TO_KVA(pa);
	return (void *)(uintptr_t)pa;
}

/*
 * Allocate a zeroed physical page during bootstrap (before UVM pool).
 */
static paddr_t
bootstrap_alloc_page(void)
{
	paddr_t pa;

	if (bootstrap_free_pa + PAGE_SIZE > bootstrap_end_pa)
		panic("pmap: out of bootstrap memory");
	pa = bootstrap_free_pa;
	bootstrap_free_pa += PAGE_SIZE;
	memset(pa_to_ptr(pa), 0, PAGE_SIZE);
	return pa;
}

/*
 * Walk a two-level page table and return a pointer to the L2 PTE
 * for the given VA.  If alloc is true, allocate missing L2 tables.
 * Returns NULL if the L2 table doesn't exist and alloc is false.
 *
 * The caller must hold any necessary locks.
 */
static pt_entry_t *
pmap_walk(pt_entry_t *l1, vaddr_t va, bool alloc, bool bootstrap)
{
	uint32_t l1idx = PX(1, va);
	uint32_t l2idx = PX(0, va);
	pt_entry_t l1e;
	pt_entry_t *l2;
	paddr_t l2pa;

	l1e = l1[l1idx];

	/* Check for superpage (leaf at L1) — can't descend further */
	if ((l1e & PTE_V) && (l1e & (PTE_R | PTE_W | PTE_X)))
		return NULL;	/* superpage, no L2 */

	if (!(l1e & PTE_V)) {
		if (!alloc)
			return NULL;

		/* Allocate a new L2 page table */
		if (bootstrap)
			l2pa = bootstrap_alloc_page();
		else {
			struct vm_page *pg;
			pg = uvm_pagealloc(NULL, 0, NULL,
			    UVM_PGA_USERESERVE | UVM_PGA_ZERO);
			if (pg == NULL)
				return NULL;
			l2pa = VM_PAGE_TO_PHYS(pg);
		}

		/* L1 entry points to L2 table: PPN only, V=1, no R/W/X */
		l1[l1idx] = PA2PTE(l2pa) | PTE_V;
		l1e = l1[l1idx];
	}

	l2 = (pt_entry_t *)pa_to_ptr(PTE2PA(l1e));
	return &l2[l2idx];
}

/* ------------------------------------------------------------------ */
/*  Bootstrap                                                          */
/* ------------------------------------------------------------------ */

/*
 * pmap_bootstrap: set up the kernel page table and enable the MMU.
 *
 * Called from cputwo_init() before main().  At this point we are
 * running at physical addresses with the MMU off.  We need to:
 *
 * 1. Set up direct-map superpages: VA 0x80000000+ → PA 0x00000000+
 * 2. Identity-map the current kernel text (PA == VA) so the PC is
 *    valid immediately after enabling the MMU.
 * 3. Write SATP to enable the MMU.
 * 4. After MMU is on, all kernel code runs at VA 0x80000000+ addresses.
 *
 * NOTE: because we are running at PA addresses before MMU is on,
 * we cannot use PA_TO_KVA() yet.  We access kernel_l1table at its
 * physical address directly.
 */
void
pmap_bootstrap(void)
{
	extern char end[];
	paddr_t pa;
	int i;

	/*
	 * Record bootstrap free memory pool.
	 * cputwo_init() already called uvm_page_physload(), but we need
	 * pages for the page tables themselves.  Steal from the top of
	 * the region that was registered.
	 */
	bootstrap_free_pa = round_page((paddr_t)(uintptr_t)end);
	bootstrap_end_pa = 0x03F00000;	/* PHYS_RAM_END */

	/*
	 * Zero the L1 table (it's in BSS so should be zero, but be safe).
	 */
	memset(kernel_l1table, 0, sizeof(kernel_l1table));

	/*
	 * Set up direct map: VA 0x80000000 → PA 0x00000000, using 4MB
	 * superpages (L1 leaf entries with R|W|X set).
	 *
	 * L1 index for VA 0x80000000 = (0x80000000 >> 22) = 512.
	 * We need 16 entries to cover 64MB (0x00000000 - 0x03FFFFFF).
	 */
	for (i = 0; i < 16; i++) {
		pa = (paddr_t)i << 22;	/* 4MB per entry */
		kernel_l1table[512 + i] = PA2PTE(pa) | PTE_V | PTE_R |
		    PTE_W | PTE_X | PTE_G;
	}

	/*
	 * Identity map: VA 0x00000000 → PA 0x00000000 for the kernel
	 * code that is currently executing.  This is needed so the PC
	 * remains valid immediately after enabling the MMU (we are
	 * running at PA 0x000xxxxx, which is VA 0x000xxxxx).
	 *
	 * We use the same superpages.  After the MMU is on and we
	 * jump to the kernel's VA 0x80000000+ address, we can remove
	 * these identity mappings.
	 */
	for (i = 0; i < 16; i++) {
		pa = (paddr_t)i << 22;
		kernel_l1table[i] = PA2PTE(pa) | PTE_V | PTE_R |
		    PTE_W | PTE_X | PTE_G;
	}

	/*
	 * Set up kernel_pmap_store.
	 */
	kernel_pmap_store.pm_l1table = kernel_l1table;
	kernel_pmap_store.pm_satp =
	    0x80000000U | ((paddr_t)(uintptr_t)kernel_l1table >> 12);
	kernel_pmap_store.pm_count = 1;

	/*
	 * Initialize kernel VM allocation pointer.
	 */
	kernel_vm_next = KERNEL_VM_BASE;

	/*
	 * Enable the MMU!
	 */
	SREG_SATP = kernel_pmap_store.pm_satp;
	cputwo_sfence();
	mmu_enabled = true;

	/*
	 * Now running with MMU on.  Both identity map (VA 0 → PA 0)
	 * and direct map (VA 0x80000000 → PA 0) are active.  The kernel
	 * is linked at PA 0 so all symbol addresses remain valid via
	 * the identity map.  The direct map provides the VA 0x80000000+
	 * view used by PA_TO_KVA after boot.
	 *
	 * pmap_init() will remove the identity map once the kernel is
	 * fully running and no longer needs VA == PA.
	 */
}

/*
 * Return the current bootstrap free PA (for cputwo_init to register
 * remaining memory with UVM after pmap_bootstrap steals pages).
 */
paddr_t
pmap_bootstrap_free_pa(void)
{

	return bootstrap_free_pa;
}

/* ------------------------------------------------------------------ */
/*  Post-bootstrap initialization                                      */
/* ------------------------------------------------------------------ */

void
pmap_init(void)
{

	/*
	 * The identity map (VA 0 → PA 0) is kept permanently because
	 * the kernel is linked at PA 0.  Kernel text/data/BSS live at
	 * low VA addresses; the identity map makes them accessible.
	 * The Global bit is set and User bit is clear, so user processes
	 * cannot access these pages.
	 *
	 * The direct map (VA 0x80000000 → PA 0) provides an alternate
	 * view used by PA_TO_KVA for accessing arbitrary physical pages.
	 */
}

void
pmap_virtual_space(vaddr_t *vstartp, vaddr_t *vendp)
{

	*vstartp = KERNEL_VM_BASE;
	*vendp = VM_MAX_KERNEL_ADDRESS;
}

/* ------------------------------------------------------------------ */
/*  Steal memory (before UVM is fully up)                              */
/* ------------------------------------------------------------------ */

vaddr_t
pmap_steal_memory(vsize_t size, vaddr_t *vstartp, vaddr_t *vendp)
{
	int npages;
	paddr_t pa;
	vaddr_t va;
	uvm_physseg_t bank;

	KASSERT(!uvm.page_init_done);

	size = round_page(size);
	npages = atop(size);

	/* Find a physseg with enough free pages */
	for (bank = uvm_physseg_get_first();
	     uvm_physseg_valid_p(bank);
	     bank = uvm_physseg_get_next(bank)) {
		if (npages <= (int)(uvm_physseg_get_avail_end(bank) -
		    uvm_physseg_get_avail_start(bank)))
			break;
	}

	if (!uvm_physseg_valid_p(bank))
		panic("pmap_steal_memory: no memory (%zu bytes)", size);

	/* Steal from the front of this segment */
	pa = ptoa(uvm_physseg_get_avail_start(bank));
	uvm_physseg_unplug(atop(pa), npages);

	va = PA_TO_KVA(pa);
	memset((void *)va, 0, size);
	return va;
}

/* ------------------------------------------------------------------ */
/*  User pmap create/destroy                                           */
/* ------------------------------------------------------------------ */

pmap_t
pmap_create(void)
{
	struct pmap *pm;
	struct vm_page *pg;
	paddr_t l1pa;

	pm = kmem_zalloc(sizeof(*pm), KM_SLEEP);

	/* Allocate L1 page table */
	pg = uvm_pagealloc(NULL, 0, NULL, UVM_PGA_USERESERVE | UVM_PGA_ZERO);
	if (pg == NULL) {
		kmem_free(pm, sizeof(*pm));
		return NULL;
	}
	l1pa = VM_PAGE_TO_PHYS(pg);
	pm->pm_l1table = (pt_entry_t *)PA_TO_KVA(l1pa);

	/*
	 * Copy kernel mappings from kernel L1 table:
	 *   - Entries 0-15: identity map (VA 0 → PA 0), needed because
	 *     kernel text is linked at PA 0 and trap handlers must be
	 *     accessible when user pmap is active.  G=1, U=0 prevents
	 *     user access (supervisor ignores U bit per arch spec).
	 *   - Entries 512-1023: direct map (VA 0x80000000 → PA 0) and
	 *     kernel VM space.
	 */
	memcpy(&pm->pm_l1table[0], &kernel_l1table[0],
	    16 * sizeof(pt_entry_t));
	memcpy(&pm->pm_l1table[512], &kernel_l1table[512],
	    512 * sizeof(pt_entry_t));

	pm->pm_satp = 0x80000000U | (l1pa >> 12);
	pm->pm_count = 1;
	return pm;
}

void
pmap_destroy(pmap_t pm)
{

	if (--pm->pm_count > 0)
		return;

	/* TODO: free all L2 page tables */
	/* TODO: free the L1 page table page */
	/* TODO: free the pmap struct */
}

void
pmap_reference(pmap_t pm)
{

	pm->pm_count++;
}

/* ------------------------------------------------------------------ */
/*  Mapping operations                                                 */
/* ------------------------------------------------------------------ */

int
pmap_enter(pmap_t pm, vaddr_t va, paddr_t pa, vm_prot_t prot, u_int flags)
{
	pt_entry_t *pte;
	pt_entry_t npte;
	bool bootstrap = (pm == pmap_kernel() && uvm.page_init_done == 0);

	pte = pmap_walk(pm->pm_l1table, va, true, bootstrap);
	if (pte == NULL)
		return ENOMEM;

	npte = PA2PTE(pa) | PTE_V;
	if (prot & VM_PROT_READ)
		npte |= PTE_R;
	if (prot & VM_PROT_WRITE)
		npte |= PTE_W;
	if (prot & VM_PROT_EXECUTE)
		npte |= PTE_X;
	if (pm != pmap_kernel())
		npte |= PTE_U;
	if (pm == pmap_kernel())
		npte |= PTE_G;
	if (flags & PMAP_WIRED)
		; /* TODO: track wired count */

	*pte = npte;
	cputwo_sfence();

	pm->pm_stats.resident_count++;
	return 0;
}

void
pmap_remove(pmap_t pm, vaddr_t sva, vaddr_t eva)
{
	vaddr_t va;
	pt_entry_t *pte;

	for (va = sva; va < eva; va += PAGE_SIZE) {
		pte = pmap_walk(pm->pm_l1table, va, false, false);
		if (pte == NULL || !(*pte & PTE_V))
			continue;
		*pte = 0;
		pm->pm_stats.resident_count--;
	}
	cputwo_sfence();
}

void
pmap_protect(pmap_t pm, vaddr_t sva, vaddr_t eva, vm_prot_t prot)
{
	vaddr_t va;
	pt_entry_t *pte;

	if (prot == VM_PROT_NONE) {
		pmap_remove(pm, sva, eva);
		return;
	}

	for (va = sva; va < eva; va += PAGE_SIZE) {
		pte = pmap_walk(pm->pm_l1table, va, false, false);
		if (pte == NULL || !(*pte & PTE_V))
			continue;

		/* Clear and re-set permission bits */
		*pte &= ~(PTE_R | PTE_W | PTE_X);
		if (prot & VM_PROT_READ)
			*pte |= PTE_R;
		if (prot & VM_PROT_WRITE)
			*pte |= PTE_W;
		if (prot & VM_PROT_EXECUTE)
			*pte |= PTE_X;
	}
	cputwo_sfence();
}

void
pmap_unwire(pmap_t pm, vaddr_t va)
{

	/* TODO: track wired pages */
}

/* ------------------------------------------------------------------ */
/*  Kernel mapping operations                                          */
/* ------------------------------------------------------------------ */

void
pmap_kenter_pa(vaddr_t va, paddr_t pa, vm_prot_t prot, u_int flags)
{
	pt_entry_t *pte;
	pt_entry_t npte;
	bool bootstrap = (uvm.page_init_done == 0);

	pte = pmap_walk(kernel_l1table, va, true, bootstrap);
	if (pte == NULL)
		panic("pmap_kenter_pa: cannot allocate L2 for va %#lx",
		    (unsigned long)va);

	npte = PA2PTE(pa) | PTE_V | PTE_G;
	if (prot & VM_PROT_READ)
		npte |= PTE_R;
	if (prot & VM_PROT_WRITE)
		npte |= PTE_W;
	if (prot & VM_PROT_EXECUTE)
		npte |= PTE_X;

	*pte = npte;
}

void
pmap_kremove(vaddr_t va, vsize_t size)
{
	vaddr_t end = va + size;

	for (; va < end; va += PAGE_SIZE) {
		pt_entry_t *pte = pmap_walk(kernel_l1table, va, false, false);
		if (pte != NULL)
			*pte = 0;
	}
	cputwo_sfence();
}

/* ------------------------------------------------------------------ */
/*  Extract / query                                                    */
/* ------------------------------------------------------------------ */

bool
pmap_extract(pmap_t pm, vaddr_t va, paddr_t *pap)
{
	pt_entry_t *l1 = pm->pm_l1table;
	uint32_t l1idx = PX(1, va);
	pt_entry_t l1e = l1[l1idx];

	if (!(l1e & PTE_V))
		return false;

	/* Check for superpage (L1 leaf) */
	if (l1e & (PTE_R | PTE_W | PTE_X)) {
		if (pap)
			*pap = PTE2PA(l1e) | (va & 0x003FFFFF);
		return true;
	}

	/* Walk to L2 */
	pt_entry_t *pte = pmap_walk(l1, va, false, false);
	if (pte == NULL || !(*pte & PTE_V))
		return false;

	if (pap)
		*pap = PTE2PA(*pte) | (va & PAGE_MASK);
	return true;
}

/* ------------------------------------------------------------------ */
/*  Activate / deactivate / update                                     */
/* ------------------------------------------------------------------ */

void
pmap_activate(struct lwp *l)
{
	struct pmap *pm = l->l_proc->p_vmspace->vm_map.pmap;

	SREG_SATP = pm->pm_satp;
	cputwo_sfence();
}

void
pmap_deactivate(struct lwp *l)
{

	/* Nothing — we switch SATP on activate */
}

void
pmap_update(pmap_t pm)
{

	cputwo_sfence();
}

/* ------------------------------------------------------------------ */
/*  Page operations                                                    */
/* ------------------------------------------------------------------ */

void
pmap_zero_page(paddr_t pa)
{

	memset((void *)PA_TO_KVA(pa), 0, PAGE_SIZE);
}

void
pmap_copy_page(paddr_t src, paddr_t dst)
{

	memcpy((void *)PA_TO_KVA(dst), (void *)PA_TO_KVA(src), PAGE_SIZE);
}

void
pmap_copy(pmap_t dst, pmap_t src, vaddr_t dstva, vsize_t size, vaddr_t srcva)
{

	/* Nothing — COW handled by UVM */
}

/* ------------------------------------------------------------------ */
/*  Page attribute tracking                                            */
/* ------------------------------------------------------------------ */

void
pmap_page_protect(struct vm_page *pg, vm_prot_t prot)
{

	/* TODO: walk all pmaps that map this page and downgrade protection */
	if (prot == VM_PROT_NONE) {
		/* TODO: remove all mappings of this page */
	}
}

bool
pmap_clear_modify(struct vm_page *pg)
{
	bool rv = (pg->mdpage.mdpg_attrs & VM_PAGE_MD_MODIFIED) != 0;

	pg->mdpage.mdpg_attrs &= ~VM_PAGE_MD_MODIFIED;
	return rv;
}

bool
pmap_clear_reference(struct vm_page *pg)
{
	bool rv = (pg->mdpage.mdpg_attrs & VM_PAGE_MD_REFERENCED) != 0;

	pg->mdpage.mdpg_attrs &= ~VM_PAGE_MD_REFERENCED;
	return rv;
}

bool
pmap_is_modified(struct vm_page *pg)
{

	return (pg->mdpage.mdpg_attrs & VM_PAGE_MD_MODIFIED) != 0;
}

bool
pmap_is_referenced(struct vm_page *pg)
{

	return (pg->mdpage.mdpg_attrs & VM_PAGE_MD_REFERENCED) != 0;
}

paddr_t
pmap_phys_address(paddr_t cookie)
{

	return cookie;
}
