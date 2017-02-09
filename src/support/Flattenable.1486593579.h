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
 * File: Flattenable.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __BHAPI_FLATTENABLE_H__
#define __BHAPI_FLATTENABLE_H__

#include "./../support/SupportDefs.h"

#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_BHAPI BFlattenable {
public:
    virtual ~BFlattenable();

	virtual bool		IsFixedSize() const = 0;

    virtual b_type_code	TypeCode() const = 0;
    virtual bool		AllowsTypeCode(b_type_code code) const;

	virtual b_size_t		FlattenedSize() const = 0;
    virtual b_status_t	Flatten(void *buffer, b_size_t numBytes) = 0;
    virtual b_status_t	Unflatten(b_type_code, const void *buffer, b_size_t numBytes) = 0;
};

#endif /* __cplusplus */

#endif /* __BHAPI_FLATTENABLE_H__ */

