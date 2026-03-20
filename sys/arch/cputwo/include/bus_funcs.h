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

#ifndef _CPUTWO_BUS_FUNCS_H_
#define _CPUTWO_BUS_FUNCS_H_

/*
 * CPUTwo bus space operations.
 *
 * Direct volatile memory access -- no function pointers, no indirection.
 * CPUTwo has a single flat memory-mapped address space.
 */

/*
 *	bus_space_map: cast physical address to volatile pointer.
 */
static __inline int
bus_space_map(bus_space_tag_t t, bus_addr_t addr, bus_size_t size,
    int flags, bus_space_handle_t *handlep)
{
	*handlep = (bus_space_handle_t)(uintptr_t)addr;
	return 0;
}

/*
 *	bus_space_unmap: no-op.
 */
static __inline void
bus_space_unmap(bus_space_tag_t t, bus_space_handle_t h, bus_size_t size)
{
}

/*
 *	bus_space_subregion: offset into existing handle.
 */
static __inline int
bus_space_subregion(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t offset, bus_size_t size, bus_space_handle_t *handlep)
{
	*handlep = (bus_space_handle_t)((volatile uint8_t *)(h) + offset);
	return 0;
}

/*
 *	bus_space_vaddr: return kernel virtual address.
 */
#define bus_space_vaddr(t, h)	((void *)(h))

/*
 * Scalar read operations.
 */
static __inline uint8_t
bus_space_read_1(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o)
{
	return *(volatile uint8_t *)((volatile uint8_t *)(h) + o);
}

static __inline uint16_t
bus_space_read_2(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o)
{
	return *(volatile uint16_t *)((volatile uint8_t *)(h) + o);
}

static __inline uint32_t
bus_space_read_4(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o)
{
	return *(volatile uint32_t *)((volatile uint8_t *)(h) + o);
}

/*
 * Scalar write operations.
 */
static __inline void
bus_space_write_1(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o,
    uint8_t v)
{
	*(volatile uint8_t *)((volatile uint8_t *)(h) + o) = v;
}

static __inline void
bus_space_write_2(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o,
    uint16_t v)
{
	*(volatile uint16_t *)((volatile uint8_t *)(h) + o) = v;
}

static __inline void
bus_space_write_4(bus_space_tag_t t, bus_space_handle_t h, bus_size_t o,
    uint32_t v)
{
	*(volatile uint32_t *)((volatile uint8_t *)(h) + o) = v;
}

/*
 * Stream variants (no byte-swapping -- CPUTwo is native endian).
 */
#define bus_space_read_stream_1(t, h, o)	bus_space_read_1(t, h, o)
#define bus_space_read_stream_2(t, h, o)	bus_space_read_2(t, h, o)
#define bus_space_read_stream_4(t, h, o)	bus_space_read_4(t, h, o)

#define bus_space_write_stream_1(t, h, o, v)	bus_space_write_1(t, h, o, v)
#define bus_space_write_stream_2(t, h, o, v)	bus_space_write_2(t, h, o, v)
#define bus_space_write_stream_4(t, h, o, v)	bus_space_write_4(t, h, o, v)

/*
 * Multi read/write: read/write same address N times.
 */
static __inline void
bus_space_read_multi_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, uint8_t *a, size_t c)
{
	while (c--)
		*a++ = bus_space_read_1(t, h, o);
}

static __inline void
bus_space_read_multi_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, uint16_t *a, size_t c)
{
	while (c--)
		*a++ = bus_space_read_2(t, h, o);
}

static __inline void
bus_space_read_multi_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, uint32_t *a, size_t c)
{
	while (c--)
		*a++ = bus_space_read_4(t, h, o);
}

static __inline void
bus_space_write_multi_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, const uint8_t *a, size_t c)
{
	while (c--)
		bus_space_write_1(t, h, o, *a++);
}

static __inline void
bus_space_write_multi_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, const uint16_t *a, size_t c)
{
	while (c--)
		bus_space_write_2(t, h, o, *a++);
}

static __inline void
bus_space_write_multi_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, const uint32_t *a, size_t c)
{
	while (c--)
		bus_space_write_4(t, h, o, *a++);
}

#define bus_space_read_multi_stream_1	bus_space_read_multi_1
#define bus_space_read_multi_stream_2	bus_space_read_multi_2
#define bus_space_read_multi_stream_4	bus_space_read_multi_4

#define bus_space_write_multi_stream_1	bus_space_write_multi_1
#define bus_space_write_multi_stream_2	bus_space_write_multi_2
#define bus_space_write_multi_stream_4	bus_space_write_multi_4

/*
 * Region read/write: read/write consecutive addresses.
 */
static __inline void
bus_space_read_region_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, uint8_t *a, size_t c)
{
	volatile uint8_t *p = (volatile uint8_t *)((volatile uint8_t *)(h) + o);
	while (c--)
		*a++ = *p++;
}

static __inline void
bus_space_read_region_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, uint16_t *a, size_t c)
{
	volatile uint16_t *p = (volatile uint16_t *)((volatile uint8_t *)(h) + o);
	while (c--)
		*a++ = *p++;
}

