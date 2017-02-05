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
 * File: Rect.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../support/SupportDefs.h"

#include "Rect.h"

#ifndef HAVE_ROUND
inline double etk_round(double value)
{
	double iValue = 0;
	double fValue = modf(value, &iValue);

	if(fValue >= 0.5) iValue += 1;
	else if(fValue <= -0.5) iValue -= 1;

	return iValue;
}
#else
#define etk_round(a) round(a)
#endif // HAVE_ROUND

bool
ERect::Contains(EPoint pt) const
{
	return(IsValid() ? pt.x >= left && pt.x <= right && pt.y >= top && pt.y <= bottom : false);
}


bool
ERect::Contains(float x, float y) const
{
	return(IsValid() ? x >= left && x <= right && y >= top && y <= bottom : false);
}


bool
ERect::Contains(ERect r) const
{
	if(r.IsValid() == false) return false;
	return(IsValid() ? r.left >= left && r.right <= right && r.top >= top && r.bottom <= bottom : false);
}


bool
ERect::Contains(float l, float t, float r, float b) const
{
	if(!(l <= r && t <= b)) return false;
	return(IsValid() ? l >= left && r <= right && t >= top && b <= bottom : false);
}


bool
ERect::operator==(ERect r) const
{
	return(r.left == left && r.right == right && r.top == top && r.bottom == bottom);
}


bool
ERect::operator!=(ERect r) const
{
	return(r.left != left || r.right != right || r.top != top || r.bottom != bottom);
}


inline ERect rect_intersection(ERect r1, ERect r2)
{
	if(r1.IsValid() == false || r2.IsValid() == false) return ERect();

	ERect r3;
	r3.left = max_c(r1.left, r2.left);
	r3.top = max_c(r1.top, r2.top);
	r3.right = min_c(r1.right, r2.right);
	r3.bottom = min_c(r1.bottom, r2.bottom);

	return r3;
}


ERect ERect::operator&(ERect r) const // intersection
{
	if(!IsValid() || !r.IsValid()) return ERect();
	return rect_intersection(*this, r);
}


ERect& ERect::operator&=(ERect r) // intersection
{
	if(!IsValid() || !r.IsValid())
		*this = ERect();
	else
		*this = rect_intersection(*this, r);

	return *this;
}


ERect ERect::operator|(ERect r) const // union
{
	if(!IsValid()) return r;
	if(!r.IsValid()) return *this;
	return(ERect(min_c(left, r.left), min_c(top, r.top), max_c(right, r.right), max_c(bottom, r.bottom)));
}


ERect& ERect::operator|=(ERect r) // union
{
	if(!IsValid())
	{
		*this = r;
	}
	else if(r.IsValid())
	{
		*this = ERect(min_c(left, r.left), min_c(top, r.top), max_c(right, r.right), max_c(bottom, r.bottom));
	}

	return *this;
}


bool
ERect::Intersects(ERect r) const
{
	return Intersects(r.left, r.top, r.right, r.bottom);
}


bool
ERect::Intersects(float l, float t, float r, float b) const
{
	if(!IsValid() || !(l <= r && t <= b)) return false;
	if(max_c(left, l) > min_c(right, r)) return false;
	if(max_c(top, t) > min_c(bottom, b)) return false;

	return true;
}


void
ERect::SetLeftTop(const EPoint pt)
{
	left = pt.x;
	top = pt.y;
}


void
ERect::SetRightBottom(const EPoint pt)
{
	right = pt.x;
	bottom = pt.y;
}


void
ERect::SetLeftBottom(const EPoint pt)
{
	left = pt.x;
	bottom = pt.y;
}


void
ERect::SetRightTop(const EPoint pt)
{
	right = pt.x;
	top = pt.y;
}


void
ERect::SetLeftTop(float x, float y)
{
	left = x;
	top = y;
}


void
ERect::SetRightBottom(float x, float y)
{
	right = x;
	bottom = y;
}


void
ERect::SetLeftBottom(float x, float y)
{
	left = x;
	bottom = y;
}


void
ERect::SetRightTop(float x, float y)
{
	right = x;
	top = y;
}


void
ERect::InsetBy(EPoint pt)
{
	left += pt.x;
	right -= pt.x;
	top += pt.y;
	bottom -= pt.y;
}


void
ERect::InsetBy(float dx, float dy)
{
	left += dx;
	right -= dx;
	top += dy;
	bottom -= dy;
}


