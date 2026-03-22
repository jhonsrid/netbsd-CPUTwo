#	$NetBSD$

# No FPU — use MI soft-float versions
SRCS+=	fabs_ieee754.c compat_ldexp_ieee754.c compat_modf_ieee754.c

# Common ieee754 constants and functions
SRCS+=	infinityf_ieee754.c infinity_ieee754.c infinityl_ieee754.c
SRCS+=	fpclassifyf_ieee754.c fpclassifyd_ieee754.c fpclassifyl_ieee754.c
SRCS+=	isinff_ieee754.c isinfd_ieee754.c isinfl_ieee754.c
SRCS+=	isnanf_ieee754.c isnand_ieee754.c isnanl_ieee754.c
SRCS+=	isfinitef_ieee754.c isfinited_ieee754.c isfinitel_ieee754.c
SRCS+=	signbitf_ieee754.c signbitd_ieee754.c signbitl_ieee754.c

SRCS+=	nanf.c
SRCS+=	flt_rounds.c

SRCS+=	setjmp.S
SRCS+=	_setjmp.S
SRCS+=	sigsetjmp.S

SRCS+=	makecontext.c _resumecontext.S swapcontext.S _lwp.c

SRCS+=	cputwo_atomic.c cputwo_fenv.c cputwo_sync.c
COPTS.cputwo_sync.c+=	-fno-builtin -ffreestanding -w

CPPFLAGS._lwp.c	+= -D_LIBC_SOURCE
