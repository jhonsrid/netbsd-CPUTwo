/*	$NetBSD$	*/

#include <sys/cdefs.h>

#include <math.h>
#include <machine/endian.h>

/* bytes for quiet NaN (IEEE single precision) — CPUTwo is little-endian */
const union __float_u __nanf =
		{ {    0,    0, 0xa0, 0x7f } };

__warn_references(__nanf, "warning: <math.h> defines NAN incorrectly for your compiler.")
