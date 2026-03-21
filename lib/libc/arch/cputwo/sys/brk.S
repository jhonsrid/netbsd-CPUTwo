/*	$NetBSD$	*/

/*
 * int brk(void *addr)
 *
 * Set the break (end of data segment) to addr.
 * Clamp to __minbrk.
 */

#include "SYS.h"

	.globl	_C_LABEL(__minbrk)
	.globl	_C_LABEL(__curbrk)
	.globl	_C_LABEL(_end)

#ifdef WEAK_ALIAS
WEAK_ALIAS(brk, _brk)
#endif

	.data
_C_LABEL(__minbrk):
	.word	_C_LABEL(_end)
	.text

ENTRY(_brk)
	/* r0 = addr (C arg0) */
	/* Clamp: if addr < __minbrk, use __minbrk */
	lw	r12, _C_LABEL(__minbrk)
	cmp	r0, r12
	bge	1f
	mov	r0, r12			/* addr = __minbrk */
1:
	/* Save addr in r4 across syscall */
	subi	sp, sp, 8
	sw	lr,  [sp, #4]
	sw	r4,  [sp, #0]
	mov	r4, r0

	/* break(addr): kernel arg0 = addr in r1 */
	mov	r1, r0
	movi	r0, SYS_break
	syscall

	cmpi	r1, 0
	bne	2f

	/* Success: update __curbrk */
	sw	r4, _C_LABEL(__curbrk)
	movi	r0, 0

	lw	r4,  [sp, #0]
	lw	lr,  [sp, #4]
	addi	sp, sp, 8
	mov	pc, lr

2:	/* Error */
	lw	r4,  [sp, #0]
	lw	lr,  [sp, #4]
	addi	sp, sp, 8
	b	_C_LABEL(__cerror)
END(_brk)
