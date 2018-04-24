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
 * File: BHAPI_wrapper_drawing.cpp
 *
 * --------------------------------------------------------------------------*/
#ifdef LINUX

#include <kits/config.h>

#ifdef HAVE_XRENDER
#include <X11/extensions/Xrender.h>
#endif

#include <BHAPI_wrapper_x11.h>

#include <kits/support/Autolock.h>
#include <kits/support/ClassInfo.h>
#include <kits/render/Pixmap.h>


EXGraphicsContext::EXGraphicsContext(EXGraphicsEngine *x11Engine)
	: BGraphicsContext(), fEngine(NULL)
{
	fEngine = x11Engine;
	if(fEngine == NULL) return;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) {fEngine = NULL; return;}

	BGraphicsContext::SetDrawingMode(B_OP_COPY);
	BGraphicsContext::SetPattern(B_SOLID_HIGH);
	BGraphicsContext::SetPenSize(0);

	bhapi::rgb_color blackColor = {0, 0, 0, 255};
	BGraphicsContext::SetHighColor(blackColor);
	BGraphicsContext::SetLowColor(blackColor);
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
	BGraphicsContext::SetClipping(BRegion());

	XRectangle clipping;
	clipping.x = 0;
	clipping.y = 0;
	clipping.width = 0;
	clipping.height = 0;
	XSetClipRectangles(fEngine->xDisplay, xGC, 0, 0, &clipping, 1, Unsorted);

	BGraphicsContext::SetSquarePointStyle(false);
}


EXGraphicsContext::~EXGraphicsContext()
{
	if(fEngine != NULL)
	{
		BAutolock <EXGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK)
			BHAPI_ERROR("[GRAPHICS]: %s --- Invalid graphics engine.", __PRETTY_FUNCTION__);

		if(xClipping != NULL) XDestroyRegion(xClipping);
		XFreeGC(fEngine->xDisplay, xGC);
		if(xHighColorAlloced) FreeXColor(fEngine, xHighColor);
		if(xLowColorAlloced) FreeXColor(fEngine, xLowColor);
	}
}


bool 
EXGraphicsContext::AllocXColor(EXGraphicsEngine *engine, bhapi::rgb_color color, unsigned long *pixel)
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


status_t 
EXGraphicsContext::GetXClipping(Region *xRegion) const
{
	if(xRegion == NULL) return B_BAD_VALUE;
	if(xClipping == NULL) return B_ERROR;

	*xRegion = xClipping;

	return B_OK;
}


status_t 
EXGraphicsContext::GetXHighColor(unsigned long *pixel) const
{
	if(pixel == NULL) return B_BAD_VALUE;
	*pixel = xHighColor;
	return B_OK;
}


status_t 
EXGraphicsContext::GetXLowColor(unsigned long *pixel) const
{
	if(pixel == NULL) return B_BAD_VALUE;
	*pixel = xLowColor;
	return B_OK;
}


void 
EXGraphicsContext::PrepareXColor()
{
	if(fEngine == NULL) return;

	if(Pattern() == B_SOLID_HIGH || Pattern() == B_SOLID_LOW)
	{
		XSetForeground(fEngine->xDisplay, xGC, (Pattern() == B_SOLID_HIGH ? xHighColor : xLowColor));
		XSetBackground(fEngine->xDisplay, xGC, (Pattern() == B_SOLID_HIGH ? xHighColor : xLowColor));
	}
	else
	{
		XSetForeground(fEngine->xDisplay, xGC, xHighColor);
		XSetBackground(fEngine->xDisplay, xGC, xLowColor);
	}
}


status_t 
EXGraphicsContext::SetDrawingMode(bhapi::drawing_mode mode)
{
	if(fEngine == NULL) return B_ERROR;
	if(DrawingMode() == mode) return B_OK;

	int function = GXcopy;
	switch(mode)
	{
		case B_OP_COPY:
//		case B_OP_OVER:
			function = GXcopy; break;
		case B_OP_XOR:
			function = GXxor; break;
//		case B_OP_ERASE:
//			function = GXclear; break;
//		case B_OP_INVERT:
//			function = GXinvert; break;
		default:
			BHAPI_WARNING("[GRAPHICS]: %s --- DrawingMode %u not support!", __PRETTY_FUNCTION__, (unsigned int)mode);
			return B_ERROR;
	}

	do {
		BAutolock <EXGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;
		XSetFunction(fEngine->xDisplay, xGC, function);
	} while(false);

	BGraphicsContext::SetDrawingMode(mode);

	return B_OK;
}


status_t 
EXGraphicsContext::SetClipping(const BRegion &region)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	Region xRegion = NULL;

	if(fEngine->ConvertRegion(&region, &xRegion) == false) return B_ERROR;

	XSetRegion(fEngine->xDisplay, xGC, xRegion);

	if(xClipping != NULL) XDestroyRegion(xClipping);
	xClipping = xRegion;

	BGraphicsContext::SetClipping(region);

	return B_OK;
}


