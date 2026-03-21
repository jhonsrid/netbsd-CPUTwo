/*	$NetBSD$	*/

/*
 * int getcontext(ucontext_t *ucp)
 *
 * After the kernel fills in the ucontext, patch the return value
 * register to 0 and the PC to our return address, so that when
 * this context is later restored via setcontext, it appears as
 * if getcontext returned 0 again.
 */

#include "SYS.h"
#include "assym.h"

#ifdef WEAK_ALIAS
WEAK_ALIAS(getcontext, _getcontext)
#endif

ENTRY(_getcontext)
	/* Save ucp pointer and lr across syscall */
	subi	sp, sp, 8
	sw	lr,  [sp, #4]
	sw	r4,  [sp, #0]
	mov	r4, r0			/* r4 = ucp */

	/* Kernel ABI: r0=sysnum, r1=ucp */
	mov	r1, r0
	movi	r0, SYS_getcontext
	syscall

	cmpi	r1, 0
	bne	1f

	/* Patch saved context so restoring it returns 0 from getcontext */
	movi	r12, 0
	sw	r12, [r4, #_UC_GREGS_R0]	/* return value = 0 */
	lw	r12, [sp, #4]			/* original lr */
	sw	r12, [r4, #_UC_GREGS_PC]	/* resume at caller */

	movi	r0, 0
	lw	r4,  [sp, #0]
	lw	lr,  [sp, #4]
	addi	sp, sp, 8
	mov	pc, lr

1:
	lw	r4,  [sp, #0]
	lw	lr,  [sp, #4]
	addi	sp, sp, 8
	b	_C_LABEL(__cerror)
END(_getcontext)
