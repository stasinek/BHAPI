/*
 * Copyright 2009-2010, Haiku, Inc. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_HAIKU_TYPES_H
#define BHAPI_HAIKU_TYPES_H

#include <HaikuConfig.h>
#include <ctype.h>

/* fixed-width types -- the __haiku_std_[u]int* types correspond to the POSIX
   [u]int*_t types, the _haiku_[u]int* types to the BeOS [u]int* types. If
   __HAIKU_BEOS_COMPATIBLE_TYPES is not defined both sets are identical. Once
   we drop compatibility for good, we can consolidate the types.
*/
typedef signed char			__haiku_std_int8;
typedef unsigned char		__haiku_std_uint8;
typedef signed short		__haiku_std_int16;
typedef unsigned short		__haiku_std_uint16;
typedef signed int			__haiku_std_int32;
typedef unsigned int		__haiku_std_uint32;
#ifdef __HAIKU_ARCH_64_BIT
typedef signed long			__haiku_std_int64;
typedef unsigned long		__haiku_std_uint64;
#else
typedef signed long long	__haiku_std_int64;
typedef unsigned long long	__haiku_std_uint64;
#endif

typedef __haiku_std_int8	__haiku_int8;
typedef __haiku_std_uint8	__haiku_uint8;
typedef __haiku_std_int16	__haiku_int16;
typedef __haiku_std_uint16	__haiku_uint16;
#ifdef __HAIKU_BEOS_COMPATIBLE_TYPES
typedef signed long int		__haiku_int32;
typedef unsigned long int	__haiku_uint32;
#else
typedef __haiku_std_int32	__haiku_int32;
typedef __haiku_std_uint32	__haiku_uint32;
#endif
typedef __haiku_std_int64	__haiku_int64;
typedef __haiku_std_uint64	__haiku_uint64;

/* address types */
typedef signed long int		__haiku_saddr_t;
typedef	unsigned long int	__haiku_addr_t;

#ifdef __HAIKU_ARCH_PHYSICAL_64_BIT
    typedef __haiku_int64	__haiku_phys_saddr_t;
    typedef __haiku_uint64	__haiku_phys_addr_t;
#else
    typedef	__haiku_int32	__haiku_phys_saddr_t;
    typedef	__haiku_uint32	__haiku_phys_addr_t;
#endif

/* address type limits */
#ifdef __HAIKU_ARCH_64_BIT
#	define __HAIKU_SADDR_MAX		(9223372036854775807LL)
#	define __HAIKU_ADDR_MAX			(18446744073709551615ULL)
#else
#	define __HAIKU_SADDR_MAX		(2147483647)
#	define __HAIKU_ADDR_MAX			(4294967295U)
#endif
#define __HAIKU_SADDR_MIN			(-__HAIKU_SADDR_MAX-1)

#ifdef __HAIKU_ARCH_PHYSICAL_64_BIT
#	define __HAIKU_PHYS_SADDR_MAX	(9223372036854775807LL)
#	define __HAIKU_PHYS_ADDR_MAX	(18446744073709551615ULL)
#else
#	define __HAIKU_PHYS_SADDR_MAX	(2147483647)
#	define __HAIKU_PHYS_ADDR_MAX	(4294967295U)
#endif
#define __HAIKU_PHYS_SADDR_MIN		(-__HAIKU_SADDR_MAX-1)

/* printf()/scanf() format prefixes */
#define	__HAIKU_STD_PRI_PREFIX_32	""
#ifdef __HAIKU_ARCH_64_BIT
#	define	__HAIKU_STD_PRI_PREFIX_64	"l"
#else
#	define	__HAIKU_STD_PRI_PREFIX_64	"ll"
#endif

#ifdef __HAIKU_BEOS_COMPATIBLE_TYPES
#	define	__HAIKU_PRI_PREFIX_32		"l"
#else
#	define	__HAIKU_PRI_PREFIX_32		__HAIKU_STD_PRI_PREFIX_32
#endif
#define	__HAIKU_PRI_PREFIX_64			__HAIKU_STD_PRI_PREFIX_64

#define __HAIKU_PRI_PREFIX_ADDR			"l"

#ifdef __HAIKU_ARCH_PHYSICAL_64_BIT
#	define __HAIKU_PRI_PREFIX_PHYS_ADDR	__HAIKU_PRI_PREFIX_64
#else
#	define __HAIKU_PRI_PREFIX_PHYS_ADDR	__HAIKU_PRI_PREFIX_32
#endif

/* a generic address type wide enough for virtual and physical addresses */
#if __HAIKU_ARCH_BITS >= __HAIKU_ARCH_PHYSICAL_BITS
    typedef __haiku_addr_t					__haiku_generic_addr_t;
