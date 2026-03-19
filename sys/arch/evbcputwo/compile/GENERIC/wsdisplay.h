#define	NWSDISPLAY	0
#ifdef _LOCORE
 .ifndef _KERNEL_OPT_NWSDISPLAY
 .global _KERNEL_OPT_NWSDISPLAY
 .equiv _KERNEL_OPT_NWSDISPLAY,0x0
 .endif
#else
__asm(" .ifndef _KERNEL_OPT_NWSDISPLAY\n .global _KERNEL_OPT_NWSDISPLAY\n .equiv _KERNEL_OPT_NWSDISPLAY,0x0\n .endif");
#endif
