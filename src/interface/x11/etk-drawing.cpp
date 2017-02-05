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
 * File: etk-drawing.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../config.h>

#ifdef HAVE_XRENDER
#include <X11/extensions/Xrender.h>
#endif

#include "etk-x11.h"

#include "./../support/Autolock.h>
#include "./../support/ClassInfo.h>
#include "./../render/Pixmap.h>


EXGraphicsContext::EXGraphicsContext(EXGraphicsEngine *x11Engine)
	: EGraphicsContext(), fEngine(NULL)
{
	fEngine = x11Engine;
	if(fEngine == NULL) return;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) {fEngine = NULL; return;}

	EGraphicsContext::SetDrawingMode(E_OP_COPY);
	EGraphicsContext::SetPattern(E_SOLID_HIGH);
	EGraphicsContext::SetPenSize(0);

	e_rgb_color blackColor = {0, 0, 0, 255};
	EGraphicsContext::SetHighColor(blackColor);
	EGraphicsContext::SetLowColor(blackColor);
	xHighColor = xLowColor = fEngine->xBlackPixel;
	xHighColorAlloced = xLowColorAlloced = false;

	XGCValues xgcvals;
	xgcvals.function = GXcopy;
	xgcvals.line_width = 0;
	xgcvals.line_style = LineSolid;
	xgcvals.fill_style = FillSolid;
	xgcvals.cap_style = CapButt;
	xgcvals.graphics_exposures = False;
	xGC = XCreateGC(fEngine->xDisplay, fEngine->xRootWindow,
			GCFunction | GCLineWidth | GCLineStyle | GCFillStyle | GCCapStyle | GCGraphicsExposures, &xgcvals);

	xClipping = NULL;
	EGraphicsContext::SetClipping(ERegion());

	XRectangle clipping;
	clipping.x = 0;
	clipping.y = 0;
	clipping.width = 0;
	clipping.height = 0;
	XSetClipRectangles(fEngine->xDisplay, xGC, 0, 0, &clipping, 1, Unsorted);

	EGraphicsContext::SetSquarePointStyle(false);
}


EXGraphicsContext::~EXGraphicsContext()
{
	if(fEngine != NULL)
	{
		EAutolock <EXGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK)
			ETK_ERROR("[GRAPHICS]: %s --- Invalid graphics engine.", __PRETTY_FUNCTION__);

		if(xClipping != NULL) XDestroyRegion(xClipping);
		XFreeGC(fEngine->xDisplay, xGC);
		if(xHighColorAlloced) FreeXColor(fEngine, xHighColor);
		if(xLowColorAlloced) FreeXColor(fEngine, xLowColor);
	}
}


bool
EXGraphicsContext::AllocXColor(EXGraphicsEngine *engine, e_rgb_color color, unsigned long *pixel)
{
	if(engine == NULL || pixel == NULL) return false;

	XColor xcolor;
	xcolor.red = (unsigned short)color.red * 257;
	xcolor.green = (unsigned short)color.green * 257;
	xcolor.blue = (unsigned short)color.blue * 257;
	xcolor.flags = DoRed | DoGreen | DoBlue;

	if(XAllocColor(engine->xDisplay, engine->xColormap, &xcolor) != 0)
	{
		*pixel = xcolor.pixel;
		return true;
	}

	return false;
}


void
EXGraphicsContext::FreeXColor(EXGraphicsEngine *engine, unsigned long pixel)
{
	if(engine != NULL) XFreeColors(engine->xDisplay, engine->xColormap, &pixel, 1, 0);
}


e_status_t
EXGraphicsContext::GetXClipping(Region *xRegion) const
{
	if(xRegion == NULL) return E_BAD_VALUE;
	if(xClipping == NULL) return E_ERROR;

	*xRegion = xClipping;

	return E_OK;
}


e_status_t
EXGraphicsContext::GetXHighColor(unsigned long *pixel) const
{
	if(pixel == NULL) return E_BAD_VALUE;
	*pixel = xHighColor;
	return E_OK;
}


e_status_t
EXGraphicsContext::GetXLowColor(unsigned long *pixel) const
{
	if(pixel == NULL) return E_BAD_VALUE;
	*pixel = xLowColor;
	return E_OK;
}


void
EXGraphicsContext::PrepareXColor()
{
	if(fEngine == NULL) return;

	if(Pattern() == E_SOLID_HIGH || Pattern() == E_SOLID_LOW)
	{
		XSetForeground(fEngine->xDisplay, xGC, (Pattern() == E_SOLID_HIGH ? xHighColor : xLowColor));
		XSetBackground(fEngine->xDisplay, xGC, (Pattern() == E_SOLID_HIGH ? xHighColor : xLowColor));
	}
	else
	{
		XSetForeground(fEngine->xDisplay, xGC, xHighColor);
		XSetBackground(fEngine->xDisplay, xGC, xLowColor);
	}
}


e_status_t
EXGraphicsContext::SetDrawingMode(e_drawing_mode mode)
{
	if(fEngine == NULL) return E_ERROR;
	if(DrawingMode() == mode) return E_OK;

	int function = GXcopy;
	switch(mode)
	{
		case E_OP_COPY:
//		case E_OP_OVER:
			function = GXcopy; break;
		case E_OP_XOR:
			function = GXxor; break;
//		case E_OP_ERASE:
//			function = GXclear; break;
//		case E_OP_INVERT:
//			function = GXinvert; break;
		default:
			ETK_WARNING("[GRAPHICS]: %s --- DrawingMode %u not support!", __PRETTY_FUNCTION__, (unsigned int)mode);
			return E_ERROR;
	}

	do {
		EAutolock <EXGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;
		XSetFunction(fEngine->xDisplay, xGC, function);
	} while(false);

	EGraphicsContext::SetDrawingMode(mode);

	return E_OK;
}


e_status_t
EXGraphicsContext::SetClipping(const ERegion &region)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	Region xRegion = NULL;

	if(fEngine->ConvertRegion(&region, &xRegion) == false) return E_ERROR;

	XSetRegion(fEngine->xDisplay, xGC, xRegion);

	if(xClipping != NULL) XDestroyRegion(xClipping);
	xClipping = xRegion;

	EGraphicsContext::SetClipping(region);

	return E_OK;
}


e_status_t
EXGraphicsContext::SetHighColor(e_rgb_color highColor)
{
	if(fEngine == NULL) return E_ERROR;

	e_rgb_color color = HighColor();
	highColor.alpha = color.alpha = 255;
	if(highColor == color) return E_OK;

	do {
		EAutolock <EXGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

		if(highColor.red == 0 && highColor.green == 0 && highColor.blue == 0)
		{
			if(xHighColorAlloced) FreeXColor(fEngine, xHighColor);
			xHighColor = fEngine->xBlackPixel;
			xHighColorAlloced = false;
		}
		else if(highColor.red == 255 && highColor.green == 255 && highColor.blue == 255)
		{
			if(xHighColorAlloced) FreeXColor(fEngine, xHighColor);
			xHighColor = fEngine->xWhitePixel;
			xHighColorAlloced = false;
		}
		else
		{
			unsigned long p;
			if(AllocXColor(fEngine, highColor, &p) == false) return E_ERROR;
			if(xHighColorAlloced) FreeXColor(fEngine, xHighColor);
			xHighColor = p;
			xHighColorAlloced = true;
		}
	} while(false);

	EGraphicsContext::SetHighColor(highColor);

	return E_OK;
}


