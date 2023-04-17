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
#include <Haiku.h>
//-------------------------------------------------------------------------------------------------
#ifndef __cplusplus
#define inline __inline
#endif
/* Define to `unsigned' if <sys/types.h> does not define. */
/* #undef size_t */
#define B_INT64_CONSTANT(x) (const __int64)(x)
//-------------------------------------------------------------------------------------------------
#ifdef _MSC_VER
#  define strcasecmp _stricmp
#endif /* _MSC_VER */
/* Long Long type and related issues */
#include <float.h>
#define B_MAXFLOAT FLT_MAX
#define B_MINFLOAT FLT_MIN
#ifndef SIZEOF_FLOAT
#	define SIZEOF_FLOAT 4
#endif
#ifndef SIZEOF_DOUBLE
#	define SIZEOF_DOUBLE 8
#endif
//-------------------------------------------------------------------------------------------------
// 	Defined to support 'hidden' commands or extensions to classes. More..
 /* for bzero */
#ifndef HAVE_BZERO
    #ifndef bzero
    #define bzero(ptr, len) memset(ptr, 0, len)
    #endif
#endif /* HAVE_BZERO */
//-------------------------------------------------------------------------------------------------
#ifndef __cplusplus
#ifndef false
    #define false (0)
#endif
#ifndef true
    #define true (!false)
#endif
#endif /* !__cplusplus */
//-------------------------------------------------------------------------------------------------
#ifndef FALSE
    #define FALSE (0)
#endif
#ifndef TRUE
    #define TRUE (!FALSE)
