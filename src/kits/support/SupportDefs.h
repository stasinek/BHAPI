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
#include <math.h>
#define 	max_c(a, b)   ((a)>(b)?(a):(b))
// 	Returns the maximum of values a and b. More...
#define 	min_c(a, b)   ((a)>(b)?(b):(a))
// 	Returns the minimum of the values a and b. More...
#ifndef __cplusplus
#define inline __inline
#endif
/* Define to `unsigned' if <sys/types.h> does not define. */
/* #undef size_t */
#define B_INT64_CONSTANT(x) (const __int64)(x)

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

// 	Defined to support 'hidden' commands or extensions to classes. More..
#include <string.h> /* for bzero */
#ifndef HAVE_BZERO
    #ifndef bzero
    #define bzero(ptr, len) memset(ptr, 0, len)
    #endif
#endif /* HAVE_BZERO */

#ifndef __cplusplus

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

extern "C" const char* B_EMPTY_STRING;

#endif /* BHAPI_SUPPORT_DEFS_H */

