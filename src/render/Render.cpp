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
 * File: Render.cpp
 *
 * --------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include <math.h>

#ifndef M_PI
	#define M_PI	3.14159265358979323846
#endif // M_PI

#include "./../support/List.h"

#include "LineGenerator.h"
#include "ArcGenerator.h"
#include "Render.h"

#ifdef _MSC_VER
	#define isnan(a)	_isnan(a)
#endif


extern bool etk_get_arc_12(EPoint &radius, EPoint &start, EPoint &end, eint32 &x, eint32 &y, EPoint &radius2, float &deltaNext);


#define SWAP(CLASS, a, b)	\
	do {			\
		CLASS tmp = a;	\
		a = b;		\
		b = tmp;	\
	} while(false)


class _LOCAL ERenderObject {
public:
	ERenderObject();
	virtual ~ERenderObject();

	virtual bool	IsValid() const = 0;
	virtual bool	Get(eint32 *y, eint32 *minX, eint32 *maxX) const = 0;
	virtual bool	Next() = 0;

	static int	cmp(const void *objectA, const void *objectB);
};


ERenderObject::ERenderObject()
{
}


ERenderObject::~ERenderObject()
{
}


int
ERenderObject::cmp(const void *objectA, const void *objectB)
{
	const ERenderObject *A = *((const ERenderObject**)objectA);
	const ERenderObject *B = *((const ERenderObject**)objectB);

	if(A->IsValid() == B->IsValid())
	{
		if(A->IsValid() == false) return 0;

		eint32 yA, yB;
		A->Get(&yA, NULL, NULL);
		B->Get(&yB, NULL, NULL);

		if(yA == yB) return 0;
		return(yA < yB ? -1 : 1);
	}

	return(A->IsValid() ? -1 : 1);
}


class _LOCAL ERenderLine : public ERenderObject {
public:
	ERenderLine(EPoint start, EPoint end);

	virtual bool	IsValid() const;
	virtual bool	Get(eint32 *y, eint32 *minX, eint32 *maxX) const;
	virtual bool	Next();

private:
	ELineGenerator fLine;
	eint32 fMinX;
	eint32 fMaxX;
	eint32 fX;
	eint32 fY;
	bool fValid;
};


ERenderLine::ERenderLine(EPoint start, EPoint end)
	: ERenderObject(), fLine(start, end), fValid(false)
{
	if(start.y > end.y) return;

	eint32 step, pixels;

	if(fLine.Start(fX, fY, step, pixels, false, 1))
	{
		fValid = true;
		fMinX = min_c(fX, fX + pixels);
		fMaxX = max_c(fX, fX + pixels);
	}
}


bool
ERenderLine::IsValid() const
{
	return fValid;
}


bool
ERenderLine::Get(eint32 *y, eint32 *minX, eint32 *maxX) const
{
	if(fValid == false) return false;

	if(y) *y = fY;
	if(minX) *minX = fMinX;
	if(maxX) *maxX = fMaxX;

	return true;
}


bool
ERenderLine::Next()
{
	if(fValid == false) return false;

	eint32 pixels;

	fY++;
	if(fLine.Next(fX, pixels) == false) {fValid = false; return false;}

	fMinX = min_c(fX, fX + pixels);
	fMaxX = max_c(fX, fX + pixels);

	return true;
}


class _LOCAL ERenderLine2 : public ERenderObject {
public:
	ERenderLine2(EPoint pt0, EPoint pt1, EPoint pt2);

	virtual bool	IsValid() const;
	virtual bool	Get(eint32 *y, eint32 *minX, eint32 *maxX) const;
	virtual bool	Next();

private:
	ELineGenerator fLine1;
	ELineGenerator fLine2;

	eint32 fMinX;
	eint32 fMaxX;

	eint32 fX1;
	eint32 fX2;
	eint32 fY1;
	eint32 fY2;

	bool fValid;
};


ERenderLine2::ERenderLine2(EPoint pt0, EPoint pt1, EPoint pt2)
	: ERenderObject(), fLine1(pt0, pt1), fLine2(pt1, pt2), fValid(false)
{
	if(pt0.y > pt1.y || pt1.y > pt2.y) return;

	eint32 tmp, pixels1, pixels2;

	if(fLine1.Start(fX1, fY1, tmp, pixels1, false, 1) && fLine2.Start(fX2, fY2, tmp, pixels2, false, 1))
	{
		fValid = true;

		fMinX = min_c(fX1, fX1 + pixels1);
		fMaxX = max_c(fX1, fX1 + pixels1);

		if(fY2 == fY1)
		{
			fMinX = min_c(fMinX, min_c(fX2, fX2 + pixels2));
			fMaxX = max_c(fMaxX, max_c(fX2, fX2 + pixels2));
		}
	}
}


bool
ERenderLine2::IsValid() const
{
	return fValid;
}


bool
ERenderLine2::Get(eint32 *y, eint32 *minX, eint32 *maxX) const
{
	if(fValid == false) return false;

	if(y) *y = fY1;
	if(minX) *minX = fMinX;
	if(maxX) *maxX = fMaxX;

	return true;
}


bool
ERenderLine2::Next()
{
	if(fValid == false) return false;

	eint32 pixels1, pixels2;
	bool line1_has_next = false;

	fY1++;
	if(fY1 <= fY2) if((line1_has_next = fLine1.Next(fX1, pixels1)) == false) fValid = (fY1 == fY2);
	if(fY1 > fY2) if(fLine2.Next(fX2, pixels2) == false) fValid = false;
	if(fValid == false) return false;

	if(line1_has_next)
	{
		fMinX = min_c(fX1, fX1 + pixels1);
		fMaxX = max_c(fX1, fX1 + pixels1);

		if(fY1 > fY2)
		{
			fMinX = min_c(fMinX, min_c(fX2, fX2 + pixels2));
			fMaxX = max_c(fMaxX, max_c(fX2, fX2 + pixels2));
		}
	}
	else
	{
		fMinX = min_c(fX2, fX2 + pixels2);
		fMaxX = max_c(fX2, fX2 + pixels2);
	}

	return true;
}


