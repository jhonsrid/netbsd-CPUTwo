/*	$NetBSD$	*/

/*
 * CPUTwo atomic operations and memory barriers.
 * Single-core CPU: all barriers are no-ops, atomics are plain operations.
 */

#include <sys/types.h>

void membar_producer(void) {}
void membar_consumer(void) {}
void membar_sync(void) {}
void membar_release(void) {}
void membar_acquire(void) {}
void membar_enter(void) {}
void membar_exit(void) {}
void _membar_producer(void) {}
void _membar_consumer(void) {}

void __libc_atomic_init(void) {}

unsigned int
_atomic_cas_uint(volatile unsigned int *ptr, unsigned int old_val,
    unsigned int new_val)
{
	unsigned int val = *ptr;
	if (val == old_val)
		*ptr = new_val;
	return val;
}

void *
atomic_cas_ptr(volatile void *ptr, void *old_val, void *new_val)
{
	void * volatile *p = (void * volatile *)ptr;
	void *val = *p;
	if (val == old_val)
		*p = new_val;
	return val;
}

unsigned int
_atomic_cas_uint_ni(volatile unsigned int *ptr, unsigned int old_val,
    unsigned int new_val)
{
	return _atomic_cas_uint(ptr, old_val, new_val);
}

void *
atomic_cas_ptr_ni(volatile void *ptr, void *old_val, void *new_val)
{
	return atomic_cas_ptr(ptr, old_val, new_val);
}

/* NetBSD atomic_ops(3) wrappers — single-core, no locking needed */
unsigned int
atomic_add_32_nv(volatile unsigned int *p, int v)
{ return (*p += v); }

void
atomic_add_32(volatile unsigned int *p, int v)
{ *p += v; }

unsigned int
atomic_inc_32_nv(volatile unsigned int *p)
{ return ++(*p); }

void
atomic_inc_32(volatile unsigned int *p)
{ ++(*p); }

unsigned int
atomic_dec_32_nv(volatile unsigned int *p)
{ return --(*p); }

void
atomic_dec_32(volatile unsigned int *p)
{ --(*p); }

unsigned int
atomic_or_32_nv(volatile unsigned int *p, unsigned int v)
{ return (*p |= v); }

void
atomic_or_32(volatile unsigned int *p, unsigned int v)
{ *p |= v; }

void
atomic_and_32(volatile unsigned int *p, unsigned int v)
{ *p &= v; }

unsigned int
atomic_and_32_nv(volatile unsigned int *p, unsigned int v)
{ return (*p &= v); }

unsigned int
atomic_swap_32(volatile unsigned int *p, unsigned int v)
{ unsigned int old = *p; *p = v; return old; }

unsigned int
atomic_cas_32(volatile unsigned int *p, unsigned int old_val,
    unsigned int new_val)
{ unsigned int v = *p; if (v == old_val) *p = new_val; return v; }
