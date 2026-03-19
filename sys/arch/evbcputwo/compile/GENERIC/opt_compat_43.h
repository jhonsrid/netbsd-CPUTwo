/* option `COMPAT_43' not defined */
#ifdef _LOCORE
 .ifndef _KERNEL_OPT_COMPAT_43
 .global _KERNEL_OPT_COMPAT_43
 .equiv _KERNEL_OPT_COMPAT_43,0x6e074def
 .endif
#else
__asm(" .ifndef _KERNEL_OPT_COMPAT_43\n .global _KERNEL_OPT_COMPAT_43\n .equiv _KERNEL_OPT_COMPAT_43,0x6e074def\n .endif");
#endif