class _LOCAL ERenderTriangle : public ERenderLine2 {
public:
	ERenderTriangle(EPoint pt0, EPoint pt1, EPoint pt2, bool stroke_edge);

	virtual bool	IsValid() const;
	virtual bool	Get(eint32 *y, eint32 *minX, eint32 *maxX) const;
	virtual bool	Next();

private:
	ELineGenerator fLine1;
	euint8 fFlags;

	bool fStrokeEdge;

	eint32 fY;
	eint32 fY0;
	eint32 fY1;

	eint32 fX1;
	eint32 fPixels1;
};


ERenderTriangle::ERenderTriangle(EPoint pt0, EPoint pt1, EPoint pt2, bool stroke_edge)
	: ERenderLine2(pt0, pt1, pt2), fLine1(pt0, pt2), fFlags(0x00), fStrokeEdge(stroke_edge)
{
	eint32 tmp;

	if(ERenderLine2::Get(&fY0, NULL, NULL) == false || fLine1.Start(fX1, fY1, tmp, fPixels1, false, 1) == false) return;
	fY = min_c(fY0, fY1);

	fFlags = 0x03;
}


bool
ERenderTriangle::IsValid() const
{
	return(fFlags != 0x00);
}


bool
ERenderTriangle::Get(eint32 *y, eint32 *_minX, eint32 *_maxX) const
{
	if(fFlags == 0x00) return false;

	if(y) *y = fY;

	if(_minX || _maxX)
	{
		eint32 minX = 0, maxX = -1;

		if(fFlags != 0x03)
		{
			if(fStrokeEdge)
			{
				if(fFlags == 0x01)
				{
					ERenderLine2::Get(NULL, &minX, &maxX);
				}
				else
				{
					minX = min_c(fX1, fX1 + fPixels1);
					maxX = max_c(fX1, fX1 + fPixels1);
				}
			}
		}
		else
		{
			eint32 minX0, maxX0;
			ERenderLine2::Get(NULL, &minX0, &maxX0);

			if(fY < max_c(fY0, fY1))
			{
				if(fStrokeEdge)
				{
					minX = fY0 < fY1 ? minX0 : min_c(fX1, fX1 + fPixels1);
					maxX = fY0 < fY1 ? maxX0 : max_c(fX1, fX1 + fPixels1);
				}
			}
			else
			{
				minX = min_c(minX0, min_c(fX1, fX1 + fPixels1));
				maxX = max_c(maxX0, max_c(fX1, fX1 + fPixels1));

				if(fStrokeEdge == false)
				{
#define SIMPLE_EXCLUDE(Start, End, f, t)		\
		if(Start <= End)			\
		{					\
			if(Start >= f) Start = t + 1;	\
			if(End <= t) End = f - 1;	\
		} (void)0
					SIMPLE_EXCLUDE(minX, maxX, minX0, maxX0);
					SIMPLE_EXCLUDE(minX, maxX, min_c(fX1, fX1 + fPixels1), max_c(fX1, fX1 + fPixels1));
#undef SIMPLE_EXCLUDE
				}
			}
		}

		if(_minX) *_minX = minX;
		if(_maxX) *_maxX = maxX;
	}

	return true;
}


bool
ERenderTriangle::Next()
{
	if(fFlags == 0x00) return false;

	if(fY >= fY0)
	{
		if(ERenderLine2::Next() == false) fFlags &= ~0x01;
	}

	if(fY >= fY1)
	{
		if(fLine1.Next(fX1, fPixels1) == false) fFlags &= ~0x02;
	}

	fY++;

	return(fFlags != 0x00);
}


ERender::ERender()
	: fDrawingMode(E_OP_COPY), fPenSize(0), fSquarePointStyle(false)
{
	fHighColor.set_to(0, 0, 0);
	fLowColor.set_to(255, 255, 255);
}


ERender::~ERender()
{
}


bool
ERender::IsValid() const
{
	return(InitCheck() == E_OK);
}


void
ERender::SetDrawingMode(e_drawing_mode drawing_mode)
{
	fDrawingMode = drawing_mode;
}


e_drawing_mode
ERender::DrawingMode() const
{
	return fDrawingMode;
}


void
ERender::SetHighColor(e_rgb_color highColor)
{
	fHighColor.set_to(highColor);
}


void
ERender::SetHighColor(euint8 r, euint8 g, euint8 b, euint8 a)
{
	e_rgb_color color;
	color.set_to(r, g, b, a);
	SetHighColor(color);
}


e_rgb_color
ERender::HighColor() const
{
	return fHighColor;
}


void
ERender::SetLowColor(e_rgb_color lowColor)
{
	fLowColor.set_to(lowColor);
}


void
ERender::SetLowColor(euint8 r, euint8 g, euint8 b, euint8 a)
{
	e_rgb_color color;
	color.set_to(r, g, b, a);
	SetLowColor(color);
}


e_rgb_color
ERender::LowColor() const
{
	return fLowColor;
}


void
ERender::SetPenSize(float pen_size)
{
	fPenSize = pen_size;
}


float
ERender::PenSize() const
{
	return fPenSize;
}


void
ERender::SetSquarePointStyle(bool state)
{
	fSquarePointStyle = state;
}


bool
ERender::IsSquarePointStyle() const
{
	return fSquarePointStyle;
}


void
ERender::PutRect(eint32 x, eint32 y, euint32 width, euint32 height, e_rgb_color color)
{
	if(width == 0 || height == 0) return;

	for(euint32 i = 0; i < height; i++, y++)
	{
		for(euint32 j = 0; j < width; j++) PutPixel(x + j, y, color);
	}
}


inline bool _is_pixel_high_color(const e_pattern &pattern, eint32 x, eint32 y)
{
	if(pattern == E_SOLID_HIGH) return true;
	else if(pattern == E_SOLID_LOW) return false;

	x %= 8;
	y %= 8;

	euint8 pat = pattern.data[y];
	if(pat & (1 << (7 - x))) return true;

	return false;
}


