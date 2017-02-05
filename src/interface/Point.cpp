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
 * File: Point.cpp
 *
 * --------------------------------------------------------------------------*/

#include <math.h>

#include "./../support/SupportDefs.h"

#include "GraphicsDefs.h"
#include "Point.h"
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


EPoint::EPoint()
{
	x = 0;
	y = 0;
}


EPoint::EPoint(float X, float Y)
{
	x = X;
	y = Y;
}


EPoint::EPoint(const EPoint& pt)
{
	x = pt.x;
	y = pt.y;
}


EPoint&
EPoint::operator=(const EPoint& from)
{
	x = from.x;
	y = from.y;
	
	return *this;
}


void
EPoint::Set(float X, float Y)
{
	x = X;
	y = Y;
}


EPoint
EPoint::operator+(const EPoint &plus) const
{
	return(EPoint(this->x + plus.x, this->y + plus.y)); 
}


EPoint
EPoint::operator-(const EPoint &minus) const
{
	return(EPoint(this->x - minus.x, this->y - minus.y));
}


EPoint&
EPoint::operator+=(const EPoint &plus)
{
	x += plus.x;
	y += plus.y;

	return *this;
}


EPoint&
EPoint::operator-=(const EPoint &minus)
{
	x -= minus.x;
	y -= minus.y;

	return *this;
}


bool
EPoint::operator!=(const EPoint &pt) const
{
	return(x != pt.x || y != pt.y);
}


bool
EPoint::operator==(const EPoint &pt) const
{
	return(x == pt.x && y == pt.y);
}

void
EPoint::ConstrainTo(ERect rect)
{
	if(!(x >= rect.left && x <= rect.right))
	{
		float left_dist = (float)fabs((double)(rect.left - x));
		float right_dist = (float)fabs((double)(rect.right - x));

		float min_dist = min_c(left_dist, right_dist);

		if(min_dist == left_dist)
			x = rect.left;
		else
			x = rect.right;
	}

	if(!(y >= rect.top && y <= rect.bottom))
	{
		float top_dist = (float)fabs((double)(rect.top - y));
		float bottom_dist = (float)fabs((double)(rect.bottom - y));

		float min_dist = min_c(top_dist, bottom_dist);

		if(min_dist == top_dist)
			y = rect.top;
		else
			y = rect.bottom;
	}
}


void
EPoint::Floor()
{
	x = (float)floor((double)x);
	y = (float)floor((double)y);
}


EPoint&
EPoint::FloorSelf()
{
	x = (float)floor((double)x);
	y = (float)floor((double)y);
	return *this;
}


EPoint
EPoint::FloorCopy() const
{
	float _x = (float)floor((double)x);
	float _y = (float)floor((double)y);
	return(EPoint(_x, _y));
}


void
EPoint::Ceil()
{
	x = (float)ceil((double)x);
	y = (float)ceil((double)y);
}


EPoint&
EPoint::CeilSelf()
{
	x = (float)ceil((double)x);
	y = (float)ceil((double)y);
	return *this;
}


EPoint
EPoint::CeilCopy() const
{
	float _x = (float)ceil((double)x);
	float _y = (float)ceil((double)y);
	return(EPoint(_x, _y));
}


void
EPoint::Round()
{
	x = (float)etk_round((double)x);
	y = (float)etk_round((double)y);
}


EPoint&
EPoint::RoundSelf()
{
	x = (float)etk_round((double)x);
	y = (float)etk_round((double)y);
	return *this;
}


EPoint
EPoint::RoundCopy() const
{
	float _x = (float)etk_round((double)x);
	float _y = (float)etk_round((double)y);
	return(EPoint(_x, _y));
}


void
EPoint::PrintToStream() const
{
	ETK_OUTPUT("EPoint(%g, %g)", x, y);
}

