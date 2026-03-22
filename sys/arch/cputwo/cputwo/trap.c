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
#include <sys/signal.h>
#include <sys/signalvar.h>
#include <sys/kernel.h>

#include <uvm/uvm_extern.h>

#include <machine/cpu.h>
#include <machine/frame.h>
#include <machine/trap.h>
#include <machine/pcb.h>
#include <machine/pmap.h>

/*
 * IC registers for interrupt dispatch.
 */
#define IC_PENDING	(*(volatile uint32_t *)0x03F02000)
#define IC_ACK		(*(volatile uint32_t *)0x03F02008)
#define IC_TIMER	0x01
#define IC_UART_RX	0x02
#define IC_UART_TX	0x04
#define IC_BLKDEV	0x08

/*
 * BADADDR supervisor register (read page fault address).
 */
#define SREG_BADADDR	(*(volatile uint32_t *)0x03FFF01C)

/* forward declaration */
void trap(struct trapframe *);

/* From clock.c */
extern void cputwo_clockintr(struct clockframe *);
/* From cputwo_ic.c */
extern uint32_t cputwo_ic_pending(void);
extern void cputwo_ic_ack(uint32_t);
/* From cputwo_uart.c */
extern void cputwo_uart_intr(void);
/* From cputwo_blk.c */
extern void cputwo_blk_intr(void);
/* From intr.c */
extern uint32_t cputwo_intr_allowed(uint32_t);

/*
 * Handle hardware interrupts (cause 0x06).
 *
 * Read IC pending register, filter by current IPL (sources at or
 * below ci_cpl are deferred and dispatched when IPL is lowered),
 * then dispatch allowed sources.
 */
static void
cputwo_interrupt(struct trapframe *tf)
{
	struct cpu_info *ci = curcpu();
	uint32_t pending, allowed;
	int saved_ipl;

	ci->ci_intr_depth++;

	pending = cputwo_ic_pending();
	allowed = cputwo_intr_allowed(pending);

	if (allowed & IC_TIMER) {
		struct clockframe cf;

		saved_ipl = _splraise(IPL_SCHED);

		cf.cf_pc = tf->tf_pc;
		cf.cf_sr = tf->tf_status;
		cf.cf_intr_depth = ci->ci_intr_depth;
		cputwo_clockintr(&cf);

		splx(saved_ipl);
	}

	if (allowed & (IC_UART_RX | IC_UART_TX)) {
		saved_ipl = _splraise(IPL_VM);
		cputwo_uart_intr();
		cputwo_ic_ack(allowed & (IC_UART_RX | IC_UART_TX));
		splx(saved_ipl);
	}

	if (allowed & IC_BLKDEV) {
		saved_ipl = _splraise(IPL_VM);
		cputwo_blk_intr();
		cputwo_ic_ack(IC_BLKDEV);
		splx(saved_ipl);
	}

	ci->ci_intr_depth--;
}

/*
 * trap: main trap handler.
 *
 * Called from exception_entry in locore.S with a trapframe on the stack.
 * Dispatches based on tf_cause:
 *   - Hardware interrupt (0x06) → cputwo_interrupt()
 *   - Syscall (0x03) → syscall()
 *   - Page faults (0x07-0x09) → uvm_fault()
 *   - Everything else → panic or signal
 */
void
trap(struct trapframe *tf)
{
	uint32_t cause = tf->tf_cause;
	int user = (tf->tf_status & 0x01) == 0;	/* bit0=0 is user mode */

	switch (cause) {
	case T_HWINT:
		cputwo_interrupt(tf);
		return;

	case T_SYSCALL: {
		struct lwp *l = curlwp;

		/*
		 * EPC points to the instruction after SYSCALL (hw sets
		 * EPC=PC+4 for syscalls).  The trapframe already has this.
		 * Call the MD syscall dispatcher via p_md.md_syscall,
		 * set up by syscall_intern().
		 */
		(*l->l_proc->p_md.md_syscall)(tf);
		return;
	}

	case T_IFAULT:
	case T_LFAULT:
	case T_SFAULT: {
		vaddr_t va = (vaddr_t)SREG_BADADDR;
		vm_prot_t ftype;
		int rv;

		if (cause == T_SFAULT)
			ftype = VM_PROT_WRITE;
		else if (cause == T_IFAULT)
			ftype = VM_PROT_EXECUTE;
		else
			ftype = VM_PROT_READ;

		struct proc *p = curproc;
		struct vm_map *map;

		if (va >= VM_MIN_KERNEL_ADDRESS || !user) {
			map = kernel_map;
		} else {
			map = &p->p_vmspace->vm_map;
		}

		rv = uvm_fault(map, trunc_page(va), ftype);
		if (rv == 0)
			return;

		if (user) {
			/* Send SIGSEGV to the process */
			/* TODO: proper signal delivery */
			panic("trap: user page fault va=%#lx pc=%#x",
			    (unsigned long)va, tf->tf_pc);
		}

		/* Kernel page fault — check onfault handler */
		struct pcb *pcb = lwp_getpcb(curlwp);
		if (pcb->pcb_onfault != NULL) {
			tf->tf_pc = (uint32_t)(uintptr_t)pcb->pcb_onfault;
			return;
		}

		panic("trap: kernel page fault va=%#lx pc=%#x cause=%u",
		    (unsigned long)va, tf->tf_pc, cause);
		break;
	}

	case T_ILLEGAL:
	case T_MISALIGN:
	case T_BUSERR:
	case T_DIVZERO:
	default:
		if (user) {
			panic("trap: user fault cause=%u pc=%#x",
			    cause, tf->tf_pc);
		}
		panic("trap: kernel fault cause=%u pc=%#x",
		    cause, tf->tf_pc);
		break;
	}
}