status_t 
EXGraphicsContext::SetHighColor(bhapi::rgb_color highColor)
{
	if(fEngine == NULL) return B_ERROR;

	bhapi::rgb_color color = HighColor();
	highColor.alpha = color.alpha = 255;
	if(highColor == color) return B_OK;

	do {
		BAutolock <EXGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

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
			if(AllocXColor(fEngine, highColor, &p) == false) return B_ERROR;
			if(xHighColorAlloced) FreeXColor(fEngine, xHighColor);
			xHighColor = p;
			xHighColorAlloced = true;
		}
	} while(false);

	BGraphicsContext::SetHighColor(highColor);

	return B_OK;
}


status_t 
EXGraphicsContext::SetLowColor(bhapi::rgb_color lowColor)
{
	if(fEngine == NULL) return B_ERROR;

	bhapi::rgb_color color = LowColor();
	lowColor.alpha = color.alpha = 255;
	if(lowColor == color) return B_OK;

	do {
		BAutolock <EXGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

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
			if(AllocXColor(fEngine, lowColor, &p) == false) return B_ERROR;
			if(xLowColorAlloced) FreeXColor(fEngine, xLowColor);
			xLowColor = p;
			xLowColorAlloced = true;
		}
	} while(false);

	BGraphicsContext::SetLowColor(lowColor);

	return B_OK;
}


status_t 
EXGraphicsContext::SetPattern(bhapi::pattern pattern)
{
	if(fEngine == NULL) return B_ERROR;
	if(Pattern() == pattern) return B_OK;

	do {
		BAutolock <EXGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

		XGCValues xgcvals;
		if(pattern == B_SOLID_HIGH || pattern == B_SOLID_LOW)
		{
			xgcvals.fill_style = FillSolid;
		}
		else
		{
			// convert bhapi::pattern to XBitmapFile format --- convert left to right.
			char pat[8];
			bzero(pat, sizeof(pat));
			for(uint8 i = 0; i < 8; i++)
				for(uint8 j = 0; j < 8; j++)
					pat[i] |= ((pattern.data[i] >> j) & 0x01) << (7 - j);

			Pixmap stipple = XCreateBitmapFromData(fEngine->xDisplay, fEngine->xRootWindow, pat, 8, 8);
			XSetStipple(fEngine->xDisplay, xGC, stipple);
			XFreePixmap(fEngine->xDisplay, stipple);

			xgcvals.fill_style = FillOpaqueStippled;
		}

		XChangeGC(fEngine->xDisplay, xGC, GCFillStyle, &xgcvals);
	} while(false);

	BGraphicsContext::SetPattern(pattern);

	return B_OK;
}


status_t 
EXGraphicsContext::SetPenSize(uint32 penSize)
{
	if(fEngine == NULL) return B_ERROR;
	if(PenSize() == penSize) return B_OK;

	do {
		BAutolock <EXGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

		XGCValues xgcvals;
		xgcvals.line_width = (int)(penSize <= 1 ? 0 : penSize);

		XChangeGC(fEngine->xDisplay, xGC, GCLineWidth, &xgcvals);
	} while(false);

	BGraphicsContext::SetPenSize(penSize);

	return B_OK;
}


static status_t bhapi::stroke_point(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				    int32 x,  int32 y)
{
	if(xDrawable == None || engine == NULL || dc == NULL) return B_ERROR;

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

	return B_OK;
}


