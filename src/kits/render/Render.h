/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
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

#ifndef BHAPI_RENDER_H
#define BHAPI_RENDER_H

#include <kits/interface/Polygon.h>
#include <kits/interface/Rect.h>
#include <kits/interface/GraphicsDefs.h>
#include <Haiku.h>

#ifdef __cplusplus /* Just for C++ */

class BHAPI_IMPEXP BRender {
public:
    BRender();
    virtual ~BRender();

	bool		IsValid() const;

    void		SetDrawingMode(bhapi::drawing_mode a_drawing_mode);
    bhapi::drawing_mode	DrawingMode() const;

    void		SetHighColor(bhapi::rgb_color highColor);
    void		SetHighColor(uint8 r,  uint8 g,  uint8 b,  uint8 a = 255);
    bhapi::rgb_color	HighColor() const;

    void		SetLowColor(bhapi::rgb_color lowColor);
    void		SetLowColor(uint8 r,  uint8 g,  uint8 b,  uint8 a = 255);
    bhapi::rgb_color	LowColor() const;

	void		SetPenSize(float pen_size);
	float		PenSize() const;

	void		SetSquarePointStyle(bool state);
	bool		IsSquarePointStyle() const;

    void		FillRect(int32_t x,  int32_t y,  uint32_t width,  uint32_t height,
                 bhapi::pattern pattern = bhapi::B_SOLID_HIGH);
    void		FillRect(BRect rect, bhapi::pattern pattern = bhapi::B_SOLID_HIGH);

    void		StrokePoint(int32_t x,  int32_t y, bhapi::pattern pattern = bhapi::B_SOLID_HIGH);
    void		StrokePoint(BPoint pt, bhapi::pattern pattern = bhapi::B_SOLID_HIGH);

    void		StrokeLine(int32_t x0,  int32_t y0,  int32_t x1,  int32_t y1,
                   bhapi::pattern pattern = bhapi::B_SOLID_HIGH);
    void		StrokeLine(BPoint pt0, BPoint pt1, bhapi::pattern pattern = bhapi::B_SOLID_HIGH);

    void		StrokeTriangle(int32_t x0,  int32_t y0,  int32_t x1,  int32_t y1,  int32_t x2,  int32_t y2, bhapi::pattern pattern = bhapi::B_SOLID_HIGH);
    void		StrokeTriangle(BPoint pt1, BPoint pt2, BPoint pt3, bhapi::pattern pattern = bhapi::B_SOLID_HIGH);
    void		FillTriangle(int32_t x0,  int32_t y0,  int32_t x1,  int32_t y1,  int32_t x2,  int32_t y2,
                     bool stroke_edge = true, bhapi::pattern pattern = bhapi::B_SOLID_HIGH);
    void		FillTriangle(BPoint pt0, BPoint pt1, BPoint pt2, bool stroke_edge = true, bhapi::pattern pattern = bhapi::B_SOLID_HIGH);

    void		StrokePolygon(const BPolygon *aPolygon, bool closed = true, bhapi::pattern pattern = bhapi::B_SOLID_HIGH);
    void		StrokePolygon(const BPoint *ptArray,  int32_t numPts, bool closed = true, bhapi::pattern pattern = bhapi::B_SOLID_HIGH);
    void		FillPolygon(const BPolygon *aPolygon, bool stroke_edge = true, bhapi::pattern pattern = bhapi::B_SOLID_HIGH);
    void		FillPolygon(const BPoint *ptArray,  int32_t numPts, bool stroke_edge = true, bhapi::pattern pattern = bhapi::B_SOLID_HIGH);

    void		StrokeEllipse(int32_t x,  int32_t y,  uint32_t width,  uint32_t height,
                      bhapi::pattern pattern = bhapi::B_SOLID_HIGH);
    void		StrokeEllipse(int32_t xCenter,  int32_t yCenter,  int32_t xRadius,  int32_t yRadius,
                      bhapi::pattern pattern = bhapi::B_SOLID_HIGH);
    void		StrokeEllipse(BRect rect, bhapi::pattern pattern = bhapi::B_SOLID_HIGH);
    void		FillEllipse(int32_t x,  int32_t y,  uint32_t width,  uint32_t height,
                    bool stroke_edge = true, bhapi::pattern pattern = bhapi::B_SOLID_HIGH);
    void		FillEllipse(int32_t xCenter,  int32_t yCenter,  int32_t xRadius,  int32_t yRadius,
                    bool stroke_edge = true, bhapi::pattern pattern = bhapi::B_SOLID_HIGH);
    void		FillEllipse(BRect rect, bool stroke_edge = true, bhapi::pattern pattern = bhapi::B_SOLID_HIGH);

    void		StrokeArc(int32_t x,  int32_t y,  uint32_t width,  uint32_t height,
                   int32_t startAngle,  int32_t endAngle,
                  bhapi::pattern pattern = bhapi::B_SOLID_HIGH);
    void		StrokeArc(int32_t xCenter,  int32_t yCenter,  int32_t xRadius,  int32_t yRadius,
                   int32_t startAngle,  int32_t endAngle,
                  bhapi::pattern pattern = bhapi::B_SOLID_HIGH);
    void		StrokeArc(BPoint ctPt, float xRadius, float yRadius,
				  float startAngle, float arcAngle,
                  bhapi::pattern pattern = bhapi::B_SOLID_HIGH);
    void		StrokeArc(BRect rect, float startAngle, float arcAngle, bhapi::pattern pattern = bhapi::B_SOLID_HIGH);
    void		StrokeArc(BRect rect, BPoint start, BPoint end, bhapi::pattern pattern = bhapi::B_SOLID_HIGH);

private:
    bhapi::drawing_mode fDrawingMode;
    bhapi::rgb_color fHighColor;
    bhapi::rgb_color fLowColor;
	float fPenSize;
	bool fSquarePointStyle;

	virtual status_t InitCheck() const = 0;
    virtual void GetFrame(int32_t *originX,  int32_t *originY,  uint32_t *width,  uint32_t *height) const = 0;
    virtual void GetPixel(int32_t x,  int32_t y, bhapi::rgb_color &color) const = 0;
    virtual void PutPixel(int32_t x,  int32_t y, bhapi::rgb_color color) = 0;
    virtual void PutRect(int32_t x,  int32_t y,  uint32_t width,  uint32_t height, bhapi::rgb_color color);

    void drawPixel(int32_t x,  int32_t y, bhapi::pattern pattern);
};
#endif /* __cplusplus */
#endif /* BHAPI_RENDER_H */