e_status_t
EXGraphicsContext::SetLowColor(e_rgb_color lowColor)
{
	if(fEngine == NULL) return E_ERROR;

	e_rgb_color color = LowColor();
	lowColor.alpha = color.alpha = 255;
	if(lowColor == color) return E_OK;

	do {
		EAutolock <EXGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

		if(lowColor.red == 0 && lowColor.green == 0 && lowColor.blue == 0)
		{
			if(xLowColorAlloced) FreeXColor(fEngine, xLowColor);
			xLowColor = fEngine->xBlackPixel;
			xLowColorAlloced = false;
		}
		else if(lowColor.red == 255 && lowColor.green == 255 && lowColor.blue == 255)
		{
			if(xLowColorAlloced) FreeXColor(fEngine, xLowColor);
			xLowColor = fEngine->xWhitePixel;
			xLowColorAlloced = false;
		}
		else
		{
			unsigned long p;
			if(AllocXColor(fEngine, lowColor, &p) == false) return E_ERROR;
			if(xLowColorAlloced) FreeXColor(fEngine, xLowColor);
			xLowColor = p;
			xLowColorAlloced = true;
		}
	} while(false);

	EGraphicsContext::SetLowColor(lowColor);

	return E_OK;
}


e_status_t
EXGraphicsContext::SetPattern(e_pattern pattern)
{
	if(fEngine == NULL) return E_ERROR;
	if(Pattern() == pattern) return E_OK;

	do {
		EAutolock <EXGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

		XGCValues xgcvals;
		if(pattern == E_SOLID_HIGH || pattern == E_SOLID_LOW)
		{
			xgcvals.fill_style = FillSolid;
		}
		else
		{
			// convert e_pattern to XBitmapFile format --- convert left to right.
			char pat[8];
			bzero(pat, sizeof(pat));
			for(euint8 i = 0; i < 8; i++)
				for(euint8 j = 0; j < 8; j++)
					pat[i] |= ((pattern.data[i] >> j) & 0x01) << (7 - j);

			Pixmap stipple = XCreateBitmapFromData(fEngine->xDisplay, fEngine->xRootWindow, pat, 8, 8);
			XSetStipple(fEngine->xDisplay, xGC, stipple);
			XFreePixmap(fEngine->xDisplay, stipple);

			xgcvals.fill_style = FillOpaqueStippled;
		}

		XChangeGC(fEngine->xDisplay, xGC, GCFillStyle, &xgcvals);
	} while(false);

	EGraphicsContext::SetPattern(pattern);

	return E_OK;
}


e_status_t
EXGraphicsContext::SetPenSize(euint32 penSize)
{
	if(fEngine == NULL) return E_ERROR;
	if(PenSize() == penSize) return E_OK;

	do {
		EAutolock <EXGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

		XGCValues xgcvals;
		xgcvals.line_width = (int)(penSize <= 1 ? 0 : penSize);

		XChangeGC(fEngine->xDisplay, xGC, GCLineWidth, &xgcvals);
	} while(false);

	EGraphicsContext::SetPenSize(penSize);

	return E_OK;
}


static e_status_t etk_stroke_point(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				   eint32 x, eint32 y)
{
	if(xDrawable == None || engine == NULL || dc == NULL) return E_ERROR;

	dc->PrepareXColor();

	if(dc->PenSize() <= 1)
	{
		XDrawPoint(engine->xDisplay, xDrawable, dc->xGC, x, y);
	}
	else if(dc->IsSquarePointStyle())
	{
		int pos = (int)((dc->PenSize() - 1) / 2);
		XFillRectangle(engine->xDisplay, xDrawable, dc->xGC, x - pos, y - pos, dc->PenSize() + 1, dc->PenSize() + 1);
	}
	else
	{
		XGCValues xgcvals;
		XGetGCValues(engine->xDisplay, dc->xGC, GCCapStyle, &xgcvals);
		int oldCapStyle = xgcvals.cap_style;
		if(oldCapStyle != CapRound)
		{
			xgcvals.cap_style = CapRound;
			XChangeGC(engine->xDisplay, dc->xGC, GCCapStyle, &xgcvals);
		}

		XDrawLine(engine->xDisplay, xDrawable, dc->xGC, x, y, x, y);

		if(oldCapStyle != CapRound)
		{
			xgcvals.cap_style = oldCapStyle;
			XChangeGC(engine->xDisplay, dc->xGC, GCCapStyle, &xgcvals);
		}
	}

	return E_OK;
}


static e_status_t etk_stroke_points(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				    const eint32 *pts, eint32 count)
{
	if(xDrawable == None || engine == NULL || dc == NULL || pts == NULL || count <= 0) return E_ERROR;

	if(dc->PenSize() <= 1)
	{
		XPoint *xPts = new XPoint[count];
		if(!xPts) return E_ERROR;

		for(eint32 i = 0; i < count; i++) {xPts[i].x = *pts++; xPts[i].y = *pts++;}

		dc->PrepareXColor();
		XDrawPoints(engine->xDisplay, xDrawable, dc->xGC, xPts, count, CoordModeOrigin);

		delete[] xPts;
	}
	else if(dc->IsSquarePointStyle())
	{
		XRectangle *rs = new XRectangle[count];
		if(!rs) return E_ERROR;

		int pos = (int)((dc->PenSize() - 1) / 2);
		for(eint32 i = 0; i < count; i++)
		{
			rs[i].x = *pts++; rs[i].y = *pts++;
			rs[i].x -= pos; rs[i].y -= pos;
			rs[i].width = dc->PenSize() + 1; rs[i].height = dc->PenSize() + 1;
		}

		dc->PrepareXColor();
		XFillRectangles(engine->xDisplay, xDrawable, dc->xGC, rs, count);

		delete[] rs;
	}
	else
	{
		XSegment *xSegs = new XSegment[count];
		if(!xSegs) return E_ERROR;

		for(eint32 i = 0; i < count; i++) {xSegs[i].x1 = xSegs[i].x2 = *pts++; xSegs[i].y1 = xSegs[i].y2 = *pts++;}

		dc->PrepareXColor();

		XGCValues xgcvals;
		XGetGCValues(engine->xDisplay, dc->xGC, GCCapStyle, &xgcvals);
		int oldCapStyle = xgcvals.cap_style;
		if(oldCapStyle != CapRound)
		{
			xgcvals.cap_style = CapRound;
			XChangeGC(engine->xDisplay, dc->xGC, GCCapStyle, &xgcvals);
		}

		XDrawSegments(engine->xDisplay, xDrawable, dc->xGC, xSegs, count);

		if(oldCapStyle != CapRound)
		{
			xgcvals.cap_style = oldCapStyle;
			XChangeGC(engine->xDisplay, dc->xGC, GCCapStyle, &xgcvals);
		}

		delete[] xSegs;
	}

	return E_OK;
}