static status_t bhapi::stroke_points(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				    const  int32 *pts,  int32 count)
{
	if(xDrawable == None || engine == NULL || dc == NULL || pts == NULL || count <= 0) return B_ERROR;

	if(dc->PenSize() <= 1)
	{
		XPoint *xPts = new XPoint[count];
		if(!xPts) return B_ERROR;

		for(int32 i = 0; i < count; i++) {xPts[i].x = *pts++; xPts[i].y = *pts++;}

		dc->PrepareXColor();
		XDrawPoints(engine->xDisplay, xDrawable, dc->xGC, xPts, count, CoordModeOrigin);

		delete[] xPts;
	}
	else if(dc->IsSquarePointStyle())
	{
		XRectangle *rs = new XRectangle[count];
		if(!rs) return B_ERROR;

		int pos = (int)((dc->PenSize() - 1) / 2);
		for(int32 i = 0; i < count; i++)
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
		if(!xSegs) return B_ERROR;

		for(int32 i = 0; i < count; i++) {xSegs[i].x1 = xSegs[i].x2 = *pts++; xSegs[i].y1 = xSegs[i].y2 = *pts++;}

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

	return B_OK;
}


static status_t bhapi::stroke_points_colors(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
					   const BList *ptsArrayLists,  int32 arrayCount,
					   const bhapi::rgb_color *high_colors)
{
	if(xDrawable == None || engine == NULL || dc == NULL || ptsArrayLists == NULL || arrayCount <= 0) return B_ERROR;

	bhapi::rgb_color oldColor = dc->HighColor();

	for(int32 k = 0; k < arrayCount; k++, ptsArrayLists++)
	{
		if(ptsArrayLists == NULL) break;

		bhapi::rgb_color color = (high_colors == NULL ? oldColor : *high_colors++);

		int32 count = ptsArrayLists->CountItems();
		if(count <= 0) continue;

		if(dc->SetHighColor(color) != B_OK) continue;

		if(dc->PenSize() <= 1)
		{
			XPoint *xPts = new XPoint[count];
			if(!xPts) continue;

			int32 _count_ = 0;
			for(int32 i = 0; i < count; i++)
			{
				const  int32 *pt = (const  int32*)ptsArrayLists->ItemAt(i);
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
			int32 _count_ = 0;
			for(int32 i = 0; i < count; i++)
			{
				const  int32 *pt = (const  int32*)ptsArrayLists->ItemAt(i);
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

			int32 _count_ = 0;
			for(int32 i = 0; i < count; i++)
			{
				const  int32 *pt = (const  int32*)ptsArrayLists->ItemAt(i);
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

	return B_OK;
}


static status_t bhapi::stroke_points_alphas(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
					  const  int32 *pts, const  uint8 *alpha,  int32 count)
{
#ifdef HAVE_XRENDER
	if(xDrawable == None || engine == NULL || dc == NULL || pts == NULL || alpha == NULL || count <= 0) return B_ERROR;

	XRectangle *xRects = NULL;
	int nRects = 0;

	if(engine->ConvertRegion(dc->Clipping(), &xRects, &nRects) != B_OK || xRects == NULL) return B_ERROR;

	bhapi::rgb_color color = dc->HighColor();
	Picture xPict = XRenderCreatePicture(engine->xDisplay, xDrawable,
					     XRenderFindVisualFormat(engine->xDisplay, engine->xVisual), 0, NULL);
	XRenderSetPictureClipRectangles(engine->xDisplay, xPict, 0, 0, xRects, nRects);
	free(xRects);

	XRenderColor xrcolor;
	xrcolor.red = (unsigned short)color.red * 257;
	xrcolor.green = (unsigned short)color.green * 257;
	xrcolor.blue = (unsigned short)color.blue * 257;

	for(int32 i = 0; i < count; i++)
	{
		xrcolor.alpha = (unsigned short)(*alpha++) * 257;
		if(dc->PenSize() <= 1)
		{
			int32 x = *pts++;
			int32 y = *pts++;
			XRenderFillRectangle(engine->xDisplay, PictOpOver, xPict, &xrcolor, x, y, 1, 1);
		}
		else if(dc->IsSquarePointStyle())
		{
			int32 x = *pts++;
			int32 y = *pts++;
			XRenderFillRectangle(engine->xDisplay, PictOpOver, xPict, &xrcolor,
					     x - (int)((dc->PenSize() - 1) / 2), y - (int)((dc->PenSize() - 1) / 2),
					     dc->PenSize(), dc->PenSize());
		}
		else
		{
			int32 x = *pts++;
			int32 y = *pts++;
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

	return B_OK;
#else
	BHAPI_WARNING("[GRAPHICS]: %s --- alpha unsupported.", __PRETTY_FUNCTION__);
	return B_ERROR;
#endif // HAVE_XRENDER
}


static status_t bhapi::stroke_line(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				   int32 x0,  int32 y0,  int32 x1,  int32 y1)
{
	if(x0 == x1 && y0 == y1) return bhapi::stroke_point(xDrawable, engine, dc, x0, y0);

	if(xDrawable == None || engine == NULL || dc == NULL) return B_ERROR;

	dc->PrepareXColor();
	XDrawLine(engine->xDisplay, xDrawable, dc->xGC, x0, y0, x1, y1);

	return B_OK;
}


static status_t bhapi::stroke_rect(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				   int32 x,  int32 y,  uint32 w,  uint32 h)
{
	if(w == 0 && h == 0)
		return bhapi::stroke_point(xDrawable, engine, dc, x, y);
	else if(w == 0 || h == 0)
		return bhapi::stroke_line(xDrawable, engine, dc, x, y, x + (int32)w, y + (int32)h);

	if(xDrawable == None || engine == NULL || dc == NULL) return B_ERROR;

	dc->PrepareXColor();
	XDrawRectangle(engine->xDisplay, xDrawable, dc->xGC, x, y, w, h);

	return B_OK;
}


static status_t bhapi::stroke_rects(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				   const  int32 *rects,  int32 count)
{
	if(xDrawable == None || engine == NULL || dc == NULL || rects == NULL || count <= 0) return B_ERROR;

	XRectangle *rs = new XRectangle[count];
	XSegment *segs = new XSegment[count];
	if(!rs || !segs)
	{
		if(rs) delete[] rs;
		if(segs) delete[] segs;
		return B_ERROR;
	}

	int32 rsCount = 0;
	int32 segsCount = 0;

	for(int32 i = 0; i < count; i++)
	{
		int32 x = *rects++;  int32 y = *rects++;  uint32 w = (uint32)(*rects++);  uint32 h = (uint32)(*rects++);
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

	return B_OK;
}


static status_t b_fill_rect(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				int32 x,  int32 y,  uint32 w,  uint32 h)
{
	if(xDrawable == None || engine == NULL || dc == NULL) return B_ERROR;

	dc->PrepareXColor();
	XFillRectangle(engine->xDisplay, xDrawable, dc->xGC, x, y, w + 1, h + 1);

	return B_OK;
}


static status_t b_fill_rects(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				 const  int32 *rects,  int32 count)
{
	if(xDrawable == None || engine == NULL || dc == NULL || rects == NULL || count <= 0) return B_ERROR;

	XRectangle *rs = new XRectangle[count];
	if(!rs) return B_ERROR;

	for(int32 i = 0; i < count; i++)
	{
		int32 x = *rects++;  int32 y = *rects++;  uint32 w = (uint32)(*rects++);  uint32 h = (uint32)(*rects++);
		rs[i].x = x; rs[i].y = y;
		rs[i].width = w + 1; rs[i].height = h + 1;
	}

	dc->PrepareXColor();
	XFillRectangles(engine->xDisplay, xDrawable, dc->xGC, rs, count);
	delete[] rs;

	return B_OK;
}


static status_t b_fill_region(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				  const BRegion &region)
{
	if(xDrawable == None || engine == NULL || dc == NULL || region.CountRects() <= 0) return B_ERROR;

	BRegion aRegion(region);
	aRegion &= *(dc->Clipping());
	if(aRegion.CountRects() == 1)
	{
		BRect r = aRegion.Frame().FloorSelf();
		return b_fill_rect(xDrawable, engine, dc, (int32)r.left, (int32)r.top, (uint32)r.Width(), (uint32)r.Height());
	}
	else if(aRegion.CountRects() <= 0) return B_ERROR;

	Region xOldRegion = NULL;
	if(dc->GetXClipping(&xOldRegion) != B_OK || xOldRegion == NULL) return B_ERROR;

	Region xRegion = XCreateRegion();
	if(xRegion == NULL) return B_ERROR;

	for(int32 i = 0; i < aRegion.CountRects(); i++)
	{
		BRect r = aRegion.RectAt(i).FloorSelf();

		XRectangle xRect;
		xRect.x = (short)r.left; xRect.y = (short)r.top;
		xRect.width = (unsigned short)r.Width() + 1; xRect.height = (unsigned short)r.Height() + 1;

		XUnionRectWithRegion(&xRect, xRegion, xRegion);
	}

	dc->PrepareXColor();

	BRect r = aRegion.Frame().FloorSelf();
	XSetRegion(engine->xDisplay, dc->xGC, xRegion);
	XFillRectangle(engine->xDisplay, xDrawable, dc->xGC,
		       (int)r.left, (int)r.top, (unsigned int)r.Width() + 1, (unsigned int)r.Height() + 1);
	XSetRegion(engine->xDisplay, dc->xGC, xOldRegion);
	XDestroyRegion(xRegion);

	return B_OK;
}


static status_t bhapi::stroke_arc(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				  int32 x,  int32 y,  uint32 w,  uint32 h, float start_angle, float end_angle)
{
	if(xDrawable == None || engine == NULL || dc == NULL) return B_ERROR;

	if(w == 0 && h == 0) return bhapi::stroke_point(xDrawable, engine, dc, x, y);
	else if(w == 0 || h == 0) return bhapi::stroke_line(xDrawable, engine, dc, x, y, x + (int32)w, y + (int32)h);

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

	return B_OK;
}


static status_t b_fill_arc(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
			        int32 x,  int32 y,  uint32 w,  uint32 h, float start_angle, float end_angle)
{
	if(xDrawable == None || engine == NULL || dc == NULL) return B_ERROR;

	if(w == 0 || h == 0) return b_fill_rect(xDrawable, engine, dc, x, y, w, h);

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

	return B_OK;
}


static status_t bhapi::stroke_round_rect(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
					int32 x,  int32 y,  uint32 w,  uint32 h,  uint32 xRadius,  uint32 yRadius)
{
	if(xDrawable == None || engine == NULL || dc == NULL) return B_ERROR;

	bool xRadiusLarge = (2 * xRadius >= w ? true : false);
	bool yRadiusLarge = (2 * yRadius >= h ? true : false);

	if(xRadius == 0 || yRadius == 0) return bhapi::stroke_rect(xDrawable, engine, dc, x, y, w, h);
	else if(xRadiusLarge && yRadiusLarge) return bhapi::stroke_arc(xDrawable, engine, dc, x, y, w, h, 0, 360);

	if(w == 0 && h == 0)
		return bhapi::stroke_point(xDrawable, engine, dc, x, y);
	else if(w == 0 || h == 0)
		return bhapi::stroke_line(xDrawable, engine, dc, x, y, x + (int32)w, y + (int32)h);

	XArc xarcs[4];
	XSegment xsegments[4];
	int nxarcs = 0;
	int nxsegments = 0;
	if(xRadiusLarge)
	{
		xarcs[0].x = x; xarcs[0].y = y;
		xarcs[0].width = w; xarcs[0].height = 2 * yRadius;
		xarcs[0].angle1 = 0; xarcs[0].angle2 = 180 * 64;

		xarcs[1].x = x; xarcs[1].y = y + (int32)(h - 2 * yRadius);
		xarcs[1].width = w; xarcs[1].height = 2 * yRadius;
		xarcs[1].angle1 = 180 * 64; xarcs[1].angle2 = 180 * 64;

		nxarcs = 2;

		xsegments[0].x1 = x; xsegments[0].y1 = y + (int32)yRadius + 1;
		xsegments[0].x2 = x; xsegments[0].y2 = y + (int32)(h - yRadius) - 1;

		xsegments[1].x1 = x + (int32)w; xsegments[1].y1 = y + (int32)yRadius + 1;
		xsegments[1].x2 = x + (int32)w; xsegments[1].y2 = y + (int32)(h - yRadius) - 1;

		nxsegments = 2;
	}
	else if(yRadiusLarge)
	{
		xarcs[0].x = x; xarcs[0].y = y;
		xarcs[0].width = 2 * xRadius; xarcs[0].height = h;
		xarcs[0].angle1 = 90 * 64; xarcs[0].angle2 = 180 * 64;

		xarcs[1].x = x + (int32)(w - 2 * xRadius); xarcs[1].y = y;
		xarcs[1].width = 2 * xRadius; xarcs[1].height = h;
		xarcs[1].angle1 = -90 * 64; xarcs[1].angle2 = 180 * 64;

		nxarcs = 2;

		xsegments[0].x1 = x + (int32)xRadius + 1; xsegments[0].y1 = y;
		xsegments[0].x2 = x + (int32)(w - xRadius) - 1; xsegments[0].y2 = y;

		xsegments[1].x1 = x + (int32)xRadius + 1; xsegments[1].y1 = y + (int32)h;
		xsegments[1].x2 = x + (int32)(w - xRadius) - 1; xsegments[1].y2 = y + (int32)h;

		nxsegments = 2;
	}
	else
	{
		xarcs[0].x = x; xarcs[0].y = y;
		xarcs[0].width = 2 * xRadius; xarcs[0].height = 2 * yRadius;
		xarcs[0].angle1 = 90 * 64; xarcs[0].angle2 = 90 * 64;

		xarcs[1].x = x + (int32)(w - 2 * xRadius); xarcs[1].y = y;
		xarcs[1].width = 2 * xRadius; xarcs[1].height = 2 * yRadius;
		xarcs[1].angle1 = 0; xarcs[1].angle2 = 90 * 64;

		xarcs[2].x = x; xarcs[2].y = y + (int32)(h - 2 * yRadius);
		xarcs[2].width = 2 * xRadius; xarcs[2].height = 2 * yRadius;
		xarcs[2].angle1 = 180 * 64; xarcs[2].angle2 = 90 * 64;

		xarcs[3].x = x + (int32)(w - 2 * xRadius); xarcs[3].y = y + (int32)(h - 2 * yRadius);
		xarcs[3].width = 2 * xRadius; xarcs[3].height = 2 * yRadius;
		xarcs[3].angle1 = 270 * 64; xarcs[3].angle2 = 90 * 64;

		nxarcs = 4;

		xsegments[0].x1 = x; xsegments[0].y1 = y + (int32)yRadius + 1;
		xsegments[0].x2 = x; xsegments[0].y2 = y + (int32)(h - yRadius) - 1;

		xsegments[1].x1 = x + (int32)w; xsegments[1].y1 = y + (int32)yRadius + 1;
		xsegments[1].x2 = x + (int32)w; xsegments[1].y2 = y + (int32)(h - yRadius) - 1;

		xsegments[2].x1 = x + (int32)xRadius + 1; xsegments[2].y1 = y;
		xsegments[2].x2 = x + (int32)(w - xRadius) - 1; xsegments[2].y2 = y;

		xsegments[3].x1 = x + (int32)xRadius + 1; xsegments[3].y1 = y + (int32)h;
		xsegments[3].x2 = x + (int32)(w - xRadius) - 1; xsegments[3].y2 = y + (int32)h;

		nxsegments = 4;
	}

	dc->PrepareXColor();
	XDrawArcs(engine->xDisplay, xDrawable, dc->xGC, xarcs, nxarcs);
	XDrawSegments(engine->xDisplay, xDrawable, dc->xGC, xsegments, nxsegments);

	return B_OK;
}


static status_t b_fill_round_rect(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				       int32 x,  int32 y,  uint32 w,  uint32 h,  uint32 xRadius,  uint32 yRadius)
{
	if(xDrawable == None || engine == NULL || dc == NULL) return B_ERROR;

	bool xRadiusLarge = (2 * xRadius >= w ? true : false);
	bool yRadiusLarge = (2 * yRadius >= h ? true : false);

	if(xRadius == 0 || yRadius == 0) return b_fill_rect(xDrawable, engine, dc, x, y, w, h);
	else if(xRadiusLarge && yRadiusLarge) return b_fill_arc(xDrawable, engine, dc, x, y, w, h, 0, 360);

	if(w == 0 || h == 0) return b_fill_rect(xDrawable, engine, dc, x, y, w, h);

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

		xarcs[1].x = x; xarcs[1].y = y + (int32)(h - 2 * yRadius);
		xarcs[1].width = w; xarcs[1].height = 2 * yRadius;
		xarcs[1].angle1 = 180 * 64; xarcs[1].angle2 = 180 * 64;

		nxarcs = 2;

		xrectangles[0].x = x; xrectangles[0].y = y + (int32)yRadius;
		xrectangles[0].width = w; xrectangles[0].height = h - 2 * yRadius;

		nxrectangles = 1;
	}
	else if(yRadiusLarge)
	{
		xarcs[0].x = x; xarcs[0].y = y;
		xarcs[0].width = 2 * xRadius; xarcs[0].height = h;
		xarcs[0].angle1 = 90 * 64; xarcs[0].angle2 = 180 * 64;

		xarcs[1].x = x + (int32)(w - 2 * xRadius); xarcs[1].y = y;
		xarcs[1].width = 2 * xRadius; xarcs[1].height = h;
		xarcs[1].angle1 = -90 * 64; xarcs[1].angle2 = 180 * 64;

		nxarcs = 2;

		xrectangles[0].x = x + (int32)xRadius; xrectangles[0].y = y;
		xrectangles[0].width = w - 2 * xRadius; xrectangles[0].height = h;

		nxrectangles = 1;
	}
	else
	{
		xarcs[0].x = x; xarcs[0].y = y;
		xarcs[0].width = 2 * xRadius; xarcs[0].height = 2 * yRadius;
		xarcs[0].angle1 = 90 * 64; xarcs[0].angle2 = 90 * 64;

		xarcs[1].x = x + (int32)(w - 2 * xRadius); xarcs[1].y = y;
		xarcs[1].width = 2 * xRadius; xarcs[1].height = 2 * yRadius;
		xarcs[1].angle1 = 0; xarcs[1].angle2 = 90 * 64;

		xarcs[2].x = x; xarcs[2].y = y + (int32)(h - 2 * yRadius);
		xarcs[2].width = 2 * xRadius; xarcs[2].height = 2 * yRadius;
		xarcs[2].angle1 = 180 * 64; xarcs[2].angle2 = 90 * 64;

		xarcs[3].x = x + (int32)(w - 2 * xRadius); xarcs[3].y = y + (int32)(h - 2 * yRadius);
		xarcs[3].width = 2 * xRadius; xarcs[3].height = 2 * yRadius;
		xarcs[3].angle1 = 270 * 64; xarcs[3].angle2 = 90 * 64;

		nxarcs = 4;

		xrectangles[0].x = x; xrectangles[0].y = y + (int32)yRadius;
		xrectangles[0].width = w; xrectangles[0].height = h - 2 * yRadius;

		xrectangles[1].x = x + (int32)xRadius; xrectangles[1].y = y;
		xrectangles[1].width = w - 2 * xRadius; xrectangles[1].height = yRadius;

		xrectangles[2].x = x + (int32)xRadius; xrectangles[2].y = y + (int32)(h - yRadius);
		xrectangles[2].width = w - 2 * xRadius; xrectangles[2].height = yRadius;

		nxrectangles = 3;
	}

	dc->PrepareXColor();
	XFillArcs(engine->xDisplay, xDrawable, dc->xGC, xarcs, nxarcs);
	XFillRectangles(engine->xDisplay, xDrawable, dc->xGC, xrectangles, nxrectangles);

	return B_OK;
}


static status_t bhapi::stroke_polygon(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				     const  int32 *pts,  int32 count, bool closed)
{
	if(xDrawable == None || engine == NULL || dc == NULL || pts == NULL || count <= 0 || count >= B_MAXINT) return B_ERROR;

	if(count == 1) return bhapi::stroke_point(xDrawable, engine, dc, pts[0], pts[1]);
	else if(count == 2) return bhapi::stroke_line(xDrawable, engine, dc, pts[0], pts[1], pts[2], pts[3]);

	XPoint *xPts = new XPoint[count + 1];
	if(!xPts) return B_ERROR;

	for(int32 i = 0; i < count; i++) {xPts[i].x = *pts++; xPts[i].y = *pts++;}

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

	return B_OK;
}


static status_t b_fill_polygon(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc,
				   const  int32 *pts,  int32 count)
{
	if(xDrawable == None || engine == NULL || dc == NULL || pts == NULL || count <= 0 || count >= B_MAXINT) return B_ERROR;

	if(count == 1) return b_fill_rect(xDrawable, engine, dc, pts[0], pts[1], 0, 0);
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

		return B_OK;
	}

	XPoint *xPts = new XPoint[count];
	if(!xPts) return B_ERROR;

	for(int32 i = 0; i < count; i++) {xPts[i].x = *pts++; xPts[i].y = *pts++;}

	int ptsCount = count;
	if(xPts[count - 1].x == xPts[0].x && xPts[count - 1].y == xPts[0].y) ptsCount--;

	dc->PrepareXColor();
	XFillPolygon(engine->xDisplay, xDrawable, dc->xGC, xPts, ptsCount, Complex, CoordModeOrigin);
	delete[] xPts;

	return B_OK;
}


static status_t b_draw_epixmap(Drawable xDrawable, EXGraphicsEngine *engine, EXGraphicsContext *dc, const BPixmap *epixmap,
				    int32 x,  int32 y,  uint32 w,  uint32 h,
				    int32 dstX,  int32 dstY,  uint32 dstW,  uint32 dstH)
{
	if(xDrawable == None || engine == NULL || dc == NULL || epixmap == NULL || epixmap->IsValid() == false) return B_ERROR;

	if(w != dstW || h != dstH)
	{
		// TODO
		BHAPI_DEBUG("[GRAPHICS]: %s --- FIXME: (w != dstW || h != dstY).", __PRETTY_FUNCTION__);
		return B_ERROR;
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
		case B_RGB24:
			xImage.byte_order = LSBFirst;
			xImage.bitmap_pad = 8;
			xImage.bitmap_unit = 8;
			xImage.bits_per_pixel = 24;
			break;

		case B_RGB24_BIG:
			xImage.byte_order = MSBFirst;
			xImage.bitmap_pad = 8;
			xImage.bitmap_unit = 8;
			xImage.bits_per_pixel = 24;
			break;

		case B_RGB32:
		case B_RGBA32:
			xImage.byte_order = LSBFirst;
			xImage.bitmap_pad = 32;
			xImage.bitmap_unit = 32;
			xImage.bits_per_pixel = 32;
			break;

		default:
			BHAPI_WARNING("[GRAPHICS]: %s --- Unsupported color space (0x%x).", __PRETTY_FUNCTION__, epixmap->ColorSpace());
			return B_ERROR;
	}

	if(XInitImage(&xImage) == 0) return B_ERROR;
	XPutImage(engine->xDisplay, xDrawable, dc->xGC, &xImage, x, y, dstX, dstY, w + 1, h + 1);
	XFlush(engine->xDisplay);

	return B_OK;
}


status_t 
EXGraphicsDrawable::StrokePoint(BGraphicsContext *_dc_,
				int32 x,  int32 y)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return bhapi::stroke_point(xPixmap, fEngine, dc, x, y);
}


status_t 
EXGraphicsDrawable::StrokePoints(BGraphicsContext *_dc_,
				 const  int32 *pts,  int32 count)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return bhapi::stroke_points(xPixmap, fEngine, dc, pts, count);
}


status_t 
EXGraphicsDrawable::StrokePoints_Colors(BGraphicsContext *_dc_,
					const BList *ptsArrayLists,  int32 arrayCount,
					const bhapi::rgb_color *highColors)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return bhapi::stroke_points_colors(xPixmap, fEngine, dc, ptsArrayLists, arrayCount, highColors);
}


status_t 
EXGraphicsDrawable::StrokePoints_Alphas(BGraphicsContext *_dc_,
					const  int32 *pts, const  uint8 *alpha,  int32 count)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return bhapi::stroke_points_alphas(xPixmap, fEngine, dc, pts, alpha, count);
}


status_t 
EXGraphicsDrawable::StrokeLine(BGraphicsContext *_dc_,
			        int32 x0,  int32 y0,  int32 x1,  int32 y1)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return bhapi::stroke_line(xPixmap, fEngine, dc, x0, y0, x1, y1);
}


status_t 
EXGraphicsDrawable::StrokePolygon(BGraphicsContext *_dc_,
				  const  int32 *pts,  int32 count, bool closed)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return bhapi::stroke_polygon(xPixmap, fEngine, dc, pts, count, closed);
}


status_t 
EXGraphicsDrawable::FillPolygon(BGraphicsContext *_dc_,
				const  int32 *pts,  int32 count)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_fill_polygon(xPixmap, fEngine, dc, pts, count);
}


