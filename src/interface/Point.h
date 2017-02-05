/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
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
 * File: Point.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_POINT_H__
#define __ETK_POINT_H__

#include "./../support/SupportDefs.h"

#ifdef __cplusplus /* Just for C++ */

class ERect;

class _IMPEXP_ETK EPoint {
public:
	float x;
	float y;

	EPoint();
	EPoint(float X, float Y);
	EPoint(const EPoint &pt);
		
	EPoint &operator=(const EPoint &from);
	void Set(float X, float Y);

	void ConstrainTo(ERect rect);

	void Ceil();
	void Floor();
	void Round();

	EPoint& FloorSelf();
	EPoint FloorCopy() const;
	EPoint& CeilSelf();
	EPoint CeilCopy() const;
	EPoint& RoundSelf();
	EPoint RoundCopy() const;

	EPoint operator+(const EPoint &plus) const;
	EPoint operator-(const EPoint &minus) const;
	EPoint& operator+=(const EPoint &plus);
	EPoint& operator-=(const EPoint &minus);

	bool operator!=(const EPoint &pt) const;
	bool operator==(const EPoint &pt) const;

	void PrintToStream() const;
};

extern _IMPEXP_ETK const EPoint E_ORIGIN;

#endif /* __cplusplus */

#endif /* __ETK_POINT_H__ */