static e_status_t etk_stroke_points_colors(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
					   const EList *ptsArrayLists, eint32 arrayCount,
					   const e_rgb_color *high_colors)
{
	if(xDrawable == None || engine == NULL || dc == NULL || ptsArrayLists == NULL || arrayCount <= 0) return E_ERROR;

	e_rgb_color oldColor = dc->HighColor();

	for(eint32 k = 0; k < arrayCount; k++, ptsArrayLists++)
	{
		if(ptsArrayLists == NULL) break;

		e_rgb_color color = (high_colors == NULL ? oldColor : *high_colors++);

		eint32 count = ptsArrayLists->CountItems();
		if(count <= 0) continue;

		if(dc->SetHighColor(color) != E_OK) continue;

		if(dc->PenSize() <= 1)
		{
			XPoint *xPts = new XPoint[count];
			if(!xPts) continue;

			eint32 _count_ = 0;
			for(eint32 i = 0; i < count; i++)
			{
				const eint32 *pt = (const eint32*)ptsArrayLists->ItemAt(i);
				if(!pt) continue;

				xPts[_count_].x = *pt++; xPts[_count_].y = *pt++;
				_count_++;
			}

			if(_count_ > 0)
			{
				dc->PrepareXColor();
				XDrawPoints(engine->xDisplay, xDrawable, dc->xGC, xPts, _count_, CoordModeOrigin);
			}

			delete[] xPts;
		}
		else if(dc->IsSquarePointStyle())
		{
			XRectangle *rs = new XRectangle[count];
			if(!rs) continue;

			int pos = (int)((dc->PenSize() - 1) / 2);
			eint32 _count_ = 0;
			for(eint32 i = 0; i < count; i++)
			{
				const eint32 *pt = (const eint32*)ptsArrayLists->ItemAt(i);
				if(!pt) continue;

				rs[_count_].x = *pt++; rs[_count_].y = *pt++;
				rs[_count_].x -= pos; rs[_count_].y -= pos;
				rs[_count_].width = dc->PenSize() + 1; rs[_count_].height = dc->PenSize() + 1;
				_count_++;
			}

			if(_count_ > 0)
			{
				dc->PrepareXColor();
				XFillRectangles(engine->xDisplay, xDrawable, dc->xGC, rs, _count_);
			}

			delete[] rs;
		}
		else
		{
			XSegment *xSegs = new XSegment[count];
			if(!xSegs) continue;

			eint32 _count_ = 0;
			for(eint32 i = 0; i < count; i++)
			{
				const eint32 *pt = (const eint32*)ptsArrayLists->ItemAt(i);
				if(!pt) continue;

				xSegs[_count_].x1 = xSegs[_count_].x2 = *pt++; xSegs[_count_].y1 = xSegs[_count_].y2 = *pt++;
				_count_++;
			}

			if(_count_ > 0)
			{
				dc->PrepareXColor();

				XGCValues xgcvals;
				XGetGCValues(engine->xDisplay, dc->xGC, GCCapStyle, &xgcvals);
				int oldCapStyle = xgcvals.cap_style;
				if(oldCapStyle != CapRound)
				{
					xgcvals.cap_style = CapRound;
					XChangeGC(engine->xDisplay, dc->xGC, GCCapStyle, &xgcvals);
				}

				XDrawSegments(engine->xDisplay, xDrawable, dc->xGC, xSegs, _count_);

				if(oldCapStyle != CapRound)
				{
					xgcvals.cap_style = oldCapStyle;
					XChangeGC(engine->xDisplay, dc->xGC, GCCapStyle, &xgcvals);
				}
			}

			delete[] xSegs;
		}
	}

	dc->SetHighColor(oldColor);

	return E_OK;
}