status_t 
EXGraphicsDrawable::StrokeRect(BGraphicsContext *_dc_,
			        int32 x,  int32 y,  uint32 w,  uint32 h)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return bhapi::stroke_rect(xPixmap, fEngine, dc, x, y, w, h);
}


status_t 
EXGraphicsDrawable::FillRect(BGraphicsContext *_dc_,
			      int32 x,  int32 y,  uint32 w,  uint32 h)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_fill_rect(xPixmap, fEngine, dc, x, y, w, h);
}


status_t 
EXGraphicsDrawable::StrokeRects(BGraphicsContext *_dc_,
				const  int32 *rects,  int32 count)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return bhapi::stroke_rects(xPixmap, fEngine, dc, rects, count);
}


status_t 
EXGraphicsDrawable::FillRects(BGraphicsContext *_dc_,
			      const  int32 *rects,  int32 count)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_fill_rects(xPixmap, fEngine, dc, rects, count);
}


status_t 
EXGraphicsDrawable::FillRegion(BGraphicsContext *_dc_,
			       const BRegion &region)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_fill_region(xPixmap, fEngine, dc, region);
}


status_t 
EXGraphicsDrawable::StrokeRoundRect(BGraphicsContext *_dc_,
				     int32 x,  int32 y,  uint32 w,  uint32 h,  uint32 xRadius,  uint32 yRadius)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return bhapi::stroke_round_rect(xPixmap, fEngine, dc, x, y, w, h, xRadius, yRadius);
}


