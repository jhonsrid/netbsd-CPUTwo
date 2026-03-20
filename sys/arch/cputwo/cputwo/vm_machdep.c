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

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/lwp.h>

#include <uvm/uvm_extern.h>

#include <machine/cpu.h>
#include <machine/frame.h>
#include <machine/pcb.h>

/* From locore.S */
extern void lwp_trampoline(void);

/*
 * cpu_lwp_fork: set up the kernel state for a new LWP (l2) forked from l1.
 *
 * 1. Copy the parent's trapframe to the child's kernel stack.
 * 2. If a new user stack is provided, update the trapframe's SP.
 * 3. Set the child's trapframe return value to 0 (fork returns 0 to child).
 * 4. Set up the switchframe so cpu_switchto() "returns" to lwp_trampoline,
 *    which calls lwp_startup() then func(arg).
 */
void
cpu_lwp_fork(struct lwp *l1, struct lwp *l2, void *stack, size_t stacksize,
    void (*func)(void *), void *arg)
{
	struct pcb *pcb1, *pcb2;
	struct trapframe *tf1, *tf2;
	struct switchframe *sf;
	vaddr_t uv;

	pcb1 = lwp_getpcb(l1);
	pcb2 = lwp_getpcb(l2);

	/*
	 * Copy the parent's PCB to the child.
	 */
	*pcb2 = *pcb1;
	pcb2->pcb_onfault = NULL;

	/*
	 * Set up the child's kernel stack.
	 * The trapframe sits at the top of the kernel stack.
	 * uv = base of the u-area (kernel stack region).
	 */
	uv = uvm_lwp_getuarea(l2);

	/*
	 * Place the trapframe at the top of the kernel stack.
	 * Kernel stack: [uv, uv + USPACE).  Trapframe at top.
	 */
	tf2 = (struct trapframe *)(uv + USPACE - sizeof(struct trapframe));
	pcb2->pcb_tf = tf2;
	l2->l_md.md_utf = tf2;

	/*
	 * Copy the parent's trapframe.
	 */
	tf1 = pcb1->pcb_tf;
	if (tf1 != NULL)
		*tf2 = *tf1;
	else
		memset(tf2, 0, sizeof(*tf2));

	/*
	 * If a new user stack is specified (e.g., for threads), update SP.
	 */
	if (stack != NULL)
		tf2->tf_r[13] = (uint32_t)((uintptr_t)stack + stacksize);

	/*
	 * Child fork returns 0.
	 */
	tf2->tf_r[0] = 0;

	/*
	 * Set up the child's trapframe as a user return frame.
	 * ESTATUS: user mode (bit 0 = 0), IE = 1 (bit 1 = 1).
	 */
	tf2->tf_status = 0x02;	/* user mode, interrupts enabled */

	/*
	 * Set up the switchframe so that when cpu_switchto() switches to l2,
	 * it "returns" to lwp_trampoline.
	 *
	 * lwp_trampoline expects:
	 *   r4 = func
	 *   r5 = arg
	 *   r6 = l2 (new lwp)
	 *   sp = points to the trapframe
	 *   lr = lwp_trampoline (but cpu_switchto restores lr from sf)
	 */
	sf = &pcb2->pcb_sf;
	memset(sf, 0, sizeof(*sf));
	sf->sf_r4 = (uint32_t)(uintptr_t)func;
	sf->sf_r5 = (uint32_t)(uintptr_t)arg;
	sf->sf_r6 = (uint32_t)(uintptr_t)l2;
	sf->sf_sp = (uint32_t)(uintptr_t)tf2;	/* stack points at trapframe */
	sf->sf_lr = (uint32_t)(uintptr_t)lwp_trampoline;
}

void
cpu_lwp_free(struct lwp *l, int proc)
{

	/* nothing */
}

void
cpu_lwp_free2(struct lwp *l)
{

	/* nothing */
}

/* forward declaration */
void cpu_setfunc(struct lwp *, void (*)(void *), void *);

/*
 * cpu_setfunc: change the function a kernel LWP will call.
 * Used for idle LWPs and softint LWPs.
 */
void
cpu_setfunc(struct lwp *l, void (*func)(void *), void *arg)
{
	struct pcb *pcb = lwp_getpcb(l);
	struct switchframe *sf = &pcb->pcb_sf;

	sf->sf_r4 = (uint32_t)(uintptr_t)func;
	sf->sf_r5 = (uint32_t)(uintptr_t)arg;
	sf->sf_r6 = (uint32_t)(uintptr_t)l;
	sf->sf_lr = (uint32_t)(uintptr_t)lwp_trampoline;
}