static e_status_t etk_stroke_points_alphas(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
					  const eint32 *pts, const euint8 *alpha, eint32 count)
{
#ifdef HAVE_XRENDER
	if(xDrawable == None || engine == NULL || dc == NULL || pts == NULL || alpha == NULL || count <= 0) return E_ERROR;

	XRectangle *xRects = NULL;
	int nRects = 0;

	if(engine->ConvertRegion(dc->Clipping(), &xRects, &nRects) != E_OK || xRects == NULL) return E_ERROR;

	e_rgb_color color = dc->HighColor();
	Picture xPict = XRenderCreatePicture(engine->xDisplay, xDrawable,
					     XRenderFindVisualFormat(engine->xDisplay, engine->xVisual), 0, NULL);
	XRenderSetPictureClipRectangles(engine->xDisplay, xPict, 0, 0, xRects, nRects);
	free(xRects);

	XRenderColor xrcolor;
	xrcolor.red = (unsigned short)color.red * 257;
	xrcolor.green = (unsigned short)color.green * 257;
	xrcolor.blue = (unsigned short)color.blue * 257;

	for(eint32 i = 0; i < count; i++)
	{
		xrcolor.alpha = (unsigned short)(*alpha++) * 257;
		if(dc->PenSize() <= 1)
		{
			eint32 x = *pts++;
			eint32 y = *pts++;
			XRenderFillRectangle(engine->xDisplay, PictOpOver, xPict, &xrcolor, x, y, 1, 1);
		}
		else if(dc->IsSquarePointStyle())
		{
			eint32 x = *pts++;
			eint32 y = *pts++;
			XRenderFillRectangle(engine->xDisplay, PictOpOver, xPict, &xrcolor,
					     x - (int)((dc->PenSize() - 1) / 2), y - (int)((dc->PenSize() - 1) / 2),
					     dc->PenSize(), dc->PenSize());
		}
		else
		{
			eint32 x = *pts++;
			eint32 y = *pts++;
			int pos = (int)((dc->PenSize() - 1) / 2);

			Pixmap srcPixmap = XCreatePixmap(engine->xDisplay, engine->xRootWindow,
							 dc->PenSize() + 1, dc->PenSize() + 1, engine->xDepth);
			Pixmap maskPixmap = XCreatePixmap(engine->xDisplay, engine->xRootWindow,
							  dc->PenSize() + 1, dc->PenSize() + 1, 1);
			Pixmap rMaskPixmap = XCreatePixmap(engine->xDisplay, engine->xRootWindow,
							   1, 1, 8);

			dc->PrepareXColor();

			XGCValues xgcvals;
			XGetGCValues(engine->xDisplay, dc->xGC, GCCapStyle | GCFunction, &xgcvals);
			int oldCapStyle = xgcvals.cap_style;
			int oldFunction = xgcvals.function;

			xgcvals.cap_style = CapRound;
			xgcvals.function = GXcopy;
			XChangeGC(engine->xDisplay, dc->xGC, GCCapStyle | GCFunction, &xgcvals);
			XDrawLine(engine->xDisplay, srcPixmap, dc->xGC, pos, pos, pos, pos);
			xgcvals.cap_style = oldCapStyle;
			xgcvals.function = oldFunction;
			XChangeGC(engine->xDisplay, dc->xGC, GCCapStyle | GCFunction, &xgcvals);

			xgcvals.cap_style = CapRound;
			xgcvals.fill_style = FillSolid;
			xgcvals.function = GXcopy;
			xgcvals.line_width = (int)dc->PenSize();
			xgcvals.line_style = LineSolid;
			GC tmpGC = XCreateGC(engine->xDisplay, maskPixmap,
					     GCCapStyle | GCFillStyle | GCFunction | GCLineWidth | GCLineStyle, &xgcvals);
			XSetForeground(engine->xDisplay, tmpGC, 0);
			XFillRectangle(engine->xDisplay, maskPixmap, tmpGC, 0, 0, dc->PenSize() + 1, dc->PenSize() + 1);
			XSetForeground(engine->xDisplay, tmpGC, 1);
			XDrawLine(engine->xDisplay, maskPixmap, tmpGC, pos, pos, pos, pos);
			XFreeGC(engine->xDisplay, tmpGC);

			XRenderPictureAttributes pa;
			pa.repeat = True;
			Picture srcPict = XRenderCreatePicture(engine->xDisplay, srcPixmap,
							       XRenderFindVisualFormat(engine->xDisplay, engine->xVisual), 0, NULL);
			Picture maskPict = XRenderCreatePicture(engine->xDisplay, rMaskPixmap,
							        XRenderFindStandardFormat(engine->xDisplay, PictStandardA8), CPRepeat, &pa);
			XRenderFillRectangle(engine->xDisplay, PictOpSrc, maskPict, &xrcolor, 0, 0, 1, 1);

			pa.clip_x_origin = x - pos;
			pa.clip_y_origin = y - pos;
			pa.clip_mask = maskPixmap;
			XRenderChangePicture(engine->xDisplay, xPict, CPClipXOrigin | CPClipYOrigin | CPClipMask, &pa);
			XRenderComposite(engine->xDisplay, PictOpOver, srcPict, maskPict, xPict,
					 0, 0, 0, 0, x - pos, y - pos,
					 dc->PenSize() + 1, dc->PenSize() + 1);

			XRenderFreePicture(engine->xDisplay, srcPict);
			XRenderFreePicture(engine->xDisplay, maskPict);

			XFreePixmap(engine->xDisplay, srcPixmap);
			XFreePixmap(engine->xDisplay, maskPixmap);
			XFreePixmap(engine->xDisplay, rMaskPixmap);
		}
	}

	XRenderFreePicture(engine->xDisplay, xPict);

	return E_OK;
#else
	ETK_WARNING("[GRAPHICS]: %s --- alpha unsupported.", __PRETTY_FUNCTION__);
	return E_ERROR;
#endif // HAVE_XRENDER
}


static e_status_t etk_stroke_line(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				  eint32 x0, eint32 y0, eint32 x1, eint32 y1)
{
	if(x0 == x1 && y0 == y1) return etk_stroke_point(xDrawable, engine, dc, x0, y0);

	if(xDrawable == None || engine == NULL || dc == NULL) return E_ERROR;

	dc->PrepareXColor();
	XDrawLine(engine->xDisplay, xDrawable, dc->xGC, x0, y0, x1, y1);

	return E_OK;
}


static e_status_t etk_stroke_rect(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				  eint32 x, eint32 y, euint32 w, euint32 h)
{
	if(w == 0 && h == 0)
		return etk_stroke_point(xDrawable, engine, dc, x, y);
	else if(w == 0 || h == 0)
		return etk_stroke_line(xDrawable, engine, dc, x, y, x + (eint32)w, y + (eint32)h);

	if(xDrawable == None || engine == NULL || dc == NULL) return E_ERROR;

	dc->PrepareXColor();
	XDrawRectangle(engine->xDisplay, xDrawable, dc->xGC, x, y, w, h);

	return E_OK;
}


static e_status_t etk_stroke_rects(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				   const eint32 *rects, eint32 count)
{
	if(xDrawable == None || engine == NULL || dc == NULL || rects == NULL || count <= 0) return E_ERROR;

	XRectangle *rs = new XRectangle[count];
	XSegment *segs = new XSegment[count];
	if(!rs || !segs)
	{
		if(rs) delete[] rs;
		if(segs) delete[] segs;
		return E_ERROR;
	}

	eint32 rsCount = 0;
	eint32 segsCount = 0;

	for(eint32 i = 0; i < count; i++)
	{
		eint32 x = *rects++; eint32 y = *rects++; euint32 w = (euint32)(*rects++); euint32 h = (euint32)(*rects++);
		if(w == 0 || h == 0)
		{
			segs[segsCount].x1 = x; segs[segsCount].y1 = y;
			segs[segsCount].x2 = x + w; segs[segsCount].y2 = y + h;
			segsCount++;
		}
		else
		{
			rs[rsCount].x = x; rs[rsCount].y = y;
			rs[rsCount].width = w; rs[rsCount].height = h;
			rsCount++;
		}
	}

	if(rsCount > 0 || segsCount > 0)
	{
		dc->PrepareXColor();
		if(rsCount > 0) XDrawRectangles(engine->xDisplay, xDrawable, dc->xGC, rs, rsCount);
		if(segsCount > 0) XDrawSegments(engine->xDisplay, xDrawable, dc->xGC, segs, segsCount);
	}

	delete[] rs;
	delete[] segs;

	return E_OK;
}


static e_status_t etk_fill_rect(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				eint32 x, eint32 y, euint32 w, euint32 h)
{
	if(xDrawable == None || engine == NULL || dc == NULL) return E_ERROR;

	dc->PrepareXColor();
	XFillRectangle(engine->xDisplay, xDrawable, dc->xGC, x, y, w + 1, h + 1);

	return E_OK;
}


static e_status_t etk_fill_rects(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				 const eint32 *rects, eint32 count)
{
	if(xDrawable == None || engine == NULL || dc == NULL || rects == NULL || count <= 0) return E_ERROR;

	XRectangle *rs = new XRectangle[count];
	if(!rs) return E_ERROR;

	for(eint32 i = 0; i < count; i++)
	{
		eint32 x = *rects++; eint32 y = *rects++; euint32 w = (euint32)(*rects++); euint32 h = (euint32)(*rects++);
		rs[i].x = x; rs[i].y = y;
		rs[i].width = w + 1; rs[i].height = h + 1;
	}

	dc->PrepareXColor();
	XFillRectangles(engine->xDisplay, xDrawable, dc->xGC, rs, count);
	delete[] rs;

	return E_OK;
}


