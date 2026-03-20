/*	$NetBSD$	*/

/*
 * CPUTwo atomic operations.
 *
 * Single-core CPU — atomicity via interrupt disable.
 */

#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD$");

#include <sys/types.h>
#include <sys/atomic.h>
#include <sys/intr.h>

/* STATUS register, bit 1 = IE */
#define CPUTWO_STATUS	(*(volatile uint32_t *)0x03FFF010)

static inline uint32_t
atomic_disable(void)
{
	uint32_t old = CPUTWO_STATUS;
	CPUTWO_STATUS = old & ~0x02;
	return old;
}

static inline void
atomic_restore(uint32_t s)
{
	CPUTWO_STATUS = s;
}

/* add */
void
atomic_add_int(volatile unsigned int *p, int v)
{
	uint32_t s = atomic_disable();
	*p += v;
	atomic_restore(s);
}

unsigned int
atomic_add_int_nv(volatile unsigned int *p, int v)
{
	uint32_t s = atomic_disable();
	*p += v;
	unsigned int nv = *p;
	atomic_restore(s);
	return nv;
}

/* inc */
void
atomic_inc_uint(volatile unsigned int *p)
{
	atomic_add_int(p, 1);
}

unsigned int
atomic_inc_uint_nv(volatile unsigned int *p)
{
	return atomic_add_int_nv(p, 1);
}

void
atomic_inc_32(volatile uint32_t *p)
{
	atomic_add_int((volatile unsigned int *)p, 1);
}

uint32_t
atomic_inc_32_nv(volatile uint32_t *p)
{
	return atomic_add_int_nv((volatile unsigned int *)p, 1);
}

/* dec */
void
atomic_dec_uint(volatile unsigned int *p)
{
	atomic_add_int(p, -1);
}

unsigned int
atomic_dec_uint_nv(volatile unsigned int *p)
{
	return atomic_add_int_nv(p, -1);
}

void
atomic_dec_32(volatile uint32_t *p)
{
	atomic_add_int((volatile unsigned int *)p, -1);
}

uint32_t
atomic_dec_32_nv(volatile uint32_t *p)
{
	return atomic_add_int_nv((volatile unsigned int *)p, -1);
}

/* cas */
unsigned int
atomic_cas_uint(volatile unsigned int *p, unsigned int expected,
    unsigned int new_val)
{
	uint32_t s = atomic_disable();
	unsigned int old = *p;
	if (old == expected)
		*p = new_val;
	atomic_restore(s);
	return old;
}

uint32_t
atomic_cas_32(volatile uint32_t *p, uint32_t expected, uint32_t new_val)
{
	return atomic_cas_uint((volatile unsigned int *)p, expected, new_val);
}

void *
atomic_cas_ptr(volatile void *p, void *expected, void *new_val)
{
	uint32_t s = atomic_disable();
	void *old = *(void *volatile *)p;
	if (old == expected)
		*(void *volatile *)p = new_val;
	atomic_restore(s);
	return old;
}

/* swap */
unsigned int
atomic_swap_uint(volatile unsigned int *p, unsigned int new_val)
{
	uint32_t s = atomic_disable();
	unsigned int old = *p;
	*p = new_val;
	atomic_restore(s);
	return old;
}

void *
atomic_swap_ptr(volatile void *p, void *new_val)
{
	uint32_t s = atomic_disable();
	void *old = *(void *volatile *)p;
	*(void *volatile *)p = new_val;
	atomic_restore(s);
	return old;
}

/* bitwise */
void
atomic_or_uint(volatile unsigned int *p, unsigned int v)
{
	uint32_t s = atomic_disable();
	*p |= v;
	atomic_restore(s);
}

void
atomic_and_uint(volatile unsigned int *p, unsigned int v)
{
	uint32_t s = atomic_disable();
	*p &= v;
	atomic_restore(s);
}

/* ulong variants (on 32-bit, ulong == uint32_t) */
void
atomic_dec_ulong(volatile unsigned long *p)
{
	uint32_t s = atomic_disable();
	(*p)--;
	atomic_restore(s);
}

unsigned long
atomic_inc_ulong_nv(volatile unsigned long *p)
{
	uint32_t s = atomic_disable();
	unsigned long nv = ++(*p);
	atomic_restore(s);
	return nv;
}

unsigned long
atomic_cas_ulong(volatile unsigned long *p, unsigned long expected,
    unsigned long new_val)
{
	uint32_t s = atomic_disable();
	unsigned long old = *p;
	if (old == expected)
		*p = new_val;
	atomic_restore(s);
	return old;
}

/* 32-bit bitwise/swap */
void
atomic_and_32(volatile uint32_t *p, uint32_t v)
{
	uint32_t s = atomic_disable();
	*p &= v;
	atomic_restore(s);
}

void
atomic_or_32(volatile uint32_t *p, uint32_t v)
{
	uint32_t s = atomic_disable();
	*p |= v;
	atomic_restore(s);
}

uint32_t
atomic_swap_32(volatile uint32_t *p, uint32_t new_val)
{
	uint32_t s = atomic_disable();
	uint32_t old = *p;
	*p = new_val;
	atomic_restore(s);
	return old;
}

/* long variants */
void
atomic_add_long(volatile unsigned long *p, long v)
{
	uint32_t s = atomic_disable();
	*p += v;
	atomic_restore(s);
}

unsigned long
atomic_add_long_nv(volatile unsigned long *p, long v)
{
	uint32_t s = atomic_disable();
	*p += v;
	unsigned long nv = *p;
	atomic_restore(s);
	return nv;
}

unsigned long
atomic_dec_ulong_nv(volatile unsigned long *p)
{
	uint32_t s = atomic_disable();
	unsigned long nv = --(*p);
	atomic_restore(s);
	return nv;
}

/* ufetch / ustore: userspace access (identity-mapped for now) */
int _ufetch_8(const uint8_t *, uint8_t *);
int
_ufetch_8(const uint8_t *uaddr, uint8_t *valp)
{
	*valp = *uaddr;
	return 0;
}

int _ufetch_16(const uint16_t *, uint16_t *);
int
_ufetch_16(const uint16_t *uaddr, uint16_t *valp)
{
	*valp = *uaddr;
	return 0;
}

int _ufetch_32(const uint32_t *, uint32_t *);
int
_ufetch_32(const uint32_t *uaddr, uint32_t *valp)
{
	*valp = *uaddr;
	return 0;
}

int _ustore_8(uint8_t *, uint8_t);
int
_ustore_8(uint8_t *uaddr, uint8_t val)
{
	*uaddr = val;
	return 0;
}

int _ustore_16(uint16_t *, uint16_t);
int
_ustore_16(uint16_t *uaddr, uint16_t val)
{
	*uaddr = val;
	return 0;
}

int _ustore_32(uint32_t *, uint32_t);
int
_ustore_32(uint32_t *uaddr, uint32_t val)
{
	*uaddr = val;
	return 0;
}

/* memory barriers: single-core, compiler barrier only */
void membar_enter(void)    { __asm volatile("" ::: "memory"); }
void membar_exit(void)     { __asm volatile("" ::: "memory"); }
void membar_producer(void) { __asm volatile("" ::: "memory"); }
void membar_consumer(void) { __asm volatile("" ::: "memory"); }
void membar_acquire(void)  { __asm volatile("" ::: "memory"); }
void membar_release(void)  { __asm volatile("" ::: "memory"); }
void membar_sync(void)     { __asm volatile("" ::: "memory"); }