inline bool _is_line_mixed_color(const e_pattern &pattern, eint32 y, bool &isHighColor)
{
	if(pattern == E_SOLID_HIGH) {isHighColor = true; return false;}
	else if(pattern == E_SOLID_LOW) {isHighColor = false; return false;}

	y %= 8;

	if(pattern.data[y] == 0x00 || pattern.data[y] == 0xff)
	{
		isHighColor = (pattern.data[y] == 0xff);
		return false;
	}

	return true;
}


void
ERender::drawPixel(eint32 x, eint32 y, e_pattern pattern)
{
	if(!IsValid()) return;

	eint32 originX = 0, originY = 0;
	euint32 w = 0, h = 0;
	GetFrame(&originX, &originY, &w, &h);

	if(x < originX || y < originY || x > originX + (eint32)w - 1 ||  y > originY + (eint32)h - 1) return;

	e_rgb_color src = {0, 0, 0, 255};
	if(fDrawingMode != E_OP_COPY) GetPixel(x, y, src);
	euint32 srcAlpha = src.alpha;

	e_rgb_color color;
	color.set_to(_is_pixel_high_color(pattern, x, y) ? fHighColor : fLowColor);

	switch(fDrawingMode)
	{
		case E_OP_OVER:
		case E_OP_ERASE:
			if(color == E_TRANSPARENT_COLOR || color == fLowColor) return;
			src.set_to((fDrawingMode == E_OP_ERASE) ? fLowColor : color);
			src.alpha = srcAlpha;
			break;

		case E_OP_XOR:
			src.set_to(color.red ^ src.red, color.green ^ src.green, color.blue ^ src.blue, srcAlpha);
			break;

		case E_OP_INVERT:
			if(color == E_TRANSPARENT_COLOR || color == fLowColor) return;
			src.set_to(255 - src.red, 255 - src.green, 255 - src.blue, srcAlpha);
			break;

		case E_OP_SELECT:
			{
				if(color == E_TRANSPARENT_COLOR || color == fLowColor) return;
				if(src.blue == fHighColor.blue && src.green == fHighColor.green && src.red == fHighColor.red)
				{
					src.set_to(fLowColor);
					src.alpha = srcAlpha;
				}
				else if(src.blue == fLowColor.blue && src.green == fLowColor.green && src.red == fLowColor.red)
				{
					src.set_to(fHighColor);
					src.alpha = srcAlpha;
				}
			}
			break;

		case E_OP_ADD:
			if(color == E_TRANSPARENT_COLOR || color == fLowColor) return;
			src.red += color.red;
			src.green += color.green;
			src.blue += color.blue;
			break;

		case E_OP_SUBTRACT:
			if(color == E_TRANSPARENT_COLOR || color == fLowColor) return;
			src.red -= color.red;
			src.green -= color.green;
			src.blue -= color.blue;
			break;

		case E_OP_BLEND:
			if(color == E_TRANSPARENT_COLOR || color == fLowColor) return;
			src.red = (euint8)(((euint16)color.red + (euint16)src.red) / 2U);
			src.green = (euint8)(((euint16)color.green + (euint16)src.green) / 2U);
			src.blue = (euint8)(((euint16)color.blue + (euint16)src.blue) / 2U);
			break;

		case E_OP_MIN:
			if(color == E_TRANSPARENT_COLOR || color == fLowColor) return;
			if(color.red < src.red) src.red = color.red;
			if(color.green < src.green) src.green = color.green;
			if(color.blue < src.blue) src.blue = color.blue;
			break;

		case E_OP_MAX:
			if(color == E_TRANSPARENT_COLOR || color == fLowColor) return;
			if(color.red > src.red) src.red = color.red;
			if(color.green > src.green) src.green = color.green;
			if(color.blue > src.blue) src.blue = color.blue;
			break;

		case E_OP_ALPHA:
			{
				if(color.alpha == 0) return;
				src.mix(color);
			}
			break;

		default:
			src.set_to(color);
			src.alpha = srcAlpha;
			break;
	}

	PutPixel(x, y, src);
}


void
ERender::FillRect(eint32 x, eint32 y, euint32 width, euint32 height, e_pattern pattern)
{
	if(width == height && width == 1) {drawPixel(x, y, pattern); return;}
	if(!IsValid() || width == 0 || height == 0) return;

	eint32 originX = 0, originY = 0;
	euint32 w = 0, h = 0;
	GetFrame(&originX, &originY, &w, &h);

	ERect r((float)x, (float)y, (float)x + (float)width - 1, (float)y + (float)height - 1);
	r &= ERect((float)originX, (float)originY, (float)(originX + (eint32)w - 1), (float)(originY + (eint32)h - 1));
	if(r.IsValid() == false) return;

	x = (eint32)r.left;
	y = (eint32)r.top;
	width = (euint32)r.Width() + 1;
	height = (euint32)r.Height() + 1;

	if((pattern == E_SOLID_HIGH || pattern == E_SOLID_LOW) && fDrawingMode == E_OP_COPY)
	{
		e_rgb_color color;
		color.set_to(pattern == E_SOLID_HIGH ? fHighColor : fLowColor);
		PutRect(x, y, width, height, color);
	}
	else for(euint32 i = 0; i < height; i++, y++)
	{
		if(fDrawingMode == E_OP_COPY)
		{
			bool isHighColor;
			if(!_is_line_mixed_color(pattern, y, isHighColor))
			{
				e_rgb_color color;
				color.set_to(isHighColor ? fHighColor : fLowColor);

				PutRect(x, y, width, 1, color);
				continue;
			}
		}

		for(euint32 j = 0; j < width; j++) drawPixel(x + j, y, pattern);
	}
}


void
ERender::FillRect(ERect rect, e_pattern pattern)
{
	if(!IsValid()) return;

	rect.Floor();
	if(!rect.IsValid()) return;

	FillRect((eint32)rect.left, (eint32)rect.top, (euint32)rect.Width() + 1, (euint32)rect.Height() + 1, pattern);
}