static e_status_t etk_fill_region(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				  const ERegion &region)
{
	if(xDrawable == None || engine == NULL || dc == NULL || region.CountRects() <= 0) return E_ERROR;

	ERegion aRegion(region);
	aRegion &= *(dc->Clipping());
	if(aRegion.CountRects() == 1)
	{
		ERect r = aRegion.Frame().FloorSelf();
		return etk_fill_rect(xDrawable, engine, dc, (eint32)r.left, (eint32)r.top, (euint32)r.Width(), (euint32)r.Height());
	}
	else if(aRegion.CountRects() <= 0) return E_ERROR;

	Region xOldRegion = NULL;
	if(dc->GetXClipping(&xOldRegion) != E_OK || xOldRegion == NULL) return E_ERROR;

	Region xRegion = XCreateRegion();
	if(xRegion == NULL) return E_ERROR;

	for(eint32 i = 0; i < aRegion.CountRects(); i++)
	{
		ERect r = aRegion.RectAt(i).FloorSelf();

		XRectangle xRect;
		xRect.x = (short)r.left; xRect.y = (short)r.top;
		xRect.width = (unsigned short)r.Width() + 1; xRect.height = (unsigned short)r.Height() + 1;

		XUnionRectWithRegion(&xRect, xRegion, xRegion);
	}

	dc->PrepareXColor();

	ERect r = aRegion.Frame().FloorSelf();
	XSetRegion(engine->xDisplay, dc->xGC, xRegion);
	XFillRectangle(engine->xDisplay, xDrawable, dc->xGC,
		       (int)r.left, (int)r.top, (unsigned int)r.Width() + 1, (unsigned int)r.Height() + 1);
	XSetRegion(engine->xDisplay, dc->xGC, xOldRegion);
	XDestroyRegion(xRegion);

	return E_OK;
}


static e_status_t etk_stroke_arc(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				 eint32 x, eint32 y, euint32 w, euint32 h, float start_angle, float end_angle)
{
	if(xDrawable == None || engine == NULL || dc == NULL) return E_ERROR;

	if(w == 0 && h == 0) return etk_stroke_point(xDrawable, engine, dc, x, y);
	else if(w == 0 || h == 0) return etk_stroke_line(xDrawable, engine, dc, x, y, x + (eint32)w, y + (eint32)h);

	if(end_angle - start_angle >= 360 || end_angle - start_angle <= -360)
	{
		start_angle = 0; end_angle = 360;
	}
	else
	{
		start_angle = (float)fmod((double)start_angle, 360);
		end_angle = (float)fmod((double)end_angle, 360);
	}

	if(start_angle < 0) start_angle = 360.f + start_angle;
	if(end_angle < 0) end_angle = 360.f + end_angle;

	dc->PrepareXColor();
	XDrawArc(engine->xDisplay, xDrawable, dc->xGC, x, y, w, h,
		 (int)(start_angle * 64.f), (int)((end_angle - start_angle) * 64.f));

	return E_OK;
}


static e_status_t etk_fill_arc(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
			       eint32 x, eint32 y, euint32 w, euint32 h, float start_angle, float end_angle)
{
	if(xDrawable == None || engine == NULL || dc == NULL) return E_ERROR;

	if(w == 0 || h == 0) return etk_fill_rect(xDrawable, engine, dc, x, y, w, h);

	if(end_angle - start_angle >= 360 || end_angle - start_angle <= -360)
	{
		start_angle = 0; end_angle = 360;
	}
	else
	{
		start_angle = (float)fmod((double)start_angle, 360);
		end_angle = (float)fmod((double)end_angle, 360);
	}

	if(start_angle < 0) start_angle = 360.f + start_angle;
	if(end_angle < 0) end_angle = 360.f + end_angle;

	dc->PrepareXColor();
	XFillArc(engine->xDisplay, xDrawable, dc->xGC, x, y, w + 1, h + 1,
		 (int)(start_angle * 64.f), (int)((end_angle - start_angle) * 64.f));

	return E_OK;
}


static e_status_t etk_stroke_round_rect(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
					eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius)
{
	if(xDrawable == None || engine == NULL || dc == NULL) return E_ERROR;

	bool xRadiusLarge = (2 * xRadius >= w ? true : false);
	bool yRadiusLarge = (2 * yRadius >= h ? true : false);

	if(xRadius == 0 || yRadius == 0) return etk_stroke_rect(xDrawable, engine, dc, x, y, w, h);
	else if(xRadiusLarge && yRadiusLarge) return etk_stroke_arc(xDrawable, engine, dc, x, y, w, h, 0, 360);

	if(w == 0 && h == 0)
		return etk_stroke_point(xDrawable, engine, dc, x, y);
	else if(w == 0 || h == 0)
		return etk_stroke_line(xDrawable, engine, dc, x, y, x + (eint32)w, y + (eint32)h);

	XArc xarcs[4];
	XSegment xsegments[4];
	int nxarcs = 0;
	int nxsegments = 0;
	if(xRadiusLarge)
	{
		xarcs[0].x = x; xarcs[0].y = y;
		xarcs[0].width = w; xarcs[0].height = 2 * yRadius;
		xarcs[0].angle1 = 0; xarcs[0].angle2 = 180 * 64;

		xarcs[1].x = x; xarcs[1].y = y + (eint32)(h - 2 * yRadius);
		xarcs[1].width = w; xarcs[1].height = 2 * yRadius;
		xarcs[1].angle1 = 180 * 64; xarcs[1].angle2 = 180 * 64;

		nxarcs = 2;

		xsegments[0].x1 = x; xsegments[0].y1 = y + (eint32)yRadius + 1;
		xsegments[0].x2 = x; xsegments[0].y2 = y + (eint32)(h - yRadius) - 1;

		xsegments[1].x1 = x + (eint32)w; xsegments[1].y1 = y + (eint32)yRadius + 1;
		xsegments[1].x2 = x + (eint32)w; xsegments[1].y2 = y + (eint32)(h - yRadius) - 1;

		nxsegments = 2;
	}
	else if(yRadiusLarge)
	{
		xarcs[0].x = x; xarcs[0].y = y;
		xarcs[0].width = 2 * xRadius; xarcs[0].height = h;
		xarcs[0].angle1 = 90 * 64; xarcs[0].angle2 = 180 * 64;

		xarcs[1].x = x + (eint32)(w - 2 * xRadius); xarcs[1].y = y;
		xarcs[1].width = 2 * xRadius; xarcs[1].height = h;
		xarcs[1].angle1 = -90 * 64; xarcs[1].angle2 = 180 * 64;

		nxarcs = 2;

		xsegments[0].x1 = x + (eint32)xRadius + 1; xsegments[0].y1 = y;
		xsegments[0].x2 = x + (eint32)(w - xRadius) - 1; xsegments[0].y2 = y;

		xsegments[1].x1 = x + (eint32)xRadius + 1; xsegments[1].y1 = y + (eint32)h;
		xsegments[1].x2 = x + (eint32)(w - xRadius) - 1; xsegments[1].y2 = y + (eint32)h;

		nxsegments = 2;
	}
	else
	{
		xarcs[0].x = x; xarcs[0].y = y;
		xarcs[0].width = 2 * xRadius; xarcs[0].height = 2 * yRadius;
		xarcs[0].angle1 = 90 * 64; xarcs[0].angle2 = 90 * 64;

		xarcs[1].x = x + (eint32)(w - 2 * xRadius); xarcs[1].y = y;
		xarcs[1].width = 2 * xRadius; xarcs[1].height = 2 * yRadius;
		xarcs[1].angle1 = 0; xarcs[1].angle2 = 90 * 64;

		xarcs[2].x = x; xarcs[2].y = y + (eint32)(h - 2 * yRadius);
		xarcs[2].width = 2 * xRadius; xarcs[2].height = 2 * yRadius;
		xarcs[2].angle1 = 180 * 64; xarcs[2].angle2 = 90 * 64;

		xarcs[3].x = x + (eint32)(w - 2 * xRadius); xarcs[3].y = y + (eint32)(h - 2 * yRadius);
		xarcs[3].width = 2 * xRadius; xarcs[3].height = 2 * yRadius;
		xarcs[3].angle1 = 270 * 64; xarcs[3].angle2 = 90 * 64;

		nxarcs = 4;

		xsegments[0].x1 = x; xsegments[0].y1 = y + (eint32)yRadius + 1;
		xsegments[0].x2 = x; xsegments[0].y2 = y + (eint32)(h - yRadius) - 1;

		xsegments[1].x1 = x + (eint32)w; xsegments[1].y1 = y + (eint32)yRadius + 1;
		xsegments[1].x2 = x + (eint32)w; xsegments[1].y2 = y + (eint32)(h - yRadius) - 1;

		xsegments[2].x1 = x + (eint32)xRadius + 1; xsegments[2].y1 = y;
		xsegments[2].x2 = x + (eint32)(w - xRadius) - 1; xsegments[2].y2 = y;

		xsegments[3].x1 = x + (eint32)xRadius + 1; xsegments[3].y1 = y + (eint32)h;
		xsegments[3].x2 = x + (eint32)(w - xRadius) - 1; xsegments[3].y2 = y + (eint32)h;

		nxsegments = 4;
	}

	dc->PrepareXColor();
	XDrawArcs(engine->xDisplay, xDrawable, dc->xGC, xarcs, nxarcs);
	XDrawSegments(engine->xDisplay, xDrawable, dc->xGC, xsegments, nxsegments);

	return E_OK;
}


