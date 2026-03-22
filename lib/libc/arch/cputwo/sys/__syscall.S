/*	$NetBSD$	*/

/*
 * __syscall(quad_t num, ...) — indirect system call.
 *
 * The C ABI passes the syscall number as a 64-bit quad in r0:r1 (little-endian).
 * On CPUTwo (32-bit), we just use the low word (r0).
 * Arguments start at r2, r3, stack.
 *
 * For the kernel: r0=sysnum, r1=arg0, r2=arg1, r3=arg2, stack=arg3+.
 * We have: r0=sysnum_lo, r1=sysnum_hi, r2=arg0, r3=arg1, stack=arg2+.
 *
 * The kernel's SYS___syscall handler reads the real code from r1,
 * and args from r2, r3, stack.  So this just works as-is.
 */

#include "SYS.h"

ENTRY(__syscall)
	/* r0 = SYS___syscall (passed by caller), args already in position */
	mov	r3, r2
	mov	r2, r1
	mov	r1, r0
	movi	r0, SYS___syscall
	syscall
	cmpi	r1, 0
	bne	1f
	mov	pc, lr
1:
	ba	_C_LABEL(__cerror)
END(__syscall)

STRONG_ALIAS(_syscall,__syscall)
WEAK_ALIAS(syscall,__syscall)
