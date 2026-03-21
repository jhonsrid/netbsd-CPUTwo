/*	$NetBSD$	*/

/*
 * pid_t fork(void)
 *
 * Kernel returns:
 *   r0 = child PID (in both parent and child)
 *   r1 = 0 in parent, 1 in child (on success); 1 on error
 *
 * Wait — on CPUTwo, r1=0 means success and r1=1 means error.
 * For fork specifically, the kernel also needs to distinguish parent
 * from child.  The kernel's syscall.c returns rval[0]=child_pid and
 * rval[1]=0 for parent, rval[0]=parent_pid and rval[1]=1 for child.
 * But our ABI uses r1 for error flag...
 *
 * Looking at the kernel syscall.c more carefully: on success (error==0),
 * tf_r[0] = rval[0] and tf_r[1] = 0.  The fork() syscall implementation
 * returns different rval[0] for parent and child.  The child gets 0
 * (rval[0]=0) and the parent gets the child pid (rval[0]=child_pid).
 *
 * So from userland: on success, r0=pid (0 in child, >0 in parent), r1=0.
 * On error: r0=errno, r1=1.
 */

#include "SYS.h"

ENTRY(__fork)
	movi	r0, SYS_fork
	syscall
	cmpi	r1, 0
	bne	1f			/* error */
	mov	pc, lr			/* return r0 = pid */
1:
	b	_C_LABEL(__cerror)
END(__fork)
