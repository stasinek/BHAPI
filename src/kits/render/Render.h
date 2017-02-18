/* --------------------------------------------------------------------------
 *
 * BHAPI++ previously named ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
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
 * File: Render.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_RENDER__H
#define BHAPI_RENDER__H
#include <kits/support/SupportDefs.h>
#include "../interface/GraphicsDefs.h"
#include "../interface/Rect.h"
#include "../interface/Polygon.h"

#ifdef __cplusplus /* Just for C++ */

class IMPEXP_BHAPI BRender {
public:
    BRender();
    virtual ~BRender();

	bool		IsValid() const;

    void		SetDrawingMode(b_drawing_mode drawing_mode);
	b_drawing_mode	DrawingMode() const;

    void		SetHighColor(b_rgb_color highColor);
    void		SetHighColor(b_uint8 r, b_uint8 g, b_uint8 b, b_uint8 a = 255);
	b_rgb_color	HighColor() const;

    void		SetLowColor(b_rgb_color lowColor);
    void		SetLowColor(b_uint8 r, b_uint8 g, b_uint8 b, b_uint8 a = 255);
	b_rgb_color	LowColor() const;

	void		SetPenSize(float pen_size);
	float		PenSize() const;

	void		SetSquarePointStyle(bool state);
	bool		IsSquarePointStyle() const;

    void		FillRect(b_int32 x, b_int32 y, b_uint32 width, b_uint32 height,
				 b_pattern pattern = B_SOLID_HIGH);
    void		FillRect(BRect rect, b_pattern pattern = B_SOLID_HIGH);

    void		StrokePoint(b_int32 x, b_int32 y, b_pattern pattern = B_SOLID_HIGH);
    void		StrokePoint(BPoint pt, b_pattern pattern = B_SOLID_HIGH);

    void		StrokeLine(b_int32 x0, b_int32 y0, b_int32 x1, b_int32 y1,
				   b_pattern pattern = B_SOLID_HIGH);
    void		StrokeLine(BPoint pt0, BPoint pt1, b_pattern pattern = B_SOLID_HIGH);

    void		StrokeTriangle(b_int32 x0, b_int32 y0, b_int32 x1, b_int32 y1, b_int32 x2, b_int32 y2, b_pattern pattern = B_SOLID_HIGH);
    void		StrokeTriangle(BPoint pt1, BPoint pt2, BPoint pt3, b_pattern pattern = B_SOLID_HIGH);
    void		FillTriangle(b_int32 x0, b_int32 y0, b_int32 x1, b_int32 y1, b_int32 x2, b_int32 y2,
				     bool stroke_edge = true, b_pattern pattern = B_SOLID_HIGH);
    void		FillTriangle(BPoint pt0, BPoint pt1, BPoint pt2, bool stroke_edge = true, b_pattern pattern = B_SOLID_HIGH);

    void		StrokePolygon(const BPolygon *aPolygon, bool closed = true, b_pattern pattern = B_SOLID_HIGH);
    void		StrokePolygon(const BPoint *ptArray, b_int32 numPts, bool closed = true, b_pattern pattern = B_SOLID_HIGH);
    void		FillPolygon(const BPolygon *aPolygon, bool stroke_edge = true, b_pattern pattern = B_SOLID_HIGH);
    void		FillPolygon(const BPoint *ptArray, b_int32 numPts, bool stroke_edge = true, b_pattern pattern = B_SOLID_HIGH);

    void		StrokeEllipse(b_int32 x, b_int32 y, b_uint32 width, b_uint32 height,
				      b_pattern pattern = B_SOLID_HIGH);
    void		StrokeEllipse(b_int32 xCenter, b_int32 yCenter, b_int32 xRadius, b_int32 yRadius,
				      b_pattern pattern = B_SOLID_HIGH);
    void		StrokeEllipse(BRect rect, b_pattern pattern = B_SOLID_HIGH);
    void		FillEllipse(b_int32 x, b_int32 y, b_uint32 width, b_uint32 height,
				    bool stroke_edge = true, b_pattern pattern = B_SOLID_HIGH);
    void		FillEllipse(b_int32 xCenter, b_int32 yCenter, b_int32 xRadius, b_int32 yRadius,
				    bool stroke_edge = true, b_pattern pattern = B_SOLID_HIGH);
    void		FillEllipse(BRect rect, bool stroke_edge = true, b_pattern pattern = B_SOLID_HIGH);

    void		StrokeArc(b_int32 x, b_int32 y, b_uint32 width, b_uint32 height,
                  b_int32 startAngle, b_int32 endAngle,
				  b_pattern pattern = B_SOLID_HIGH);
    void		StrokeArc(b_int32 xCenter, b_int32 yCenter, b_int32 xRadius, b_int32 yRadius,
                  b_int32 startAngle, b_int32 endAngle,
				  b_pattern pattern = B_SOLID_HIGH);
    void		StrokeArc(BPoint ctPt, float xRadius, float yRadius,
				  float startAngle, float arcAngle,
				  b_pattern pattern = B_SOLID_HIGH);
    void		StrokeArc(BRect rect, float startAngle, float arcAngle, b_pattern pattern = B_SOLID_HIGH);
    void		StrokeArc(BRect rect, BPoint start, BPoint end, b_pattern pattern = B_SOLID_HIGH);

private:
	b_drawing_mode fDrawingMode;
	b_rgb_color fHighColor;
	b_rgb_color fLowColor;
	float fPenSize;
	bool fSquarePointStyle;

	virtual b_status_t InitCheck() const = 0;
    virtual void GetFrame(b_int32 *originX, b_int32 *originY, b_uint32 *width, b_uint32 *height) const = 0;
    virtual void GetPixel(b_int32 x, b_int32 y, b_rgb_color &color) const = 0;
    virtual void PutPixel(b_int32 x, b_int32 y, b_rgb_color color) = 0;
    virtual void PutRect(b_int32 x, b_int32 y, b_uint32 width, b_uint32 height, b_rgb_color color);

    void drawPixel(b_int32 x, b_int32 y, b_pattern pattern);
};

#endif /* __cplusplus */

#endif /* BHAPI_RENDER__H */

