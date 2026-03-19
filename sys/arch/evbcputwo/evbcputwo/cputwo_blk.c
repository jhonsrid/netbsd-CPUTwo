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
 * CPUTwo block device driver.
 *
 * Simple synchronous block device at 0x03F03000:
 *   +0x00  Sector   (r/w)   Sector number (LBA)
 *   +0x04  Buffer   (r/w)   Physical address of 512-byte buffer (must be aligned)
 *   +0x08  Command  (write) 1 = read sector, 2 = write sector
 *   +0x0C  Status   (read)  0 = idle, 1 = busy, 2 = error
 *   +0x10  Control  (r/w)   bit 0 = IRQ enable
 *
 * Commands complete synchronously in the emulator (instantly from
 * the guest's perspective), so no interrupt/DMA handling is needed.
 *
 * This driver is not attached during initial bring-up (ramdisk root).
 * It will be needed later for booting from a disk image.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD$");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/buf.h>

#define BLK_BASE	0x03F03000
#define BLK_SECTOR	(*(volatile uint32_t *)(BLK_BASE + 0x00))
#define BLK_BUFFER	(*(volatile uint32_t *)(BLK_BASE + 0x04))
#define BLK_COMMAND	(*(volatile uint32_t *)(BLK_BASE + 0x08))
#define BLK_STATUS	(*(volatile uint32_t *)(BLK_BASE + 0x0C))
#define BLK_CONTROL	(*(volatile uint32_t *)(BLK_BASE + 0x10))

#define BLK_CMD_READ	1
#define BLK_CMD_WRITE	2
#define BLK_STAT_IDLE	0
#define BLK_STAT_BUSY	1
#define BLK_STAT_ERROR	2

#define BLK_SECTOR_SIZE	512

/*
 * Read a single 512-byte sector.
 * buf must be physically contiguous and 512-byte aligned.
 * Returns 0 on success, EIO on error.
 */
int
cputwo_blk_read(uint32_t sector, paddr_t buf)
{

	BLK_SECTOR = sector;
	BLK_BUFFER = (uint32_t)buf;
	BLK_COMMAND = BLK_CMD_READ;

	/* Command completes synchronously in the emulator */
	if (BLK_STATUS == BLK_STAT_ERROR)
		return EIO;
	return 0;
}

/*
 * Write a single 512-byte sector.
 * buf must be physically contiguous and 512-byte aligned.
 * Returns 0 on success, EIO on error.
 */
int
cputwo_blk_write(uint32_t sector, paddr_t buf)
{

	BLK_SECTOR = sector;
	BLK_BUFFER = (uint32_t)buf;
	BLK_COMMAND = BLK_CMD_WRITE;

	if (BLK_STATUS == BLK_STAT_ERROR)
		return EIO;
	return 0;
}

/*
 * Block device interrupt handler (for future async I/O support).
 */
void
cputwo_blk_intr(void)
{

	/* Synchronous operation — nothing to do */
}