static e_status_t etk_fill_round_rect(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				      eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius)
{
	if(xDrawable == None || engine == NULL || dc == NULL) return E_ERROR;

	bool xRadiusLarge = (2 * xRadius >= w ? true : false);
	bool yRadiusLarge = (2 * yRadius >= h ? true : false);

	if(xRadius == 0 || yRadius == 0) return etk_fill_rect(xDrawable, engine, dc, x, y, w, h);
	else if(xRadiusLarge && yRadiusLarge) return etk_fill_arc(xDrawable, engine, dc, x, y, w, h, 0, 360);

	if(w == 0 || h == 0) return etk_fill_rect(xDrawable, engine, dc, x, y, w, h);

	w += 1;
	h += 1;

	XArc xarcs[4];
	XRectangle xrectangles[3];
	int nxarcs = 0;
	int nxrectangles = 0;
	if(xRadiusLarge)
	{
		xarcs[0].x = x; xarcs[0].y = y;
		xarcs[0].width = w; xarcs[0].height = 2 * yRadius;
		xarcs[0].angle1 = 0; xarcs[0].angle2 = 180 * 64;

		xarcs[1].x = x; xarcs[1].y = y + (eint32)(h - 2 * yRadius);
		xarcs[1].width = w; xarcs[1].height = 2 * yRadius;
		xarcs[1].angle1 = 180 * 64; xarcs[1].angle2 = 180 * 64;

		nxarcs = 2;

		xrectangles[0].x = x; xrectangles[0].y = y + (eint32)yRadius;
		xrectangles[0].width = w; xrectangles[0].height = h - 2 * yRadius;

		nxrectangles = 1;
	}
	else if(yRadiusLarge)
	{
		xarcs[0].x = x; xarcs[0].y = y;
		xarcs[0].width = 2 * xRadius; xarcs[0].height = h;
		xarcs[0].angle1 = 90 * 64; xarcs[0].angle2 = 180 * 64;

		xarcs[1].x = x + (eint32)(w - 2 * xRadius); xarcs[1].y = y;
		xarcs[1].width = 2 * xRadius; xarcs[1].height = h;
		xarcs[1].angle1 = -90 * 64; xarcs[1].angle2 = 180 * 64;

		nxarcs = 2;

		xrectangles[0].x = x + (eint32)xRadius; xrectangles[0].y = y;
		xrectangles[0].width = w - 2 * xRadius; xrectangles[0].height = h;

		nxrectangles = 1;
	}
	else
	{
		xarcs[0].x = x; xarcs[0].y = y;
		xarcs[0].width = 2 * xRadius; xarcs[0].height = 2 * yRadius;
		xarcs[0].angle1 = 90 * 64; xarcs[0].angle2 = 90 * 64;

		xarcs[1].x = x + (eint32)(w - 2 * xRadius); xarcs[1].y = y;
		xarcs[1].width = 2 * xRadius; xarcs[1].height = 2 * yRadius;
		xarcs[1].angle1 = 0; xarcs[1].angle2 = 90 * 64;

		xarcs[2].x = x; xarcs[2].y = y + (eint32)(h - 2 * yRadius);
		xarcs[2].width = 2 * xRadius; xarcs[2].height = 2 * yRadius;
		xarcs[2].angle1 = 180 * 64; xarcs[2].angle2 = 90 * 64;

		xarcs[3].x = x + (eint32)(w - 2 * xRadius); xarcs[3].y = y + (eint32)(h - 2 * yRadius);
		xarcs[3].width = 2 * xRadius; xarcs[3].height = 2 * yRadius;
		xarcs[3].angle1 = 270 * 64; xarcs[3].angle2 = 90 * 64;

		nxarcs = 4;

		xrectangles[0].x = x; xrectangles[0].y = y + (eint32)yRadius;
		xrectangles[0].width = w; xrectangles[0].height = h - 2 * yRadius;

		xrectangles[1].x = x + (eint32)xRadius; xrectangles[1].y = y;
		xrectangles[1].width = w - 2 * xRadius; xrectangles[1].height = yRadius;

		xrectangles[2].x = x + (eint32)xRadius; xrectangles[2].y = y + (eint32)(h - yRadius);
		xrectangles[2].width = w - 2 * xRadius; xrectangles[2].height = yRadius;

		nxrectangles = 3;
	}

	dc->PrepareXColor();
	XFillArcs(engine->xDisplay, xDrawable, dc->xGC, xarcs, nxarcs);
	XFillRectangles(engine->xDisplay, xDrawable, dc->xGC, xrectangles, nxrectangles);

	return E_OK;
}


