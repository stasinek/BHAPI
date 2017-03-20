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
 * File: StreamIO.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __BHAPI_STREAM_IO_H__
#define __BHAPI_STREAM_IO_H__

#include "DataIO.h"

#ifdef __cplusplus /* Just for C++ */

class BString;

class _IMPEXP_BHAPI BStreamIO : public EDataIO {
public:
    BStreamIO();
    virtual ~BStreamIO();

	virtual  ssize_t		Read(void *buffer, size_t size);
	virtual  ssize_t		Write(const void *buffer, size_t size);

    BStreamIO 		&operator<<(__be_int8 value);
    BStreamIO 		&operator<<(__be_uint8 value);
    BStreamIO 		&operator<<(__be_int16 value);
    BStreamIO 		&operator<<(__be_uint16 value);
    BStreamIO 		&operator<<(__be_int32 value);
    BStreamIO 		&operator<<(__be_uint32 value);
    BStreamIO 		&operator<<(__be_int64 value);
    BStreamIO 		&operator<<(__be_uint64 value);
    BStreamIO 		&operator<<(float value);
    BStreamIO 		&operator<<(double value);
    BStreamIO 		&operator<<(const void *value);
    BStreamIO 		&operator<<(bool value);
    BStreamIO 		&operator<<(const char *str);
    BStreamIO 		&operator<<(const BString &str);
    BStreamIO 		&operator<<(BStreamIO &stream);

	// TODO: operator>>()
};

extern _IMPEXP_BHAPI BStreamIO& endl;
extern _IMPEXP_BHAPI BStreamIO& ends;

extern _IMPEXP_BHAPI BStreamIO& EIn;
extern _IMPEXP_BHAPI BStreamIO& EOut;
extern _IMPEXP_BHAPI BStreamIO& EErr;

#endif /* __cplusplus */

#endif /* __BHAPI_STREAM_IO_H__ */