void
ERender::StrokePoint(eint32 x, eint32 y, e_pattern pattern)
{
	if(!IsValid()) return;

	if(fPenSize <= 1)
	{
		eint32 originX = 0, originY = 0;
		euint32 w = 0, h = 0;
		GetFrame(&originX, &originY, &w, &h);

		if(x < originX || y < originY || x > originX + (eint32)w - 1 ||  y > originY + (eint32)h - 1) return;

		drawPixel(x, y, pattern);
	}
	else
	{
		ERect rect;
		rect.left = (float)x + 0.5f - fPenSize / 2.f;
		rect.top = (float)y + 0.5f - fPenSize / 2.f;
		rect.right = rect.left + fPenSize;
		rect.bottom = rect.top + fPenSize;

		if(fSquarePointStyle) FillRect(rect, pattern);
		else FillEllipse(rect, true, pattern);
	}
}


void
ERender::StrokePoint(EPoint pt, e_pattern pattern)
{
	if(!IsValid()) return;

	if(fPenSize <= 1)
	{
		eint32 x, y, originX = 0, originY = 0;
		euint32 w = 0, h = 0;

		GetFrame(&originX, &originY, &w, &h);

		pt.Floor();
		x = (eint32)pt.x;
		y = (eint32)pt.y;

		if(x < originX || y < originY || x > originX + (eint32)w - 1 ||  y > originY + (eint32)h - 1) return;

		drawPixel(x, y, pattern);
	}
	else
	{
		ERect rect;
		rect.left = pt.x - fPenSize / 2.f;
		rect.top = pt.y - fPenSize / 2.f;
		rect.right = rect.left + fPenSize;
		rect.bottom = rect.top + fPenSize;

		if(fSquarePointStyle) FillRect(rect, pattern);
		else FillEllipse(rect, true, pattern);
	}
}


void
ERender::StrokeLine(eint32 x0, eint32 y0, eint32 x1, eint32 y1, e_pattern pattern)
{
	StrokeLine(EPoint((float)x0 + 0.5f, (float)y0 + 0.5f), EPoint((float)x1 + 0.5f, (float)y1 + 0.5f), pattern);
}


void
ERender::StrokeLine(EPoint pt0, EPoint pt1, e_pattern pattern)
{
	if(!IsValid()) return;

	// TODO: clipping

	if(fPenSize > 1)
	{
		// TODO
		ETK_WARNING("[RENDER]: %s --- not support large pen yet.", __PRETTY_FUNCTION__);
		return;
	}
	else
	{
		if(pt0.y > pt1.y) SWAP(EPoint, pt0, pt1); // for compacting with FillTriangle()

		ELineGenerator line(pt0, pt1);
		eint32 x, y, step, pixels;

		if(line.Start(x, y, step, pixels, false, 1) == false) return;
		do
		{
			if(pixels >= 0) FillRect(x, y, 1 + pixels, 1, pattern);
			else FillRect(x + pixels, y, 1 - pixels, 1, pattern);
		} while(y++, line.Next(x, pixels));
	}
}


void
ERender::StrokeTriangle(eint32 x0, eint32 y0, eint32 x1, eint32 y1, eint32 x2, eint32 y2, e_pattern pattern)
{
	EPoint pts[3];
	pts[0].Set((float)x0 + 0.5f, (float)y0 + 0.5f);
	pts[1].Set((float)x1 + 0.5f, (float)y1 + 0.5f);
	pts[2].Set((float)x2 + 0.5f, (float)y2 + 0.5f);
	StrokePolygon(pts, 3, true, pattern);
}


void
ERender::StrokeTriangle(EPoint pt1, EPoint pt2, EPoint pt3, e_pattern pattern)
{
	EPoint pts[3];
	pts[0] = pt1;
	pts[1] = pt2;
	pts[2] = pt3;
	StrokePolygon(pts, 3, true, pattern);
}


void
ERender::FillTriangle(eint32 x0, eint32 y0, eint32 x1, eint32 y1, eint32 x2, eint32 y2, bool stroke_edge, e_pattern pattern)
{
	FillTriangle(EPoint((float)x0 + 0.5f, (float)y0 + 0.5f),
		     EPoint((float)x1 + 0.5f, (float)y1 + 0.5f),
		     EPoint((float)x2 + 0.5f, (float)y2 + 0.5f),
		     stroke_edge, pattern);
}


void
ERender::FillTriangle(EPoint pt0, EPoint pt1, EPoint pt2, bool stroke_edge, e_pattern pattern)
{
	if(!IsValid()) return;

	// TODO: clipping

	if(pt0.y > pt1.y) SWAP(EPoint, pt0, pt1);
	if(pt0.y > pt2.y) SWAP(EPoint, pt0, pt2);
	if(pt1.y > pt2.y) SWAP(EPoint, pt1, pt2);

	ERenderTriangle triangle(pt0, pt1, pt2, stroke_edge);

	do
	{
		eint32 y, minX, maxX;
		if(triangle.Get(&y, &minX, &maxX) == false) break;
		if(minX <= maxX) FillRect(minX, y, maxX - minX + 1, 1, pattern);
	} while(triangle.Next());
}


void
ERender::StrokeEllipse(eint32 x, eint32 y, euint32 width, euint32 height, e_pattern pattern)
{
	if(!IsValid() || width == 0 || height == 0) return;

	ERect rect;
	rect.SetLeftTop((float)x + 0.5f, (float)y + 0.5f);
	rect.SetRightBottom((float)x + (float)width - 0.5f, (float)y + (float)height - 0.5f);

	StrokeEllipse(rect, pattern);
}


