/* $NetBSD$ */

#include <machine/endian.h>

#if BYTE_ORDER == BIG_ENDIAN
#define IEEE_BIG_ENDIAN
#else
#define IEEE_LITTLE_ENDIAN
#endif
