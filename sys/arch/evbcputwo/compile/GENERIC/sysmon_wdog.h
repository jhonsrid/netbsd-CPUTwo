#define	NSYSMON_WDOG	0
#ifdef _LOCORE
 .ifndef _KERNEL_OPT_NSYSMON_WDOG
 .global _KERNEL_OPT_NSYSMON_WDOG
 .equiv _KERNEL_OPT_NSYSMON_WDOG,0x0
 .endif
#else
__asm(" .ifndef _KERNEL_OPT_NSYSMON_WDOG\n .global _KERNEL_OPT_NSYSMON_WDOG\n .equiv _KERNEL_OPT_NSYSMON_WDOG,0x0\n .endif");
#endif
