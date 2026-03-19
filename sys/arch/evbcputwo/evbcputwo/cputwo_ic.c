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
 * CPUTwo interrupt controller driver.
 *
 * Simple 4-source interrupt controller at 0x03F02000:
 *   +0x00  Pending  (read-only)   Latched interrupt bits
 *   +0x04  Enable   (read/write)  1 = source enabled
 *   +0x08  Ack      (write-only)  Write 1 to clear pending bit
 *
 * Sources:
 *   bit 0 = Timer
 *   bit 1 = UART RX
 *   bit 2 = UART TX
 *   bit 3 = Block device
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD$");

#include <sys/param.h>
#include <sys/systm.h>

#define IC_BASE		0x03F02000
#define IC_PENDING	(*(volatile uint32_t *)(IC_BASE + 0x00))
#define IC_ENABLE	(*(volatile uint32_t *)(IC_BASE + 0x04))
#define IC_ACK		(*(volatile uint32_t *)(IC_BASE + 0x08))

#define IC_TIMER	0x01
#define IC_UART_RX	0x02
#define IC_UART_TX	0x04
#define IC_BLKDEV	0x08

/*
 * Initialize the interrupt controller.
 * Called from cputwo_init() or cpu_initclocks().
 * Enables timer and UART RX interrupts.
 */
void
cputwo_ic_init(void)
{

	IC_ENABLE = IC_TIMER | IC_UART_RX;
}

/*
 * Enable an interrupt source.
 */
void
cputwo_ic_enable(uint32_t mask)
{

	IC_ENABLE = IC_ENABLE | mask;
}

/*
 * Disable an interrupt source.
 */
void
cputwo_ic_disable(uint32_t mask)
{

	IC_ENABLE = IC_ENABLE & ~mask;
}

/*
 * Read pending interrupts (pending & enabled).
 */
uint32_t
cputwo_ic_pending(void)
{

	return IC_PENDING;
}

/*
 * Acknowledge (clear) interrupt sources.
 */
void
cputwo_ic_ack(uint32_t mask)
{

	IC_ACK = mask;
}
