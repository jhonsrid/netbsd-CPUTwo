#define	NWSKBD	0
#ifdef _LOCORE
 .ifndef _KERNEL_OPT_NWSKBD
 .global _KERNEL_OPT_NWSKBD
 .equiv _KERNEL_OPT_NWSKBD,0x0
 .endif
#else
__asm(" .ifndef _KERNEL_OPT_NWSKBD\n .global _KERNEL_OPT_NWSKBD\n .equiv _KERNEL_OPT_NWSKBD,0x0\n .endif");
#endif
