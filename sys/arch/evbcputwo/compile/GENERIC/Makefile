KERNEL_BUILD=/Users/john/netbsd-CPUTwo/sys/arch/evbcputwo/conf/GENERIC
IDENT= \
	-DCOMPAT_UTILS \
	-DDIAGNOSTIC \

MACHINE=evbcputwo
S=	/Users/john/netbsd-CPUTwo/sys
DEFTEXTADDR=0x00000000
BOARDTYPE=evbcputwo
BOARDMKFRAG=${CPUTWO}/conf/mk.evbcputwo
#	$NetBSD$

# Makefile for NetBSD/cputwo
#
# This makefile is constructed from a machine description:
#	config machineid
# Most changes should be made in the machine description
#	/sys/arch/<machine>/conf/``machineid''
# after which you should do
#	config machineid

USETOOLS?=	no
NEED_OWN_INSTALL_TARGET?=no
.include <bsd.own.mk>

##
## (1) port identification
##
.ifndef S
S=		../../../..
.endif
THISCPUTWO=	$S/arch/${MACHINE}
CPUTWO=		$S/arch/cputwo
GENASSYM_CONF=	${CPUTWO}/cputwo/genassym.cf
.-include "${THISCPUTWO}/conf/Makefile.${MACHINE}.inc"

##
## (2) compile settings
##
CPPFLAGS+=	-D${MACHINE}
AFLAGS+=	-x assembler-with-cpp

##
## (3) libkern and compat netBSD/cputwo
##
OPT_MODULAR=	

##
## (4) local objects, compile rules, and dependencies
##
.if !defined(SYSTEM_FIRST_OBJ)
SYSTEM_FIRST_OBJ=	locore.o
.endif
.if !defined(SYSTEM_FIRST_SFILE)
SYSTEM_FIRST_SFILE=	${CPUTWO}/cputwo/locore.S
.endif
MD_OBJS=	${SYSTEM_FIRST_OBJ} locore.o
MD_CFILES=
MD_SFILES=	${SYSTEM_FIRST_SFILE} ${CPUTWO}/cputwo/locore.S

${SYSTEM_FIRST_OBJ}: ${SYSTEM_FIRST_SFILE} assym.h
	${NORMAL_S}

locore.o: ${CPUTWO}/cputwo/locore.S assym.h
	${NORMAL_S}

##
## (5) link settings
##
TEXTADDR?=		${DEFTEXTADDR}
LINKFORMAT=		-N
LINKFLAGS_NORMAL=	-X
EXTRA_LINKFLAGS=	${BOARDMKFRAG_LINKFLAGS}

##
## (6) port specific target dependencies
##

##
## (7) misc settings
##

##
## (8) config(8) parameters
##
MACHINE_PARAM=		MACHINE=evbcputwo
MACHINE_PARAM+=		MACHINE_ARCH=cputwo