void
ERender::FillEllipse(eint32 x, eint32 y, euint32 width, euint32 height, bool stroke_edge, e_pattern pattern)
{
	if(!IsValid() || width == 0 || height == 0) return;

	ERect rect;
	rect.SetLeftTop((float)x + 0.5f, (float)y + 0.5f);
	rect.SetRightBottom((float)x + (float)width - 0.5f, (float)y + (float)height - 0.5f);

	FillEllipse(rect, stroke_edge, pattern);
}


void
ERender::StrokeEllipse(eint32 xCenter, eint32 yCenter, eint32 xRadius, eint32 yRadius, e_pattern pattern)
{
	if(!IsValid()) return;

	if(xRadius < 0) xRadius = -xRadius;
	if(yRadius < 0) yRadius = -yRadius;

	ERect rect;
	rect.left = (float)(xCenter - xRadius);
	rect.right = (float)(xCenter + xRadius);
	rect.top = (float)(yCenter - yRadius);
	rect.bottom = (float)(yCenter + yRadius);

	rect.OffsetBy(0.5, 0.5);

	StrokeEllipse(rect, pattern);
}


void
ERender::FillEllipse(eint32 xCenter, eint32 yCenter, eint32 xRadius, eint32 yRadius, bool stroke_edge, e_pattern pattern)
{
	if(!IsValid()) return;

	if(xRadius < 0) xRadius = -xRadius;
	if(yRadius < 0) yRadius = -yRadius;

	ERect rect;
	rect.left = (float)(xCenter - xRadius);
	rect.right = (float)(xCenter + xRadius);
	rect.top = (float)(yCenter - yRadius);
	rect.bottom = (float)(yCenter + yRadius);

	rect.OffsetBy(0.5, 0.5);

	FillEllipse(rect, stroke_edge, pattern);
}


void
ERender::StrokeEllipse(ERect rect, e_pattern pattern)
{
	if(!IsValid() || !rect.IsValid()) return;

	// TODO: clipping

	if(fPenSize > 1)
	{
		// TODO
		ETK_WARNING("[RENDER]: %s --- not support large pen yet.", __PRETTY_FUNCTION__);
		return;
	}
	else
	{
		EPoint radius;
		radius.x = rect.Width() / 2.f;
		radius.y = rect.Height() / 2.f;

		EPoint start, end, radius2;
		start.Set(-radius.x, 0);
		end.Set(radius.x, 0);
		radius2.Set(-1, -1);

		eint32 x, y;
		eint32 xCenter, yCenter;
		float deltaNext;

		EPoint center = rect.Center().FloorSelf();
		xCenter = (eint32)center.x;
		yCenter = (eint32)center.y;

		while(etk_get_arc_12(radius, start, end, x, y, radius2, deltaNext))
		{
			if(x > 0) break;

			drawPixel(xCenter + x, yCenter + y, pattern);
			if(x != 0) drawPixel(xCenter - x, yCenter + y, pattern);

			if(y == 0) continue;
			drawPixel(xCenter + x, yCenter - y, pattern);
			if(x != 0) drawPixel(xCenter - x, yCenter - y, pattern);
		}
	}
}


void
ERender::FillEllipse(ERect rect, bool stroke_edge, e_pattern pattern)
{
	if(!IsValid() || !rect.IsValid()) return;

	// TODO: clipping

	EPoint radius;
	radius.x = rect.Width() / 2.f;
	radius.y = rect.Height() / 2.f;

	EPoint start, end, radius2;
	start.Set(-radius.x, 0);
	end.Set(radius.x, 0);
	radius2.Set(-1, -1);

	eint32 x, y, old_x = 0, old_y = 0, last_y = 0;
	eint32 xCenter, yCenter;
	float deltaNext;
	bool first = true;

	EPoint center = rect.Center().FloorSelf();
	xCenter = (eint32)center.x;
	yCenter = (eint32)center.y;

	while(true)
	{
		bool status = etk_get_arc_12(radius, start, end, x, y, radius2, deltaNext);

		if(first)
		{
			if(!status) break;
			old_x = x; old_y = last_y = y; first = false;
		}

		if(status && old_x == x) {last_y = y; continue;}

		if(stroke_edge)
			last_y = min_c(old_y, last_y);
		else
			last_y = max_c(old_y, last_y) + 1;

		if(last_y == 0)
		{
			drawPixel(xCenter + old_x, yCenter, pattern);
			if(old_x != 0) drawPixel(xCenter - old_x, yCenter, pattern);
		}
		else if(last_y < 0)
		{
			FillRect(xCenter + old_x, yCenter + last_y, 1, -(last_y * 2 - 1), pattern);
			if(old_x != 0) FillRect(xCenter - old_x, yCenter + last_y, 1, -(last_y * 2 - 1), pattern);
		}

		if(!status || x > 0) break;
		old_x = x; old_y = last_y = y;
	}
}


void
ERender::StrokeArc(eint32 x, eint32 y, euint32 width, euint32 height,
		   eint32 startAngle, eint32 endAngle, e_pattern pattern)
{
	if(!IsValid() || width == 0 || height == 0) return;

	ERect rect;
	rect.SetLeftTop((float)x + 0.5f, (float)y + 0.5f);
	rect.SetRightBottom((float)x + (float)width - 0.5f, (float)y + (float)height - 0.5f);

	StrokeArc(rect, (float)startAngle / 64.f, (float)(endAngle - startAngle) / 64.f, pattern);
}


void
ERender::StrokeArc(eint32 xCenter, eint32 yCenter, eint32 xRadius, eint32 yRadius,
		   eint32 startAngle, eint32 endAngle, e_pattern pattern)
{
	if(!IsValid()) return;

	EPoint ctPt((float)xCenter + 0.5f, (float)yCenter + 0.5f);

	StrokeArc(ctPt, (float)xRadius, (float)yRadius, (float)startAngle / 64.f, (float)(endAngle - startAngle) / 64.f, pattern);
}


