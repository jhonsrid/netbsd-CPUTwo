#include <sys/cdefs.h>

#include "opt_config.h"

/*
 * Add either (or both) of
 *
 *	options INCLUDE_CONFIG_FILE
 *	options INCLUDE_JUST_CONFIG
 *
 * to your kernel config file to embed it in the resulting
 * kernel.  The latter option does not include files that are
 * included (recursively) by your config file.  The embedded
 * data be extracted by using the command:
 *
 *	strings netbsd | sed -n 's/^_CFG_//p' | unvis
 */

#ifdef CONFIG_FILE
#if defined(INCLUDE_CONFIG_FILE) || defined(INCLUDE_JUST_CONFIG)

static const char config[] __used =

#ifdef INCLUDE_CONFIG_FILE

"_CFG_### START CONFIG FILE \"/Users/john/netbsd-CPUTwo/sys/arch/evbcputwo/conf/GENERIC\"\n"

#endif /* INCLUDE_CONFIG_FILE */

"_CFG_#\\011$NetBSD$\n"
"_CFG_#\n"
"_CFG_#\\011GENERIC kernel configuration for CPUTwo QEMU board\n"
"_CFG_#\n"
"_CFG_\n"
"_CFG_include \"arch/evbcputwo/conf/std.evbcputwo\"\n"
"_CFG_\n"
"_CFG_maxusers\\01132\n"
"_CFG_\n"
"_CFG_# Standard system options\n"
"_CFG_options\\011\\011DDB\\011\\011\\011# in-kernel debugger\n"
"_CFG_#options\\011DDB_HISTORY_SIZE=512\\011# enable history editing\n"
"_CFG_options\\011\\011DIAGNOSTIC\\011\\011# internal consistency checks\n"
"_CFG_#options\\011DEBUG\\011\\011\\011# expensive debugging checks/support\n"
"_CFG_\n"
"_CFG_# File systems\n"
"_CFG_file-system\\011FFS\\011\\011\\011# UFS\n"
"_CFG_file-system\\011MFS\\011\\011\\011# memory file system\n"
"_CFG_file-system\\011NFS\\011\\011\\011# Network File System client\n"
"_CFG_file-system\\011KERNFS\\011\\011\\011# /kern\n"
"_CFG_file-system\\011PROCFS\\011\\011\\011# /proc\n"
"_CFG_\n"
"_CFG_# Networking\n"
"_CFG_options\\011\\011INET\\011\\011\\011# IP + ICMP + TCP + UDP\n"
"_CFG_\n"
"_CFG_# Kernel root -- use memory disk for initial bring-up\n"
"_CFG_config\\011\\011netbsd\\011root on md0a type ffs\n"
"_CFG_options\\011\\011MEMORY_DISK_HOOKS\n"
"_CFG_options\\011\\011MEMORY_DISK_IS_ROOT\n"
"_CFG_options\\011\\011MEMORY_DISK_ROOT_SIZE=4096\\011# 2MB ramdisk\n"
"_CFG_\n"
"_CFG_# Pseudo-devices\n"
"_CFG_pseudo-device\\011md\\011\\011\\011# memory disk (for ramdisk root)\n"
"_CFG_pseudo-device\\011loop\\011\\011\\011# network loopback\n"
"_CFG_pseudo-device\\011pty\\011\\011\\011# pseudo-terminals\n"
"_CFG_\n"
"_CFG_# Devices\n"
"_CFG_mainbus0\\011at root\n"
"_CFG_cpu0\\011\\011at mainbus?\n"
"_CFG_com0\\011\\011at mainbus? addr 0x03F00000\n"
#ifdef INCLUDE_CONFIG_FILE

