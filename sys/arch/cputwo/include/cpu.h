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

#ifndef _CPUTWO_CPU_H_
#define _CPUTWO_CPU_H_

#if defined(_KERNEL) || defined(_KMEMUSER)

/*
 * Clock interrupt frame.
 * cf_sr bit 0 = supervisor mode (1=supervisor, 0=user).
 */
struct clockframe {
	uintptr_t cf_pc;
	uint32_t cf_sr;		/* STATUS register at time of interrupt */
	int cf_intr_depth;
};

/* User mode if supervisor bit is clear */
#define CLKF_USERMODE(cf)	(((cf)->cf_sr & 1) == 0)
#define CLKF_PC(cf)		((cf)->cf_pc)
#define CLKF_INTR(cf)		((cf)->cf_intr_depth > 0)

#include <sys/cpu_data.h>
#include <sys/device_if.h>
#include <sys/intr.h>

struct cpu_info {
	struct cpu_data ci_data;
	device_t ci_dev;
	struct lwp *ci_curlwp;
	struct lwp *ci_onproc;
	struct lwp *ci_softlwps[SOFTINT_COUNT];

	int ci_mtx_oldspl;
	int ci_mtx_count;

	int ci_want_resched;
	int ci_cpl;			/* current priority level */
	u_int ci_softints;
	volatile u_int ci_intr_depth;
};

/*
 * CPUTwo is single-core, no dedicated curlwp register.
 * We store the global cpu_info and access curlwp through it.
 */
extern struct cpu_info cpu_info_store;

static __inline struct cpu_info *
curcpu(void)
{
	return &cpu_info_store;
}

#define	curlwp		(curcpu()->ci_curlwp)

static __inline cpuid_t
cpu_number(void)
{
	return 0;
}

void	cpu_proc_fork(struct proc *, struct proc *);
void	cpu_signotify(struct lwp *);
void	cpu_need_proftick(struct lwp *l);
void	cpu_boot_secondary_processors(void);

/* Single-CPU iterator */
#define CPU_INFO_ITERATOR	cpuid_t
#define CPU_INFO_FOREACH(cii, ci) \
	(cii) = 0; (cii) == 0 && ((ci) = curcpu()); (cii)++

static __inline void
cpu_dosoftints(void)
{
	extern void dosoftints(void);
	struct cpu_info * const ci = curcpu();
	if (ci->ci_intr_depth == 0
	    && (ci->ci_data.cpu_softints >> ci->ci_cpl) > 0)
		dosoftints();
}

static __inline bool
cpu_intr_p(void)
{
	return curcpu()->ci_intr_depth > 0;
}

#endif /* _KERNEL || _KMEMUSER */

#endif /* _CPUTWO_CPU_H_ */
