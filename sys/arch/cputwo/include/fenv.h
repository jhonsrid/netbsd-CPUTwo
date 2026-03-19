/*	$NetBSD$	*/

/*
 * Based on ieeefp.h written by J.T. Conklin, Apr 28, 1995
 * Public domain.
 */

#ifndef _CPUTWO_FENV_H_
#define _CPUTWO_FENV_H_

typedef int fenv_t;
typedef int fexcept_t;

#define	FE_INVALID	0x01
#define	FE_DIVBYZERO	0x02
#define	FE_OVERFLOW	0x04
#define	FE_UNDERFLOW	0x08
#define	FE_INEXACT	0x10

#define	FE_ALL_EXCEPT	0x1f

#define	FE_TONEAREST	0	/* round to nearest representable number */
#define	FE_TOWARDZERO	1	/* round to zero (truncate) */
#define	FE_UPWARD	2	/* round toward positive infinity */
#define	FE_DOWNWARD	3	/* round toward negative infinity */

__BEGIN_DECLS

/* Default floating-point environment */
extern const fenv_t	__fe_dfl_env;
#define FE_DFL_ENV	(&__fe_dfl_env)

__END_DECLS

#endif /* _CPUTWO_FENV_H_ */
