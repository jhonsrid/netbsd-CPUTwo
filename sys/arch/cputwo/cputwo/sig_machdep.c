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
 * CPUTwo signal delivery and context save/restore.
 *
 * Signal frame layout on user stack (sigframe_siginfo):
 *   - siginfo_t         (signal info)
 *   - ucontext_t        (saved user context: registers, signal mask)
 *
 * Signal delivery:
 *   1. Push sigframe onto user stack
 *   2. Set trapframe PC = signal handler
 *   3. Set trapframe r0 = signal number, r1 = &siginfo, r2 = &ucontext
 *   4. Set trapframe SP = new stack with sigframe
 *   5. Set trapframe LR = signal trampoline (calls sigreturn)
 *
 * On sigreturn:
 *   1. Copy ucontext from user stack
 *   2. Restore register context from mcontext
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/signal.h>
#include <sys/signalvar.h>
#include <sys/proc.h>
#include <sys/lwp.h>
#include <sys/ucontext.h>
#include <sys/syscallargs.h>

#include <machine/frame.h>
#include <machine/pcb.h>
#include <machine/mcontext.h>

/*
 * cpu_getmcontext: save the current register state into an mcontext.
 */
void
cpu_getmcontext(struct lwp *l, mcontext_t *mcp, unsigned int *flags)
{
	struct trapframe *tf = l->l_md.md_utf;
	__greg_t *gr = mcp->__gregs;
	int i;

	for (i = 0; i < 15; i++)
		gr[_REG_R0 + i] = (__greg_t)tf->tf_r[i];
	gr[_REG_PC] = (__greg_t)tf->tf_pc;
	gr[_REG_STATUS] = (__greg_t)tf->tf_status;

	*flags |= _UC_CPU;
}

/*
 * cpu_setmcontext: restore register state from an mcontext.
 */
int
cpu_setmcontext(struct lwp *l, const mcontext_t *mcp, unsigned int flags)
{
	struct trapframe *tf = l->l_md.md_utf;
	const __greg_t *gr = mcp->__gregs;
	int i;

	if (flags & _UC_CPU) {
		for (i = 0; i < 15; i++)
			tf->tf_r[i] = (uint32_t)gr[_REG_R0 + i];
		tf->tf_pc = (uint32_t)gr[_REG_PC];
		/* Don't restore STATUS — could escalate privilege */
	}

	return 0;
}

/*
 * Signal frame pushed onto the user stack.
 */
struct sigframe_siginfo {
	siginfo_t sf_si;
	ucontext_t sf_uc;
};

/*
 * sendsig_siginfo: deliver a signal to a process.
 *
 * Push a signal frame onto the user stack and arrange for the process
 * to call the signal handler when it returns to user mode.
 */
void
sendsig_siginfo(const ksiginfo_t *ksi, const sigset_t *mask)
{
	struct lwp *l = curlwp;
	struct proc *p = l->l_proc;
	struct sigacts *ps = p->p_sigacts;
	struct trapframe *tf = l->l_md.md_utf;
	int sig = ksi->ksi_signo;
	sig_t catcher = SIGACTION(p, sig).sa_handler;
	struct sigframe_siginfo *fp, frame;
	int onstack, error;

	/* Determine if signal should be on the alternate signal stack */
	fp = getframe(l, sig, &onstack);
	fp--;

	/* Build the signal frame */
	memset(&frame, 0, sizeof(frame));
	frame.sf_si._info = ksi->ksi_info;
	frame.sf_uc.uc_link = l->l_ctxlink;
	frame.sf_uc.uc_sigmask = *mask;
	frame.sf_uc.uc_flags = _UC_SIGMASK;
	frame.sf_uc.uc_flags |= (l->l_sigstk.ss_flags & SS_ONSTACK)
	    ? _UC_SETSTACK : _UC_CLRSTACK;

	sendsig_reset(l, sig);
	mutex_exit(p->p_lock);

	/* Save current context */
	cpu_getmcontext(l, &frame.sf_uc.uc_mcontext, &frame.sf_uc.uc_flags);

	/* Copy signal frame to user stack */
	error = copyout(&frame, fp, sizeof(frame));
	mutex_enter(p->p_lock);

	if (error != 0) {
		sigexit(l, SIGILL);
		/* NOTREACHED */
	}

	/*
	 * Set up the trapframe to call the signal handler:
	 *   r0 = signal number
	 *   r1 = pointer to siginfo on user stack
	 *   r2 = pointer to ucontext on user stack
	 *   PC = signal handler address
	 *   SP = signal frame on user stack
	 *   LR = signal trampoline (calls sigreturn)
	 */
	tf->tf_r[0] = sig;
	tf->tf_r[1] = (uint32_t)(uintptr_t)&fp->sf_si;
	tf->tf_r[2] = (uint32_t)(uintptr_t)&fp->sf_uc;
	tf->tf_pc = (uint32_t)(uintptr_t)catcher;
	tf->tf_r[13] = (uint32_t)(uintptr_t)fp;
	tf->tf_r[14] = (uint32_t)(uintptr_t)ps->sa_sigdesc[sig].sd_tramp;

	if (onstack)
		l->l_sigstk.ss_flags |= SS_ONSTACK;
}

/*
 * sys___sigreturn14: restore context after signal handler returns.
 */
int
sys___sigreturn14(struct lwp *l, const struct sys___sigreturn14_args *uap,
    register_t *retval)
{
	/* {
		syscallarg(struct ucontext *) sigcntxp;
	} */
	struct proc *p = l->l_proc;
	struct ucontext uc;
	int error;

	error = copyin(SCARG(uap, sigcntxp), &uc, sizeof(uc));
	if (error)
		return error;

	/* Restore the signal mask */
	mutex_enter(p->p_lock);
	error = setucontext(l, &uc);
	mutex_exit(p->p_lock);

	if (error)
		return error;

	return EJUSTRETURN;
}
