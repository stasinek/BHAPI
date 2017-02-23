/* --------------------------------------------------------------------------
 *
 * BHAPI++ previously named ETK++, The Easy Toolkit for C++ programing
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

#ifndef BHAPI_SUPPORT_DEFS__H
#define BHAPI_SUPPORT_DEFS__H

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

#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h> /* for bzero */
#include <math.h>
#include <stdio.h>
#include <ctype.h>

/* The size of a `float', as computed by sizeof. */
#define SIZEOF_FLOAT 4

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
    #define B_MAXUINT64 ULLONG_MAX
    #define B_MAXINT64   LLONG_MAX
    #define B_MININT64   LLONG_MIN
#endif
#ifdef __GNUC__
    #define B_MAXUINT64 __LONG_LONG_MAX__
    #define B_MAXINT64  __LONG_LONG_MAX__
    #define B_MININT64  __LONG_LONG_MIN__
#else
    #ifndef B_MAXINT64
    #define B_MAXUINT64 ULLONG_MAX
    #define B_MAXINT64   LLONG_MAX
    #define B_MININT64   LLONG_MIN
    #endif // B_MAXINT64
#endif


#include <float.h>
#define B_MAXFLOAT FLT_MAX
#define B_MINFLOAT FLT_MIN

#ifndef SIZEOF_FLOAT
    #define SIZEOF_FLOAT 4
#endif

#ifndef SIZEOF_DOUBLE
    #define SIZEOF_DOUBLE 8
#endif

typedef unsigned __int32 b_unichar32;
typedef unsigned __int16 b_unichar16;

typedef	__int8 b_int8;
typedef	unsigned __int8 b_uint8;
typedef	__int16 b_int16;
typedef	unsigned __int16 b_uint16;
typedef	__int32 b_int32;
typedef	unsigned __int32 b_uint32;
typedef	__int64 b_int64;
typedef	unsigned __int64 b_uint64;

typedef b_int32	b_status_t;
typedef b_uint32	b_type_code;
typedef b_uint32	b_perform_code;
typedef b_int64	b_bigtime_t;
typedef b_int64	b_thread_id;
typedef size_t	b_address_t;
typedef size_t	b_size_t;

enum {
    B_ANY_TYPE                  = 'ANYT',
    B_BOOL_TYPE 				= 'BOOL',
    B_CHAR_TYPE 				= 'CHAR',
    B_DOUBLE_TYPE 				= 'DBLE',
    B_FLOAT_TYPE 				= 'FLOT',
    B_INT64_TYPE 				= 'LLNG',
    B_INT32_TYPE 				= 'LONG',
    B_INT16_TYPE 				= 'SHRT',
    B_INT8_TYPE 				= 'BYTE',
    B_MESSAGE_TYPE				= 'MSGG',
    B_MESSENGER_TYPE			= 'MSNG',
    B_POINTER_TYPE				= 'PNTR',
    B_SIZE_T_TYPE	 			= 'SIZT',
    B_b_size_t_TYPE	 			= 'SSZT',
    B_STRING_TYPE 				= 'CSTR',
    B_UINT64_TYPE				= 'ULLG',
    B_UINT32_TYPE				= 'ULNG',
    B_UINT16_TYPE 				= 'USHT',
    B_UINT8_TYPE 				= 'UBYT',
    B_POINT_TYPE				= 'SPNT',
    B_RECT_TYPE                 = 'RECT',
    B_MIME_TYPE                 = 'MIME',
    B_UNKNOWN_TYPE				= 'UNKN'
};

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
#ifndef _EXPORT
#  if defined(BHAPI_OS_WIN32) || defined(BHAPI_OS_CYGWIN) || (defined(BHAPI_OS_BEOS) && defined(BHAPI_BIG_ENDIAN))
#    define _EXPORT __declspec(dllexport)
#  else
#    define _EXPORT
#  endif
#endif /* _EXPORT */

#ifndef _IMPORT
#  if defined(BHAPI_OS_WIN32) || defined(BHAPI_OS_CYGWIN) || (defined(BHAPI_OS_BEOS) && defined(BHAPI_BIG_ENDIAN))
#    define _IMPORT __declspec(dllimport)
#  else
#    define _IMPORT
#  endif
#endif /* _IMPORT */

#ifndef _LOCAL
#  if (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ >= 3 && __GNUC_PATCHLEVEL__ > 3) && !defined(__MINGW32__)
#    define _LOCAL __attribute__((visibility("hidden")))
#  else
#    define _LOCAL
#  endif
#endif /* _LOCAL */

#ifdef BHAPI_BUILD_LIBRARY
    #define IMPEXP_BHAPI _EXPORT
#else /* !BHAPI_BUILD_LIBRARY */
    #define IMPEXP_BHAPI _IMPORT
#endif /* BHAPI_BUILD_LIBRARY */
#define EXP_BHAPI _EXPORT

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/*
extern IMPEXP_BHAPI const b_uint8 bhapi_major_version;
extern IMPEXP_BHAPI const b_uint8 bhapi_minor_version;
extern IMPEXP_BHAPI const b_uint8 bhapi_micro_version;
extern IMPEXP_BHAPI const b_uint8 bhapi_interface_age;
extern IMPEXP_BHAPI const b_uint16 bhapi_binary_age;
*/
#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#ifdef __cplusplus

#ifdef BHAPI_OS_WIN32
    #ifdef _WIN32
        #include <winsock2.h>
        #include <windows.h>
	#endif
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

	#if defined(_MSC_VER) && _MSC_VER <= 0x4b0
		#define for	if (0); else for
	#endif
#endif /* BHAPI_OS_WIN32 */

#endif /* __cplusplus */

/* seek_mode */
enum {
    B_SEEK_SET = 0,
    B_SEEK_CUR,
    B_SEEK_END,
};

#ifndef BHAPI_DEBUG__H
//#include "../kernel/Debug.h"
#endif

#endif /* BHAPI_SUPPORT_DEFS__H */

