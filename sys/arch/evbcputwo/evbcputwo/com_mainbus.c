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

/*
 * UART (com) attachment to mainbus for evbcputwo.
 *
 * The CPUTwo UART is at 0x03F00000:
 *   +0x00  Status  (bit0=TX ready, bit1=RX available)
 *   +0x04  TX data
 *   +0x08  RX data
 *   +0x0C  Control (bit0=RX IRQ enable, bit1=TX IRQ enable)
 *
 * The interrupt controller has UART RX on IRQ 1 and UART TX on IRQ 2.
 *
 * For initial bring-up, this is a minimal attachment.  The early polled
 * console in machdep.c (consinit) handles I/O until this driver is
 * extended with full interrupt-driven support.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>

#include <sys/bus.h>
#include <machine/intr.h>

#define CPUTWO_UART_BASE	0x03F00000

/*
 * Import the mainbus_attach_args definition.  This struct is defined
 * in mainbus.c; we duplicate just the layout here to avoid a shared
 * header for the minimal bring-up case.
 */
struct mainbus_attach_args {
	const char	*ma_name;
	bus_addr_t	 ma_addr;
	int		 ma_irq;
};

struct com_mainbus_softc {
	device_t	 sc_dev;
	bus_addr_t	 sc_addr;
	int		 sc_irq;
};

static int	com_mainbus_match(device_t, cfdata_t, void *);
static void	com_mainbus_attach(device_t, device_t, void *);

CFATTACH_DECL_NEW(com_mainbus, sizeof(struct com_mainbus_softc),
    com_mainbus_match, com_mainbus_attach, NULL, NULL);

static int
com_mainbus_match(device_t parent, cfdata_t cf, void *aux)
{
	struct mainbus_attach_args *ma = aux;

	if (strcmp(ma->ma_name, "com") != 0)
		return 0;

	if (ma->ma_addr != CPUTWO_UART_BASE)
		return 0;

	return 1;
}

static void
com_mainbus_attach(device_t parent, device_t self, void *aux)
{
	struct com_mainbus_softc *sc = device_private(self);
	struct mainbus_attach_args *ma = aux;

	sc->sc_dev  = self;
	sc->sc_addr = ma->ma_addr;
	sc->sc_irq  = ma->ma_irq;

	aprint_naive("\n");
	aprint_normal(": CPUTwo UART at 0x%lx irq %d\n",
	    (unsigned long)sc->sc_addr, sc->sc_irq);

	/*
	 * The early polled console (consinit in machdep.c) is already
	 * active.  For now, this attachment simply claims the device
	 * in the autoconf tree.  Full interrupt-driven operation will
	 * be added when bus_space is wired up.
	 */
}
