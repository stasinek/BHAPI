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
 * File: StreamIO.cpp
 *
 * --------------------------------------------------------------------------*/

#include "StreamIO.h"

#include "../support/StringClass.h"
#include "../support/StandardIO.h"
#include "../kernel/Debug.h"
#include "../support/Errors.h"

static BStreamIO _endl;
static BStreamIO _ends;
static BStandardIO _EIn(0);
static BStandardIO _EOut(1);
static BStandardIO _EErr(2);

BHAPI_EXPORT BStreamIO &endl = _endl;
BHAPI_EXPORT BStreamIO &ends = _ends;
BHAPI_EXPORT BStreamIO &EIn = _EIn;
BHAPI_EXPORT BStreamIO &EOut = _EOut;
BHAPI_EXPORT BStreamIO &EErr = _EErr;

BStreamIO::BStreamIO()
    : BDataIO()
{
}


BStreamIO::~BStreamIO()
{
}


ssize_t BStreamIO::Read(void *a_buffer, size_t size)
{
    return (ssize_t)B_ERROR;
}


ssize_t BStreamIO::Write(const void *a_buffer, size_t size)
{
    return (ssize_t)B_ERROR;
}


BStreamIO& BStreamIO::operator<<(int8 value)
{
    BString str;
    str << value;
    Write(str.String(), str.Length());
    return *this;
}


BStreamIO&
BStreamIO::operator<<(uint8 value)
{
    BString str;
    str << value;
    Write(str.String(), str.Length());
    return *this;
}


BStreamIO&
BStreamIO::operator<<(int16 value)
{
    BString str;
    str << value;
    Write(str.String(), str.Length());
    return *this;
}


BStreamIO&
BStreamIO::operator<<(uint16 value)
{
    BString str;
    str << value;
    Write(str.String(), str.Length());
    return *this;
}


BStreamIO&
BStreamIO::operator<<(int32 value)
{
    BString str;
    str << value;
    Write(str.String(), str.Length());
    return *this;
}


BStreamIO&
BStreamIO::operator<<(uint32 value)
{
    BString str;
    str << value;
    Write(str.String(), str.Length());
    return *this;
}


BStreamIO&
BStreamIO::operator<<(int64 value)
{
    BString str;
    str << value;
    Write(str.String(), str.Length());
    return *this;
}


BStreamIO&
BStreamIO::operator<<(uint64 value)
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

     int8 buf[512];
     ssize_t len;

    bzero(buf, sizeof(buf));
    if((len = stream.Read(&buf[0], sizeof(buf))) > 0)
    {
         ssize_t nWritten;
        while((nWritten = Write(&buf[0], (size_t)len)) > 0)
        {
            if  (len <= nWritten) break;
            else len -= nWritten;
//			if(len - nWritten == 0 || (len -= nWritten) < 0) break; size_t is unsigned! cant be < 0
            memmove(&buf[0], &buf[nWritten], (size_t)len);
        }
    }

    return *this;
}

