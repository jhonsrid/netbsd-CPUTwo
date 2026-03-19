/* option `KTRACE' not defined */
#ifdef _LOCORE
 .ifndef _KERNEL_OPT_KTRACE
 .global _KERNEL_OPT_KTRACE
 .equiv _KERNEL_OPT_KTRACE,0x6e074def
 .endif
#else
__asm(" .ifndef _KERNEL_OPT_KTRACE\n .global _KERNEL_OPT_KTRACE\n .equiv _KERNEL_OPT_KTRACE,0x6e074def\n .endif");
#endif
