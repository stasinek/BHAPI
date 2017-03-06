/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
 *
 * BHAPI++ library is a freeware; it may be used and distributed according to
 * the terms of The MIT License.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 * IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * File: SupportDefs.h
 * Description: Definition for macro and type
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_SUPPORT_DEFS_H
#define BHAPI_SUPPORT_DEFS_H

#ifndef UNICODE
#define UNICODE
#endif
# ifdef BHAPI_OS_WIN32
# undef WIN32
#define WIN32
#endif
# ifdef BHAPI_OS_LINUX
# undef LINUX
#define LINUX
#endif
# ifdef BHAPI_OS_MACOS
# undef MACOS
#define MACOS
#endif

#include "TypeConstants.h"

#include <../include/stdarg.h>
#include <../include/stddef.h>
#include <../include/stdlib.h>
#include <../include/stdio.h>
#include <../include/string.h> /* for bzero */
#include <../include/ctype.h>
#include <../include/math.h>
#include <../include/c++/algorithm>
#ifndef _MSC_VER
#include <../include/sys/cdefs.h>
#include <../include/sys/types.h>
#endif

#define 	max_c(a, b)   ((a)>(b)?(a):(b))
// 	Returns the maximum of values a and b. More...
#define 	min_c(a, b)   ((a)>(b)?(b):(a))
// 	Returns the minimum of the values a and b. More...
/* The size of a `int', as computed by sizeof. */
#define SIZEOF_INT 4
/* The size of a `long', as computed by sizeof. */
#define SIZEOF_LONG 4
/* The size of a `long long', as computed by sizeof. */
#if _MSC_VER > 0x4b0
    #define SIZEOF_LONG_LONG 8
#endif
/* The size of a `short', as computed by sizeof. */
#define SIZEOF_SHORT 2
/* The size of a `void *', as computed by sizeof. */
#define SIZEOF_VOID_P 4
/* The size of a `__int64', as computed by sizeof. */
#define SIZEOF___INT64 8

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
    STACK_DIRECTION > 0 => grows toward higher addresses
    STACK_DIRECTION < 0 => grows toward lower addresses
    STACK_DIRECTION = 0 => direction of growth unknown */
/* #undef STACK_DIRECTION */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1
/* Define to 1 if the X Window System is missing or not being used. */
#define X_DISPLAY_MISSING 1
/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */
/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
#define inline __inline
#endif
/* Define to `unsigned' if <sys/types.h> does not define. */
/* #undef size_t */
#define B_INT64_CONSTANT(x) (const __int64)(x)

#include <limits.h>
#define B_MININT32 INT_MIN
#define B_MAXUINT32 UINT_MAX
#define B_MAXINT32 INT_MAX
#ifdef _MSC_VER
#  define strcasecmp _stricmp
#endif /* _MSC_VER */
/* Long Long type and related issues */
#ifdef _MSC_VER /* MSC compiler */
#	ifndef LLONG_MAX
#		define LLONG_MAX    9223372036854775807I64
#	endif
#	ifndef LLONG_MIN
#		define LLONG_MIN    (-LLONG_MAX - 1I64)
#	endif
#	ifndef ULLONG_MAX
#		define ULLONG_MAX LLONG_MAX
#	endif
#	define B_MAXUINT64 ULLONG_MAX
#	define B_MAXINT64   LLONG_MAX
#	define B_MININT64   LLONG_MIN
#endif
#ifdef __GNUC__
#	define B_MAXUINT64 __LONG_LONG_MAX__
#	define B_MAXINT64  __LONG_LONG_MAX__
#	define B_MININT64  __LONG_LONG_MIN__
#else
#	ifndef B_MAXINT64
#	define B_MAXUINT64 ULLONG_MAX
#	define B_MAXINT64   LLONG_MAX
#	define B_MININT64   LLONG_MIN
#	endif // B_MAXINT64
#endif

#include <float.h>
#define B_MAXFLOAT FLT_MAX
#define B_MINFLOAT FLT_MIN
#ifndef SIZEOF_FLOAT
#	define SIZEOF_FLOAT 4
#endif
#ifndef SIZEOF_DOUBLE
#	define SIZEOF_DOUBLE 8
#endif

typedef unsigned __int32 b_unichar32;
typedef unsigned __int16 b_unichar16;

typedef __int64     b_thread_id;
typedef size_t      b_address_t;
typedef size_t      b_size_t;

/*
typedef	__int8 int8_t;
typedef	unsigned __int8 uint8_t;
typedef	__int16 int16_t;
typedef	unsigned __int16 uint16_t;
typedef	__int32 int32_t;
typedef	unsigned __int32 uint32_t;
typedef	__int64 int64_t;
typedef	unsigned __int64 uint64_t;
*/

