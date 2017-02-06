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
 * File: StreamIO.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../support/String.h"
#include "./../private/StandardIO.h"

#include "StreamIO.h"


static EStreamIO _endl;
static EStreamIO _ends;
static EStandardIO _EIn(0);
static EStandardIO _EOut(1);
static EStandardIO _EErr(2);

_IMPEXP_ETK EStreamIO &endl = _endl;
_IMPEXP_ETK EStreamIO &ends = _ends;
_IMPEXP_ETK EStreamIO &EIn = _EIn;
_IMPEXP_ETK EStreamIO &EOut = _EOut;
_IMPEXP_ETK EStreamIO &EErr = _EErr;


EStreamIO::EStreamIO()
	: EDataIO()
{
}


EStreamIO::~EStreamIO()
{
}


ssize_t
EStreamIO::Read(void *buffer, size_t size)
{
	return E_ERROR;
}


ssize_t
EStreamIO::Write(const void *buffer, size_t size)
{
	return E_ERROR;
}


EStreamIO&
EStreamIO::operator<<(eint8 value)
{
	EString str;
	str << value;
	Write(str.String(), str.Length());
	return *this;
}


EStreamIO&
EStreamIO::operator<<(euint8 value)
{
	EString str;
	str << value;
	Write(str.String(), str.Length());
	return *this;
}


EStreamIO&
EStreamIO::operator<<(eint16 value)
{
	EString str;
	str << value;
	Write(str.String(), str.Length());
	return *this;
}


EStreamIO&
EStreamIO::operator<<(euint16 value)
{
	EString str;
	str << value;
	Write(str.String(), str.Length());
	return *this;
}


EStreamIO&
EStreamIO::operator<<(eint32 value)
{
	EString str;
	str << value;
	Write(str.String(), str.Length());
	return *this;
}


EStreamIO&
EStreamIO::operator<<(euint32 value)
{
	EString str;
	str << value;
	Write(str.String(), str.Length());
	return *this;
}


EStreamIO&
EStreamIO::operator<<(eint64 value)
{
	EString str;
	str << value;
	Write(str.String(), str.Length());
	return *this;
}


EStreamIO&
EStreamIO::operator<<(euint64 value)
{
	EString str;
	str << value;
	Write(str.String(), str.Length());
	return *this;
}


EStreamIO&
EStreamIO::operator<<(float value)
{
	EString str;
	str << value;
	Write(str.String(), str.Length());
	return *this;
}


EStreamIO&
EStreamIO::operator<<(double value)
{
	EString str;
	str << value;
	Write(str.String(), str.Length());
	return *this;
}


EStreamIO&
EStreamIO::operator<<(const void *value)
{
	EString str;
	str.AppendFormat("%p", value);
	Write(str.String(), str.Length());
	return *this;
}


EStreamIO&
EStreamIO::operator<<(bool value)
{
	return operator<<(value ? "ture" : "false");
}


EStreamIO&
EStreamIO::operator<<(const char *str)
{
	if(!(str == NULL || *str == 0)) Write(str, strlen(str));
	return *this;
}


EStreamIO&
EStreamIO::operator<<(const EString &str)
{
	if(str.Length() > 0) Write(str.String(), str.Length());
	return *this;
}


EStreamIO&
EStreamIO::operator<<(EStreamIO &stream)
{
	if(&stream == &endl || &stream == &ends)
		return operator<<(&stream == &endl ? '\n' : ' ');

	eint8 buf[512];
	ssize_t len;

	bzero(buf, sizeof(buf));
	if((len = stream.Read(&buf[0], sizeof(buf))) > 0)
	{
		ssize_t nWritten;
		while((nWritten = Write(&buf[0], (size_t)len)) > 0)
		{
			if(len - nWritten == 0 || (len -= nWritten) < 0) break;
			memmove(&buf[0], &buf[nWritten], (size_t)len);
		}
	}

	return *this;
}

