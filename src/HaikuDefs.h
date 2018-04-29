#ifndef BHAPI_HAIKU_DEFS_H
#define BHAPI_HAIKU_DEFS_H

#include <HaikuConfig.h>

#ifndef UNICODE
#define UNICODE
#endif

# ifdef BHAPI_OS_WIN32
# undef  WIN32
#define  WIN32
# undef _WIN32
#define _WIN32
#endif
# ifdef BHAPI_OS_LINUX
# undef LINUX
#define LINUX
#endif
# ifdef BHAPI_OS_MACOS
# undef MACOS
#define MACOS
#endif

#if !defined(__BSD__) & !defined(__LINUX__) & !defined(WIN32) & !defined(__MACOS__)
#define __B_NATIVE__
#endif

#ifdef BHAPI_OS_WIN32
#	ifdef __GNUC__
#		ifndef _stdcall
#		define _stdcall  __attribute__((stdcall))
#		endif /* stdcall */
#	endif /* __GNUC__ */
#endif /* BHAPI_OS_WIN32 */

/*#ifdef BHAPI_OS_WIN32
    #ifdef _WIN32
        #include <kits/netwinsock2.h>
        #include <kits/netwindows.h>
    #endif
    #if defined(_MSC_VER) && _MSC_VER <= 0x4b0
        #define for	if (0); else for
    #endif
#endif // BHAPI_OS_WIN32
*/

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

#define BHAPI_EXPORT __EXPORT
#define BHAPI_IMPORT __IMPORT

#ifdef BHAPI_BUILD_LIBRARY
    #define BHAPI_IMPEXP __EXPORT
#else /* !BHAPI_BUILD_LIBRARY */
    #define BHAPI_IMPEXP __IMPORT
#endif /* BHAPI_BUILD_LIBRARY */

#ifndef __LOCAL
#  if (__GNUC__ > 3 || __GNUC__ == 3 && __GNUC_MINOR__ >= 3 && __GNUC_PATCHLEVEL__ > 3) && !defined(__MINGW32__)
#    define __LOCAL __attribute__((visibility("hidden")))
#  else
#    define __LOCAL
#  endif
#endif /* __LOCAL */

#ifndef BHAPI_LOCAL
#define BHAPI_LOCAL __LOCAL
#endif /* __LOCAL */

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

#include <limits.h>

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

#define B_MININT32 INT_MIN
#define B_MAXUINT32 UINT_MAX
#define B_MAXINT32 INT_MAX

// Build on Windows for Windows
#ifdef WIN32
#ifndef __clang__
#include <sys/cdefs.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#endif

#include <kits/support/SupportDefs.h>
#include <kits/storage/StorageDefs.h>
#include <kits/interface/InterfaceDefs.h>

#endif
