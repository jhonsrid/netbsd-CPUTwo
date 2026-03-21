/*	$NetBSD$	*/

/*
 * int pipe(int fildes[2])
 *
 * Kernel returns two file descriptors in rval[0] and rval[1],
 * which map to r0 and... well, on CPUTwo, r1 is the error flag.
 *
 * Looking at the NetBSD pipe syscall: it returns fd[0] in rval[0]
 * and fd[1] in rval[1].  But our ABI puts 0 in r1 on success.
 *
 * The kernel syscall.c sets tf_r[0] = rval[0] and tf_r[1] = 0 on success.
 * So rval[1] (the second fd) is lost in the standard return path.
 *
 * This means pipe() needs special kernel support — the kernel must
 * store both fds somewhere the userland stub can retrieve them.
 * On MIPS, v0=fd0 and v1=fd1.  We need a similar mechanism.
 *
 * For now: use the SYS_pipe2 syscall which takes a fildes[2] pointer,
 * avoiding the two-return-value problem entirely.
 */

#include "SYS.h"

#ifdef WEAK_ALIAS
WEAK_ALIAS(pipe, _pipe)
#endif

ENTRY(_pipe)
	/* pipe2(fildes, 0) — pipe with no flags */
	mov	r2, r0			/* r2 = fildes (will become r2 in kernel = arg1) */
	movi	r1, 0			/* r1 = flags = 0 (will become r1 in kernel = arg0... */

	/* Actually, the kernel reads args starting at r1 for normal syscalls.
	 * pipe2(fildes, flags): arg0=fildes, arg1=flags
	 * Kernel sees: r1=arg0=fildes, r2=arg1=flags
	 * So: r1=fildes(r0), r2=flags(0) */
	mov	r1, r0			/* r1 = fildes (kernel arg0) */
	movi	r2, 0			/* r2 = flags (kernel arg1) */
	movi	r0, SYS_pipe2
	syscall
	cmpi	r1, 0
	bne	1f
	movi	r0, 0
	mov	pc, lr
1:
	b	_C_LABEL(__cerror)
END(_pipe)
