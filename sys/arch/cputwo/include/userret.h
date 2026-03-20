/*	$NetBSD$	*/

#ifndef _CPUTWO_USERRET_H_
#define _CPUTWO_USERRET_H_

#include <sys/userret.h>

static __inline void
userret(struct lwp *l)
{

	l->l_md.md_astpending = 0;

	if (l->l_pflag & LP_OWEUPC) {
		l->l_pflag &= ~LP_OWEUPC;
		ADDUPROF(l);
	}

	mi_userret(l);
}

#endif /* !_CPUTWO_USERRET_H_ */
