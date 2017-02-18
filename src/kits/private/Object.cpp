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
 * File: Object.cpp
 *
 * --------------------------------------------------------------------------*/

#include "Object.h"

BObject::BObject()
	: fVitalities(0)
{
}


BObject::~BObject()
{
}


BObject&
BObject::operator+=(b_uint64 vitalities)
{
	if(fVitalities <= B_MAXUINT64 - vitalities) fVitalities += vitalities;
	return *this;
}


BObject&
BObject::operator-=(b_uint64 vitalities)
{
	fVitalities -= min_c(fVitalities, vitalities);
	return *this;
}


BObject&
BObject::operator++()
{
	return BObject::operator+=(1);
}


BObject&
BObject::operator--()
{
	return BObject::operator-=(1);
}


b_uint64
BObject::Vitalities() const
{
	return fVitalities;
}


void bhapi_delete_object(void *object)
{
	BObject *obj = reinterpret_cast<BObject*>(object);
	if(obj != NULL) delete obj;
}