#endif
//-------------------------------------------------------------------------------------------------
#include <math.h>
#ifndef min_c
#define min_c(a, b)  ((a) > (b) ? (b) : (a))
#endif
#ifndef min
#define min(a, b) ((a) > (b) ? (b) : (a))
#endif
#ifndef MIN
#define MIN(a, b) min(a,b)
#endif
#ifndef max_c
#define max_c(a, b)  ((a) > (b) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a, b) max(a,b)
#endif
//-------------------------------------------------------------------------------------------------
#ifndef NULL
#  ifdef __cplusplus
#    define NULL        (0L)
#  else /* !__cplusplus */
#    define NULL        ((void*)0)
#  endif /* !__cplusplus */
#endif
//-------------------------------------------------------------------------------------------------
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
//-------------------------------------------------------------------------------------------------
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
#define 	B_PRId32   __HAIKU_PRI_PREFIX_32 "d"
#define 	B_PRIi32   __HAIKU_PRI_PREFIX_32 "i"
#define 	B_PRId64   __HAIKU_PRI_PREFIX_64 "d"
#define 	B_PRIi64   __HAIKU_PRI_PREFIX_64 "i"
#define 	B_PRIu8   "u"
#define 	B_PRIo8   "o"
#define 	B_PRIx8   "x"
#define 	B_PRIX8   "X"
#define 	B_PRIu16   "u"
#define 	B_PRIo16   "o"
#define 	B_PRIx16   "x"
#define 	B_PRIX16   "X"
#define 	B_PRIu32   __HAIKU_PRI_PREFIX_32 "u"
#define 	B_PRIo32   __HAIKU_PRI_PREFIX_32 "o"
#define 	B_PRIx32   __HAIKU_PRI_PREFIX_32 "x"
#define 	B_PRIX32   __HAIKU_PRI_PREFIX_32 "X"
#define 	B_PRIu64   __HAIKU_PRI_PREFIX_64 "u"
#define 	B_PRIo64   __HAIKU_PRI_PREFIX_64 "o"
#define 	B_PRIx64   __HAIKU_PRI_PREFIX_64 "x"
#define 	B_PRIX64   __HAIKU_PRI_PREFIX_64 "X"
#define 	B_SCNd8   "hhd"
#define 	B_SCNi8   "hhi"
#define 	B_SCNd16   "hd"
#define 	B_SCNi16   "hi"
#define 	B_SCNd32   __HAIKU_PRI_PREFIX_32 "d"
#define 	B_SCNi32   __HAIKU_PRI_PREFIX_32 "i"
#define 	B_SCNd64   __HAIKU_PRI_PREFIX_64 "d"
#define 	B_SCNi64   __HAIKU_PRI_PREFIX_64 "i"
#define 	B_SCNu8   "hhu"
#define 	B_SCNo8   "hho"
#define 	B_SCNx8   "hhx"
#define 	B_SCNu16   "hu"
#define 	B_SCNo16   "ho"
#define 	B_SCNx16   "hx"
#define 	B_SCNu32   __HAIKU_PRI_PREFIX_32 "u"
#define 	B_SCNo32   __HAIKU_PRI_PREFIX_32 "o"
#define 	B_SCNx32   __HAIKU_PRI_PREFIX_32 "x"
#define 	B_SCNu64   __HAIKU_PRI_PREFIX_64 "u"
#define 	B_SCNo64   __HAIKU_PRI_PREFIX_64 "o"
#define 	B_SCNx64   __HAIKU_PRI_PREFIX_64 "x"
//
#define 	B_PRIuSIZE   __HAIKU_PRI_PREFIX_ADDR "u"
#define 	B_PRIoSIZE   __HAIKU_PRI_PREFIX_ADDR "o"
#define 	B_PRIxSIZE   __HAIKU_PRI_PREFIX_ADDR "x"
#define 	B_PRIXSIZE   __HAIKU_PRI_PREFIX_ADDR "X"
#define 	B_PRIdSSIZE  __HAIKU_PRI_PREFIX_ADDR "d"
#define 	B_PRIiSSIZE  __HAIKU_PRI_PREFIX_ADDR "i"
#define 	B_PRIuADDR   __HAIKU_PRI_PREFIX_ADDR "u"
#define 	B_PRIoADDR   __HAIKU_PRI_PREFIX_ADDR "o"
#define 	B_PRIxADDR   __HAIKU_PRI_PREFIX_ADDR "x"
#define 	B_PRIXADDR   __HAIKU_PRI_PREFIX_ADDR "X"
#define 	B_PRIdOFF       B_PRId64
#define 	B_PRIiOFF       B_PRIi64
#define 	B_PRIdDEV       B_PRId32
#define 	B_PRIiDEV       B_PRIi32
#define 	B_PRIdINO       B_PRId64
#define 	B_PRIiINO       B_PRIi64
#define 	B_PRIdTIME      B_PRId32
#define 	B_PRIiTIME      B_PRIi32
//-------------------------------------------------------------------------------------------------
/* printf()/scanf() format strings for [u]int* types */
#define B_PRId8			"d"
#define B_PRIi8			"i"
#define B_PRId16		"d"
#define B_PRIi16		"i"
#define B_PRId32		__HAIKU_PRI_PREFIX_32 "d"
#define B_PRIi32		__HAIKU_PRI_PREFIX_32 "i"
#define B_PRId64		__HAIKU_PRI_PREFIX_64 "d"
#define B_PRIi64		__HAIKU_PRI_PREFIX_64 "i"
#define B_PRIu8			"u"
#define B_PRIo8			"o"
#define B_PRIx8			"x"
#define B_PRIX8			"X"
#define B_PRIu16		"u"
#define B_PRIo16		"o"
#define B_PRIx16		"x"
#define B_PRIX16		"X"
#define B_PRIu32		__HAIKU_PRI_PREFIX_32 "u"
#define B_PRIo32		__HAIKU_PRI_PREFIX_32 "o"
#define B_PRIx32		__HAIKU_PRI_PREFIX_32 "x"
#define B_PRIX32		__HAIKU_PRI_PREFIX_32 "X"
#define B_PRIu64		__HAIKU_PRI_PREFIX_64 "u"
#define B_PRIo64		__HAIKU_PRI_PREFIX_64 "o"
#define B_PRIx64		__HAIKU_PRI_PREFIX_64 "x"
#define B_PRIX64		__HAIKU_PRI_PREFIX_64 "X"

