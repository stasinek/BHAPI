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
 * File: BHAPI_wrapper_pixmap.cpp
 *
 * --------------------------------------------------------------------------*/

#ifdef LINUX

#include <BHAPI_wrapper_x11.h>
#include <kits/support/Autolock.h>
#include <kits/support/ClassInfo.h>


EXGraphicsDrawable::EXGraphicsDrawable(EXGraphicsEngine *x11Engine,  uint32_t w,  uint32_t h)
	: BGraphicsDrawable(), fEngine(NULL)
{
	if(w == B_MAXUINT32 || h == B_MAXUINT32)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return;
	}

	fEngine = x11Engine;
	if(fEngine == NULL) return;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) {fEngine = NULL; return;}

	xPixmap = XCreatePixmap(fEngine->xDisplay, fEngine->xRootWindow, w + 1, h + 1, fEngine->xDepth);
#ifdef HAVE_XFT
	xDraw = XftDrawCreate(fEngine->xDisplay, xPixmap, fEngine->xVisual, fEngine->xColormap);
#endif

	bhapi::rgb_color whiteColor = bhapi::make_rgb_color(255, 255, 255, 255);
	BGraphicsDrawable::SetBackgroundColor(whiteColor);
	xBackground = fEngine->xWhitePixel;
	xBackgroundAlloced = false;

	XGCValues xgcvals;
	xgcvals.function = GXcopy;
	xgcvals.foreground = xBackground;
	xgcvals.line_width = 0;
	xgcvals.line_style = LineSolid;
	xgcvals.fill_style = FillSolid;
	xgcvals.cap_style = CapButt;
	xgcvals.graphics_exposures = False;
	xGC = XCreateGC(fEngine->xDisplay, fEngine->xRootWindow,
			GCFunction | GCForeground | GCLineWidth |
			GCLineStyle | GCFillStyle | GCCapStyle | GCGraphicsExposures, &xgcvals);

	XFillRectangle(fEngine->xDisplay, xPixmap, xGC, 0, 0, w + 1, h + 1);
}


EXGraphicsDrawable::~EXGraphicsDrawable()
{
	if(fEngine != NULL)
	{
		BAutolock <EXGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK)
			BHAPI_ERROR("[GRAPHICS]: %s --- Invalid graphics engine.", __PRETTY_FUNCTION__);

		XFreePixmap(fEngine->xDisplay, xPixmap);
		XFreeGC(fEngine->xDisplay, xGC);

		if(xBackgroundAlloced) EXGraphicsContext::FreeXColor(fEngine, xBackground);

#ifdef HAVE_XFT
		if(xDraw) XftDrawDestroy(xDraw);
#endif
	}
}


status_t EXGraphicsDrawable::SetBackgroundColor(bhapi::rgb_color bkColor)
{
	if(fEngine == NULL) return B_ERROR;

	bhapi::rgb_color color = BackgroundColor();
	bkColor.alpha = color.alpha = 255;

	if(bkColor == color) return B_OK;

	do {
		BAutolock <EXGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

		if(bkColor.red == 0 && bkColor.green == 0 && bkColor.blue == 0)
		{
			if(xBackgroundAlloced) EXGraphicsContext::FreeXColor(fEngine, xBackground);
			xBackground = fEngine->xBlackPixel;
			xBackgroundAlloced = false;
		}
		else if(bkColor.red == 255 && bkColor.green == 255 && bkColor.blue == 255)
		{
			if(xBackgroundAlloced) EXGraphicsContext::FreeXColor(fEngine, xBackground);
			xBackground = fEngine->xWhitePixel;
			xBackgroundAlloced = false;
		}
		else
		{
			unsigned long p;
			if(EXGraphicsContext::AllocXColor(fEngine, bkColor, &p) == false) return B_ERROR;
			if(xBackgroundAlloced) EXGraphicsContext::FreeXColor(fEngine, xBackground);
			xBackground = p;
			xBackgroundAlloced = true;
		}

		// TODO: clear content of pixmap
	} while(false);

	BGraphicsDrawable::SetBackgroundColor(bkColor);

	return B_OK;
}


status_t EXGraphicsDrawable::ResizeTo(uint32_t w,  uint32_t h)
{
	if(w == B_MAXUINT32 || h == B_MAXUINT32)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	XSetForeground(fEngine->xDisplay, xGC, xBackground);
	XSetFunction(fEngine->xDisplay, xGC, GXcopy);

	Pixmap newXPixmap = XCreatePixmap(fEngine->xDisplay, fEngine->xRootWindow, w + 1, h + 1, fEngine->xDepth);
#ifdef HAVE_XFT
	if(xDraw) XftDrawChange(xDraw, newXPixmap);
#endif

	XFillRectangle(fEngine->xDisplay, newXPixmap, xGC, 0, 0, w + 1, h + 1);
	XCopyArea(fEngine->xDisplay, xPixmap, newXPixmap, xGC, 0, 0, w + 1, h + 1, 0, 0);

	XFreePixmap(fEngine->xDisplay, xPixmap);
	xPixmap = newXPixmap;

	return B_OK;
}


status_t EXGraphicsDrawable::CopyTo(BGraphicsContext *_dc_,
			   BGraphicsDrawable *dstDrawable,
			    int32_t x,  int32_t y,  uint32_t w,  uint32_t h,
			    int32_t dstX,  int32_t dstY,  uint32_t dstW,  uint32_t dstH)
{
	if(w != dstW || h != dstH)
	{
		// TODO
		BHAPI_DEBUG("[GRAPHICS]: %s --- FIXME: (w != dstW || h != dstY).", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	if(w == B_MAXUINT32 || h == B_MAXUINT32 || dstW == B_MAXUINT32 || dstH == B_MAXUINT32)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	if(fEngine == NULL || dstDrawable == NULL) return B_ERROR;

	BAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	EXGraphicsContext *dc = cast_as(_dc_, EXGraphicsContext);
	if(dc == NULL || dc->fEngine != fEngine) return B_ERROR;
	if(dc->DrawingMode() != B_OP_COPY)
	{
		// TODO
		BHAPI_DEBUG("[GRAPHICS]: %s --- FIXME: unsupported drawing mode.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	EXGraphicsWindow *win = NULL;
	EXGraphicsDrawable *pix = NULL;

	status_t retVal = B_OK;

	if((win = cast_as(dstDrawable, EXGraphicsWindow)) != NULL)
		XCopyArea(fEngine->xDisplay, xPixmap, win->xWindow, xGC, x, y, w + 1, h + 1, dstX, dstY);
	else if((pix = cast_as(dstDrawable, EXGraphicsDrawable)) != NULL)
		XCopyArea(fEngine->xDisplay, xPixmap, pix->xPixmap, xGC, x, y, w + 1, h + 1, dstX, dstY);
	else
		retVal = B_ERROR;

	XFlush(fEngine->xDisplay);

	return retVal;
}

#endif /* LINUX */
