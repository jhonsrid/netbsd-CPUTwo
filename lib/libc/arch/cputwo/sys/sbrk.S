/*	$NetBSD$	*/

/*
 * void *sbrk(intptr_t incr)
 *
 * Increase the break by incr bytes, return old break.
 */

#include "SYS.h"

	.globl	_C_LABEL(__curbrk)
	.globl	_C_LABEL(_end)

#ifdef WEAK_ALIAS
WEAK_ALIAS(sbrk, _sbrk)
#endif

	.data
_C_LABEL(__curbrk):
	.word	_C_LABEL(_end)
	.text

ENTRY(_sbrk)
	/* r0 = incr */
	subi	sp, sp, 12
	sw	lr,  [sp, #8]
	sw	r4,  [sp, #4]
	sw	r5,  [sp, #0]

	lw	r4, _C_LABEL(__curbrk)	/* r4 = old curbrk */
	add	r5, r0, r4		/* r5 = new break = curbrk + incr */

	/* break(new_break): kernel arg0 = r5 -> r1 */
	mov	r1, r5
	movi	r0, SYS_break
	syscall

	cmpi	r1, 0
	bne	1f

	/* Success: update __curbrk, return old break */
	sw	r5, _C_LABEL(__curbrk)
	mov	r0, r4			/* return old break */

	lw	r5,  [sp, #0]
	lw	r4,  [sp, #4]
	lw	lr,  [sp, #8]
	addi	sp, sp, 12
	mov	pc, lr

1:	/* Error */
	lw	r5,  [sp, #0]
	lw	r4,  [sp, #4]
	lw	lr,  [sp, #8]
	addi	sp, sp, 12
	b	_C_LABEL(__cerror)
END(_sbrk)