status_t 
EXGraphicsDrawable::FillRoundRect(BGraphicsContext *_dc_,
				   int32 x,  int32 y,  uint32 w,  uint32 h,  uint32 xRadius,  uint32 yRadius)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_fill_round_rect(xPixmap, fEngine, dc, x, y, w, h, xRadius, yRadius);
}


status_t 
EXGraphicsDrawable::StrokeArc(BGraphicsContext *_dc_,
			       int32 x,  int32 y,  uint32 w,  uint32 h, float startAngle, float endAngle)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return bhapi::stroke_arc(xPixmap, fEngine, dc, x, y, w, h, startAngle, endAngle);
}


status_t 
EXGraphicsDrawable::FillArc(BGraphicsContext *_dc_,
			     int32 x,  int32 y,  uint32 w,  uint32 h, float startAngle, float endAngle)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_fill_arc(xPixmap, fEngine, dc, x, y, w, h, startAngle, endAngle);
}


status_t 
EXGraphicsDrawable::DrawPixmap(BGraphicsContext *_dc_, const BPixmap *pix,
			        int32 x,  int32 y,  uint32 w,  uint32 h,
			        int32 dstX,  int32 dstY,  uint32 dstW,  uint32 dstH)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_draw_epixmap(xPixmap, fEngine, dc, pix, x, y, w, h, dstX, dstY, dstW, dstH);
}