static e_status_t etk_stroke_polygon(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				     const eint32 *pts, eint32 count, bool closed)
{
	if(xDrawable == None || engine == NULL || dc == NULL || pts == NULL || count <= 0 || count >= E_MAXINT) return E_ERROR;

	if(count == 1) return etk_stroke_point(xDrawable, engine, dc, pts[0], pts[1]);
	else if(count == 2) return etk_stroke_line(xDrawable, engine, dc, pts[0], pts[1], pts[2], pts[3]);

	XPoint *xPts = new XPoint[count + 1];
	if(!xPts) return E_ERROR;

	for(eint32 i = 0; i < count; i++) {xPts[i].x = *pts++; xPts[i].y = *pts++;}

	int ptsCount = count;
	if((xPts[count - 1].x != xPts[0].x || xPts[count - 1].y != xPts[0].y) && closed)
	{
		xPts[count].x = xPts[0].x;
		xPts[count].y = xPts[0].y;
		ptsCount++;
	}

	dc->PrepareXColor();
	XDrawLines(engine->xDisplay, xDrawable, dc->xGC, xPts, ptsCount, CoordModeOrigin);
	delete[] xPts;

	return E_OK;
}


static e_status_t etk_fill_polygon(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				   const eint32 *pts, eint32 count)
{
	if(xDrawable == None || engine == NULL || dc == NULL || pts == NULL || count <= 0 || count >= E_MAXINT) return E_ERROR;

	if(count == 1) return etk_fill_rect(xDrawable, engine, dc, pts[0], pts[1], 0, 0);
	else if(count == 2)
	{
		dc->PrepareXColor();

		XGCValues xgcvals;
		XGetGCValues(engine->xDisplay, dc->xGC, GCLineWidth, &xgcvals);
		int oldLineWidth = xgcvals.line_width;
		if(oldLineWidth != 0)
		{
			xgcvals.line_width = 0;
			XChangeGC(engine->xDisplay, dc->xGC, GCLineWidth, &xgcvals);
		}

		XDrawLine(engine->xDisplay, xDrawable, dc->xGC, pts[0], pts[1], pts[2], pts[3]);

		if(oldLineWidth != 0)
		{
			xgcvals.line_width = oldLineWidth;
			XChangeGC(engine->xDisplay, dc->xGC, GCLineWidth, &xgcvals);
		}

		return E_OK;
	}

	XPoint *xPts = new XPoint[count];
	if(!xPts) return E_ERROR;

	for(eint32 i = 0; i < count; i++) {xPts[i].x = *pts++; xPts[i].y = *pts++;}

	int ptsCount = count;
	if(xPts[count - 1].x == xPts[0].x && xPts[count - 1].y == xPts[0].y) ptsCount--;

	dc->PrepareXColor();
	XFillPolygon(engine->xDisplay, xDrawable, dc->xGC, xPts, ptsCount, Complex, CoordModeOrigin);
	delete[] xPts;

	return E_OK;
}


static e_status_t etk_draw_epixmap(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc, const EPixmap *epixmap,
				   eint32 x, eint32 y, euint32 w, euint32 h,
				   eint32 dstX, eint32 dstY, euint32 dstW, euint32 dstH)
{
	if(xDrawable == None || engine == NULL || dc == NULL || epixmap == NULL || epixmap->IsValid() == false) return E_ERROR;

	if(w != dstW || h != dstH)
	{
		// TODO
		ETK_DEBUG("[GRAPHICS]: %s --- FIXME: (w != dstW || h != dstY).", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	XImage xImage;
	bzero(&xImage, sizeof(XImage));
	xImage.width = (int)epixmap->Bounds().Width() + 1;
	xImage.height = (int)epixmap->Bounds().Height() + 1;
	xImage.xoffset = 0;
	xImage.format = ZPixmap;
	xImage.data = (char*)epixmap->Bits();
	xImage.bytes_per_line = (int)epixmap->BytesPerRow();
	xImage.depth = engine->xDepth;

	switch(epixmap->ColorSpace())
	{
		case E_RGB24:
			xImage.byte_order = LSBFirst;
			xImage.bitmap_pad = 8;
			xImage.bitmap_unit = 8;
			xImage.bits_per_pixel = 24;
			break;

		case E_RGB24_BIG:
			xImage.byte_order = MSBFirst;
			xImage.bitmap_pad = 8;
			xImage.bitmap_unit = 8;
			xImage.bits_per_pixel = 24;
			break;

		case E_RGB32:
		case E_RGBA32:
			xImage.byte_order = LSBFirst;
			xImage.bitmap_pad = 32;
			xImage.bitmap_unit = 32;
			xImage.bits_per_pixel = 32;
			break;

		default:
			ETK_WARNING("[GRAPHICS]: %s --- Unsupported color space (0x%x).", __PRETTY_FUNCTION__, epixmap->ColorSpace());
			return E_ERROR;
	}

	if(XInitImage(&xImage) == 0) return E_ERROR;
	XPutImage(engine->xDisplay, xDrawable, dc->xGC, &xImage, x, y, dstX, dstY, w + 1, h + 1);
	XFlush(engine->xDisplay);

	return E_OK;
}


e_status_t
EXGraphicsDrawable::StrokePoint(EGraphicsContext *_dc_,
				eint32 x, eint32 y)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_stroke_point(xPixmap, fEngine, dc, x, y);
}


e_status_t
EXGraphicsDrawable::StrokePoints(EGraphicsContext *_dc_,
				 const eint32 *pts, eint32 count)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_stroke_points(xPixmap, fEngine, dc, pts, count);
}


e_status_t
EXGraphicsDrawable::StrokePoints_Colors(EGraphicsContext *_dc_,
					const EList *ptsArrayLists, eint32 arrayCount,
					const e_rgb_color *highColors)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_stroke_points_colors(xPixmap, fEngine, dc, ptsArrayLists, arrayCount, highColors);
}


e_status_t
EXGraphicsDrawable::StrokePoints_Alphas(EGraphicsContext *_dc_,
					const eint32 *pts, const euint8 *alpha, eint32 count)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_stroke_points_alphas(xPixmap, fEngine, dc, pts, alpha, count);
}


e_status_t
EXGraphicsDrawable::StrokeLine(EGraphicsContext *_dc_,
			       eint32 x0, eint32 y0, eint32 x1, eint32 y1)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_stroke_line(xPixmap, fEngine, dc, x0, y0, x1, y1);
}


e_status_t
EXGraphicsDrawable::StrokePolygon(EGraphicsContext *_dc_,
				  const eint32 *pts, eint32 count, bool closed)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_stroke_polygon(xPixmap, fEngine, dc, pts, count, closed);
}


