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
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _CPUTWO_DB_MACHDEP_H_
#define _CPUTWO_DB_MACHDEP_H_

#include <cputwo/frame.h>

typedef long int db_expr_t;
#define DDB_EXPR_FMT "lx"
typedef uintptr_t db_addr_t;
#define DDB_ADDR_FMT PRIxPTR

#define BKPT_ADDR(addr)		(addr)
#define BKPT_SIZE		4
/*
 * Use HALT instruction (opcode 0x12, encoding 0x12000000) as breakpoint.
 */
#define BKPT_INSN		0x12000000
#define	BKPT_SET(insn, addr)	(BKPT_INSN)

typedef struct trapframe db_regs_t;
extern db_regs_t ddb_regs;
#define DDB_REGS		(&ddb_regs)
#define PC_REGS(tf)		((tf)->tf_pc)

#define	DB_TRAP_UNKNOWN		0
#define	DB_TRAP_BREAKPOINT	1
#define	DB_TRAP_BKPT_INSN	2
#define	DB_TRAP_WATCHPOINT	3

#define IS_BREAKPOINT_TRAP(type, code) \
	((type) == DB_TRAP_BREAKPOINT || (type) == DB_TRAP_BKPT_INSN)
#define IS_WATCHPOINT_TRAP(type, code) \
	((type) == DB_TRAP_WATCHPOINT)

/*
 * CPUTwo instruction classification for DDB.
 *
 * Instruction encoding: opcode in bits [31:24], rd in [23:20], rs1 in [19:16].
 *
 * Opcodes:
 *   JMP  = 0x0E (rd=lr for call: jmp lr, target)
 *   CALLR= 0x2A (rd=lr for indirect call: callr lr, rs1)
 *   MOV  = 0x27 (rd=pc, rs1=lr for return: mov pc, lr)
 *   LW   = 0x0B, LH=0x1C, LHU=0x1D, LB=0x1E, LBU=0x1F
 *   SW   = 0x0C, SH=0x20, SB=0x21
 *   SYSRET=0x11, KRET=0x3F
 */
static __inline bool
inst_call(db_expr_t insn)
{
	unsigned int op = (insn >> 24) & 0xFF;
	unsigned int rd = (insn >> 20) & 0xF;
	/* JMP with rd=lr (r14), or CALLR with rd=lr */
	return (op == 0x0E && rd == 14) || (op == 0x2A && rd == 14);
}

static __inline bool
inst_load(db_expr_t insn)
{
	unsigned int op = (insn >> 24) & 0xFF;
	/* LW=0x0B, LH=0x1C, LHU=0x1D, LB=0x1E, LBU=0x1F */
	return op == 0x0B || (op >= 0x1C && op <= 0x1F);
}

static __inline bool
inst_return(db_expr_t insn)
{
	/* mov pc, lr = MOV(0x27) rd=r15(0xF) rs1=r14(0xE) = 0x27FE0000 */
	return insn == 0x27FE0000;
}

static __inline bool
inst_store(db_expr_t insn)
{
	unsigned int op = (insn >> 24) & 0xFF;
	/* SW=0x0C, SH=0x20, SB=0x21 */
	return op == 0x0C || op == 0x20 || op == 0x21;
}

static __inline bool
inst_trap_return(db_expr_t insn)
{
	/* SYSRET=0x11000000, KRET=0x3F000000 */
	return insn == 0x11000000 || insn == 0x3F000000;
}

#endif /* _CPUTWO_DB_MACHDEP_H_ */
