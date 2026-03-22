/*	$NetBSD$	*/

/*
 * pid_t vfork(void)
 *
 * Same as fork, but the child borrows the parent's address space.
 */

#include "SYS.h"

ENTRY(__vfork14)
	movi	r0, SYS___vfork14
	syscall
	cmpi	r1, 0
	bne	1f
	mov	pc, lr
1:
	ba	_C_LABEL(__cerror)
END(__vfork14)