#define B_SCNd8			"hhd"
#define B_SCNi8			"hhi"
#define B_SCNd16		"hd"
#define B_SCNi16		"hi"
#define B_SCNd32		__HAIKU_PRI_PREFIX_32 "d"
#define B_SCNi32		__HAIKU_PRI_PREFIX_32 "i"
#define B_SCNd64		__HAIKU_PRI_PREFIX_64 "d"
#define B_SCNi64		__HAIKU_PRI_PREFIX_64 "i"
#define B_SCNu8			"hhu"
#define B_SCNo8			"hho"
#define B_SCNx8			"hhx"
#define B_SCNu16		"hu"
#define B_SCNo16		"ho"
#define B_SCNx16		"hx"
#define B_SCNu32		__HAIKU_PRI_PREFIX_32 "u"
#define B_SCNo32		__HAIKU_PRI_PREFIX_32 "o"
#define B_SCNx32		__HAIKU_PRI_PREFIX_32 "x"
#define B_SCNu64		__HAIKU_PRI_PREFIX_64 "u"
#define B_SCNo64		__HAIKU_PRI_PREFIX_64 "o"
#define B_SCNx64		__HAIKU_PRI_PREFIX_64 "x"

/* printf()/scanf() format strings for some standard types */
//-------------------------------------------------------------------------------------------------
/* size_t */
#define B_PRIuSIZE		__HAIKU_PRI_PREFIX_ADDR "u"
#define B_PRIoSIZE		__HAIKU_PRI_PREFIX_ADDR "o"
#define B_PRIxSIZE		__HAIKU_PRI_PREFIX_ADDR "x"
#define B_PRIXSIZE		__HAIKU_PRI_PREFIX_ADDR "X"

#define B_SCNuSIZE		__HAIKU_PRI_PREFIX_ADDR "u"
#define B_SCNoSIZE		__HAIKU_PRI_PREFIX_ADDR "o"
#define B_SCNxSIZE		__HAIKU_PRI_PREFIX_ADDR "x"
//-------------------------------------------------------------------------------------------------
/* ssize_t */
#define B_PRIdSSIZE		__HAIKU_PRI_PREFIX_ADDR "d"
#define B_PRIiSSIZE		__HAIKU_PRI_PREFIX_ADDR "i"

#define B_SCNdSSIZE		__HAIKU_PRI_PREFIX_ADDR "d"
#define B_SCNiSSIZE		__HAIKU_PRI_PREFIX_ADDR "i"
//-------------------------------------------------------------------------------------------------
/* addr_t */
#define B_PRIuADDR		__HAIKU_PRI_PREFIX_ADDR "u"
#define B_PRIoADDR		__HAIKU_PRI_PREFIX_ADDR "o"
#define B_PRIxADDR		__HAIKU_PRI_PREFIX_ADDR "x"
#define B_PRIXADDR		__HAIKU_PRI_PREFIX_ADDR "X"

#define B_SCNuADDR		__HAIKU_PRI_PREFIX_ADDR "u"
#define B_SCNoADDR		__HAIKU_PRI_PREFIX_ADDR "o"
#define B_SCNxADDR		__HAIKU_PRI_PREFIX_ADDR "x"
//-------------------------------------------------------------------------------------------------
/* phys_addr_t */
#define B_PRIuPHYSADDR	__HAIKU_PRI_PREFIX_PHYS_ADDR "u"
#define B_PRIoPHYSADDR	__HAIKU_PRI_PREFIX_PHYS_ADDR "o"
#define B_PRIxPHYSADDR	__HAIKU_PRI_PREFIX_PHYS_ADDR "x"
#define B_PRIXPHYSADDR	__HAIKU_PRI_PREFIX_PHYS_ADDR "X"

