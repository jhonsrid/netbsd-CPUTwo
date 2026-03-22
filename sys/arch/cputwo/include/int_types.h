/*	$NetBSD$	*/

#ifndef _CPUTWO_INT_TYPES_H_
#define _CPUTWO_INT_TYPES_H_

/*
 * Use compiler builtins via common_int_types.h when available and not
 * in the tools build (where nbinclude/sys/ lacks that header).
 */
#if defined(__UINTPTR_TYPE__) && !defined(HAVE_NBTOOL_CONFIG_H)
#include <sys/common_int_types.h>
#else
#include <sys/cdefs.h>

/* 7.18.1.1 Exact-width integer types */
typedef	signed char		 __int8_t;
typedef	unsigned char		__uint8_t;
typedef	short int		__int16_t;
typedef	unsigned short int     __uint16_t;
typedef	int			__int32_t;
typedef	unsigned int	       __uint32_t;
/* LONGLONG */
typedef	long long int		__int64_t;
/* LONGLONG */
typedef	unsigned long long int __uint64_t;

#define	__BIT_TYPES_DEFINED__

/* 7.18.1.4 Integer types capable of holding object pointers */
#ifdef _LP64
typedef long int	       __intptr_t;
typedef unsigned long int     __uintptr_t;
#else
typedef	int		       __intptr_t;
typedef	unsigned int	      __uintptr_t;
#endif

#endif /* !__UINTPTR_TYPE__ || HAVE_NBTOOL_CONFIG_H */

#endif /* !_CPUTWO_INT_TYPES_H_ */
