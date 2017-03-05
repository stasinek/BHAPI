/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
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
 * File: Debug.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_DEBUG_H
#define BHAPI_DEBUG_H
#include "../support/SupportDefs.h"

#ifdef __cplusplus
#include <new>
#endif /* __cplusplus */

#if defined(__MWERKS__) || defined(_MSC_VER)
/* for Metrowerks, etc. */
#ifndef __FUNCTION__
#define __FUNCTION__	"Unknown"
#endif
#endif

#ifndef __PRETTY_FUNCTION__
#ifndef __GNUC__
#define __PRETTY_FUNCTION__	__FUNCTION__
#endif /* __GNUC__ */
#endif /* __PRETTY_FUNCTION__ */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __cplusplus
namespace bhapi {
#endif /* __cplusplus */
typedef enum debug_level {
    DEBUG_NORMAL,
    DEBUG_OUTPUT,
    DEBUG_WARNING,
    DEBUG_ERROR
} debug_level;

IMPEXPBHAPI void debug_log(bhapi::debug_level level, const char *format, va_list ap);
#ifdef __cplusplus
} /* namespace */
#endif /* __cplusplus */

#ifndef BHAPI_DEBUG
#ifdef BHAPI_DISABLE_MORE_CHECKS
#	if __GNUC__ >= 3 || defined(__INTEL_COMPILER)
#		define BHAPI_DEBUG(...)				(void)0
#	else
#		ifdef __GNUC__
#			define BHAPI_DEBUG(a...)			(void)0
#		else
static void BHAPI_DEBUG(const char *format, ...)
{
}
#		endif
#	endif
#else
IMPEXPBHAPI void BHAPI_DEBUG(const char *format, ...);
#endif
#endif

IMPEXPBHAPI void BHAPI_OUTPUT(const char *format, ...);

#ifndef BHAPI_WARNING
#ifdef BHAPI_DISABLE_CHECKS
#	if __GNUC__ >= 3 || defined(__INTEL_COMPILER)
#		define BHAPI_WARNING(...)				(void)0
#	else
#		ifdef __GNUC__
#			define BHAPI_WARNING(a...)		(void)0
#		else
static void BHAPI_WARNING(const char *format, ...)
{
}
#		endif
#	endif
#else
IMPEXPBHAPI void BHAPI_WARNING(const char *format, ...);
#endif
#endif

IMPEXPBHAPI void BHAPI_ERROR(const char *format, ...);

#ifdef BHAPI_BUILD_WITH_MEMORY_TRACING
IMPEXPBHAPI void* calloc(size_t nmemb, size_t size, const char *file, int line, const char *method);
IMPEXPBHAPI void* malloc(size_t size, const char *file, int line, const char *method);
IMPEXPBHAPI void* realloc(void *ptr, size_t size, const char *file, int line, const char *method);
IMPEXPBHAPI void  free(void *ptr, const char *file, int line, const char *method);
#define calloc(a, b)	bhapi::calloc(a, b, __FILE__, __LINE__, "calloc")
#define malloc(a)	bhapi::malloc(a, __FILE__, __LINE__, "malloc")
#define realloc(a, b)	bhapi::realloc(a, b, __FILE__, __LINE__, "realloc")
#define free(a)		bhapi::free(a, __FILE__, __LINE__, "free")
#endif /* BHAPI_BUILD_WITH_MEMORY_TRACING */

#ifdef __cplusplus
} /* extern "C" */

#ifdef BHAPI_BUILD_WITH_MEMORY_TRACING
#ifdef __cplusplus
namespace bhapi {
#endif /* __cplusplus */
struct memory_flag_t {};
#ifdef __cplusplus
} /* namespace */
#endif /* __cplusplus */
IMPEXPBHAPI void* operator new(size_t size, const char *file, int line, const char *method, struct bhapi::memory_flag_t *flag);
IMPEXPBHAPI void* operator new[](size_t size, const char *file, int line, const char *method, struct bhapi::memory_flag_t *flag);
IMPEXPBHAPI void operator delete(void *ptr, const char *file, int line, const char *method, struct bhapi::memory_flag_t *flag);
IMPEXPBHAPI void operator delete[](void *ptr, const char *file, int line, const char *method, struct bhapi::memory_flag_t *flag);
#define new new(__FILE__, __LINE__, NULL, (struct bhapi::memory_flag_t*)0)
#endif /* BHAPI_BUILD_WITH_MEMORY_TRACING */

/* Private */
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __cplusplus
}
#endif

/* Debug macros */
#ifdef BHAPI_DEBUG
    #define SET_DEBUG_ENABLED(FLAG)
    #define	IS_DEBUG_ENABLED()		DEBUG

    #define SERIAL_PRINT(ARGS)		BHAPI_DEBUG(ARGS)
    #define PRINT(ARGS) 			BHAPI_DEBUG(ARGS)
    #define PRINT_OBJECT(OBJ)		if (IS_DEBUG_ENABLED) {		\
                                        PRINT(("%s\t", #OBJ));	\
                                        (OBJ).PrintToStream(); 	\
                                    } ((void)0)
    #define TRACE()					BHAPI_DEBUG("File: %s, Line: %d, Thread: %" \
                                        B_PRId32 "\n", __FILE__, __LINE__, \
                                        bhapi::find_thread(NULL))

    #define SERIAL_TRACE()			TRACE()

    #define DEBUGGER(MSG)			BHAPI_DEBUG(MSG)
    #if !defined(ASSERT)
        #define ASSERT(E)			assert(E)
    #endif

    #define ASSERT_WITH_MESSAGE(expr, msg) if (!expr) { BHAPI_DEBUG(msg); assert(expr); }

    #define TRESPASS()			BHAPI_DEBUG("Should not be here");

    #define DEBUG_ONLY(arg)		arg

#else /* DEBUG == 0 */
    #define SET_DEBUG_ENABLED(FLAG)			(void)0
    #define	IS_DEBUG_ENABLED()				(void)0

    #define SERIAL_PRINT(ARGS)				(void)0
    #define PRINT(ARGS)						(void)0
    #define PRINT_OBJECT(OBJ)				(void)0
    #define TRACE()							(void)0
    #define SERIAL_TRACE()					(void)0

    #define DEBUGGER(MSG)					(void)0
    #if !defined(ASSERT)
        #define ASSERT(E)					(void)0
    #endif
    #define ASSERT_WITH_MESSAGE(expr, msg)	(void)0
    #define TRESPASS()						(void)0
    #define DEBUG_ONLY(x)
#endif

/* STATIC_ASSERT is a compile-time check that can be used to             */
/* verify static expressions such as: STATIC_ASSERT(sizeof(int64) == 8); */
#define STATIC_ASSERT(x)								\
    do {												\
        struct __staticAssertStruct__ {					\
            char __static_assert_failed__[2*(x) - 1];	\
        };												\
    } while (false)

#endif /* __cplusplus */
#endif /* BHAPI_DEBUG_H */
