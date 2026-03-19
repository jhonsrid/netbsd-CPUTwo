/* option `CPU_IN_CKSUM' not defined */
#ifdef _LOCORE
 .ifndef _KERNEL_OPT_CPU_IN_CKSUM
 .global _KERNEL_OPT_CPU_IN_CKSUM
 .equiv _KERNEL_OPT_CPU_IN_CKSUM,0x6e074def
 .endif
#else
__asm(" .ifndef _KERNEL_OPT_CPU_IN_CKSUM\n .global _KERNEL_OPT_CPU_IN_CKSUM\n .equiv _KERNEL_OPT_CPU_IN_CKSUM,0x6e074def\n .endif");
#endif