void
ERender::StrokeArc(EPoint ctPt, float xRadius, float yRadius, float startAngle, float arcAngle, e_pattern pattern)
{
	if(!IsValid()) return;

	if(xRadius < 0) xRadius = -xRadius;
	if(yRadius < 0) yRadius = -yRadius;

	ERect rect;
	rect.SetLeftTop(ctPt - EPoint(xRadius, yRadius));
	rect.SetRightBottom(ctPt + EPoint(xRadius, yRadius));

	StrokeArc(rect, startAngle, arcAngle, pattern);
}


void
ERender::StrokeArc(ERect rect, float startAngle, float arcAngle, e_pattern pattern)
{
	if(!IsValid() || !rect.IsValid()) return;

	startAngle = (float)fmod((double)startAngle, (double)360);
	if(arcAngle >= 360.f || arcAngle <= -360.f) arcAngle = 360;
	if(startAngle < 0) startAngle += 360.f;

	EPoint radius;
	radius.x = rect.Width() / 2.f;
	radius.y = rect.Height() / 2.f;

	EPoint start, end;
	start.x = rect.Center().x + radius.x * (float)cos(M_PI * (double)startAngle / 180.f);
	start.y = rect.Center().y - radius.y * (float)sin(M_PI * (double)startAngle / 180.f);
	end.x = rect.Center().x + radius.x * (float)cos(M_PI * (double)(startAngle + arcAngle) / 180.f);
	end.y = rect.Center().y - radius.y * (float)sin(M_PI * (double)(startAngle + arcAngle) / 180.f);

	StrokeArc(rect, start, end, pattern);
}


void
ERender::StrokeArc(ERect rect, EPoint start, EPoint end, e_pattern pattern)
{
	if(!IsValid() || !rect.IsValid()) return;

	// TODO: clipping

	if(fPenSize > 1)
	{
		// TODO
		ETK_WARNING("[RENDER]: %s --- not support large pen yet.", __PRETTY_FUNCTION__);
		return;
	}
	else
	{
		EArcGenerator arc(rect.Center(), rect.Width() / 2.f, rect.Height() / 2.f, start, end);

		eint32 x, y, step, pixels, centerY = (eint32)floor((double)rect.Center().y);
		bool firstTime = true;
		bool both = false;

		while(firstTime ? arc.Start(x, y, step, pixels, both, true, 1) : arc.Next(y, pixels, both))
		{
			if(!firstTime)
				x += (step > 0 ? 1 : -1);
			else
				firstTime = false;

			if(pixels == 0)
			{
				drawPixel(x, y, pattern);
				if(both && y != centerY - (y - centerY)) drawPixel(x, centerY - (y - centerY), pattern);
			}
			else if(pixels > 0)
			{
				FillRect(x, y, 1, 1 + pixels, pattern);
				if(both)
				{
					eint32 y1 = centerY - (y - centerY) - pixels;
					if(y1 == y + pixels) {y1--; pixels--;}
					if(pixels > 0) FillRect(x, y1, 1, 1 + pixels, pattern);
				}
			}
			else /* pixels < 0 */
			{
				FillRect(x, y + pixels, 1, 1 - pixels, pattern);
				if(both)
				{
					eint32 y1 = centerY - (y - centerY);
					if(y1 == y) {y1++; pixels++;}
					if(pixels < 0) FillRect(x, y1, 1, 1 - pixels, pattern);
				}
			}
		}
	}
}


static bool etk_get_line_intersection(EPoint line0_start, EPoint line0_end, EPoint line1_start, EPoint line1_end,
				      EPoint *pt, bool line0_extend = false, bool line1_extend = false)
{
	if(pt == NULL) return false;

	if(line0_start == line1_start || line0_start == line1_end) {*pt = line0_start; return true;}
	if(line0_end == line1_end) {*pt = line0_end; return true;}

	ERect rect0, rect1;
	rect0.SetLeftTop(min_c(line0_start.x, line0_end.x), min_c(line0_start.y, line0_end.y));
	rect0.SetRightBottom(max_c(line0_start.x, line0_end.x), max_c(line0_start.y, line0_end.y));
	rect1.SetLeftTop(min_c(line1_start.x, line1_end.x), min_c(line1_start.y, line1_end.y));
	rect1.SetRightBottom(max_c(line1_start.x, line1_end.x), max_c(line1_start.y, line1_end.y));

	ERect r = (rect0 & rect1);
	if(r.IsValid() == false && line0_extend == false && line1_extend == false) return false;

	if(r.LeftTop() == r.RightBottom())
	{
		if(r.LeftTop() == line0_start || r.LeftTop() == line0_end || r.LeftTop() == line1_start || r.LeftTop() == line1_end)
		{
			*pt = r.LeftTop();
			return true;
		}
		if(line0_extend == false && line1_extend == false) return false;
	}

	EPoint delta0 = line0_end - line0_start;
	EPoint delta1 = line1_end - line1_start;

	float a = delta0.y * delta1.x - delta1.y * delta0.x;
	if(a == 0.f || line0_end == line1_start)
	{
		if(a != 0.f) {*pt = line0_end; return true;}
		if(line0_end != line1_start) return false; // parallel or same

		// same line, here it return the point for polygon-drawing
		*pt = (delta0.x * delta0.x + delta0.y * delta0.y > delta1.x * delta1.x + delta1.y * delta1.y ? line1_end : line0_start);
		return true;
	}

	float b = delta0.x * delta1.x * (line1_start.y - line0_start.y) +
		  (line0_start.x * delta0.y * delta1.x - line1_start.x * delta1.y * delta0.x);
	pt->x = b / a;
	if(isnan(pt->x)) return false;

	if(delta0.x != 0.f) pt->y = line0_start.y + (pt->x - line0_start.x) * delta0.y / delta0.x;
	if(delta0.x == 0.f || isnan(pt->y)) pt->y = line1_start.y + (pt->x - line1_start.x) * delta1.y / delta1.x;
	if(isnan(pt->y)) return false;

	return((line0_extend || rect0.Contains(*pt)) && (line1_extend || rect1.Contains(*pt)));
}


void
ERender::StrokePolygon(const EPolygon *aPolygon, bool closed, e_pattern pattern)
{
	if(!IsValid() || aPolygon == NULL) return;
	StrokePolygon(aPolygon->Points(), aPolygon->CountPoints(), closed, pattern);
}


