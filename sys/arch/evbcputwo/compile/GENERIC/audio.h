#define	NAUDIO	0
#ifdef _LOCORE
 .ifndef _KERNEL_OPT_NAUDIO
 .global _KERNEL_OPT_NAUDIO
 .equiv _KERNEL_OPT_NAUDIO,0x0
 .endif
#else
__asm(" .ifndef _KERNEL_OPT_NAUDIO\n .global _KERNEL_OPT_NAUDIO\n .equiv _KERNEL_OPT_NAUDIO,0x0\n .endif");
#endif
