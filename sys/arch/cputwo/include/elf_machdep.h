/*	$NetBSD$	*/

#ifndef _CPUTWO_ELF_MACHDEP_H_
#define _CPUTWO_ELF_MACHDEP_H_

/*
 * CPUTwo ELF machine-dependent definitions.
 * Uses the same ELF machine number as the LLVM backend.
 */

#define	ELF32_MACHDEP_ENDIANNESS	ELFDATA2LSB
#define	ELF32_MACHDEP_ID_CASES		\
		case EM_CPUTWO:		\
			break;

#define	ELF64_MACHDEP_ID_CASES		/* none */

#define	ELF32_MACHDEP_ID		EM_CPUTWO

#define	KERN_ELFSIZE			32
#define	ARCH_ELFSIZE			32

/* Relocation types (matching LLVM CPUTwo backend) */
#define	R_CPUTWO_NONE		0
#define	R_CPUTWO_32		1
#define	R_CPUTWO_PC20		2
#define	R_CPUTWO_HI16		3
#define	R_CPUTWO_LO16		4

#define	R_TYPE(name)		R_CPUTWO_ ## name

#endif /* _CPUTWO_ELF_MACHDEP_H_ */
