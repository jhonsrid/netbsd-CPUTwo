/* $NetBSD$ */

#ifndef _CPUTWO_PROFILE_H_
#define _CPUTWO_PROFILE_H_

#define	_MCOUNT_DECL void _mcount

/* No profiling support for cputwo yet. */
#define MCOUNT_ASM_NAME "__mcount"
#define	PLTSYM

#endif /* _CPUTWO_PROFILE_H_ */
