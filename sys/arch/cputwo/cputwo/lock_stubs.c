/*
 * Copyright (c) 2024 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * CPUTwo mutex fast-path stubs.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD$");

#define __MUTEX_PRIVATE

#include <sys/param.h>
#include <sys/mutex.h>
#include <sys/lwp.h>
#include <sys/cpu.h>
#include <sys/atomic.h>

#include <machine/lock.h>
#include <machine/intr.h>

/*
 * mutex_enter: fast path for adaptive mutexes.
 *
 * Try CAS owner from 0 to curlwp.  For spin mutexes, owner has
 * MUTEX_BIT_SPIN set so CAS fails and we go to the vector path.
 */
void
mutex_enter(kmutex_t *mtx)
{
	uintptr_t curthread = (uintptr_t)curlwp;

	if (__predict_true(
	    atomic_cas_ulong((volatile unsigned long *)&mtx->mtx_owner,
	    0, curthread) == 0))
		return;

	mutex_vector_enter(mtx);
}

/*
 * mutex_exit: fast path for adaptive mutexes.
 */
void
mutex_exit(kmutex_t *mtx)
{
	uintptr_t curthread = (uintptr_t)curlwp;

	if (__predict_true(
	    atomic_cas_ulong((volatile unsigned long *)&mtx->mtx_owner,
	    curthread, 0) == curthread))
		return;

	mutex_vector_exit(mtx);
}

/* mutex_spin_enter/exit provided by kern_mutex.c (no __HAVE_SPIN_MUTEX_STUBS) */
