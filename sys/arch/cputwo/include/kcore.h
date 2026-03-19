/* $NetBSD$ */

#ifndef _CPUTWO_KCORE_H_
#define _CPUTWO_KCORE_H_

typedef struct cpu_kcore_hdr {
	uint64_t kh_misc[8];
	phys_ram_seg_t kh_ramsegs[0];
} cpu_kcore_hdr_t;

#endif /* _CPUTWO_KCORE_H_ */