status_t 
EXGraphicsWindow::StrokePoint(BGraphicsContext *_dc_,
			       int32 x,  int32 y)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return bhapi::stroke_point(xWindow, fEngine, dc, x, y);
}


status_t 
EXGraphicsWindow::StrokePoints(BGraphicsContext *_dc_,
			       const  int32 *pts,  int32 count)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return bhapi::stroke_points(xWindow, fEngine, dc, pts, count);
}


status_t 
EXGraphicsWindow::StrokePoints_Colors(BGraphicsContext *_dc_,
				      const BList *ptsArrayLists,  int32 arrayCount,
				      const bhapi::rgb_color *highColors)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return bhapi::stroke_points_colors(xWindow, fEngine, dc, ptsArrayLists, arrayCount, highColors);
}


status_t 
EXGraphicsWindow::StrokePoints_Alphas(BGraphicsContext *_dc_,
				      const  int32 *pts, const  uint8 *alpha,  int32 count)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return bhapi::stroke_points_alphas(xWindow, fEngine, dc, pts, alpha, count);
}


status_t 
EXGraphicsWindow::StrokeLine(BGraphicsContext *_dc_,
			      int32 x0,  int32 y0,  int32 x1,  int32 y1)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return bhapi::stroke_line(xWindow, fEngine, dc, x0, y0, x1, y1);
}


