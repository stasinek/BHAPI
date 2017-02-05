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
 * File: Render.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_RENDER_H__
#define __ETK_RENDER_H__

#include "./../interface/GraphicsDefs.h>
#include "./../interface/Rect.h>
#include "./../interface/Polygon.h>

#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_ETK ERender {
public:
	ERender();
	virtual ~ERender();

	bool		IsValid() const;

	void		SetDrawingMode(e_drawing_mode drawing_mode);
	e_drawing_mode	DrawingMode() const;

	void		SetHighColor(e_rgb_color highColor);
	void		SetHighColor(euint8 r, euint8 g, euint8 b, euint8 a = 255);
	e_rgb_color	HighColor() const;

	void		SetLowColor(e_rgb_color lowColor);
	void		SetLowColor(euint8 r, euint8 g, euint8 b, euint8 a = 255);
	e_rgb_color	LowColor() const;

	void		SetPenSize(float pen_size);
	float		PenSize() const;

	void		SetSquarePointStyle(bool state);
	bool		IsSquarePointStyle() const;

	void		FillRect(eint32 x, eint32 y, euint32 width, euint32 height,
				 e_pattern pattern = E_SOLID_HIGH);
	void		FillRect(ERect rect, e_pattern pattern = E_SOLID_HIGH);

	void		StrokePoint(eint32 x, eint32 y, e_pattern pattern = E_SOLID_HIGH);
	void		StrokePoint(EPoint pt, e_pattern pattern = E_SOLID_HIGH);

	void		StrokeLine(eint32 x0, eint32 y0, eint32 x1, eint32 y1,
				   e_pattern pattern = E_SOLID_HIGH);
	void		StrokeLine(EPoint pt0, EPoint pt1, e_pattern pattern = E_SOLID_HIGH);

	void		StrokeTriangle(eint32 x0, eint32 y0, eint32 x1, eint32 y1, eint32 x2, eint32 y2, e_pattern pattern = E_SOLID_HIGH);
	void		StrokeTriangle(EPoint pt1, EPoint pt2, EPoint pt3, e_pattern pattern = E_SOLID_HIGH);
	void		FillTriangle(eint32 x0, eint32 y0, eint32 x1, eint32 y1, eint32 x2, eint32 y2,
				     bool stroke_edge = true, e_pattern pattern = E_SOLID_HIGH);
	void		FillTriangle(EPoint pt0, EPoint pt1, EPoint pt2, bool stroke_edge = true, e_pattern pattern = E_SOLID_HIGH);

	void		StrokePolygon(const EPolygon *aPolygon, bool closed = true, e_pattern pattern = E_SOLID_HIGH);
	void		StrokePolygon(const EPoint *ptArray, eint32 numPts, bool closed = true, e_pattern pattern = E_SOLID_HIGH);
	void		FillPolygon(const EPolygon *aPolygon, bool stroke_edge = true, e_pattern pattern = E_SOLID_HIGH);
	void		FillPolygon(const EPoint *ptArray, eint32 numPts, bool stroke_edge = true, e_pattern pattern = E_SOLID_HIGH);

	void		StrokeEllipse(eint32 x, eint32 y, euint32 width, euint32 height,
				      e_pattern pattern = E_SOLID_HIGH);
	void		StrokeEllipse(eint32 xCenter, eint32 yCenter, eint32 xRadius, eint32 yRadius,
				      e_pattern pattern = E_SOLID_HIGH);
	void		StrokeEllipse(ERect rect, e_pattern pattern = E_SOLID_HIGH);
	void		FillEllipse(eint32 x, eint32 y, euint32 width, euint32 height,
				    bool stroke_edge = true, e_pattern pattern = E_SOLID_HIGH);
	void		FillEllipse(eint32 xCenter, eint32 yCenter, eint32 xRadius, eint32 yRadius,
				    bool stroke_edge = true, e_pattern pattern = E_SOLID_HIGH);
	void		FillEllipse(ERect rect, bool stroke_edge = true, e_pattern pattern = E_SOLID_HIGH);

	void		StrokeArc(eint32 x, eint32 y, euint32 width, euint32 height,
				  eint32 startAngle, eint32 endAngle,
				  e_pattern pattern = E_SOLID_HIGH);
	void		StrokeArc(eint32 xCenter, eint32 yCenter, eint32 xRadius, eint32 yRadius,
				  eint32 startAngle, eint32 endAngle,
				  e_pattern pattern = E_SOLID_HIGH);
	void		StrokeArc(EPoint ctPt, float xRadius, float yRadius,
				  float startAngle, float arcAngle,
				  e_pattern pattern = E_SOLID_HIGH);
	void		StrokeArc(ERect rect, float startAngle, float arcAngle, e_pattern pattern = E_SOLID_HIGH);
	void		StrokeArc(ERect rect, EPoint start, EPoint end, e_pattern pattern = E_SOLID_HIGH);

private:
	e_drawing_mode fDrawingMode;
	e_rgb_color fHighColor;
	e_rgb_color fLowColor;
	float fPenSize;
	bool fSquarePointStyle;

	virtual e_status_t InitCheck() const = 0;
	virtual void GetFrame(eint32 *originX, eint32 *originY, euint32 *width, euint32 *height) const = 0;
	virtual void GetPixel(eint32 x, eint32 y, e_rgb_color &color) const = 0;
	virtual void PutPixel(eint32 x, eint32 y, e_rgb_color color) = 0;
	virtual void PutRect(eint32 x, eint32 y, euint32 width, euint32 height, e_rgb_color color);

	void drawPixel(eint32 x, eint32 y, e_pattern pattern);
};

#endif /* __cplusplus */

#endif /* __ETK_RENDER_H__ */

