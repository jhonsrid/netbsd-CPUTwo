/*	$NetBSD$	*/

/*
 * CPUTwo has no FPU.  Software floating-point always rounds to nearest.
 */

#include <sys/cdefs.h>

int __flt_rounds(void);

int
__flt_rounds(void)
{
	return 1;	/* FE_TONEAREST */
}
