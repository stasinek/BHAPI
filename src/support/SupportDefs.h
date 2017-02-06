/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
 *
 * ETK++ library is a freeware; it may be used and distributed according to
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

#ifndef __ETK_SUPPORT_DEFS_H__
#define __ETK_SUPPORT_DEFS_H__

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

#define ETK_COMPILATION
#ifdef _WIN32
#define ETK_OS_WIN32
#endif

#include <string.h> /* for bzero */
#include "Errors.h"

#define E_INT64_CONSTANT(x) (const __int64)(x)

#include <limits.h>
#define E_MININT32 INT_MIN
#define E_MAXUINT32 UINT_MAX
#define E_MAXINT32 INT_MAX
#define E_MAXUINT64 ULLONG_MAX
#define E_MAXINT64 LLONG_MAX
#include <float.h>
#define E_MAXFLOAT FLT_MAX
#define E_MINFLOAT FLT_MIN

#ifndef SIZEOF_FLOAT
#define SIZEOF_FLOAT 4
#endif

#ifndef SIZEOF_DOUBLE
#define SIZEOF_DOUBLE 8
#endif

typedef __int32 eunichar32;
typedef __int16 eunichar;

typedef	__int8 eint8;
typedef	unsigned __int8 euint8;
typedef	__int16 eint16;
typedef	unsigned __int16 euint16;
typedef	__int32 eint32;
typedef	unsigned __int32 euint32;
typedef	__int64 eint64;
typedef	unsigned __int64 euint64;

typedef eint32	e_status_t;
typedef euint32	e_type_code;
typedef euint32	e_perform_code;
typedef eint64	e_bigtime_t;
typedef eint64	e_thread_id;
typedef size_t	e_address_t;

enum {
    E_ANY_TYPE                  = 'ANYT',
	E_BOOL_TYPE 				= 'BOOL',
	E_CHAR_TYPE 				= 'CHAR',
	E_DOUBLE_TYPE 				= 'DBLE',
	E_FLOAT_TYPE 				= 'FLOT',
	E_INT64_TYPE 				= 'LLNG',
	E_INT32_TYPE 				= 'LONG',
	E_INT16_TYPE 				= 'SHRT',
	E_INT8_TYPE 				= 'BYTE',
	E_MESSAGE_TYPE				= 'MSGG',
	E_MESSENGER_TYPE			= 'MSNG',
	E_POINTER_TYPE				= 'PNTR',
	E_SIZE_T_TYPE	 			= 'SIZT',
	E_SSIZE_T_TYPE	 			= 'SSZT',
	E_STRING_TYPE 				= 'CSTR',
	E_UINT64_TYPE				= 'ULLG',
	E_UINT32_TYPE				= 'ULNG',
	E_UINT16_TYPE 				= 'USHT',
	E_UINT8_TYPE 				= 'UBYT',
	E_POINT_TYPE				= 'SPNT',
    E_RECT_TYPE                 = 'RECT',
    E_MIME_TYPE                 = 'MIME',
	E_UNKNOWN_TYPE				= 'UNKN'
};

#ifndef HAVE_BZERO
	#ifndef bzero
	#define bzero(ptr, len) memset(ptr, 0, len)
	#endif
#endif /* HAVE_BZERO */

#ifndef __cplusplus

typedef	eint8	bool;

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

#ifdef ETK_OS_WIN32
#	ifdef __GNUC__
#		ifndef _stdcall
#		define _stdcall  __attribute__((stdcall))
#		endif /* stdcall */
#	endif /* __GNUC__ */
#endif /* ETK_OS_WIN32 */

/* We prefix variable declarations so they can
 * properly get exported in windows dlls or Metrowerks'.
 */
#ifndef _EXPORT
#  if defined(ETK_OS_WIN32) || defined(ETK_OS_CYGWIN) || (defined(ETK_OS_BEOS) && defined(ETK_BIG_ENDIAN))
#    define _EXPORT __declspec(dllexport)
#  else
#    define _EXPORT
#  endif
#endif /* _EXPORT */

#ifndef _IMPORT
#  if defined(ETK_OS_WIN32) || defined(ETK_OS_CYGWIN) || (defined(ETK_OS_BEOS) && defined(ETK_BIG_ENDIAN))
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

#ifdef ETK_COMPILATION
    #define _IMPEXP_ETK _EXPORT
#else /* !ETK_COMPILATION */
    #define _IMPEXP_ETK _IMPORT
#endif /* ETK_COMPILATION */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern _IMPEXP_ETK const euint8 etk_major_version;
extern _IMPEXP_ETK const euint8 etk_minor_version;
extern _IMPEXP_ETK const euint8 etk_micro_version;
extern _IMPEXP_ETK const euint8 etk_interface_age;
extern _IMPEXP_ETK const euint16 etk_binary_age;

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#ifdef __cplusplus

#ifdef ETK_OS_WIN32
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
#endif /* ETK_OS_WIN32 */

#endif /* __cplusplus */

/* seek_mode */
enum {
	E_SEEK_SET = 0,
	E_SEEK_CUR,
	E_SEEK_END,
};

#ifndef __ETK_DEBUG_H__
#include "./../kernel/Debug.h"
#endif

#endif /* __ETK_SUPPORT_DEFS_H__ */

