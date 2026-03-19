#define	NBRIDGE	0
#ifdef _LOCORE
 .ifndef _KERNEL_OPT_NBRIDGE
 .global _KERNEL_OPT_NBRIDGE
 .equiv _KERNEL_OPT_NBRIDGE,0x0
 .endif
#else
__asm(" .ifndef _KERNEL_OPT_NBRIDGE\n .global _KERNEL_OPT_NBRIDGE\n .equiv _KERNEL_OPT_NBRIDGE,0x0\n .endif");
#endif