e_status_t
EXGraphicsDrawable::FillPolygon(EGraphicsContext *_dc_,
				const eint32 *pts, eint32 count)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_fill_polygon(xPixmap, fEngine, dc, pts, count);
}


e_status_t
EXGraphicsDrawable::StrokeRect(EGraphicsContext *_dc_,
			       eint32 x, eint32 y, euint32 w, euint32 h)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_stroke_rect(xPixmap, fEngine, dc, x, y, w, h);
}


e_status_t
EXGraphicsDrawable::FillRect(EGraphicsContext *_dc_,
			     eint32 x, eint32 y, euint32 w, euint32 h)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_fill_rect(xPixmap, fEngine, dc, x, y, w, h);
}


e_status_t
EXGraphicsDrawable::StrokeRects(EGraphicsContext *_dc_,
				const eint32 *rects, eint32 count)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_stroke_rects(xPixmap, fEngine, dc, rects, count);
}


e_status_t
EXGraphicsDrawable::FillRects(EGraphicsContext *_dc_,
			      const eint32 *rects, eint32 count)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_fill_rects(xPixmap, fEngine, dc, rects, count);
}


e_status_t
EXGraphicsDrawable::FillRegion(EGraphicsContext *_dc_,
			       const ERegion &region)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_fill_region(xPixmap, fEngine, dc, region);
}


e_status_t
EXGraphicsDrawable::StrokeRoundRect(EGraphicsContext *_dc_,
				    eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_stroke_round_rect(xPixmap, fEngine, dc, x, y, w, h, xRadius, yRadius);
}


e_status_t
EXGraphicsDrawable::FillRoundRect(EGraphicsContext *_dc_,
				  eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_fill_round_rect(xPixmap, fEngine, dc, x, y, w, h, xRadius, yRadius);
}


e_status_t
EXGraphicsDrawable::StrokeArc(EGraphicsContext *_dc_,
			      eint32 x, eint32 y, euint32 w, euint32 h, float startAngle, float endAngle)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_stroke_arc(xPixmap, fEngine, dc, x, y, w, h, startAngle, endAngle);
}


e_status_t
EXGraphicsDrawable::FillArc(EGraphicsContext *_dc_,
			    eint32 x, eint32 y, euint32 w, euint32 h, float startAngle, float endAngle)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_fill_arc(xPixmap, fEngine, dc, x, y, w, h, startAngle, endAngle);
}


e_status_t
EXGraphicsDrawable::DrawPixmap(EGraphicsContext *_dc_, const EPixmap *pix,
			       eint32 x, eint32 y, euint32 w, euint32 h,
			       eint32 dstX, eint32 dstY, euint32 dstW, euint32 dstH)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_draw_epixmap(xPixmap, fEngine, dc, pix, x, y, w, h, dstX, dstY, dstW, dstH);
}

e_status_t
EXGraphicsWindow::StrokePoint(EGraphicsContext *_dc_,
			      eint32 x, eint32 y)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_stroke_point(xWindow, fEngine, dc, x, y);
}


e_status_t
EXGraphicsWindow::StrokePoints(EGraphicsContext *_dc_,
			       const eint32 *pts, eint32 count)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_stroke_points(xWindow, fEngine, dc, pts, count);
}


e_status_t
EXGraphicsWindow::StrokePoints_Colors(EGraphicsContext *_dc_,
				      const EList *ptsArrayLists, eint32 arrayCount,
				      const e_rgb_color *highColors)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_stroke_points_colors(xWindow, fEngine, dc, ptsArrayLists, arrayCount, highColors);
}


e_status_t
EXGraphicsWindow::StrokePoints_Alphas(EGraphicsContext *_dc_,
				      const eint32 *pts, const euint8 *alpha, eint32 count)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_stroke_points_alphas(xWindow, fEngine, dc, pts, alpha, count);
}


e_status_t
EXGraphicsWindow::StrokeLine(EGraphicsContext *_dc_,
			     eint32 x0, eint32 y0, eint32 x1, eint32 y1)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_stroke_line(xWindow, fEngine, dc, x0, y0, x1, y1);
}


e_status_t
EXGraphicsWindow::StrokePolygon(EGraphicsContext *_dc_,
				const eint32 *pts, eint32 count, bool closed)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_stroke_polygon(xWindow, fEngine, dc, pts, count, closed);
}


e_status_t
EXGraphicsWindow::FillPolygon(EGraphicsContext *_dc_,
			      const eint32 *pts, eint32 count)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_fill_polygon(xWindow, fEngine, dc, pts, count);
}


e_status_t
EXGraphicsWindow::StrokeRect(EGraphicsContext *_dc_,
			     eint32 x, eint32 y, euint32 w, euint32 h)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_stroke_rect(xWindow, fEngine, dc, x, y, w, h);
}


e_status_t
EXGraphicsWindow::FillRect(EGraphicsContext *_dc_,
			   eint32 x, eint32 y, euint32 w, euint32 h)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_fill_rect(xWindow, fEngine, dc, x, y, w, h);
}


e_status_t
EXGraphicsWindow::StrokeRects(EGraphicsContext *_dc_,
			      const eint32 *rects, eint32 count)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_stroke_rects(xWindow, fEngine, dc, rects, count);
}


e_status_t
EXGraphicsWindow::FillRects(EGraphicsContext *_dc_,
			    const eint32 *rects, eint32 count)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_fill_rects(xWindow, fEngine, dc, rects, count);
}


e_status_t
EXGraphicsWindow::FillRegion(EGraphicsContext *_dc_,
			     const ERegion &region)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_fill_region(xWindow, fEngine, dc, region);
}


e_status_t
EXGraphicsWindow::StrokeRoundRect(EGraphicsContext *_dc_,
				  eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_stroke_round_rect(xWindow, fEngine, dc, x, y, w, h, xRadius, yRadius);
}


e_status_t
EXGraphicsWindow::FillRoundRect(EGraphicsContext *_dc_,
				eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_fill_round_rect(xWindow, fEngine, dc, x, y, w, h, xRadius, yRadius);
}


e_status_t
EXGraphicsWindow::StrokeArc(EGraphicsContext *_dc_,
			    eint32 x, eint32 y, euint32 w, euint32 h, float startAngle, float endAngle)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_stroke_arc(xWindow, fEngine, dc, x, y, w, h, startAngle, endAngle);
}


e_status_t
EXGraphicsWindow::FillArc(EGraphicsContext *_dc_,
			  eint32 x, eint32 y, euint32 w, euint32 h, float startAngle, float endAngle)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_fill_arc(xWindow, fEngine, dc, x, y, w, h, startAngle, endAngle);
}


e_status_t
EXGraphicsWindow::DrawPixmap(EGraphicsContext *_dc_, const EPixmap *pix,
			     eint32 x, eint32 y, euint32 w, euint32 h,
			     eint32 dstX, eint32 dstY, euint32 dstW, euint32 dstH)
{
	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_draw_epixmap(xWindow, fEngine, dc, pix, x, y, w, h, dstX, dstY, dstW, dstH);
}

