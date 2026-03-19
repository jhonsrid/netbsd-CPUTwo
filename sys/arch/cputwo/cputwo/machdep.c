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
#include <sys/proc.h>
#include <sys/reboot.h>
#include <sys/mount.h>

#include <uvm/uvm_extern.h>

#include <machine/cpu.h>
#include <machine/pcb.h>
#include <machine/pmap.h>

char machine[] = "cputwo";
char machine_arch[] = "cputwo";

struct cpu_info cpu_info_store;

struct pmap kernel_pmap_store;

void
cpu_startup(void)
{

	/* Print memory info */
	printf("total memory = %luK\n", (unsigned long)ctob(physmem) / 1024);

	/* Allocate kernel submaps */
	uvm_map_init();

	printf("avail memory = %luK\n",
	    (unsigned long)uvmexp.free * PAGE_SIZE / 1024);
}

__dead void
cpu_reboot(int howto, const char *bootstr)
{

	/* TODO: proper shutdown sequence */
	for (;;)
		;
}

void
cpu_dumpconf(void)
{

	/* TODO: configure crash dump */
}

void
cpu_need_resched(struct cpu_info *ci, struct lwp *l, int flags)
{

	ci->ci_want_resched = 1;
	/* TODO: send IPI or set AST */
}

void
cpu_signotify(struct lwp *l)
{

	/* TODO: arrange for AST on return to userland */
}

void
cpu_need_proftick(struct lwp *l)
{

	/* TODO: arrange for profiling tick */
}

void
cpu_proc_fork(struct proc *p1, struct proc *p2)
{

	/* nothing */
}

void
cpu_boot_secondary_processors(void)
{

	/* single CPU, nothing to do */
}

void
delay(unsigned int us)
{
	volatile int i;

	/* TODO: calibrate this loop */
	for (i = us * 10; i > 0; i--)
		;
}
