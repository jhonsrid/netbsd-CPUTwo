/*
 * Automatically generated file from /Users/john/netbsd-CPUTwo/sys/conf/newvers.sh
 * Do not edit.
 */
#include <sys/cdefs.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/exec.h>
#include <sys/exec_elf.h>

const char ostype[] = "NetBSD";
const char osrelease[] = "10.1_STABLE";
const char sccs[] = "@(#)" "NetBSD 10.1_STABLE (GENERIC) #0: Fri Mar 20 09:54:36 GMT 2026\n"
"	john@johns-Mac-Studio-5.local:/Users/john/netbsd-CPUTwo/sys/arch/evbcputwo/compile/GENERIC\n";
const char version[] = "NetBSD 10.1_STABLE (GENERIC) #0: Fri Mar 20 09:54:36 GMT 2026\n"
"	john@johns-Mac-Studio-5.local:/Users/john/netbsd-CPUTwo/sys/arch/evbcputwo/compile/GENERIC\n";
const char buildinfo[] = "";
const char kernel_ident[] = "GENERIC";
const char copyright[] = "Copyright (c) 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003,\n"
"    2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013,\n"
"    2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021, 2022, 2023,\n"
"    2024, 2025, 2026\n"
"    The NetBSD Foundation, Inc.  All rights reserved.\n"
"Copyright (c) 1982, 1986, 1989, 1991, 1993\n"
"    The Regents of the University of California.  All rights reserved.\n"
"\n";

/*
 * NetBSD identity note.
 */
#ifdef __arm__
#define _SHT_NOTE	%note
#else
#define _SHT_NOTE	@note
#endif

#define	_S(TAG)	__STRING(TAG)
__asm(
	".section\t\".note.netbsd.ident\", \"\"," _S(_SHT_NOTE) "\n"
	"\t.p2align\t2\n"
	"\t.long\t" _S(ELF_NOTE_NETBSD_NAMESZ) "\n"
	"\t.long\t" _S(ELF_NOTE_NETBSD_DESCSZ) "\n"
	"\t.long\t" _S(ELF_NOTE_TYPE_NETBSD_TAG) "\n"
	"\t.ascii\t" _S(ELF_NOTE_NETBSD_NAME) "\n"
	"\t.long\t" _S(__NetBSD_Version__) "\n"
	"\t.p2align\t2\n"
);