#define INTERSECTS(v, s1, e1, s2, e2)		\
	(max_c(s1, s2) <= min_c(e1, e2) ? ((v = (((eint64)min_c(s1, s2)) << 32) | (eint64)max_c(e1, e2)), true) : false)
static void include_region(eint64 *region, eint32 *count, eint32 minX, eint32 maxX)
{
	if(minX > maxX) return;

	eint32 i = 0;
	while(i < *count)
	{
		eint64 v0 = *(region + i);

		if(INTERSECTS(v0, (eint32)(v0 >> 32), (eint32)(v0 & 0xffffffff), minX, maxX) == false) {i++; continue;}

		minX = (eint32)(v0 >> 32);
		maxX = (eint32)(v0 & 0xffffffff);

		if(i < *count - 1) memmove(region + i, region + i + 1, (size_t)(*count - i - 1) * sizeof(eint64));
		(*count) -= 1;
	}

	*(region + ((*count)++)) = (((eint64)minX) << 32) | (eint64)maxX;
}
#undef INTERSECTS


static void etk_stroke_objects(ERender *render, EList *objects, e_pattern pattern)
{
	if(objects->CountItems() <= 0) return;

	ERenderObject *aObject;

	objects->SortItems(ERenderObject::cmp);

	eint64 *region = (eint64*)malloc(sizeof(eint64) * objects->CountItems());
	if(region == NULL)
	{
		while((aObject = (ERenderObject*)objects->RemoveItem((eint32)0)) != NULL) delete aObject;
		return;
	}

	while(objects->CountItems() > 0)
	{
		eint32 curY, tmpY, minX, maxX;
		eint32 count = 0;

		aObject = (ERenderObject*)objects->FirstItem();
		if(aObject->Get(&curY, &minX, &maxX) == false)
		{
			objects->RemoveItem((eint32)0);
			delete aObject;
			continue;
		}

		include_region(region, &count, minX, maxX);

		for(eint32 i = 1; i < objects->CountItems(); i++)
		{
			aObject = (ERenderObject*)objects->ItemAt(i);

			if(aObject->Get(&tmpY, &minX, &maxX) == false)
			{
				objects->RemoveItem(i);
				delete aObject;
				i--;
				continue;
			}

			if(tmpY != curY) break;
			aObject->Next();

			include_region(region, &count, minX, maxX);
		}

		((ERenderObject*)objects->FirstItem())->Next();

		for(eint32 i = 0; i < count; i++)
		{
			eint64 v = *(region + i);
			minX = (eint32)(v >> 32);
			maxX = (eint32)(v & 0xffffffff);

			render->FillRect(minX, curY, maxX - minX + 1, 1, pattern);
		}
	}

	free(region);
}


void
ERender::StrokePolygon(const EPoint *ptArray, eint32 numPts, bool closed, e_pattern pattern)
{
	if(!IsValid() || ptArray == NULL || numPts <= 0) return;

	// TODO: clipping

	if(numPts < 3)
	{
		StrokeLine(ptArray[0], ptArray[numPts == 1 ? 0 : 1], pattern);
		return;
	}

	if(fPenSize > 1)
	{
		// TODO
		ETK_WARNING("[RENDER]: %s --- not support large pen yet.", __PRETTY_FUNCTION__);
		return;
	}
	else
	{
		EList lines;
		EPoint pt0, pt1;

		for(eint32 k = 0; k < numPts; k++)
		{
			if(k == numPts - 1 && (closed == false || *(ptArray + k) == *ptArray)) break;

			pt0 = *(ptArray + k);
			pt1 = *(ptArray + (k < numPts - 1 ? k + 1 : 0));

			if(pt0.y > pt1.y) SWAP(EPoint, pt0, pt1);
			ERenderLine *aLine = new ERenderLine(pt0, pt1);

			if(lines.AddItem(aLine) == false) {delete aLine; break;}
		}

		etk_stroke_objects(this, &lines, pattern);
	}
}


void
ERender::FillPolygon(const EPolygon *aPolygon, bool stroke_edge, e_pattern pattern)
{
	if(!IsValid() || aPolygon == NULL) return;
	FillPolygon(aPolygon->Points(), aPolygon->CountPoints(), stroke_edge, pattern);
}


#if 0
static bool etk_triangle_contains(EPoint pt0, EPoint pt1, EPoint pt2, EPoint aPt, bool ignore_edge)
{
	if(aPt == pt0 || aPt == pt1 || aPt == pt2) return(ignore_edge ? false : true);

	ERect r;
	r.SetLeftTop(min_c(min_c(pt0.x, pt1.x), pt2.x), min_c(min_c(pt0.y, pt1.y), pt2.y));
	r.SetRightBottom(max_c(max_c(pt0.x, pt1.x), pt2.x), max_c(max_c(pt0.y, pt1.y), pt2.y));
	if(r.Contains(aPt) == false) return false;

	EPoint tmp, center;
	center.x = pt0.x / 3.f + pt1.x / 3.f + pt2.x / 3.f;
	center.y = pt0.y / 3.f + pt1.y / 3.f + pt2.y / 3.f;

	if(etk_get_line_intersection(aPt, center, pt0, pt1, &tmp)) return(ignore_edge ? false : tmp == aPt);
	if(etk_get_line_intersection(aPt, center, pt0, pt2, &tmp)) return(ignore_edge ? false : tmp == aPt);
	if(etk_get_line_intersection(aPt, center, pt1, pt2, &tmp)) return(ignore_edge ? false : tmp == aPt);

	return true;
}
#endif


