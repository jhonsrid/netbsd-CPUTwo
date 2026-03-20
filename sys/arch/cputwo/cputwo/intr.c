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
 * CPUTwo interrupt priority level (IPL) management.
 *
 * IPL is implemented in software.  The single IC enable register is
 * not directly manipulated per-IPL; instead, interrupt handlers check
 * the current IPL and defer sources that are masked.
 *
 * IC source → IPL mapping:
 *   Timer (bit 0)     → IPL_SCHED (6)
 *   UART RX (bit 1)   → IPL_VM (5)
 *   UART TX (bit 2)   → IPL_VM (5)
 *   Block dev (bit 3)  → IPL_VM (5)
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/cpu.h>
#include <sys/intr.h>

#include <machine/cpu.h>
#include <machine/intr.h>

/* forward declarations */
void dosoftints(void);
uint32_t cputwo_intr_allowed(uint32_t);

/*
 * Supervisor STATUS register (memory-mapped).
 * Bit 1 = IE (interrupt enable).
 */
#define CPUTWO_STATUS	(*(volatile uint32_t *)0x03FFF010)

static inline uint32_t
disable_interrupts(void)
{
	uint32_t old = CPUTWO_STATUS;
	CPUTWO_STATUS = old & ~0x2;
	return old;
}

static inline void
restore_interrupts(uint32_t saved)
{
	CPUTWO_STATUS = saved;
}

/*
 * IPL for each IC source.
 */
static const int ic_source_ipl[4] = {
	[0] = IPL_SCHED,	/* timer */
	[1] = IPL_VM,		/* UART RX */
	[2] = IPL_VM,		/* UART TX */
	[3] = IPL_VM,		/* block device */
};

/* ------------------------------------------------------------------ */
/*  SPL functions                                                      */
/* ------------------------------------------------------------------ */

int
_splraise(int ipl)
{
	struct cpu_info *ci = curcpu();
	uint32_t s = disable_interrupts();
	int oldipl = ci->ci_cpl;

	if (ipl > ci->ci_cpl)
		ci->ci_cpl = ipl;
	restore_interrupts(s);
	return oldipl;
}

int
_spllower(int ipl)
{
	struct cpu_info *ci = curcpu();
	uint32_t s = disable_interrupts();
	int oldipl = ci->ci_cpl;

	ci->ci_cpl = ipl;
	restore_interrupts(s);

	if (ci->ci_intr_depth == 0)
		dosoftints();
	return oldipl;
}

void
splx(int savedipl)
{
	struct cpu_info *ci = curcpu();
	uint32_t s = disable_interrupts();

	ci->ci_cpl = savedipl;
	restore_interrupts(s);

	if (ci->ci_intr_depth == 0)
		dosoftints();
}

/* ------------------------------------------------------------------ */
/*  Hardware interrupt IPL filtering                                   */
/* ------------------------------------------------------------------ */

/*
 * Filter pending IC sources against current IPL.
 * Returns bitmask of sources allowed at the current IPL.
 * Sources at or below ci_cpl are left pending in the IC hardware
 * and will be dispatched when a subsequent interrupt re-reads IC_PENDING.
 */
uint32_t
cputwo_intr_allowed(uint32_t pending)
{
	struct cpu_info *ci = curcpu();
	int cpl = ci->ci_cpl;
	uint32_t allowed = 0;
	int i;

	for (i = 0; i < 4; i++) {
		if (!(pending & (1u << i)))
			continue;
		if (ic_source_ipl[i] > cpl)
			allowed |= (1u << i);
	}
	return allowed;
}

/* ------------------------------------------------------------------ */
/*  Soft interrupt dispatch                                            */
/* ------------------------------------------------------------------ */

/*
 * dosoftints: called when IPL is lowered.
 *
 * Without __HAVE_FAST_SOFTINTS, the MI kernel handles soft interrupts
 * via dedicated LWPs that are scheduled by softint_schedule().
 * Our job here is just to request a reschedule if softints are pending,
 * so the scheduler picks up the softint LWPs.
 */
void
dosoftints(void)
{
	struct cpu_info *ci = curcpu();

	if (ci->ci_data.cpu_softints >> ci->ci_cpl)
		ci->ci_want_resched = 1;
}
