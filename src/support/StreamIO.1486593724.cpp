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
 * File: StreamIO.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../support/StringMe.h"
#include "./../private/StandardIO.h"

#include "StreamIO.h"


static BStreamIO _endl;
static BStreamIO _ends;
static BStandardIO _EIn(0);
static BStandardIO _EOut(1);
static BStandardIO _EErr(2);

_IMPEXP_BHAPI BStreamIO &endl = _endl;
_IMPEXP_BHAPI BStreamIO &ends = _ends;
_IMPEXP_BHAPI BStreamIO &EIn = _EIn;
_IMPEXP_BHAPI BStreamIO &EOut = _EOut;
_IMPEXP_BHAPI BStreamIO &EErr = _EErr;


BStreamIO::BStreamIO()
	: EDataIO()
{
}


BStreamIO::~BStreamIO()
{
}


b_size_t
BStreamIO::Read(void *buffer, size_t size)
{
	return B_ERROR;
}


b_size_t
BStreamIO::Write(const void *buffer, size_t size)
{
	return B_ERROR;
}


BStreamIO&
BStreamIO::operator<<(b_int8 value)
{
	BString str;
	str << value;
	Write(str.String(), str.Length());
	return *this;
}


BStreamIO&
BStreamIO::operator<<(b_uint8 value)
{
	BString str;
	str << value;
	Write(str.String(), str.Length());
	return *this;
}


BStreamIO&
BStreamIO::operator<<(b_int16 value)
{
	BString str;
	str << value;
	Write(str.String(), str.Length());
	return *this;
}


BStreamIO&
BStreamIO::operator<<(b_uint16 value)
{
	BString str;
	str << value;
	Write(str.String(), str.Length());
	return *this;
}


BStreamIO&
BStreamIO::operator<<(b_int32 value)
{
	BString str;
	str << value;
	Write(str.String(), str.Length());
	return *this;
}


BStreamIO&
BStreamIO::operator<<(b_uint32 value)
{
	BString str;
	str << value;
	Write(str.String(), str.Length());
	return *this;
}


BStreamIO&
BStreamIO::operator<<(b_int64 value)
{
	BString str;
	str << value;
	Write(str.String(), str.Length());
	return *this;
}


BStreamIO&
BStreamIO::operator<<(b_uint64 value)
{
	BString str;
	str << value;
	Write(str.String(), str.Length());
	return *this;
}


BStreamIO&
BStreamIO::operator<<(float value)
{
	BString str;
	str << value;
	Write(str.String(), str.Length());
	return *this;
}


BStreamIO&
BStreamIO::operator<<(double value)
{
	BString str;
	str << value;
	Write(str.String(), str.Length());
	return *this;
}


BStreamIO&
BStreamIO::operator<<(const void *value)
{
	BString str;
	str.AppendFormat("%p", value);
	Write(str.String(), str.Length());
	return *this;
}


BStreamIO&
BStreamIO::operator<<(bool value)
{
	return operator<<(value ? "ture" : "false");
}


BStreamIO&
BStreamIO::operator<<(const char *str)
{
	if(!(str == NULL || *str == 0)) Write(str, strlen(str));
	return *this;
}


BStreamIO&
BStreamIO::operator<<(const BString &str)
{
	if(str.Length() > 0) Write(str.String(), str.Length());
	return *this;
}


BStreamIO&
BStreamIO::operator<<(BStreamIO &stream)
{
	if(&stream == &endl || &stream == &ends)
		return operator<<(&stream == &endl ? '\n' : ' ');

	b_int8 buf[512];
	b_size_t len;

	bzero(buf, sizeof(buf));
	if((len = stream.Read(&buf[0], sizeof(buf))) > 0)
	{
		b_size_t nWritten;
		while((nWritten = Write(&buf[0], (size_t)len)) > 0)
		{
			if(len - nWritten == 0 || (len -= nWritten) < 0) break;
			memmove(&buf[0], &buf[nWritten], (size_t)len);
		}
	}

	return *this;
}

