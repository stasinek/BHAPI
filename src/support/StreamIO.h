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
 * File: StreamIO.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_STREAM_IO_H__
#define __ETK_STREAM_IO_H__

#include "DataIO.h"

#ifdef __cplusplus /* Just for C++ */

class EString;

class _IMPEXP_ETK EStreamIO : public EDataIO {
public:
	EStreamIO();
	virtual ~EStreamIO();

	virtual ssize_t		Read(void *buffer, size_t size);
	virtual ssize_t		Write(const void *buffer, size_t size);

	EStreamIO 		&operator<<(eint8 value);
	EStreamIO 		&operator<<(euint8 value);
	EStreamIO 		&operator<<(eint16 value);
	EStreamIO 		&operator<<(euint16 value);
	EStreamIO 		&operator<<(eint32 value);
	EStreamIO 		&operator<<(euint32 value);
	EStreamIO 		&operator<<(eint64 value);
	EStreamIO 		&operator<<(euint64 value);
	EStreamIO 		&operator<<(float value);
	EStreamIO 		&operator<<(double value);
	EStreamIO 		&operator<<(const void *value);
	EStreamIO 		&operator<<(bool value);
	EStreamIO 		&operator<<(const char *str);
	EStreamIO 		&operator<<(const EString &str);
	EStreamIO 		&operator<<(EStreamIO &stream);

	// TODO: operator>>()
};

extern _IMPEXP_ETK EStreamIO& endl;
extern _IMPEXP_ETK EStreamIO& ends;

extern _IMPEXP_ETK EStreamIO& EIn;
extern _IMPEXP_ETK EStreamIO& EOut;
extern _IMPEXP_ETK EStreamIO& EErr;

#endif /* __cplusplus */

#endif /* __ETK_STREAM_IO_H__ */