#	define __HAIKU_GENERIC_ADDR_MAX			__HAIKU_ADDR_MAX
#	define __HAIKU_PRI_PREFIX_GENERIC_ADDR	__HAIKU_PRI_PREFIX_ADDR
#else
    typedef __haiku_phys_addr_t				__haiku_generic_addr_t;
#	define __HAIKU_GENERIC_ADDR_MAX			__HAIKU_PHYS_ADDR_MAX
#	define __HAIKU_PRI_PREFIX_GENERIC_ADDR	__HAIKU_PRI_PREFIX_PHYS_ADDR
#endif

typedef __haiku_addr_t   ptr_t;
typedef __haiku_addr_t   addr_t;

#ifdef _MSC_VER
typedef __int8   int8_t;
typedef unsigned __int8   uint8_t;
typedef __int16  int16_t;
typedef unsigned __int16  uint16_t;
typedef __int32  int32_t;
typedef unsigned __int32  uint32_t;
typedef __int64  int64_t;
typedef unsigned __int64  uint64_t;
#endif

/*typedef	__int8 int8_t;
typedef	unsigned __int8 uint8_t;
typedef	__int16 int16_t;
typedef	unsigned __int16 uint16_t;
typedef	__int32 int32_t;
typedef	unsigned __int32 uint32_t;
typedef	__int64 int64_t;
typedef	unsigned __int64 uint64_t;
*/

typedef	__int8  int8;
typedef	unsigned __int8 uint8;
typedef	__int16 int16;
typedef	unsigned __int16 uint16;
typedef	__int32 int32;
typedef	unsigned __int32 uint32;
typedef	__int64 int64;
typedef	unsigned __int64 uint64;

typedef	__int8  __be_int8;
typedef	unsigned __int8  __be_uint8;
typedef	__int16  __be_int16;
typedef	unsigned __int16  __be_uint16;
typedef	__int32  __be_int32;
typedef	unsigned __int32  __be_uint32;
typedef	__int64  __be_int64;
typedef	unsigned __int64  __be_uint64;

typedef volatile __int8 	vint8;
typedef volatile unsigned __int8 	vuint8;
typedef volatile __int16 	vint16;
typedef volatile unsigned __int16 	vuint16;
typedef volatile __int32 	vint32;
typedef volatile unsigned __int32 	vuint32;
typedef volatile __int64 	vint64;
typedef volatile unsigned __int64 	vuint64;

typedef volatile long 	vlong;
typedef volatile unsigned long 	vulong;
typedef volatile int 	vint;
typedef volatile unsigned int 	vuint;
typedef volatile short 	vshort;
typedef volatile unsigned short vushort;
typedef volatile char 	vchar;
typedef volatile unsigned char 	vuchar;

typedef unsigned char 	uchar;
typedef unsigned short 	unichar;
typedef unsigned __int16 unichar16;
typedef unsigned __int32 unichar32;

typedef __int32 __haiku_status_t;
typedef __int32 __be_status_t;
typedef __int32 status_t;
// 	Represents one of the status codes defined in Error.h. More...
typedef __int64 __haiku_bigtime_t;
typedef __int64 __be_bigtime_t;
typedef __int64 bigtime_t;
// 	Represents time. The unit depends on the context of the function. More...
typedef __int64 __haiku_nanotime_t;
typedef __int64 __be_nanotime_t;
typedef __int64 nanotime_t;
// 	Represents time in nanoseconds.
typedef unsigned __int32 	__haiku_type_code;
typedef unsigned __int32 	__be_type_code;
typedef unsigned __int32 	type_code;
// 	Represents a certain type of data. See HaikuConstants.h for possible values. More...
typedef unsigned __int32 	__haiku_perform_code;
typedef unsigned __int32 	__be_perform_code;
typedef unsigned __int32 	perform_code;

typedef __haiku_phys_addr_t  gid_t;
typedef __haiku_phys_addr_t  uid_t;
typedef __haiku_phys_addr_t  uint;
typedef unsigned short ushort;

#ifndef __GNUC__
#include <wrappers/win32/pthreads/pthread.h>
#else
#include <pthread.h>
#endif

#undef NO_OLDNAMES
#ifndef _MSC_VER
//#include <sys/types.h>
#endif
// POSIX
#include <posix/sys/types.h>

typedef size_t __be_size_t;
#ifndef __clang__
    #ifndef _SSIZE_T_DEFINED
        typedef size_t ssize_t;
    #endif
#endif // __GNUC__
#endif	/* BHAPI_HAIKU_TYPES_H */
