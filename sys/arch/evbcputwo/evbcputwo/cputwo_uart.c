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
 * CPUTwo UART driver.
 *
 * Simple UART at 0x03F00000:
 *   +0x00  Status   (read)   bit 0 = TX ready, bit 1 = RX available
 *   +0x04  TX data  (write)  Low 8 bits transmitted
 *   +0x08  RX data  (read)   Low 8 bits received (0 if none)
 *   +0x0C  Control  (r/w)    bit 0 = RX IRQ enable, bit 1 = TX IRQ enable
 *
 * Interrupts:
 *   IC bit 1 = UART RX (level: asserted while RX data available)
 *   IC bit 2 = UART TX (edge: fires on busy->ready transition)
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/tty.h>
#include <sys/conf.h>

#define UART_BASE	0x03F00000
#define UART_STATUS	(*(volatile uint32_t *)(UART_BASE + 0x00))
#define UART_TX		(*(volatile uint32_t *)(UART_BASE + 0x04))
#define UART_RX		(*(volatile uint32_t *)(UART_BASE + 0x08))
#define UART_CTRL	(*(volatile uint32_t *)(UART_BASE + 0x0C))

#define UART_STATUS_TX_READY	0x01
#define UART_STATUS_RX_AVAIL	0x02
#define UART_CTRL_RX_IRQ	0x01
#define UART_CTRL_TX_IRQ	0x02

/* forward declarations */
void cputwo_uart_init(void);
void cputwo_uart_putc(int);
int cputwo_uart_getc(void);
void cputwo_uart_intr(void);

/*
 * Initialize the UART hardware.
 * Enable RX interrupts; TX is polled for now.
 */
void
cputwo_uart_init(void)
{

	UART_CTRL = UART_CTRL_RX_IRQ;
}

/*
 * Polled put character — spin until TX is ready.
 * Used by the early console (consinit) and by panic output.
 */
void
cputwo_uart_putc(int c)
{

	while ((UART_STATUS & UART_STATUS_TX_READY) == 0)
		;
	UART_TX = (uint32_t)(unsigned char)c;
}

/*
 * Polled get character — return -1 if nothing available.
 */
int
cputwo_uart_getc(void)
{

	if (UART_STATUS & UART_STATUS_RX_AVAIL)
		return (int)(UART_RX & 0xFF);
	return -1;
}

/*
 * UART interrupt handler.
 * Called from the trap handler when IC bits 1 or 2 are pending.
 * Drains the RX FIFO and feeds characters to the console layer.
 */
void
cputwo_uart_intr(void)
{
	int c;

	/* Drain received characters */
	while ((c = cputwo_uart_getc()) != -1) {
		/* TODO: feed to tty layer via cn_check_magic / ttyinput */
#ifdef DDB
		extern int db_active;
		if (db_active) {
			/* DDB is active — don't feed to console */
			continue;
		}
#endif
	}

	/* TX complete handling would go here if we used TX interrupts */
}