#define B_SCNuPHYSADDR	__HAIKU_PRI_PREFIX_PHYS_ADDR "u"
#define B_SCNoPHYSADDR	__HAIKU_PRI_PREFIX_PHYS_ADDR "o"
#define B_SCNxPHYSADDR	__HAIKU_PRI_PREFIX_PHYS_ADDR "x"
//-------------------------------------------------------------------------------------------------
/* generic_addr_t */
#define B_PRIuGENADDR	__HAIKU_PRI_PREFIX_GENERIC_ADDR "u"
#define B_PRIoGENADDR	__HAIKU_PRI_PREFIX_GENERIC_ADDR "o"
#define B_PRIxGENADDR	__HAIKU_PRI_PREFIX_GENERIC_ADDR "x"
#define B_PRIXGENADDR	__HAIKU_PRI_PREFIX_GENERIC_ADDR "X"

#define B_SCNuGENADDR	__HAIKU_PRI_PREFIX_GENERIC_ADDR "u"
#define B_SCNoGENADDR	__HAIKU_PRI_PREFIX_GENERIC_ADDR "o"
#define B_SCNxGENADDR	__HAIKU_PRI_PREFIX_GENERIC_ADDR "x"
//-------------------------------------------------------------------------------------------------
/* off_t */
#define B_PRIdOFF		B_PRId64
#define B_PRIiOFF		B_PRIi64
#define B_PRIxOFF		B_PRIx64

