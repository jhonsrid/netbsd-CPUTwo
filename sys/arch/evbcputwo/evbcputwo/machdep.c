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
#include <sys/kernel.h>
#include <sys/device.h>
#include <sys/lwp.h>
#include <sys/proc.h>

#include <uvm/uvm_extern.h>

#include <machine/cpu.h>
#include <machine/pmap.h>

#include <dev/cons.h>

/*
 * Supervisor registers (memory-mapped, always bypass MMU).
 */
#define	SREG_BASE	0x03FFF000
#define	SREG_EPC	(*(volatile uint32_t *)(SREG_BASE + 0x00))
#define	SREG_EFLAGS	(*(volatile uint32_t *)(SREG_BASE + 0x04))
#define	SREG_EVEC	(*(volatile uint32_t *)(SREG_BASE + 0x08))
#define	SREG_CAUSE	(*(volatile uint32_t *)(SREG_BASE + 0x0C))
#define	SREG_STATUS	(*(volatile uint32_t *)(SREG_BASE + 0x10))
#define	SREG_ESTATUS	(*(volatile uint32_t *)(SREG_BASE + 0x14))
#define	SREG_SATP	(*(volatile uint32_t *)(SREG_BASE + 0x18))
#define	SREG_BADADDR	(*(volatile uint32_t *)(SREG_BASE + 0x1C))

/*
 * Physical memory layout:
 *   0x00000000 - 0x03EFFFFF : RAM (63 MB)
 *   0x03F00000 - 0x03FFFFFF : MMIO (bypasses MMU)
 *
 * Kernel virtual base: 0x80000000 (direct map: VA 0x80000000 = PA 0x00000000)
 */
#define	PHYS_RAM_END	0x03F00000	/* first byte past RAM */

/* Linker-provided symbol marking end of kernel BSS */
extern char end[];

/* Exception entry point from locore.S */
extern void exception_entry(void);

/*
 * Board-level initialization for evbcputwo.
 * Called from cputwo_start.S after stack setup, running at physical
 * addresses with MMU off.
 */
void
cputwo_init(void)
{
	paddr_t first_free_pa;

	/*
	 * Step 1: Set up cpu_info for bootstrap CPU.
	 * lwp0 is the initial kernel thread; it must be set as curlwp
	 * before anything that touches per-CPU state.
	 */
	memset(&cpu_info_store, 0, sizeof(cpu_info_store));
	cpu_info_store.ci_curlwp = &lwp0;

	/*
	 * Step 2: Set up exception vector table.
	 * EVEC register holds the BASE ADDRESS of a table of 32-bit handler
	 * addresses.  On exception with cause N, hardware loads the handler
	 * from EVEC + (N * 4) and jumps to it.  We need at least 10 entries
	 * (causes 0x00-0x09).  Use 16 for headroom.
	 */
	{
		static uint32_t evec_table[16];
		int i;

		for (i = 0; i < 16; i++)
			evec_table[i] = (uint32_t)(uintptr_t)exception_entry;
		SREG_EVEC = (uint32_t)(uintptr_t)evec_table;
	}

	/*
	 * Step 3: Early console for printf/panic.
	 */
	consinit();

	printf("CPUTwo NetBSD bootstrap\n");

	/*
	 * Step 4: Compute physical memory layout.
	 *
	 * Total physical RAM: 0x00000000 to PHYS_RAM_END (63 MB).
	 * Kernel occupies: 0x00000000 to end[].
	 * Free memory: round_page(&end) to PHYS_RAM_END.
	 */
	physmem = atop(PHYS_RAM_END);

	first_free_pa = round_page((paddr_t)(uintptr_t)end);

	printf("kernel end = %p, first free PA = 0x%lx\n",
	    end, (unsigned long)first_free_pa);
	printf("physical memory: %lu KB (%lu pages)\n",
	    (unsigned long)(PHYS_RAM_END / 1024),
	    (unsigned long)physmem);

	/*
	 * Step 5: Bootstrap the pmap (kernel page tables).
	 * pmap_bootstrap() may steal physical pages for page table
	 * allocation via bootstrap_alloc_page().  We register free
	 * memory with UVM AFTER pmap_bootstrap so the stolen pages
	 * are excluded from the free pool.
	 */
	pmap_bootstrap();

	printf("pmap_bootstrap done\n");

	/*
	 * Step 6: Register remaining free physical memory with UVM.
	 *
	 * pmap_bootstrap() advanced its internal free pointer past any
	 * pages it allocated.  We query it to get the real first free PA.
	 */
	{
		extern paddr_t pmap_bootstrap_free_pa(void);
		paddr_t real_free = pmap_bootstrap_free_pa();

		uvm_page_physload(atop(real_free), atop(PHYS_RAM_END),
		    atop(real_free), atop(PHYS_RAM_END),
		    VM_FREELIST_DEFAULT);

		printf("free memory: 0x%lx - 0x%lx (%lu KB)\n",
		    (unsigned long)real_free,
		    (unsigned long)PHYS_RAM_END,
		    (unsigned long)(PHYS_RAM_END - real_free) / 1024);
	}

	/*
	 * Step 7: Enter main kernel initialization.
	 */
	main();

	/* NOTREACHED */
	panic("main() returned");
}

/*
 * Early console via polled UART at 0x03F00000.
 *
 * UART register layout:
 *   +0x00  STATUS  (bit 0 = TX ready, bit 1 = RX available)
 *   +0x04  TX data
 *   +0x08  RX data
 */
#define UART_STATUS	(*(volatile uint32_t *)0x03F00000)
#define UART_TX		(*(volatile uint32_t *)0x03F00004)
#define UART_RX		(*(volatile uint32_t *)0x03F00008)
#define UART_TX_READY	0x01
#define UART_RX_AVAIL	0x02

static void
cputwo_cnputc(dev_t dev, int c)
{

	while ((UART_STATUS & UART_TX_READY) == 0)
		;
	UART_TX = c;
}

static int
cputwo_cngetc(dev_t dev)
{

	while ((UART_STATUS & UART_RX_AVAIL) == 0)
		;
	return UART_RX;
}

static struct consdev cputwo_consdev = {
	.cn_getc = cputwo_cngetc,
	.cn_putc = cputwo_cnputc,
	.cn_pollc = nullcnpollc,
	.cn_dev = NODEV,
	.cn_pri = CN_NORMAL,
};

void
consinit(void)
{

	cn_tab = &cputwo_consdev;
}
