#	$NetBSD$

.include <bsd.own.mk>

SRCS+=	__sigtramp2.S

CPPFLAGS+= -I.
CPPFLAGS.assym.h+=-D__LIBC12_SOURCE__ -D_KERNTYPES

# No FPU, always softfloat
.include <softfloat/Makefile.inc>
CPPFLAGS+= -DSOFTFLOAT

SRCS+=	fixunsgen_ieee754.c
SRCS+=	fixunsgen64_ieee754.c