void
ERect::OffsetBy(EPoint pt)
{
	left += pt.x;
	right += pt.x;
	top += pt.y;
	bottom += pt.y;
}


void
ERect::OffsetBy(float dx, float dy)
{
	left += dx;
	right += dx;
	top += dy;
	bottom += dy;
}


void
ERect::OffsetTo(EPoint pt)
{
	float width = right - left;
	float height = bottom - top;

	left = pt.x;
	right = left + width;

	top = pt.y;
	bottom = top + height;
}


void
ERect::OffsetTo(float x, float y)
{
	float width = right - left;
	float height = bottom - top;

	left = x;
	right = left + width;

	top = y;
	bottom = top + height;
}


ERect&
ERect::InsetBySelf(EPoint pt)
{
	InsetBy(pt);
	return *this;
}


ERect&
ERect::InsetBySelf(float dx, float dy)
{
	InsetBy(dx, dy);
	return *this;
}


ERect
ERect::InsetByCopy(EPoint pt) const
{
	return(ERect(left + pt.x, top + pt.y, right - pt.x, bottom - pt.y));
}


ERect
ERect::InsetByCopy(float dx, float dy) const
{
	return(ERect(left + dx, top + dy, right - dx, bottom - dy));
}


ERect&
ERect::OffsetBySelf(EPoint pt)
{
	OffsetBy(pt);
	return *this;
}


ERect&
ERect::OffsetBySelf(float dx, float dy)
{
	OffsetBy(dx, dy);
	return *this;
}


ERect
ERect::OffsetByCopy(EPoint pt) const
{
	return(ERect(left + pt.x, top + pt.y, right + pt.x, bottom + pt.y));
}


ERect
ERect::OffsetByCopy(float dx, float dy) const
{
	return(ERect(left + dx, top + dy, right + dx, bottom + dy));
}


ERect&
ERect::OffsetToSelf(EPoint pt)
{
	OffsetTo(pt);
	return *this;
}


ERect&
ERect::OffsetToSelf(float x, float y)
{
	OffsetTo(x, y);
	return *this;
}


ERect
ERect::OffsetToCopy(EPoint pt) const
{
	float width = Width();
	float height = Height();

	return(ERect(pt.x, pt.y, pt.x + width, pt.y + height));
}


ERect
ERect::OffsetToCopy(float x, float y) const
{
	float width = Width();
	float height = Height();

	return(ERect(x, y, x + width, y + height));
}


void
ERect::Floor()
{
	left = (float)floor((double)left);
	right = (float)floor((double)right);
	top = (float)floor((double)top);
	bottom = (float)floor((double)bottom);
}


ERect&
ERect::FloorSelf()
{
	Floor();
	return *this;
}


ERect
ERect::FloorCopy() const
{
	float _left = (float)floor((double)left);
	float _right = (float)floor((double)right);
	float _top = (float)floor((double)top);
	float _bottom = (float)floor((double)bottom);

	return(ERect(_left, _top, _right, _bottom));
}


void
ERect::Ceil()
{
	left = (float)ceil((double)left);
	right = (float)ceil((double)right);
	top = (float)ceil((double)top);
	bottom = (float)ceil((double)bottom);
}


ERect&
ERect::CeilSelf()
{
	Ceil();
	return *this;
}


ERect
ERect::CeilCopy() const
{
	float _left = (float)ceil((double)left);
	float _right = (float)ceil((double)right);
	float _top = (float)ceil((double)top);
	float _bottom = (float)ceil((double)bottom);

	return(ERect(_left, _top, _right, _bottom));
}


void
ERect::Round()
{
	left = (float)etk_round((double)left);
	right = (float)etk_round((double)right);
	top = (float)etk_round((double)top);
	bottom = (float)etk_round((double)bottom);
}


ERect&
ERect::RoundSelf()
{
	Round();
	return *this;
}


ERect
ERect::RoundCopy() const
{
	float _left = (float)etk_round((double)left);
	float _right = (float)etk_round((double)right);
	float _top = (float)etk_round((double)top);
	float _bottom = (float)etk_round((double)bottom);

	return(ERect(_left, _top, _right, _bottom));
}


void
ERect::PrintToStream() const
{
	ETK_OUTPUT("ERect(%g, %g, %g, %g)", left, top, right, bottom);
}

