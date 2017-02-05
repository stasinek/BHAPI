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
 * File: ByteOrder.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_BYTE_ORDER_H__
#define __ETK_BYTE_ORDER_H__

#include "SupportDefs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum e_swap_action {
	E_SWAP_HOST_TO_LENDIAN,
	E_SWAP_HOST_TO_BENDIAN,
	E_SWAP_LENDIAN_TO_HOST,
	E_SWAP_BENDIAN_TO_HOST,
	E_SWAP_ALWAYS
} e_swap_action;

_IMPEXP_ETK e_status_t			e_swap_data(e_type_code type, void *data, size_t len, e_swap_action action);
_IMPEXP_ETK bool			e_is_type_swapped(e_type_code type);
_IMPEXP_ETK float			e_swap_float(float value);
_IMPEXP_ETK double			e_swap_double(double value);

#define E_SWAP_INT16(v)			((((v) & 0xff) << 8) | (((v) >> 8) & 0xff))
#define E_SWAP_INT32(v)			((E_SWAP_INT16((v) & 0xffff) << 16) | E_SWAP_INT16(((v) >> 16) & 0xffff))
#define E_SWAP_INT64(v)			((E_SWAP_INT32((v) & 0xffffffff) << 32) | E_SWAP_INT32(((v) >> 32) & 0xffffffff))
#define E_SWAP_FLOAT(v)			e_swap_float(v)
#define E_SWAP_DOUBLE(v)		e_swap_double(v)

#ifdef ETK_LITTLE_ENDIAN

#define E_HOST_IS_LENDIAN		1
#define E_HOST_IS_BENDIAN		0

#define E_HOST_TO_LENDIAN_INT16(v)	((v) & 0xffff)
#define E_HOST_TO_LENDIAN_INT32(v)	((v) & 0xffffffff)
#define E_HOST_TO_LENDIAN_INT64(v)	(v)
#define E_HOST_TO_LENDIAN_FLOAT(v)	(v)
#define E_HOST_TO_LENDIAN_DOUBLE(v)	(v)

#define E_HOST_TO_BENDIAN_INT16(v)	E_SWAP_INT16(v)
#define E_HOST_TO_BENDIAN_INT32(v)	E_SWAP_INT32(v)
#define E_HOST_TO_BENDIAN_INT64(v)	E_SWAP_INT64(v)
#define E_HOST_TO_BENDIAN_FLOAT(v)	E_SWAP_FLOAT(v)
#define E_HOST_TO_BENDIAN_DOUBLE(v)	E_SWAP_DOUBLE(v)

#define E_LENDIAN_TO_HOST_INT16(v)	((v) & 0xffff)
#define E_LENDIAN_TO_HOST_INT32(v)	((v) & 0xffffffff)
#define E_LENDIAN_TO_HOST_INT64(v)	(v)
#define E_LENDIAN_TO_HOST_FLOAT(v)	(v)
#define E_LENDIAN_TO_HOST_DOUBLE(v)	(v)

#define E_BENDIAN_TO_HOST_INT16(v)	E_SWAP_INT16(v)
#define E_BENDIAN_TO_HOST_INT32(v)	E_SWAP_INT32(v)
#define E_BENDIAN_TO_HOST_INT64(v)	E_SWAP_INT64(v)
#define E_BENDIAN_TO_HOST_FLOAT(v)	E_SWAP_FLOAT(v)
#define E_BENDIAN_TO_HOST_DOUBLE(v)	E_SWAP_DOUBLE(v)

#else /* ETK_BIG_ENDIAN */

#define E_HOST_IS_LENDIAN		0
#define E_HOST_IS_BENDIAN		1

#define E_HOST_TO_LENDIAN_INT16(v)	E_SWAP_INT16(v)
#define E_HOST_TO_LENDIAN_INT32(v)	E_SWAP_INT32(v)
#define E_HOST_TO_LENDIAN_INT64(v)	E_SWAP_INT64(v)
#define E_HOST_TO_LENDIAN_FLOAT(v)	E_SWAP_FLOAT(v)
#define E_HOST_TO_LENDIAN_DOUBLE(v)	E_SWAP_DOUBLE(v)

#define E_HOST_TO_BENDIAN_INT16(v)	((v) & 0xffff)
#define E_HOST_TO_BENDIAN_INT32(v)	((v) & 0xffffffff)
#define E_HOST_TO_BENDIAN_INT64(v)	(v)
#define E_HOST_TO_BENDIAN_FLOAT(v)	(v)
#define E_HOST_TO_BENDIAN_DOUBLE(v)	(v)

#define E_LENDIAN_TO_HOST_INT16(v)	E_SWAP_INT16(v)
#define E_LENDIAN_TO_HOST_INT32(v)	E_SWAP_INT32(v)
#define E_LENDIAN_TO_HOST_INT64(v)	E_SWAP_INT64(v)
#define E_LENDIAN_TO_HOST_FLOAT(v)	E_SWAP_FLOAT(v)
#define E_LENDIAN_TO_HOST_DOUBLE(v)	E_SWAP_DOUBLE(v)

#define E_BENDIAN_TO_HOST_INT16(v)	((v) & 0xffff)
#define E_BENDIAN_TO_HOST_INT32(v)	((v) & 0xffffffff)
#define E_BENDIAN_TO_HOST_INT64(v)	(v)
#define E_BENDIAN_TO_HOST_FLOAT(v)	(v)
#define E_BENDIAN_TO_HOST_DOUBLE(v)	(v)

#endif /* ETK_LITTLE_ENDIAN */


#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __ETK_BYTE_ORDER_H__ */

