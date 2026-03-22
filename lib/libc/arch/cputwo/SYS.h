/*	$NetBSD$	*/

#ifndef _CPUTWO_SYS_H_
#define _CPUTWO_SYS_H_

/*
 * Copyright (c) 2024 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.
 */

/*
 * CPUTwo system call macros for libc.
 *
 * Syscall convention:
 *   r0 = syscall number
 *   r1, r2, r3 = first three arguments (remaining on stack)
 *   SYSCALL instruction (CAUSE=0x03, EPC=PC+4)
 *
 * Return convention:
 *   r0 = return value (or errno on error)
 *   r1 = 0 on success, 1 on error
 *
 * The userland calling convention has arguments in r0-r3 (C ABI), but
 * for syscalls the kernel shifts: syscall number in r0, args start at r1.
 * The SYSTRAP macro loads r0 and executes SYSCALL; the caller must place
 * arguments in r1-r3 beforehand (which the C ABI puts in r0-r2, so we
 * need a shift in the wrapper or rely on the kernel taking args from the
 * right registers based on the C call convention).
 *
 * Actually, the kernel reads:
 *   r0 = syscall number
 *   r1 = arg0, r2 = arg1, r3 = arg2, stack = arg3+
 *
 * The C ABI delivers:
 *   r0 = arg0, r1 = arg1, r2 = arg2, r3 = arg3
 *
 * So the PSEUDO macro needs to shift the arguments:
 *   move r3, r2    (shift arg2 -> r3)
 *   move r2, r1    (shift arg1 -> r2)
 *   move r1, r0    (shift arg0 -> r1)
 *   li   r0, SYS_x (load syscall number)
 *   syscall
 *
 * Note: for syscalls with >3 args, the 4th C argument (r3) must go to
 * the stack, which it already is if the caller follows the ABI and
 * spills r3.  The kernel reads additional args from the user stack at
 * sp+0, sp+4, ....  Since the C ABI puts arg4+ on the stack starting
 * at sp+16 (after the 4-word register save area), the kernel's copyin
 * from sp accounts for this.  Actually — reviewing syscall.c, the kernel
 * does copyin from tf->tf_r[13] (i.e., the user sp) directly, with
 * no offset for a register save area.  So the libc stubs may need to
 * push extra args at sp+0.  For most syscalls (<=3 args), this is fine.
 * For more args, the PSEUDO approach works because the C caller already
 * places arg4+ on the stack.  The tricky part is arg3 (originally in r3
 * per the C ABI) — after shifting, it needs to be at the user sp.
 *
 * For simplicity and correctness, we handle <=3 arg syscalls via the
 * PSEUDO macro (which covers the vast majority of syscalls).  Complex
 * multi-arg stubs (fork, pipe, brk, etc.) are written by hand.
 */

#include <sys/syscall.h>
#include <machine/asm.h>

/*
 * SYSTRAP(x) — load syscall number and execute SYSCALL.
 * Arguments must already be in r1-r3.
 */
#define SYSTRAP(x)							\
	movi	r0, SYS_ ## x;						\
	syscall

/*
 * PSEUDO(x, y) — standard syscall wrapper.
 *
 * Shift args: C calling convention puts args in r0-r3, but syscall
 * convention wants syscall number in r0 and args starting at r1.
 *
 * For syscalls with 4+ args, arg3 (in r3) must be pushed to the stack.
 * However, the C ABI guarantees the caller has set up a stack frame
 * with arg4+ at [sp+16], and the kernel copies from user sp.
 * We handle the common case here; 4+ arg syscalls need hand-written stubs.
 */
#define PSEUDO(x,y)							\
ENTRY(x);								\
	mov	r3, r2;		/* arg2 -> r3 */			\
	mov	r2, r1;		/* arg1 -> r2 */			\
	mov	r1, r0;		/* arg0 -> r1 */			\
	SYSTRAP(y);		/* r0 = SYS_y, syscall */		\
	cmpi	r1, 0;							\
	bne	.L ## x ## _err;					\
	mov	pc, lr;		/* return r0 */				\
.L ## x ## _err:							\
	ba	_C_LABEL(__cerror);					\
END(x)

#define PSEUDO_NOERROR(x,y)						\
ENTRY(x);								\
	mov	r3, r2;							\
	mov	r2, r1;							\
	mov	r1, r0;							\
	SYSTRAP(y);							\
	mov	pc, lr;							\
END(x)

/*
 * RSYSCALL(x) — simple syscall with same entrypoint and syscall name.
 */
#define RSYSCALL(x)		PSEUDO(x,x)
#define RSYSCALL_NOERROR(x)	PSEUDO_NOERROR(x,x)

/*
 * WSYSCALL(weak, strong) — syscall with weak alias.
 */
#define WSYSCALL(weak,strong)						\
	WEAK_ALIAS(weak,strong);					\
	PSEUDO(strong,weak)

#endif /* !_CPUTWO_SYS_H_ */