"_CFG_### END CONFIG FILE \"/Users/john/netbsd-CPUTwo/sys/arch/evbcputwo/conf/GENERIC\"\n"
"_CFG_### (included from \"arch/evbcputwo/conf/std.evbcputwo\")\n"
"_CFG_###> #\\011$NetBSD$\n"
"_CFG_###> \n"
"_CFG_###> machine\\011\\011evbcputwo cputwo\n"
"_CFG_###> include\\011\\011\"conf/std\"\\011\\011# MI standard options\n"
"_CFG_###> \n"
"_CFG_###> options\\011\\011EXEC_ELF32\\011\\011# exec ELF32 binaries\n"
"_CFG_###> \n"
"_CFG_###> makeoptions\\011DEFTEXTADDR=\"0x00000000\"\n"
"_CFG_###> makeoptions\\011BOARDTYPE=\"evbcputwo\"\n"
"_CFG_###> makeoptions\\011BOARDMKFRAG=\"${CPUTWO}/conf/mk.evbcputwo\"\n"
"_CFG_###> \n"
"_CFG_###> # files.evbcputwo is auto-included by config(8) from \"machine evbcputwo\"\n"
"_CFG_###> # files.cputwo is auto-included by config(8) from \"machine ... cputwo\"\n"
"_CFG_### (end include \"arch/evbcputwo/conf/std.evbcputwo\")\n"
"_CFG_### (included from \"conf/std\")\n"
"_CFG_###> # $NetBSD: std,v 1.23 2019/01/27 02:08:41 pgoyette Exp $\n"
"_CFG_###> #\n"
"_CFG_###> # standard MI 'options'\n"
"_CFG_###> #\n"
"_CFG_###> # this file is for options which can't be off-by-default for some reasons.\n"
"_CFG_###> # \"it's commonly used\" is NOT a good reason to enable options here.\n"
"_CFG_###> \n"
"_CFG_###> #\n"
"_CFG_###> # Always include \"kern\" attribute (module).  Other attributes don't need to\n"
"_CFG_###> # depend on \"kern\".\n"
"_CFG_###> #\n"
"_CFG_###> select\\011kern\n"
"_CFG_###> \n"
"_CFG_###> # Always include the \"vfs\" attribute (module).  Although all of the\n"
"_CFG_###> # ufs/xxx file systems depend on the vfs attribute, it is not required\n"
"_CFG_###> # that any file system actually be built-in to the kernel.  (At least\n"
"_CFG_###> # on some architectures, file system modules can be loaded at boot\n"
"_CFG_###> # time.)\n"
"_CFG_###> \n"
"_CFG_###> select vfs\n"
"_CFG_###> \n"
"_CFG_###> select\\011net\\011\\011# XXX Clean up dependency\n"
"_CFG_###> \n"
"_CFG_###> # the following options are on-by-default to keep\n"
"_CFG_###> # kernel config file compatibility.\n"
"_CFG_###> options\\011VMSWAP\\011\\011# Swap device/file support\n"
"_CFG_###> options\\011BUFQ_FCFS\\011# First-come First-serve strategy\n"
"_CFG_###> options\\011BUFQ_DISKSORT\\011# Traditional min seek sort strategy\n"
"_CFG_###> options\\011RFC2292\\011\\011# Previous version of Adv. Sockets API for IPv6 \n"
"_CFG_###> options\\011PTRACE\\011\\011# Include ptrace(2) syscall\n"
"_CFG_###> options\\011PTRACE_HOOKS\\011# Include ptrace hooks\n"
"_CFG_###> options\\011COREDUMP\\011# allow processes to coredump.\n"
"_CFG_###> options\\011AIO\\011\\011# POSIX asynchronous I/O\n"
"_CFG_###> options\\011MQUEUE\\011\\011# POSIX message queues\n"
"_CFG_###> \n"
"_CFG_###> # Common compatibility functions. They happen to be needed even when\n"
"_CFG_###> # no compatibility option is explicitly enabled.\n"
"_CFG_###> #\n"
"_CFG_###> options        COMPAT_UTILS\n"
"_CFG_###> \n"
"_CFG_###> #\n"
"_CFG_###> # Security model.\n"
"_CFG_###> #\n"
"_CFG_###> options\\011secmodel_bsd44\\011# Traditional 4.4BSD security model\n"
"_CFG_###> \n"
"_CFG_###> #\n"
"_CFG_###> # Scheduling algorithm\n"
"_CFG_###> #\n"
"_CFG_###> options\\011SCHED_4BSD\n"
"_CFG_###> \n"
"_CFG_###> pseudo-device\\011cpuctl\n"
"_CFG_###> \n"
"_CFG_###> #\n"
"_CFG_###> # Kernel entropy pool and random-number generator pseudodevice.\n"
"_CFG_###> # The pseudodevice might stop being \"std\" when the two are torn\n"
"_CFG_###> # apart some day but the entropy pool itself never will (they are\n"
"_CFG_###> # presently implemented in the same source file)\n"
"_CFG_###> #\n"
"_CFG_###> pseudo-device\\011rnd\n"
"_CFG_### (end include \"conf/std\")\n"
#endif /* INCLUDE_CONFIG_FILE */
;
#endif /* INCLUDE_CONFIG_FILE || INCLUDE_JUST_CONFIG */
#endif /* CONFIG_FILE */
