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

#include <string.h> /* for bzero */
#include "Errors.h"

typedef eint32	e_status_t;
typedef eint64	e_bigtime_t;
typedef eint64	e_thread_id;
typedef euint32	e_type_code;
typedef euint32	e_perform_code;

enum {
	E_ANY_TYPE 				= 'ANYT',
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
	E_RECT_TYPE				= 'RECT',
	E_MIME_TYPE				= 'MIME',
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

