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
 * CPUTwo block device driver — ld(4) backend.
 *
 * Simple synchronous block device at 0x03F03000:
 *   +0x00  Sector   (r/w)   Sector number (LBA)
 *   +0x04  Buffer   (r/w)   Physical address of 512-byte buffer (must be aligned)
 *   +0x08  Command  (write) 1 = read sector, 2 = write sector
 *   +0x0C  Status   (read)  0 = idle, 1 = busy, 2 = error
 *   +0x10  Control  (r/w)   bit 0 = IRQ enable
 *   +0x14  Size     (read)  Total sector count
 *
 * Commands complete synchronously in the emulator (instantly from
 * the guest's perspective).
 *
 * The device expects a 512-byte-aligned physical address in the Buffer
 * register.  Since bp->b_data may be a kernel virtual address that is
 * not 512-byte aligned, we use a static bounce buffer.
 *
 * Attaches as ld0 at mainbus, providing a standard disk device.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/device.h>
#include <sys/buf.h>
#include <sys/bufq.h>
#include <sys/disk.h>
#include <sys/bus.h>

#include <dev/ldvar.h>

/*
 * Hardware register offsets.
 */
#define BLK_REG_SECTOR		0x00
#define BLK_REG_BUFFER		0x04
#define BLK_REG_COMMAND		0x08
#define BLK_REG_STATUS		0x0C
#define BLK_REG_CONTROL		0x10
#define BLK_REG_SIZE		0x14

#define BLK_CMD_READ		1
#define BLK_CMD_WRITE		2

#define BLK_STAT_IDLE		0
#define BLK_STAT_BUSY		1
#define BLK_STAT_ERROR		2

#define BLK_SECTOR_SIZE		512

/*
 * Kernel direct-map: VA = PA + 0x80000000.
 * MMIO region (>= 0x03F00000) bypasses the MMU entirely, so
 * volatile accesses to the physical address work in supervisor mode.
 */
#define KERNBASE		0x80000000UL
#define KVA_TO_PA(va)		((paddr_t)((vaddr_t)(va) - KERNBASE))

#define CPUTWO_BLK_BASE		0x03F03000

#define BLK_READ(off)		(*(volatile uint32_t *)(CPUTWO_BLK_BASE + (off)))
#define BLK_WRITE(off, val)	(*(volatile uint32_t *)(CPUTWO_BLK_BASE + (off)) = (val))

/*
 * Import mainbus_attach_args.  Duplicated here to avoid a shared header
 * (same pattern as com_mainbus.c).
 */
struct mainbus_attach_args {
	const char	*ma_name;
	bus_addr_t	 ma_addr;
	int		 ma_irq;
};

/*
 * Per-device softc.  Embeds ld_softc as the first member so that
 * the ld(4) framework can cast freely.
 */
struct cputwo_blk_softc {
	struct ld_softc	 sc_ld;
	bus_addr_t	 sc_addr;
	int		 sc_irq;
};

/*
 * 512-byte aligned bounce buffer for sector I/O.
 * The block device DMA engine reads/writes from a physical address
 * that must be 512-byte aligned.  Kernel buf data may not meet this
 * requirement, so we bounce through this buffer one sector at a time.
 */
static uint8_t blk_bounce[BLK_SECTOR_SIZE]
    __attribute__((aligned(BLK_SECTOR_SIZE)));

static int	cputwo_blk_match(device_t, cfdata_t, void *);
static void	cputwo_blk_attach(device_t, device_t, void *);
static int	cputwo_blk_start(struct ld_softc *, struct buf *);
static int	cputwo_blk_dump(struct ld_softc *, void *, int, int);

CFATTACH_DECL_NEW(ld_mainbus, sizeof(struct cputwo_blk_softc),
    cputwo_blk_match, cputwo_blk_attach, NULL, NULL);

static int
cputwo_blk_match(device_t parent, cfdata_t cf, void *aux)
{
	struct mainbus_attach_args *ma = aux;

	if (strcmp(ma->ma_name, "ld") != 0)
		return 0;
	if (ma->ma_addr != CPUTWO_BLK_BASE)
		return 0;
	return 1;
}

static void
cputwo_blk_attach(device_t parent, device_t self, void *aux)
{
	struct cputwo_blk_softc *sc = device_private(self);
	struct ld_softc *ld = &sc->sc_ld;
	struct mainbus_attach_args *ma = aux;
	uint32_t nsectors;

	sc->sc_addr = ma->ma_addr;
	sc->sc_irq  = ma->ma_irq;

	aprint_naive("\n");

	/* Query device capacity via SIZE register */
	nsectors = BLK_READ(BLK_REG_SIZE);
	if (nsectors == 0) {
		aprint_normal(": CPUTwo block device at 0x%lx: no media\n",
		    (unsigned long)sc->sc_addr);
		return;
	}

	aprint_normal(": CPUTwo block device at 0x%lx, %u sectors (%uKB)\n",
	    (unsigned long)sc->sc_addr, nsectors,
	    (nsectors * BLK_SECTOR_SIZE) / 1024);

	/* Configure the ld(4) framework */
	ld->sc_dv = self;
	ld->sc_secperunit = nsectors;
	ld->sc_secsize = BLK_SECTOR_SIZE;
	ld->sc_maxxfer = MAXPHYS;
	ld->sc_maxqueuecnt = 1;		/* synchronous, one at a time */
	ld->sc_start = cputwo_blk_start;
	ld->sc_dump = cputwo_blk_dump;
	ld->sc_flags = LDF_ENABLED;

	ldattach(ld, BUFQ_DISK_DEFAULT_STRAT);
}

/*
 * Transfer a single 512-byte sector.
 * Returns 0 on success, EIO on error.
 */
static int
cputwo_blk_xfer(uint32_t sector, paddr_t pa, int cmd)
{

	BLK_WRITE(BLK_REG_SECTOR, sector);
	BLK_WRITE(BLK_REG_BUFFER, (uint32_t)pa);
	BLK_WRITE(BLK_REG_COMMAND, cmd);

	/* Device completes synchronously; poll to confirm. */
	while (BLK_READ(BLK_REG_STATUS) == BLK_STAT_BUSY)
		;

	if (BLK_READ(BLK_REG_STATUS) == BLK_STAT_ERROR)
		return EIO;
	return 0;
}

/*
 * ld(4) sc_start callback — called by the disk framework to issue I/O.
 *
 * bp->b_rawblkno is the starting DEV_BSIZE (512-byte) block number.
 * bp->b_bcount is the byte count.
 * bp->b_data is the kernel virtual address of the data buffer.
 * bp->b_flags & B_READ indicates direction.
 *
 * We transfer one sector at a time through the bounce buffer because
 * the hardware requires a 512-byte-aligned physical address and
 * bp->b_data may not be suitably aligned.
 */
static int
cputwo_blk_start(struct ld_softc *ld, struct buf *bp)
{
	uint32_t sector;
	int nsectors;
	int cmd;
	int error;
	int i;
	uint8_t *data;
	paddr_t bounce_pa;

	sector = bp->b_rawblkno;
	nsectors = bp->b_bcount / BLK_SECTOR_SIZE;
	data = bp->b_data;
	cmd = (bp->b_flags & B_READ) ? BLK_CMD_READ : BLK_CMD_WRITE;
	bounce_pa = KVA_TO_PA((vaddr_t)blk_bounce);

	for (i = 0; i < nsectors; i++) {
		if (cmd == BLK_CMD_WRITE)
			memcpy(blk_bounce, data + i * BLK_SECTOR_SIZE,
			    BLK_SECTOR_SIZE);

		error = cputwo_blk_xfer(sector + i, bounce_pa, cmd);
		if (error) {
			bp->b_error = error;
			bp->b_resid = bp->b_bcount - i * BLK_SECTOR_SIZE;
			lddone(ld, bp);
			return 0;
		}

		if (cmd == BLK_CMD_READ)
			memcpy(data + i * BLK_SECTOR_SIZE, blk_bounce,
			    BLK_SECTOR_SIZE);
	}

	bp->b_resid = 0;
	lddone(ld, bp);
	return 0;
}

/*
 * ld(4) sc_dump callback — for kernel crash dumps.
 * Writes nblk DEV_BSIZE blocks starting at blkno from va.
 */
static int
cputwo_blk_dump(struct ld_softc *ld, void *va, int blkno, int nblk)
{
	uint8_t *data = va;
	paddr_t bounce_pa;
	int i;

	bounce_pa = KVA_TO_PA((vaddr_t)blk_bounce);

	for (i = 0; i < nblk; i++) {
		memcpy(blk_bounce, data + i * BLK_SECTOR_SIZE,
		    BLK_SECTOR_SIZE);

		if (cputwo_blk_xfer(blkno + i, bounce_pa, BLK_CMD_WRITE))
			return EIO;
	}
	return 0;
}

/*
 * Block device interrupt handler — called from trap.c.
 * I/O is synchronous so nothing to do, but the symbol must exist.
 */
void cputwo_blk_intr(void);

void
cputwo_blk_intr(void)
{

	/* Synchronous operation — nothing to do */
}
