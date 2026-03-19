/* option `KERNEL_BASE_VIRT' not defined */
#ifdef _LOCORE
 .ifndef _KERNEL_OPT_KERNEL_BASE_VIRT
 .global _KERNEL_OPT_KERNEL_BASE_VIRT
 .equiv _KERNEL_OPT_KERNEL_BASE_VIRT,0x6e074def
 .endif
#else
__asm(" .ifndef _KERNEL_OPT_KERNEL_BASE_VIRT\n .global _KERNEL_OPT_KERNEL_BASE_VIRT\n .equiv _KERNEL_OPT_KERNEL_BASE_VIRT,0x6e074def\n .endif");
#endif
/* option `KERNEL_BASE_PHYS' not defined */
#ifdef _LOCORE
 .ifndef _KERNEL_OPT_KERNEL_BASE_PHYS
 .global _KERNEL_OPT_KERNEL_BASE_PHYS
 .equiv _KERNEL_OPT_KERNEL_BASE_PHYS,0x6e074def
 .endif
#else
__asm(" .ifndef _KERNEL_OPT_KERNEL_BASE_PHYS\n .global _KERNEL_OPT_KERNEL_BASE_PHYS\n .equiv _KERNEL_OPT_KERNEL_BASE_PHYS,0x6e074def\n .endif");
#endif