static __inline void
bus_space_read_region_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, uint32_t *a, size_t c)
{
	volatile uint32_t *p = (volatile uint32_t *)((volatile uint8_t *)(h) + o);
	while (c--)
		*a++ = *p++;
}

static __inline void
bus_space_write_region_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, const uint8_t *a, size_t c)
{
	volatile uint8_t *p = (volatile uint8_t *)((volatile uint8_t *)(h) + o);
	while (c--)
		*p++ = *a++;
}

static __inline void
bus_space_write_region_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, const uint16_t *a, size_t c)
{
	volatile uint16_t *p = (volatile uint16_t *)((volatile uint8_t *)(h) + o);
	while (c--)
		*p++ = *a++;
}

static __inline void
bus_space_write_region_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, const uint32_t *a, size_t c)
{
	volatile uint32_t *p = (volatile uint32_t *)((volatile uint8_t *)(h) + o);
	while (c--)
		*p++ = *a++;
}

#define bus_space_read_region_stream_2	bus_space_read_region_2
#define bus_space_read_region_stream_4	bus_space_read_region_4

#define bus_space_write_region_stream_2	bus_space_write_region_2
#define bus_space_write_region_stream_4	bus_space_write_region_4

/*
 * Set region: write same value to consecutive addresses.
 */
static __inline void
bus_space_set_region_1(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, uint8_t v, size_t c)
{
	volatile uint8_t *p = (volatile uint8_t *)((volatile uint8_t *)(h) + o);
	while (c--)
		*p++ = v;
}

static __inline void
bus_space_set_region_2(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, uint16_t v, size_t c)
{
	volatile uint16_t *p = (volatile uint16_t *)((volatile uint8_t *)(h) + o);
	while (c--)
		*p++ = v;
}

static __inline void
bus_space_set_region_4(bus_space_tag_t t, bus_space_handle_t h,
    bus_size_t o, uint32_t v, size_t c)
{
	volatile uint32_t *p = (volatile uint32_t *)((volatile uint8_t *)(h) + o);
	while (c--)
		*p++ = v;
}

#define bus_space_set_region_stream_2	bus_space_set_region_2
#define bus_space_set_region_stream_4	bus_space_set_region_4

/*
 * Copy region.
 */
static __inline void
bus_space_copy_region_1(bus_space_tag_t t,
    bus_space_handle_t h1, bus_size_t o1,
    bus_space_handle_t h2, bus_size_t o2, size_t c)
{
	volatile uint8_t *s = (volatile uint8_t *)((volatile uint8_t *)(h1) + o1);
	volatile uint8_t *d = (volatile uint8_t *)((volatile uint8_t *)(h2) + o2);
	while (c--)
		*d++ = *s++;
}

static __inline void
bus_space_copy_region_2(bus_space_tag_t t,
    bus_space_handle_t h1, bus_size_t o1,
    bus_space_handle_t h2, bus_size_t o2, size_t c)
{
	volatile uint16_t *s = (volatile uint16_t *)((volatile uint8_t *)(h1) + o1);
	volatile uint16_t *d = (volatile uint16_t *)((volatile uint8_t *)(h2) + o2);
	while (c--)
		*d++ = *s++;
}

static __inline void
bus_space_copy_region_4(bus_space_tag_t t,
    bus_space_handle_t h1, bus_size_t o1,
    bus_space_handle_t h2, bus_size_t o2, size_t c)
{
	volatile uint32_t *s = (volatile uint32_t *)((volatile uint8_t *)(h1) + o1);
	volatile uint32_t *d = (volatile uint32_t *)((volatile uint8_t *)(h2) + o2);
	while (c--)
		*d++ = *s++;
}

/*
 * Bus space barrier: compiler barrier only (no cache on CPUTwo).
 */
#define bus_space_barrier(t, h, o, l, f)	__asm volatile("" ::: "memory")

/*
 * Bus DMA stubs -- CPUTwo has no DMA engine.
 * Define as errors/no-ops so the kernel compiles.
 */
#define bus_dmamap_create(t, s, n, m, b, f, p)		(ENOMEM)
#define bus_dmamap_destroy(t, p)			((void)0)
#define bus_dmamap_load(t, m, b, s, p, f)		(ENOMEM)
#define bus_dmamap_load_mbuf(t, m, b, f)		(ENOMEM)
#define bus_dmamap_load_uio(t, m, u, f)			(ENOMEM)
#define bus_dmamap_load_raw(t, m, sg, n, s, f)		(ENOMEM)
#define bus_dmamap_unload(t, p)				((void)0)
#define bus_dmamap_sync(t, p, o, l, ops)		((void)0)

#define bus_dmamem_alloc(t, s, a, b, sg, n, r, f)	(ENOMEM)
#define bus_dmamem_free(t, sg, n)			((void)0)
#define bus_dmamem_map(t, sg, n, s, k, f)		(ENOMEM)
#define bus_dmamem_unmap(t, k, s)			((void)0)
#define bus_dmamem_mmap(t, sg, n, o, p, f)		(-1)

#define bus_dmatag_subregion(t, mna, mxa, nt, f)	(EOPNOTSUPP)
#define bus_dmatag_destroy(t)				((void)0)

#endif /* _CPUTWO_BUS_FUNCS_H_ */