#define B_SCNdOFF		B_SCNd64
#define B_SCNiOFF		B_SCNi64
#define B_SCNxOFF		B_SCNx64
//-------------------------------------------------------------------------------------------------
/* dev_t */
#define B_PRIdDEV		B_PRId32
#define B_PRIiDEV		B_PRIi32
//-------------------------------------------------------------------------------------------------
/* ino_t */
#define B_PRIdINO		B_PRId64
#define B_PRIiINO		B_PRIi64
//-------------------------------------------------------------------------------------------------
/* time_t */
#define B_PRIdTIME		B_PRId32
#define B_PRIiTIME		B_PRIi32
//-------------------------------------------------------------------------------------------------
/* bigtime_t */
#define B_PRIdBIGTIME	B_PRId64
#define B_PRIiBIGTIME	B_PRIi64
//-------------------------------------------------------------------------------------------------
/* Printed width of a pointer with the %p format (minus 0x prefix). */
#ifdef B_HAIKU_64_BIT
#	define B_PRINTF_POINTER_WIDTH	16
#else
#	define B_PRINTF_POINTER_WIDTH	8
#endif
//-------------------------------------------------------------------------------------------------
/* Empty string ("") */
#ifdef __cplusplus
extern "C" const char* B_EMPTY_STRING;
#endif
//-------------------------------------------------------------------------------------------------
/* Grandfathering */
#ifndef __cplusplus
#	include <stdbool.h>
#endif
//-------------------------------------------------------------------------------------------------
/* Count items in an array, count_of is a common define */
#define B_COUNT_OF(a) (sizeof(a) / sizeof(a[0]))
/* Obsolete or discouraged API */
//-------------------------------------------------------------------------------------------------
/* Use the built-in atomic functions, if requested and available. */
//-------------------------------------------------------------------------------------------------
#ifdef __cplusplus
namespace bhapi {
#endif
//-------------------------------------------------------------------------------------------------
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7) || defined(__clang__)
static __inline__ void atomic_set(int32_t_t* value, int32_t_t newValue)
{
    __atomic_store_n(value, newValue, __ATOMIC_RELEASE);
}
//-------------------------------------------------------------------------------------------------
static __inline__ int32_t_t atomic_get_and_set(int32_t_t* value, int32_t_t newValue)
{
    return __atomic_exchange_n(value, newValue, __ATOMIC_SEQ_CST);
}
//-------------------------------------------------------------------------------------------------
static __inline__ int32_t_t atomic_test_and_set(int32_t_t* value, int32_t_t newValue, int32_t_t testAgainst)
{
    __atomic_compare_exchange_n(value, &testAgainst, newValue, 1,
        __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return testAgainst;
}
//-------------------------------------------------------------------------------------------------
static __inline__ int32_t_t atomic_add(int32_t_t* value, int32_t_t addValue)
{
    return __atomic_fetch_add(value, addValue, __ATOMIC_SEQ_CST);
}
//-------------------------------------------------------------------------------------------------
static __inline__ int32_t_t atomic_and(int32_t_t* value, int32_t_t andValue)
{
    return __atomic_fetch_and(value, andValue, __ATOMIC_SEQ_CST);
}
//-------------------------------------------------------------------------------------------------
static __inline__ int32_t_t atomic_or(int32_t_t* value, int32_t_t orValue)
{
    return __atomic_fetch_or(value, orValue, __ATOMIC_SEQ_CST);
}
//-------------------------------------------------------------------------------------------------
static __inline__ int32_t_t atomic_get(int32_t_t* value)
{
    return __atomic_load_n(value, __ATOMIC_ACQUIRE);
}
//-------------------------------------------------------------------------------------------------
static __inline__ void atomic_set64(int64_t* value, int64_t newValue)
{
    __atomic_store_n(value, newValue, __ATOMIC_RELEASE);
}
//-------------------------------------------------------------------------------------------------
static __inline__ int64_t atomic_get_and_set64(int64_t* value, int64_t newValue)
{
    return __atomic_exchange_n(value, newValue, __ATOMIC_SEQ_CST);
}
//-------------------------------------------------------------------------------------------------
static __inline__ int64_t atomic_test_and_set64(int64_t* value, int64_t newValue, int64_t testAgainst)
{
    __atomic_compare_exchange_n(value, &testAgainst, newValue, 1,
        __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return testAgainst;
}
//-------------------------------------------------------------------------------------------------
static __inline__ int64_t atomic_add64(int64_t* value, int64_t addValue)
{
    return __atomic_fetch_add(value, addValue, __ATOMIC_SEQ_CST);
}
//-------------------------------------------------------------------------------------------------
static __inline__ int64_t atomic_and64(int64_t* value, int64_t andValue)
{
    return __atomic_fetch_and(value, andValue, __ATOMIC_SEQ_CST);
}
//-------------------------------------------------------------------------------------------------
static __inline__ int64_t atomic_or64(int64_t* value, int64_t orValue)
{
    return __atomic_fetch_or(value, orValue, __ATOMIC_SEQ_CST);
}
//-------------------------------------------------------------------------------------------------
static __inline__ int64_t atomic_get64(int64_t* value)
{
    return __atomic_load_n(value, __ATOMIC_ACQUIRE);
}
#else	/* __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7) */
//-------------------------------------------------------------------------------------------------
/* Atomic functions; previous value is returned */
extern void		atomic_set(int32_t_t* value, int32_t_t newValue);
extern int32_t_t	atomic_get_and_set(int32_t_t* value, int32_t_t newValue);
extern int32_t_t	atomic_test_and_set(int32_t_t *value, int32_t_t newValue, int32_t_t testAgainst);
extern int32_t_t	atomic_add(int32_t_t *value, int32_t_t addValue);
extern int32_t_t	atomic_and(int32_t_t *value, int32_t_t andValue);
extern int32_t_t	atomic_or(int32_t_t *value, int32_t_t orValue);
extern int32_t_t	atomic_get(int32_t_t *value);

extern void		atomic_set64(int64_t* value, int64_t newValue);
extern int64_t	atomic_get_and_set64(int64_t* value, int64_t newValue);
extern int64_t	atomic_test_and_set64(int64_t *value, int64_t newValue, int64_t testAgainst);
extern int64_t	atomic_add64(int64_t *value, int64_t addValue);
extern int64_t	atomic_and64(int64_t *value, int64_t andValue);
extern int64_t	atomic_or64(int64_t *value, int64_t orValue);
extern int64_t	atomic_get64(int64_t *value);
//-------------------------------------------------------------------------------------------------
extern void*	get_stack_frame(void);
//-------------------------------------------------------------------------------------------------
#endif // GCC
#ifdef __cplusplus
}
#endif // namespace bhapi
#endif /* BHAPI_SUPPORT_DEFS_H */
//-------------------------------------------------------------------------------------------------
