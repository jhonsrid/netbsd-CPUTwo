/*	$NetBSD$	*/

#include <sys/types.h>
#include <ucontext.h>
#include <lwp.h>
#include <stdlib.h>

#include <machine/mcontext.h>

void
_lwp_makecontext(ucontext_t *ucp, void (*start)(void *),
    void *arg, void *private, caddr_t stk, size_t stksize)
{
	__greg_t *gr = ucp->uc_mcontext.__gregs;

	/* Set up the stack */
	uintptr_t sp = ((uintptr_t)stk + stksize) & ~7;

	gr[_REG_SP] = (__greg_t)sp;
	gr[_REG_PC] = (__greg_t)(uintptr_t)start;
	gr[_REG_R0] = (__greg_t)(uintptr_t)arg;

	/* When start returns, call _lwp_exit */
	gr[_REG_LR] = (__greg_t)(uintptr_t)_lwp_exit;
}
