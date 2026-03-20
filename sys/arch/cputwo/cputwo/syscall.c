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
 * CPUTwo system call interface.
 *
 * Calling convention:
 *   r0 = syscall number
 *   r1, r2, r3 = first three arguments
 *   additional arguments on user stack (at sp+0, sp+4, ...)
 *
 * Return convention:
 *   r0 = return value (or errno on error)
 *   r1 = 0 on success, 1 on error
 *
 * On SYSCALL instruction:
 *   Hardware sets EPC = PC+4 (address after SYSCALL), CAUSE = 0x03.
 *   The trapframe's tf_pc already contains the return address.
 *
 * For ERESTART, we back up tf_pc by 4 to re-execute the SYSCALL.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/lwp.h>
#include <sys/signal.h>
#include <sys/syscall.h>
#include <sys/syscallvar.h>
#include <sys/syscall_stats.h>

#include <uvm/uvm_extern.h>

#include <machine/cpu.h>
#include <machine/frame.h>
#include <machine/pcb.h>
#include <machine/userret.h>

static void cputwo_syscall(struct lwp *, struct trapframe *);

/*
 * syscall_intern: called once per process to set up the syscall handler.
 * Required by __HAVE_SYSCALL_INTERN in types.h.
 */
void
syscall_intern(struct proc *p)
{

	p->p_md.md_syscall = cputwo_syscall;
}

/*
 * cputwo_syscall: handle a system call from userland.
 */
static void
cputwo_syscall(struct lwp *l, struct trapframe *tf)
{
	struct proc *p = l->l_proc;
	const struct sysent *callp;
	int error, nargs;
	size_t argsize;
	register_t code, ocode, args[8], rval[2];
	uint32_t opc;

	curcpu()->ci_data.cpu_nsyscall++;

	/*
	 * Save the PC for ERESTART.  tf_pc is already PC+4 (after SYSCALL).
	 * To re-execute, we set tf_pc = opc - 4.
	 */
	opc = tf->tf_pc;

	/*
	 * Syscall number in r0.
	 */
	ocode = code = tf->tf_r[0];

	callp = p->p_emul->e_sysent;

	switch (code) {
	case SYS_syscall:
		/*
		 * Indirect syscall: real code is first argument (r1).
		 */
		code = tf->tf_r[1];
		break;
	case SYS___syscall:
		/*
		 * Like syscall but code is a quad.  On 32-bit, take
		 * the low word (r1).
		 */
		code = tf->tf_r[1];
		break;
	default:
		break;
	}

	if (code < 0 || code >= p->p_emul->e_nsysent)
		callp += p->p_emul->e_nosys;
	else
		callp += code;

	argsize = callp->sy_argsize;
	nargs = callp->sy_narg;

	/*
	 * Collect arguments.
	 *
	 * Normal syscall: args in r1, r2, r3, then user stack.
	 * SYS_syscall:    args in r2, r3, then user stack.
	 * SYS___syscall:  args in r2, r3, then user stack.
	 */
	error = 0;

	if (ocode == SYS_syscall || ocode == SYS___syscall) {
		/* First arg was consumed as the real syscall number */
		if (argsize) {
			args[0] = tf->tf_r[2];
			args[1] = tf->tf_r[3];
			if (argsize > 2 * sizeof(register_t)) {
				error = copyin(
				    (void *)(uintptr_t)tf->tf_r[13],
				    &args[2],
				    argsize - 2 * sizeof(register_t));
			}
		}
	} else {
		if (argsize) {
			args[0] = tf->tf_r[1];
			args[1] = tf->tf_r[2];
			args[2] = tf->tf_r[3];
			if (argsize > 3 * sizeof(register_t)) {
				error = copyin(
				    (void *)(uintptr_t)tf->tf_r[13],
				    &args[3],
				    argsize - 3 * sizeof(register_t));
			}
		}
	}

	if (error)
		goto bad;

	rval[0] = 0;
	rval[1] = 0;

	error = sy_invoke(callp, l, args, rval, code);

	switch (error) {
	case 0:
		/* Success: return value in r0, error flag = 0 in r1 */
		tf->tf_r[0] = rval[0];
		tf->tf_r[1] = 0;
		break;

	case ERESTART:
		/*
		 * Back up PC to re-execute the SYSCALL instruction.
		 * SYSCALL is 4 bytes, and EPC was set to PC+4.
		 */
		tf->tf_pc = opc - 4;
		break;

	case EJUSTRETURN:
		/* Nothing to do — trapframe already set up */
		break;

	default:
	bad:
		/* Error: errno in r0, error flag = 1 in r1 */
		if (p->p_emul->e_errno)
			error = p->p_emul->e_errno[error];
		tf->tf_r[0] = error;
		tf->tf_r[1] = 1;
		break;
	}

	userret(l);
}
