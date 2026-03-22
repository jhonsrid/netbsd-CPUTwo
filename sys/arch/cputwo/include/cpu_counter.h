/* $NetBSD$ */

#ifndef _CPUTWO_CPU_COUNTER_H_
#define _CPUTWO_CPU_COUNTER_H_

/*
 * CPUTwo specific support for CPU counter.
 */

#ifdef _KERNEL

#include <sys/cpu.h>

#define cpu_hascounter()	(curcpu()->ci_data.cpu_cc_freq != 0)
#define cpu_counter()		cpu_counter32()

/*
 * Read the timer's remaining count as a cycle counter.
 * The timer counts down from PERIOD, so we invert to get an
 * upward-counting value.  Before the timer is started, reads 0.
 */
#define CPUTWO_TIMER_COUNT	(*(volatile uint32_t *)0x03F01000)

static __inline uint32_t
cpu_counter32(void)
{
	return 0xFFFFFFFFu - CPUTWO_TIMER_COUNT;
}

static __inline uint64_t
cpu_frequency(struct cpu_info *ci)
{
	return ci->ci_data.cpu_cc_freq;
}

#endif /* _KERNEL */

#endif /* _CPUTWO_CPU_COUNTER_H_ */
