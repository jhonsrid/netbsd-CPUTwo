/* CPUTwo __sync/__atomic builtins — single-core stubs.
 * Use asm labels to avoid clang's builtin redeclaration check. */

_Bool __cputwo_atomic_is_lock_free(unsigned int size, const volatile void *p)
    __asm__("__atomic_is_lock_free");
_Bool __cputwo_atomic_is_lock_free(unsigned int size, const volatile void *p)
{ return (size <= 4) ? 1 : 0; }

#define SYNC_STUB(name, type, op) \
  type __cputwo_##name(volatile type *p, type v, ...) __asm__(#name); \
  type __cputwo_##name(volatile type *p, type v, ...) { type old = *p; op; return old; }

#define CAS_STUB(name, type) \
  type __cputwo_##name(volatile type *p, type o, type n, ...) __asm__(#name); \
  type __cputwo_##name(volatile type *p, type o, type n, ...) { type v = *p; if (v == o) *p = n; return v; }

SYNC_STUB(__sync_fetch_and_add_4, int, *p += v)
SYNC_STUB(__sync_fetch_and_sub_4, int, *p -= v)
SYNC_STUB(__sync_fetch_and_or_4, int, *p |= v)
SYNC_STUB(__sync_fetch_and_and_4, int, *p &= v)
CAS_STUB(__sync_val_compare_and_swap_4, int)

SYNC_STUB(__sync_fetch_and_add_8, long long, *p += v)
SYNC_STUB(__sync_fetch_and_or_8, long long, *p |= v)
CAS_STUB(__sync_val_compare_and_swap_8, long long)

long long __cputwo_sync_lock_test_and_set_8(volatile long long *p, long long v, ...) __asm__("__sync_lock_test_and_set_8");
long long __cputwo_sync_lock_test_and_set_8(volatile long long *p, long long v, ...) { long long old = *p; *p = v; return old; }

void __cputwo_sync_lock_release_8(volatile long long *p, ...) __asm__("__sync_lock_release_8");
void __cputwo_sync_lock_release_8(volatile long long *p, ...) { *p = 0; }
