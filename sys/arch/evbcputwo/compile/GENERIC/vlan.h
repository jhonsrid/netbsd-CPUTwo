#define	NVLAN	0
#ifdef _LOCORE
 .ifndef _KERNEL_OPT_NVLAN
 .global _KERNEL_OPT_NVLAN
 .equiv _KERNEL_OPT_NVLAN,0x0
 .endif
#else
__asm(" .ifndef _KERNEL_OPT_NVLAN\n .global _KERNEL_OPT_NVLAN\n .equiv _KERNEL_OPT_NVLAN,0x0\n .endif");
#endif
