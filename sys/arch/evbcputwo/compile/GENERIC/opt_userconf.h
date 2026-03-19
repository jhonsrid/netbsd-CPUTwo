/* option `USERCONF' not defined */
#ifdef _LOCORE
 .ifndef _KERNEL_OPT_USERCONF
 .global _KERNEL_OPT_USERCONF
 .equiv _KERNEL_OPT_USERCONF,0x6e074def
 .endif
#else
__asm(" .ifndef _KERNEL_OPT_USERCONF\n .global _KERNEL_OPT_USERCONF\n .equiv _KERNEL_OPT_USERCONF,0x6e074def\n .endif");
#endif
