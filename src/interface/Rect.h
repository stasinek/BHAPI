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
 * File: Rect.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_RECT_H__
#define __ETK_RECT_H__

#include <math.h>
#include "./../interface/Point.h"

#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_ETK ERect {
public:
	float left;
	float top;
	float right;
	float bottom;

	ERect();
	ERect(const ERect &r);
	ERect(float l, float t, float r, float b);
	ERect(EPoint leftTop, EPoint rightBottom);

	ERect &operator=(const ERect &from);
	void Set(float l, float t, float r, float b);

	EPoint LeftTop() const;
	EPoint RightBottom() const;
	EPoint LeftBottom() const;
	EPoint RightTop() const;
	EPoint Center() const;

	void SetLeftTop(const EPoint pt);
	void SetRightBottom(const EPoint pt);
	void SetLeftBottom(const EPoint pt);
	void SetRightTop(const EPoint pt);

	void SetLeftTop(float x, float y);
	void SetRightBottom(float x, float y);
	void SetLeftBottom(float x, float y);
	void SetRightTop(float x, float y);

	void InsetBy(EPoint pt);
	void InsetBy(float dx, float dy);
	void OffsetBy(EPoint pt);
	void OffsetBy(float dx, float dy);
	void OffsetTo(EPoint pt);
	void OffsetTo(float x, float y);

	void Floor();
	void Ceil();
	void Round();

	ERect& InsetBySelf(EPoint pt);
	ERect& InsetBySelf(float dx, float dy);
	ERect InsetByCopy(EPoint pt) const;
	ERect InsetByCopy(float dx, float dy) const;
	ERect& OffsetBySelf(EPoint pt);
	ERect& OffsetBySelf(float dx, float dy);
	ERect OffsetByCopy(EPoint pt) const;
	ERect OffsetByCopy(float dx, float dy) const;
	ERect& OffsetToSelf(EPoint pt);
	ERect& OffsetToSelf(float x, float y);
	ERect OffsetToCopy(EPoint pt) const;
	ERect OffsetToCopy(float x, float y) const;

	ERect& FloorSelf();
	ERect FloorCopy() const;
	ERect& CeilSelf();
	ERect CeilCopy() const;
	ERect& RoundSelf();
	ERect RoundCopy() const;

	bool operator==(ERect r) const;
	bool operator!=(ERect r) const;

	ERect operator&(ERect r) const;
	ERect operator|(ERect r) const;

	ERect& operator&=(ERect r);
	ERect& operator|=(ERect r);

	bool IsValid() const;
	float Width() const;
	eint32 IntegerWidth() const;
	float Height() const;
	eint32 IntegerHeight() const;

	bool Intersects(ERect r) const;
	bool Intersects(float l, float t, float r, float b) const;

	bool Contains(EPoint pt) const;
	bool Contains(float x, float y) const;
	bool Contains(ERect r) const;
	bool Contains(float l, float t, float r, float b) const;

	void PrintToStream() const;
};


inline EPoint ERect::LeftTop() const
{
	return(EPoint(left, top));
}


inline EPoint ERect::RightBottom() const
{
	return(EPoint(right, bottom));
}


inline EPoint ERect::LeftBottom() const
{
	return(EPoint(left, bottom));
}


inline EPoint ERect::RightTop() const
{
	return(EPoint(right, top));
}


inline EPoint ERect::Center() const
{
	return(EPoint(left + (right - left) / 2, top + (bottom - top) / 2));
}


inline ERect::ERect()
{
	top = left = 0;
	bottom = right = -1;
}


inline ERect::ERect(float l, float t, float r, float b)
{
	left = l;
	top = t;
	right = r;
	bottom = b;
}


inline ERect::ERect(const ERect &r)
{
	left = r.left;
	top = r.top;
	right = r.right;
	bottom = r.bottom;
}


inline ERect::ERect(EPoint leftTop, EPoint rightBottom)
{
	left = leftTop.x;
	top = leftTop.y;
	right = rightBottom.x;
	bottom = rightBottom.y;
}


inline ERect& ERect::operator=(const ERect& from)
{
	left = from.left;
	top = from.top;
	right = from.right;
	bottom = from.bottom;
	return *this;
}


inline void ERect::Set(float l, float t, float r, float b)
{
	left = l;
	top = t;
	right = r;
	bottom = b;
}


inline bool ERect::IsValid() const
{
	return(left <= right && top <= bottom);
}


inline eint32 ERect::IntegerWidth() const
{
	return((eint32)ceil((double)(right - left)));
}


inline float ERect::Width() const
{
	return(right - left);
}


inline eint32 ERect::IntegerHeight() const
{
	return((eint32)ceil((double)(bottom - top)));
}


inline float ERect::Height() const
{
	return(bottom - top);
}

#endif /* __cplusplus */

#endif /* __ETK_RECT_H__ */

