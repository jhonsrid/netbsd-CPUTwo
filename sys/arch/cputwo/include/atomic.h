/*	$NetBSD$	*/

/*
 * CPUTwo atomic operations.
 *
 * Single-core CPU with no cache coherency concerns.
 * Atomicity is achieved by disabling interrupts around
 * read-modify-write sequences.
 */

#ifndef _CPUTWO_ATOMIC_H_
#define _CPUTWO_ATOMIC_H_

#include <sys/types.h>

/*
 * Interrupt disable/restore for atomic sections.
 * STATUS register bit 1 = IE.
 */
static __inline uint32_t
__cputwo_atomic_disable(void)
{
	uint32_t old = *(volatile uint32_t *)0x03FFF010;
	*(volatile uint32_t *)0x03FFF010 = old & ~0x02;
	return old;
}

static __inline void
__cputwo_atomic_restore(uint32_t s)
{
	*(volatile uint32_t *)0x03FFF010 = s;
}

static __inline unsigned int
atomic_add_int_nv(volatile unsigned int *p, int v)
{
	uint32_t s = __cputwo_atomic_disable();
	*p += v;
	unsigned int nv = *p;
	__cputwo_atomic_restore(s);
	return nv;
}

static __inline void
atomic_add_int(volatile unsigned int *p, int v)
{
	uint32_t s = __cputwo_atomic_disable();
	*p += v;
	__cputwo_atomic_restore(s);
}

static __inline void
atomic_inc_uint(volatile unsigned int *p)
{
	atomic_add_int(p, 1);
}

static __inline unsigned int
atomic_inc_uint_nv(volatile unsigned int *p)
{
	return atomic_add_int_nv(p, 1);
}

static __inline void
atomic_inc_32(volatile uint32_t *p)
{
	atomic_add_int((volatile unsigned int *)p, 1);
}

static __inline uint32_t
atomic_inc_32_nv(volatile uint32_t *p)
{
	return atomic_add_int_nv((volatile unsigned int *)p, 1);
}

static __inline void
atomic_dec_uint(volatile unsigned int *p)
{
	atomic_add_int(p, -1);
}

static __inline unsigned int
atomic_dec_uint_nv(volatile unsigned int *p)
{
	return atomic_add_int_nv(p, -1);
}

static __inline void
atomic_dec_32(volatile uint32_t *p)
{
	atomic_add_int((volatile unsigned int *)p, -1);
}

static __inline uint32_t
atomic_dec_32_nv(volatile uint32_t *p)
{
	return atomic_add_int_nv((volatile unsigned int *)p, -1);
}

static __inline unsigned int
atomic_cas_uint(volatile unsigned int *p, unsigned int expected,
    unsigned int new_val)
{
	uint32_t s = __cputwo_atomic_disable();
	unsigned int old = *p;
	if (old == expected)
		*p = new_val;
	__cputwo_atomic_restore(s);
	return old;
}

static __inline uint32_t
atomic_cas_32(volatile uint32_t *p, uint32_t expected, uint32_t new_val)
{
	return atomic_cas_uint((volatile unsigned int *)p, expected, new_val);
}

static __inline void *
atomic_cas_ptr(volatile void *p, void *expected, void *new_val)
{
	uint32_t s = __cputwo_atomic_disable();
	void *old = *(void *volatile *)p;
	if (old == expected)
		*(void *volatile *)p = new_val;
	__cputwo_atomic_restore(s);
	return old;
}

static __inline unsigned int
atomic_swap_uint(volatile unsigned int *p, unsigned int new_val)
{
	uint32_t s = __cputwo_atomic_disable();
	unsigned int old = *p;
	*p = new_val;
	__cputwo_atomic_restore(s);
	return old;
}

static __inline void *
atomic_swap_ptr(volatile void *p, void *new_val)
{
	uint32_t s = __cputwo_atomic_disable();
	void *old = *(void *volatile *)p;
	*(void *volatile *)p = new_val;
	__cputwo_atomic_restore(s);
	return old;
}

static __inline void
atomic_or_uint(volatile unsigned int *p, unsigned int v)
{
	uint32_t s = __cputwo_atomic_disable();
	*p |= v;
	__cputwo_atomic_restore(s);
}

static __inline void
atomic_and_uint(volatile unsigned int *p, unsigned int v)
{
	uint32_t s = __cputwo_atomic_disable();
	*p &= v;
	__cputwo_atomic_restore(s);
}

/* Memory barriers: single-core, compiler barrier only */
static __inline void membar_enter(void)   { __asm volatile("" ::: "memory"); }
static __inline void membar_exit(void)    { __asm volatile("" ::: "memory"); }
static __inline void membar_producer(void){ __asm volatile("" ::: "memory"); }
static __inline void membar_consumer(void){ __asm volatile("" ::: "memory"); }
static __inline void membar_acquire(void) { __asm volatile("" ::: "memory"); }
static __inline void membar_release(void) { __asm volatile("" ::: "memory"); }
static __inline void membar_sync(void)    { __asm volatile("" ::: "memory"); }

#endif /* _CPUTWO_ATOMIC_H_ */