typedef	__int8 int8;
typedef	unsigned __int8 uint8;
typedef	__int16 int16;
typedef	unsigned __int16 uint16;
typedef	__int32 int32;
typedef	unsigned __int32 uint32;
typedef	__int64 int64;
typedef	unsigned __int64 uint64;

typedef	__int8 b_int8;
typedef	unsigned __int8 b_uint8;
typedef	__int16 b_int16;
typedef	unsigned __int16 b_uint16;
typedef	__int32 b_int32;
typedef	unsigned __int32 b_uint32;
typedef	__int64 b_int64;
typedef	unsigned __int64 b_uint64;

typedef volatile __int8 	vint8;
typedef volatile unsigned __int8 	vuint8;
typedef volatile __int16 	vint16;
typedef volatile unsigned __int16 	vuint16;
typedef volatile __int32 	vint32;
typedef volatile unsigned __int32 	vuint32;
typedef volatile __int64 	vint64;
typedef volatile unsigned __int64 	vuint64;

typedef volatile long 	vlong;
typedef volatile int 	vint;
typedef volatile short 	vshort;
typedef volatile char 	vchar;
typedef volatile unsigned long 	vulong;
typedef volatile unsigned int 	vuint;
typedef volatile unsigned short 	vushort;
typedef volatile unsigned char 	vuchar;

typedef unsigned char 	uchar;
typedef unsigned short 	unichar;

typedef __int32     __haiku_status_t;
typedef __int32     b_status_t;
typedef __int32 	status_t;
// 	Represents one of the status codes defined in Error.h. More...
typedef __int64     __haiku_bigtime_t;
typedef __int64     b_bigtime_t;
typedef __int64 	bigtime_t;
// 	Represents time. The unit depends on the context of the function. More...
typedef __int64 	__haiku_nanotime_t;
typedef __int64 	b_nanotime_t;
typedef __int64 	nanotime_t;
// 	Represents time in nanoseconds.
typedef unsigned __int32 	__haiku_type_code;
typedef unsigned __int32 	b_type_code;
typedef unsigned __int32 	type_code;
// 	Represents a certain type of data. See TypeConstants.h for possible values. More...
typedef unsigned __int32 	__haiku_perform_code;
typedef unsigned __int32 	b_perform_code;
typedef unsigned __int32 	perform_code;
// 	Defined to support 'hidden' commands or extensions to classes. More..

#ifndef _SSIZE_T_DEFINED
typedef size_t ssize_t;
#endif

#ifdef _MSC_VER
typedef __int8  int8_t;
typedef unsigned __int8  uint8_t;
typedef __int16  int16_t;
typedef unsigned __int16  uint16_t;
typedef __int32  int32_t;
typedef unsigned __int32  uint32_t;
typedef __int64  int64_t;
typedef unsigned __int64  uint64_t;
#endif

#ifndef HAVE_BZERO
    #ifndef bzero
    #define bzero(ptr, len) memset(ptr, 0, len)
    #endif
#endif /* HAVE_BZERO */

#ifndef __cplusplus

typedef	b_int8	bool;

#ifndef false
    #define false (0)
#endif

#ifndef true
    #define true (!false)
#endif

#endif /* !__cplusplus */

#ifndef FALSE
    #define FALSE (0)
#endif
#ifndef TRUE
    #define TRUE (!FALSE)
#endif
#ifndef min_c
    #define min_c(a, b)  ((a) > (b) ? (b) : (a))
#endif
#ifndef max_c
    #define max_c(a, b)  ((a) > (b) ? (a) : (b))
#endif

#ifndef __cplusplus
    #ifndef min
    #define min(a, b) ((a) > (b) ? (b) : (a))
    #endif

    #ifndef max
    #define max(a, b) ((a) > (b) ? (a) : (b))
    #endif
#endif /* !__cplusplus */

#ifndef NULL
#  ifdef __cplusplus
#    define NULL        (0L)
#  else /* !__cplusplus */
#    define NULL        ((void*)0)
#  endif /* !__cplusplus */
#endif

#ifdef BHAPI_OS_WIN32
#	ifdef __GNUC__
#		ifndef _stdcall
#		define _stdcall  __attribute__((stdcall))
#		endif /* stdcall */
#	endif /* __GNUC__ */
#endif /* BHAPI_OS_WIN32 */

/* We prefix variable declarations so they can
 * properly get exported in windows dlls or Metrowerks'.
 */
#ifndef __EXPORT
#  if defined(BHAPI_OS_WIN32) || defined(BHAPI_OS_CYGWIN) || (defined(BHAPI_OS_BEOS) && defined(BHAPI_BIG_ENDIAN))
#    define __EXPORT __declspec(dllexport)
#  else
#    define __EXPORT
#  endif
#endif /* __EXPORT */

