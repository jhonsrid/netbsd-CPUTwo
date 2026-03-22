/*	$NetBSD$	*/

/*
 * int ptrace(int request, pid_t pid, void *addr, int data)
 *
 * ptrace must clear errno before the syscall because a successful
 * ptrace can return -1 (e.g., reading a word that happens to be -1).
 */

#include "SYS.h"

ENTRY(ptrace)
	/* Clear errno first */
	subi	sp, sp, 8
	sw	lr, [sp + 4]
	sw	r4, [sp]
	/* Save args: r0-r3 are caller-saved but we need them after __errno */
	/* Actually, just clear errno via __errno() then redo the syscall.
	 * Simpler: store 0 to *__errno() */
	/* Save all 4 args on stack */
	subi	sp, sp, 16
	sw	r0, [sp]
	sw	r1, [sp + 4]
	sw	r2, [sp + 8]
	sw	r3, [sp + 12]

	jmp	lr, _C_LABEL(__errno)	/* r0 = &errno */
	movi	r12, 0
	sw	r12, [r0]		/* *__errno() = 0 */

	/* Restore args */
	lw	r0, [sp]
	lw	r1, [sp + 4]
	lw	r2, [sp + 8]
	lw	r3, [sp + 12]
	addi	sp, sp, 16

	lw	r4, [sp]
	lw	lr, [sp + 4]
	addi	sp, sp, 8

	/* Now do the syscall: ptrace(request, pid, addr, data)
	 * Kernel ABI: r0=sysnum, r1=request, r2=pid, r3=addr, stack=data */
	mov	r3, r2		/* arg2 -> r3 */
	mov	r2, r1		/* arg1 -> r2 */
	mov	r1, r0		/* arg0 -> r1 */
	movi	r0, SYS_ptrace
	syscall
	cmpi	r1, 0
	bne	1f
	mov	pc, lr
1:
	ba	_C_LABEL(__cerror)
END(ptrace)
