#define	NWSMUX	0
#ifdef _LOCORE
 .ifndef _KERNEL_OPT_NWSMUX
 .global _KERNEL_OPT_NWSMUX
 .equiv _KERNEL_OPT_NWSMUX,0x0
 .endif
#else
__asm(" .ifndef _KERNEL_OPT_NWSMUX\n .global _KERNEL_OPT_NWSMUX\n .equiv _KERNEL_OPT_NWSMUX,0x0\n .endif");
#endif