status_t 
EXGraphicsWindow::StrokePolygon(BGraphicsContext *_dc_,
				const  int32 *pts,  int32 count, bool closed)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return bhapi::stroke_polygon(xWindow, fEngine, dc, pts, count, closed);
}


status_t 
EXGraphicsWindow::FillPolygon(BGraphicsContext *_dc_,
			      const  int32 *pts,  int32 count)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_fill_polygon(xWindow, fEngine, dc, pts, count);
}


status_t 
EXGraphicsWindow::StrokeRect(BGraphicsContext *_dc_,
			      int32 x,  int32 y,  uint32 w,  uint32 h)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return bhapi::stroke_rect(xWindow, fEngine, dc, x, y, w, h);
}


status_t 
EXGraphicsWindow::FillRect(BGraphicsContext *_dc_,
			    int32 x,  int32 y,  uint32 w,  uint32 h)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_fill_rect(xWindow, fEngine, dc, x, y, w, h);
}


status_t 
EXGraphicsWindow::StrokeRects(BGraphicsContext *_dc_,
			      const  int32 *rects,  int32 count)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return bhapi::stroke_rects(xWindow, fEngine, dc, rects, count);
}


status_t 
EXGraphicsWindow::FillRects(BGraphicsContext *_dc_,
			    const  int32 *rects,  int32 count)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_fill_rects(xWindow, fEngine, dc, rects, count);
}


