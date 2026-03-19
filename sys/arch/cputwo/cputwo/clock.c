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

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/device.h>

#include <machine/cpu.h>

/*
 * CPUTwo Timer (MMIO at 0x03F01000, VA 0x83F01000 when MMU is on).
 *
 * +0x00  PERIOD    Write countdown value; timer auto-reloads on expiry.
 * +0x04  CTRL      bit 0 = enable, bit 1 = IRQ enable.
 *
 * Timer fires an interrupt when countdown reaches 0, then reloads PERIOD.
 * A period of 100000 gives approximately 100 Hz at the emulator default rate.
 */
#define	TIMER_BASE	0x03F01000
#define	TIMER_PERIOD	(*(volatile uint32_t *)(TIMER_BASE + 0x00))
#define	TIMER_CTRL	(*(volatile uint32_t *)(TIMER_BASE + 0x04))

#define	TIMER_CTRL_ENABLE	0x01
#define	TIMER_CTRL_IRQ_ENABLE	0x02

/*
 * CPUTwo Interrupt Controller (MMIO at 0x03F02000).
 *
 * +0x00  PENDING   Read: bitmask of pending interrupts.
 * +0x04  ENABLE    Read/Write: bitmask of enabled interrupts.
 * +0x08  ACK       Write: bitmask of interrupts to acknowledge (clear pending).
 *
 * Interrupt bits:
 *   bit 0 = timer
 *   bit 1 = UART RX
 *   bit 2 = UART TX
 *   bit 3 = block device
 */
#define	IC_BASE		0x03F02000
#define	IC_PENDING	(*(volatile uint32_t *)(IC_BASE + 0x00))
#define	IC_ENABLE	(*(volatile uint32_t *)(IC_BASE + 0x04))
#define	IC_ACK		(*(volatile uint32_t *)(IC_BASE + 0x08))

#define	IC_TIMER	0x01
#define	IC_UART_RX	0x02
#define	IC_UART_TX	0x04
#define	IC_BLKDEV	0x08

/*
 * Timer period for ~100 Hz at emulator default clock rate.
 */
#define	TIMER_HZ_PERIOD	100000

/*
 * cpu_initclocks: start the system clock.
 *
 * Called once during boot from main() -> initclocks().
 * Programs the hardware timer for hz (typically 100) ticks per second
 * and enables the timer interrupt in the interrupt controller.
 */
void
cpu_initclocks(void)
{

	/* Stop timer while configuring */
	TIMER_CTRL = 0;

	/* Set countdown period for ~100 Hz */
	TIMER_PERIOD = TIMER_HZ_PERIOD;

	/* Enable timer interrupt in the interrupt controller */
	IC_ENABLE = IC_ENABLE | IC_TIMER;

	/* Start the timer with IRQ generation enabled */
	TIMER_CTRL = TIMER_CTRL_ENABLE | TIMER_CTRL_IRQ_ENABLE;
}

/*
 * cputwo_clockintr: handle timer interrupt.
 *
 * Called from the trap handler (trap.c) when a timer interrupt is detected.
 * Acknowledges the interrupt in the IC and calls hardclock() to drive
 * the kernel clock, scheduling, timeout callouts, etc.
 */
void
cputwo_clockintr(struct clockframe *cf)
{

	/* Acknowledge the timer interrupt in the IC */
	IC_ACK = IC_TIMER;

	/* Drive the kernel clock */
	hardclock(cf);
}

/*
 * setstatclockrate: set the rate of the statistics clock.
 *
 * CPUTwo has no separate statistics clock hardware; the single timer
 * serves as both hardclock and stathz source.  Nothing to do here.
 */
void
setstatclockrate(int rate)
{

	/* no separate statistics clock on CPUTwo */
}

/*
 * delay: busy-wait for at least `us` microseconds.
 *
 * This is a rough calibration loop suitable for short delays during
 * driver initialization.  The loop count is approximate and depends
 * on the emulator's execution speed.
 */
void
delay(unsigned int us)
{
	volatile unsigned int i;

	/*
	 * Approximate busy loop.  Each iteration is a few instructions;
	 * factor of 10 is a rough estimate for the emulator clock rate.
	 * This does not need to be precise -- it is used for short
	 * hardware-settling delays.
	 */
	for (i = us * 10; i > 0; i--)
		;
}
