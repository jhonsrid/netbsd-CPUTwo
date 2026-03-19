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

#include <dev/cons.h>

/*
 * Board-level initialization for evbcputwo.
 * Called from cputwo_start.S after stack setup.
 */
void
cputwo_init(void)
{

	consinit();

	/* TODO: set up memory regions, parse DTB, etc. */

	main();

	/* NOTREACHED */
	panic("main() returned");
}

/*
 * Early console via polled UART at 0x03F00000.
 */
#define UART_STATUS	(*(volatile uint32_t *)0x03F00000)
#define UART_TX		(*(volatile uint32_t *)0x03F00004)
#define UART_RX		(*(volatile uint32_t *)0x03F00008)
#define UART_TX_READY	0x01
#define UART_RX_AVAIL	0x02

static void
cputwo_cnputc(dev_t dev, int c)
{

	while ((UART_STATUS & UART_TX_READY) == 0)
		;
	UART_TX = c;
}

static int
cputwo_cngetc(dev_t dev)
{

	while ((UART_STATUS & UART_RX_AVAIL) == 0)
		;
	return UART_RX;
}

static struct consdev cputwo_consdev = {
	.cn_getc = cputwo_cngetc,
	.cn_putc = cputwo_cnputc,
	.cn_pollc = nullcnpollc,
	.cn_dev = NODEV,
	.cn_pri = CN_NORMAL,
};

void
consinit(void)
{

	cn_tab = &cputwo_consdev;
}