status_t 
EXGraphicsWindow::FillRegion(BGraphicsContext *_dc_,
			     const BRegion &region)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_fill_region(xWindow, fEngine, dc, region);
}


status_t 
EXGraphicsWindow::StrokeRoundRect(BGraphicsContext *_dc_,
				   int32 x,  int32 y,  uint32 w,  uint32 h,  uint32 xRadius,  uint32 yRadius)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return bhapi::stroke_round_rect(xWindow, fEngine, dc, x, y, w, h, xRadius, yRadius);
}


status_t 
EXGraphicsWindow::FillRoundRect(BGraphicsContext *_dc_,
				int32 x,  int32 y,  uint32 w,  uint32 h,  uint32 xRadius,  uint32 yRadius)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_fill_round_rect(xWindow, fEngine, dc, x, y, w, h, xRadius, yRadius);
}


status_t 
EXGraphicsWindow::StrokeArc(BGraphicsContext *_dc_,
			     int32 x,  int32 y,  uint32 w,  uint32 h, float startAngle, float endAngle)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return bhapi::stroke_arc(xWindow, fEngine, dc, x, y, w, h, startAngle, endAngle);
}


status_t 
EXGraphicsWindow::FillArc(BGraphicsContext *_dc_,
			   int32 x,  int32 y,  uint32 w,  uint32 h, float startAngle, float endAngle)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_fill_arc(xWindow, fEngine, dc, x, y, w, h, startAngle, endAngle);
}


status_t 
EXGraphicsWindow::DrawPixmap(BGraphicsContext *_dc_, const BPixmap *pix,
			      int32 x,  int32 y,  uint32 w,  uint32 h,
			      int32 dstX,  int32 dstY,  uint32 dstW,  uint32 dstH)
{
	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(fEngine == NULL || dc == NULL || dc->fEngine != fEngine) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_draw_epixmap(xWindow, fEngine, dc, pix, x, y, w, h, dstX, dstY, dstW, dstH);
}

#endif /* LINUX */