#ifndef __IMPORT
#  if defined(BHAPI_OS_WIN32) || defined(BHAPI_OS_CYGWIN) || (defined(BHAPI_OS_BEOS) && defined(BHAPI_BIG_ENDIAN))
#    define __IMPORT __declspec(dllimport)
#  else
#    define __IMPORT
#  endif
#endif /* __IMPORT */

#ifndef __LOCAL
#  if (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ >= 3 && __GNUC_PATCHLEVEL__ > 3) && !defined(__MINGW32__)
#    define __LOCAL __attribute__((visibility("hidden")))
#  else
#    define __LOCAL
#  endif
#endif /* __LOCAL */

#ifndef LOCALBHAPI
    #define LOCALBHAPI __LOCAL
#define BHAPI_LOCAL __LOCAL
#endif /* __LOCAL */

#ifdef BHAPI_BUILD_LIBRARY
    #define IMPEXPBHAPI __EXPORT
#define BHAPI_IMPEXP __EXPORT
#else /* !BHAPI_BUILD_LIBRARY */
    #define IMPEXPBHAPI __IMPORT
#define BHAPI_IMPEXP __IMPORT
#endif /* BHAPI_BUILD_LIBRARY */
#define EXPORTBHAPI __EXPORT
#define IMPORTBHAPI __IMPORT
#define BHAPI_EXPORT __EXPORT
#define BHAPI_IMPORT __IMPORT

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/*
extern IMPEXPBHAPI const b_uint8 bhapi::major_version;
extern IMPEXPBHAPI const b_uint8 bhapi::minor_version;
extern IMPEXPBHAPI const b_uint8 bhapi::micro_version;
extern IMPEXPBHAPI const b_uint8 bhapi::interface_age;
extern IMPEXPBHAPI const b_uint16 bhapi::binary_age;
*/
#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#if defined(__GNUC__) || defined(__clang__) || defined(__MINGW32__)
    #ifdef PostMessage
        #undef PostMessage
    #endif
    #ifdef SendMessage
        #undef SendMessage
    #endif
    #ifdef DispatchMessage
        #undef DispatchMessage
    #endif
    #ifdef CreateWindow
        #undef CreateWindow
    #endif
#endif

#ifdef BHAPI_OS_WIN32
    #ifdef _WIN32
        #include <winsock2.h>
        #include <windows.h>
    #endif
    #if defined(_MSC_VER) && _MSC_VER <= 0x4b0
        #define for	if (0); else for
    #endif
#endif /* BHAPI_OS_WIN32 */

#ifdef __cplusplus
#endif /* __cplusplus */

/* seek_mode */
enum {
    B_SEEK_SET = 0,
    B_SEEK_CUR,
    B_SEEK_END,
};

#define 	B_PRId8   "d"
#define 	B_PRIi8   "i"
#define 	B_PRId16   "d"
#define 	B_PRIi16   "i"
#define 	B_PRId32   _HAIKU_PRI_PREFIX_32 "d"
#define 	B_PRIi32   _HAIKU_PRI_PREFIX_32 "i"
#define 	B_PRId64   _HAIKU_PRI_PREFIX_64 "d"
#define 	B_PRIi64   _HAIKU_PRI_PREFIX_64 "i"
#define 	B_PRIu8   "u"
#define 	B_PRIo8   "o"
#define 	B_PRIx8   "x"
#define 	B_PRIX8   "X"
#define 	B_PRIu16   "u"
#define 	B_PRIo16   "o"
#define 	B_PRIx16   "x"
#define 	B_PRIX16   "X"
#define 	B_PRIu32   _HAIKU_PRI_PREFIX_32 "u"
#define 	B_PRIo32   _HAIKU_PRI_PREFIX_32 "o"
#define 	B_PRIx32   _HAIKU_PRI_PREFIX_32 "x"
#define 	B_PRIX32   _HAIKU_PRI_PREFIX_32 "X"
#define 	B_PRIu64   _HAIKU_PRI_PREFIX_64 "u"
#define 	B_PRIo64   _HAIKU_PRI_PREFIX_64 "o"
#define 	B_PRIx64   _HAIKU_PRI_PREFIX_64 "x"
#define 	B_PRIX64   _HAIKU_PRI_PREFIX_64 "X"
#define 	B_SCNd8   "hhd"
#define 	B_SCNi8   "hhi"
#define 	B_SCNd16   "hd"
#define 	B_SCNi16   "hi"
#define 	B_SCNd32   _HAIKU_PRI_PREFIX_32 "d"
#define 	B_SCNi32   _HAIKU_PRI_PREFIX_32 "i"
#define 	B_SCNd64   _HAIKU_PRI_PREFIX_64 "d"
#define 	B_SCNi64   _HAIKU_PRI_PREFIX_64 "i"
#define 	B_SCNu8   "hhu"
#define 	B_SCNo8   "hho"
#define 	B_SCNx8   "hhx"
#define 	B_SCNu16   "hu"
#define 	B_SCNo16   "ho"
#define 	B_SCNx16   "hx"
#define 	B_SCNu32   _HAIKU_PRI_PREFIX_32 "u"
#define 	B_SCNo32   _HAIKU_PRI_PREFIX_32 "o"
#define 	B_SCNx32   _HAIKU_PRI_PREFIX_32 "x"
#define 	B_SCNu64   _HAIKU_PRI_PREFIX_64 "u"
#define 	B_SCNo64   _HAIKU_PRI_PREFIX_64 "o"
#define 	B_SCNx64   _HAIKU_PRI_PREFIX_64 "x"
//
#define 	B_PRIuSIZE   _HAIKU_PRI_PREFIX_ADDR "u"
#define 	B_PRIoSIZE   _HAIKU_PRI_PREFIX_ADDR "o"
#define 	B_PRIxSIZE   _HAIKU_PRI_PREFIX_ADDR "x"
#define 	B_PRIXSIZE   _HAIKU_PRI_PREFIX_ADDR "X"
#define 	B_PRIdSSIZE   _HAIKU_PRI_PREFIX_ADDR "d"
#define 	B_PRIiSSIZE   _HAIKU_PRI_PREFIX_ADDR "i"
#define 	B_PRIuADDR   _HAIKU_PRI_PREFIX_ADDR "u"
#define 	B_PRIoADDR   _HAIKU_PRI_PREFIX_ADDR "o"
#define 	B_PRIxADDR   _HAIKU_PRI_PREFIX_ADDR "x"
#define 	B_PRIXADDR   _HAIKU_PRI_PREFIX_ADDR "X"
#define 	B_PRIdOFF       B_PRId64
#define 	B_PRIiOFF       B_PRIi64
#define 	B_PRIdDEV       B_PRId32
#define 	B_PRIiDEV       B_PRIi32
#define 	B_PRIdINO       B_PRId64
#define 	B_PRIiINO       B_PRIi64
#define 	B_PRIdTIME      B_PRId32
#define 	B_PRIiTIME      B_PRIi32

