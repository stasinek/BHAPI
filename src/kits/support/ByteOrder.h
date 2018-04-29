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
 * File: ByteOrder.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_BYTE_ORDER_H
#define BHAPI_BYTE_ORDER_H
#include <Haiku.h>

/* swap directions */
typedef enum {
    B_SWAP_HOST_TO_LENDIAN,
    B_SWAP_HOST_TO_BENDIAN,
    B_SWAP_LENDIAN_TO_HOST,
    B_SWAP_BENDIAN_TO_HOST,
    B_SWAP_ALWAYS
} swap_action;

/* BSD/networking macros */
#ifndef htonl
#	define htonl(x) B_HOST_TO_BENDIAN_INT32(x)
#	define ntohl(x) B_BENDIAN_TO_HOST_INT32(x)
#	define htons(x) B_HOST_TO_BENDIAN_INT16(x)
#	define ntohs(x) B_BENDIAN_TO_HOST_INT16(x)
#endif

/* always swap macros */
#define B_SWAP_DOUBLE(arg)   __swap_double(arg)
#define B_SWAP_FLOAT(arg)    __swap_float(arg)
#define B_SWAP_INT64(arg)    __swap_int64(arg)
#define B_SWAP_INT32(arg)    __swap_int32(arg)
#define B_SWAP_INT16(arg)    __swap_int16(arg)

#if BYTE_ORDER == __LITTLE_ENDIAN
/* Host is little endian */

#define B_HOST_IS_LENDIAN 1
#define B_HOST_IS_BENDIAN 0

/* Host native to little endian */
#define B_HOST_TO_LENDIAN_DOUBLE(arg)	(double)(arg)
#define B_HOST_TO_LENDIAN_FLOAT(arg)	(float)(arg)
#define B_HOST_TO_LENDIAN_INT64(arg)	(uint64)(arg)
#define B_HOST_TO_LENDIAN_INT32(arg)	(uint32)(arg)
#define B_HOST_TO_LENDIAN_INT16(arg)	(uint16)(arg)

/* Little endian to host native */
#define B_LENDIAN_TO_HOST_DOUBLE(arg)	(double)(arg)
#define B_LENDIAN_TO_HOST_FLOAT(arg)	(float)(arg)
#define B_LENDIAN_TO_HOST_INT64(arg)	(uint64)(arg)
#define B_LENDIAN_TO_HOST_INT32(arg)	(uint32)(arg)
#define B_LENDIAN_TO_HOST_INT16(arg)	(uint16)(arg)

/* Host native to big endian */
#define B_HOST_TO_BENDIAN_DOUBLE(arg)	__swap_double(arg)
#define B_HOST_TO_BENDIAN_FLOAT(arg)	__swap_float(arg)
#define B_HOST_TO_BENDIAN_INT64(arg)	__swap_int64(arg)
#define B_HOST_TO_BENDIAN_INT32(arg)	__swap_int32(arg)
#define B_HOST_TO_BENDIAN_INT16(arg)	__swap_int16(arg)

/* Big endian to host native */
#define B_BENDIAN_TO_HOST_DOUBLE(arg)	__swap_double(arg)
#define B_BENDIAN_TO_HOST_FLOAT(arg)	__swap_float(arg)
#define B_BENDIAN_TO_HOST_INT64(arg)	__swap_int64(arg)
#define B_BENDIAN_TO_HOST_INT32(arg)	__swap_int32(arg)
#define B_BENDIAN_TO_HOST_INT16(arg)	__swap_int16(arg)

#else	/* BYTE_ORDER */
/* Host is big endian */

#define B_HOST_IS_LENDIAN 0
#define B_HOST_IS_BENDIAN 1

/* Host native to little endian */
#define B_HOST_TO_LENDIAN_DOUBLE(arg)	__swap_double(arg)
#define B_HOST_TO_LENDIAN_FLOAT(arg)	__swap_float(arg)
#define B_HOST_TO_LENDIAN_INT64(arg)	__swap_int64(arg)
#define B_HOST_TO_LENDIAN_INT32(arg)	__swap_int32(arg)
#define B_HOST_TO_LENDIAN_INT16(arg)	__swap_int16(arg)

/* Little endian to host native */
#define B_LENDIAN_TO_HOST_DOUBLE(arg)	__swap_double(arg)
#define B_LENDIAN_TO_HOST_FLOAT(arg)	__swap_float(arg)
#define B_LENDIAN_TO_HOST_INT64(arg)	__swap_int64(arg)
#define B_LENDIAN_TO_HOST_INT32(arg)	__swap_int32(arg)
#define B_LENDIAN_TO_HOST_INT16(arg)	__swap_int16(arg)

/* Host native to big endian */
#define B_HOST_TO_BENDIAN_DOUBLE(arg)	(double)(arg)
#define B_HOST_TO_BENDIAN_FLOAT(arg)	(float)(arg)
#define B_HOST_TO_BENDIAN_INT64(arg)	(uint64)(arg)
#define B_HOST_TO_BENDIAN_INT32(arg)	(uint32)(arg)
#define B_HOST_TO_BENDIAN_INT16(arg)	(uint16)(arg)
/* Big endian to host native */
#define B_BENDIAN_TO_HOST_DOUBLE(arg)	(double)(arg)
#define B_BENDIAN_TO_HOST_FLOAT(arg)	(float)(arg)
#define B_BENDIAN_TO_HOST_INT64(arg)	(uint64)(arg)
#define B_BENDIAN_TO_HOST_INT32(arg)	(uint32)(arg)
#define B_BENDIAN_TO_HOST_INT16(arg)	(uint16)(arg)
#endif	/* BYTE_ORDER */

#ifdef __cplusplus /* Just for C++ */
namespace bhapi {
#endif

BHAPI_IMPEXP status_t       swap_data(type_code type, void *data, size_t len, swap_action action);
BHAPI_IMPEXP bool			is_type_swapped(type_code type);
BHAPI_IMPEXP float			__swap_float(float value);
BHAPI_IMPEXP double			__swap_double(double value);
#if __GNUC__ >= 4
#define __swap_int64(arg)	(uint64)__builtin_bswap64(arg)
#define __swap_int32(arg)	(uint32)__builtin_bswap32(arg)
#define __swap_int16(arg)	(uint16)__builtin_bswap16(arg)
#else
extern uint64 __swap_int64(uint64 arg);
extern uint32 __swap_int32(uint32 arg);
extern uint16 __swap_int16(uint16 arg);
#endif

#ifdef __cplusplus
} /* namespace */
#endif
#endif /* BHAPI_BYTE_ORDER_H */

