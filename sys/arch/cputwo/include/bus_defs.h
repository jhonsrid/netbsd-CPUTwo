/*	$NetBSD$	*/

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

#ifndef _CPUTWO_BUS_DEFS_H_
#define _CPUTWO_BUS_DEFS_H_

/*
 * CPUTwo bus space definitions.
 *
 * CPUTwo has simple memory-mapped I/O only (no separate I/O space).
 * MMIO region: 0x03F00000 - 0x03FFFFFF.
 * All bus operations are direct volatile memory accesses.
 */

/*
 * Bus address/size types.
 */
typedef uint32_t	bus_addr_t;
typedef uint32_t	bus_size_t;

#define PRIxBUSADDR	PRIx32
#define PRIxBUSSIZE	PRIx32
#define PRIuBUSSIZE	PRIu32

/*
 * Bus space handle: just a pointer to volatile memory.
 */
typedef volatile uint32_t	*bus_space_handle_t;

#define PRIxBSH		PRIxPTR

/*
 * Bus space tag: unused on CPUTwo (single flat address space).
 */
typedef int	bus_space_tag_t;

/*
 * Bus space map flags.
 */
#define BUS_SPACE_MAP_CACHEABLE		0x01
#define BUS_SPACE_MAP_LINEAR		0x02
#define BUS_SPACE_MAP_PREFETCHABLE	0x04

#define __BUS_SPACE_HAS_STREAM_METHODS

/*
 * Bus space barrier flags.
 */
#define BUS_SPACE_BARRIER_READ		0x01
#define BUS_SPACE_BARRIER_WRITE		0x02

#define BUS_SPACE_ALIGNED_POINTER(p, t)	ALIGNED_POINTER(p, t)

/*
 * Bus DMA definitions.
 */
#define BUS_DMA_WAITOK		0x000
#define BUS_DMA_NOWAIT		0x001
#define BUS_DMA_ALLOCNOW	0x002
#define BUS_DMA_COHERENT	0x004
#define BUS_DMA_STREAMING	0x008
#define BUS_DMA_BUS1		0x010
#define BUS_DMA_BUS2		0x020
#define BUS_DMA_BUS3		0x040
#define BUS_DMA_BUS4		0x080
#define BUS_DMA_READ		0x100
#define BUS_DMA_WRITE		0x200
#define BUS_DMA_NOCACHE		0x400

#ifndef BUS_DMA_DONTCACHE
#define BUS_DMA_DONTCACHE	BUS_DMA_NOCACHE
#endif

#define BUS_DMASYNC_PREREAD	0x01
#define BUS_DMASYNC_POSTREAD	0x02
#define BUS_DMASYNC_PREWRITE	0x04
#define BUS_DMASYNC_POSTWRITE	0x08

/* Forwards needed by prototypes below. */
struct proc;
struct mbuf;
struct uio;

/*
 * Bus DMA tag: minimal, no real DMA engine on CPUTwo.
 */
typedef uint32_t	bus_dma_tag_t;

#define BUS_DMA_TAG_VALID(t)	((t) != (bus_dma_tag_t)0)

/*
 * Bus DMA segment.
 */
struct cputwo_bus_dma_segment {
	bus_addr_t	ds_addr;
	bus_size_t	ds_len;
};
typedef struct cputwo_bus_dma_segment	bus_dma_segment_t;

/*
 * Bus DMA map (minimal stub).
 */
typedef struct cputwo_bus_dmamap	*bus_dmamap_t;

struct cputwo_bus_dmamap {
	bus_size_t	_dm_size;
	int		_dm_segcnt;
	bus_size_t	_dm_maxmaxsegsz;
	bus_size_t	_dm_boundary;
	int		_dm_flags;
	bus_size_t	dm_maxsegsz;
	bus_size_t	dm_mapsize;
	int		dm_nsegs;
	bus_dma_segment_t dm_segs[1];
};

#endif /* _CPUTWO_BUS_DEFS_H_ */