#ifndef BHAPI_DEBUG_H
//#include "../kernel/Debug.h"
#endif

#ifdef __cplusplus
namespace bhapi {
#endif /* __cplusplus */
IMPEXPBHAPI int32 	atomic_add (int32 *value, int32 addValue);
// 	Atomically add the value of addValue to value. More...
IMPEXPBHAPI int64 	atomic_add64 (int64 *value, int64 addValue);
// 	Atomically add the value of addValue to value. More...
IMPEXPBHAPI int32 	atomic_and (int32 *value, int32 andValue);
// 	Atomically perform a bitwise AND operation of andValue to the variable andValue. More...
IMPEXPBHAPI int64 	atomic_and64 (int64 *value, int64 andValue);
// 	Atomically perform a bitwise AND operation of andValue to the variable andValue. More...
IMPEXPBHAPI int32 	atomic_get (int32 *value);
// 	Atomically return the value of value. More...
IMPEXPBHAPI int64 	atomic_get64 (int64 *value);
// 	Atomically return the value of value. More...
IMPEXPBHAPI int32 	atomic_get_and_set (int32 *value, int32 newValue);
// 	Atomically set the variable value to newvalue and return the old value. More...
IMPEXPBHAPI int64 	atomic_get_and_set64 (int64 *value, int64 newValue);
// 	Atomically set the variable value to newvalue and return the old value. More...
IMPEXPBHAPI int32 	atomic_or (int32 *value, int32 orValue);
// 	Atomically perform a bitwise OR operation of orValue to the variable andValue. More...
IMPEXPBHAPI int64 	atomic_or64 (int64 *value, int64 orValue);
// 	Atomically perform a bitwise OR operation of orValue to the variable andValue. More...
IMPEXPBHAPI void 	atomic_set (int32 *value, int32 newValue);
// 	Atomically set the variable value to newvalue. More...
IMPEXPBHAPI void 	atomic_set64 (int64 *value, int64 newValue);
// 	Atomically set the variable value to newvalue. More...
IMPEXPBHAPI int32 	atomic_test_and_set (int32 *value, int32 newValue, int32 testAgainst);
// 	Atomically set the variable value to newValue if the current value is testAgainst. More...
IMPEXPBHAPI int64 	atomic_test_and_set64 (int64 *value, int64 newValue, int64 testAgainst);
// 	Atomically set the variable value to newValue if the current value is testAgainst. More...
IMPEXPBHAPI void * 	get_stack_frame (void);
// 	Internal function.
#ifdef __cplusplus
} /* namespace */
#endif /* __cplusplus */

extern "C" const char* B_EMPTY_STRING;

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

#endif /* BHAPI_SUPPORT_DEFS_H */

