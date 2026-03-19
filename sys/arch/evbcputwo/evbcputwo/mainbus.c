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
 * Mainbus driver for evbcputwo.
 * Enumerates known on-board devices via a static device table.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>

#include "locators.h"

/*
 * Attach arguments passed from mainbus to child devices.
 */
struct mainbus_attach_args {
	const char	*ma_name;	/* device name */
	bus_addr_t	 ma_addr;	/* device base address */
	int		 ma_irq;	/* interrupt number, or -1 */
};

/*
 * Static table of known on-board devices.
 */
static const struct {
	const char	*name;
	bus_addr_t	 addr;
	int		 irq;
} mainbus_devs[] = {
	{ "cpu",	0,		-1 },
	{ "com",	0x03F00000,	 1 },	/* UART, IRQ 1 (uart_rx) */
	{ NULL,		0,		 0 },
};

struct mainbus_softc {
	device_t sc_dev;
};

static int	mainbus_match(device_t, cfdata_t, void *);
static void	mainbus_attach(device_t, device_t, void *);
static int	mainbus_submatch(device_t, cfdata_t, const int *, void *);
static int	mainbus_print(void *, const char *);

CFATTACH_DECL_NEW(mainbus, sizeof(struct mainbus_softc),
    mainbus_match, mainbus_attach, NULL, NULL);

/* Prevent duplicate attachment. */
static bool mainbus_found;

static int
mainbus_match(device_t parent, cfdata_t cf, void *aux)
{

	if (mainbus_found)
		return 0;
	return 1;
}

static void
mainbus_attach(device_t parent, device_t self, void *aux)
{
	struct mainbus_softc *sc = device_private(self);
	struct mainbus_attach_args ma;
	int i;

	mainbus_found = true;
	sc->sc_dev = self;

	aprint_naive("\n");
	aprint_normal("\n");

	for (i = 0; mainbus_devs[i].name != NULL; i++) {
		ma.ma_name = mainbus_devs[i].name;
		ma.ma_addr = mainbus_devs[i].addr;
		ma.ma_irq  = mainbus_devs[i].irq;
		config_found(self, &ma, mainbus_print,
		    CFARGS(.submatch = mainbus_submatch));
	}
}

/*
 * Submatch: check the locator address against the device table entry.
 * If the config file specifies a particular address, it must match.
 */
static int
mainbus_submatch(device_t parent, cfdata_t cf, const int *ldesc, void *aux)
{
	struct mainbus_attach_args *ma = aux;

	if (cf->cf_loc[MAINBUSCF_ADDR] != MAINBUSCF_ADDR_DEFAULT &&
	    cf->cf_loc[MAINBUSCF_ADDR] != (int)ma->ma_addr)
		return 0;

	return config_match(parent, cf, aux);
}

/*
 * Print function for unattached children.
 */
static int
mainbus_print(void *aux, const char *pnp)
{
	struct mainbus_attach_args *ma = aux;

	if (pnp != NULL)
		return QUIET;

	if (ma->ma_addr != (bus_addr_t)MAINBUSCF_ADDR_DEFAULT)
		aprint_normal(" addr 0x%lx", (unsigned long)ma->ma_addr);

	return UNCONF;
}