void
ERender::FillPolygon(const EPoint *ptArray, eint32 numPts, bool stroke_edge, e_pattern pattern)
{
	EPoint *aPt = NULL;
	EPolygon *aPolygon = NULL;

	if(!IsValid() || ptArray == NULL || numPts <= 0) return;

	while(numPts > 3)
	{
		if(ptArray[numPts - 1] == ptArray[0]) {numPts--; continue;}
		break;
	}

	if(numPts <= 3)
	{
		FillTriangle(ptArray[0],
			     ptArray[max_c(0, min_c(numPts - 2, 1))],
			     ptArray[max_c(0, min_c(numPts - 1, 2))],
			     stroke_edge, pattern);
		return;
	}

	EList pts(numPts);
	bool readyForDraw = true;

	for(eint32 i = 0; i < numPts; i++)
	{
		if(!(i == 0 || ptArray[i] != ptArray[i - 1])) continue;

		aPt = new EPoint(ptArray[i]);
		if(pts.AddItem(aPt)) continue;

		delete aPt;
		readyForDraw = false;
		break;
	}

	EPoint psPt, pePt, sPt, ePt, iPt;
	EList polygons;
	EPolygon tmp;

	for(eint32 i = 2; readyForDraw && i <= pts.CountItems(); i++) // split to polygons
	{
		if(i < 2) continue;

		sPt = *((EPoint*)pts.ItemAt(i - 1));
		ePt = (i < pts.CountItems() ? *((EPoint*)pts.ItemAt(i)) : *((EPoint*)pts.FirstItem()));

		for(eint32 k = i; readyForDraw && k >= 2; k--)
		{
			psPt = *((EPoint*)pts.ItemAt(k - 2));
			pePt = *((EPoint*)pts.ItemAt(k - 1));

			if(etk_get_line_intersection(psPt, pePt, sPt, ePt, &iPt) == false ||
			   iPt == sPt || (iPt == ePt && i == pts.CountItems())) continue;

			aPolygon = new EPolygon(&iPt, 1);
			for(eint32 m = k - 1; m < i; m++)
			{
				aPt = (EPoint*)pts.ItemAt(k - 1);
				if(aPt == NULL || aPolygon->AddPoints(aPt, 1, false) == false) break;

				if(m == i - 1) *aPt = iPt;
				else {pts.RemoveItem(k - 1); delete aPt;}
			}

			if(aPolygon->CountPoints() != (i - k + 2) || polygons.AddItem(aPolygon) == false)
			{
				delete aPolygon;
				readyForDraw = false;
				break;
			}

			i = k - 1;

			break;
		}
	}

	while((aPt = (EPoint*)pts.RemoveItem((eint32)0)) != NULL)
	{
		if(readyForDraw) readyForDraw = tmp.AddPoints(aPt, 1, false);
		delete aPt;
	}

	aPolygon = &tmp;
	EList objects;

	do {
		if(stroke_edge) for(eint32 i = 0; readyForDraw && i < aPolygon->CountPoints(); i++)
		{
			sPt = (*aPolygon)[i];
			ePt = (*aPolygon)[(i < aPolygon->CountPoints() - 1) ? i + 1 : 0];

			if(sPt.y > ePt.y) SWAP(EPoint, sPt, ePt);
			ERenderLine *aLine = new ERenderLine(sPt, ePt);
			if(objects.AddItem(aLine) == false) {delete aLine; readyForDraw = false; break;}
		}

		while(readyForDraw && aPolygon->CountPoints() > 0)
		{
			EPolygon drawingPolygon;

			eint32 flags[2] = {0, 0};
			for(eint32 i = 0; i < aPolygon->CountPoints(); i++)
			{
				sPt = (*aPolygon)[i == 0 ? aPolygon->CountPoints() - 1: i - 1];
				iPt = (*aPolygon)[i];
				ePt = (*aPolygon)[(i < aPolygon->CountPoints() - 1) ? i + 1 : 0];

				psPt = sPt - iPt;
				pePt = ePt - iPt;

				float zValue = psPt.x * pePt.y - psPt.y * pePt.x;
				if(zValue == 0.f) continue;
				if(zValue > 0.f) flags[0] += 1;
				else flags[1] += 1;
			}

			if(flags[0] == 0 || flags[1] == 0)
			{
				drawingPolygon = *aPolygon;
				if(drawingPolygon.CountPoints() != aPolygon->CountPoints()) {readyForDraw = false; break;}
				aPolygon->RemovePoints(0, aPolygon->CountPoints() - 1, false);
			}
			else
			{
				// TODO: split polygon to polygons without nooks
				ETK_WARNING("[RENDER]: %s --- TODO", __PRETTY_FUNCTION__);
				break;
			}

			while(readyForDraw)
			{
				EPoint pt0 = drawingPolygon[0];
				EPoint pt1 = drawingPolygon[max_c(0, drawingPolygon.CountPoints() - 2)];
				EPoint pt2 = drawingPolygon[drawingPolygon.CountPoints() - 1];

				if(pt0.y > pt1.y) SWAP(EPoint, pt0, pt1);
				if(drawingPolygon.CountPoints() > 3)
				{
					ERenderLine *aLine = new ERenderLine(pt0, pt1);
					if(objects.AddItem(aLine) == false) {delete aLine; readyForDraw = false; break;}
				}

				if(pt0.y > pt2.y) SWAP(EPoint, pt0, pt2);
				if(pt1.y > pt2.y) SWAP(EPoint, pt1, pt2);
				ERenderTriangle *aTriangle = new ERenderTriangle(pt0, pt1, pt2, false);
				if(objects.AddItem(aTriangle) == false) {delete aTriangle; readyForDraw = false; break;}

				if(drawingPolygon.CountPoints() <= 3) break;
				drawingPolygon.RemovePoint(drawingPolygon.CountPoints() - 1, false);
			}
		}

		if(aPolygon != (&tmp)) delete aPolygon;
	} while((aPolygon = (EPolygon*)polygons.RemoveItem((eint32)0)) != NULL);

	if(readyForDraw)
	{
		etk_stroke_objects(this, &objects, pattern);
	}
	else
	{
		ERenderObject *aObject;
		while((aObject = (ERenderObject*)objects.RemoveItem((eint32)0)) != NULL) delete aObject;
	}
}

