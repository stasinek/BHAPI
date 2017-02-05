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
 * File: etk-pixmap.cpp
 *
 * --------------------------------------------------------------------------*/

#ifdef LINUX

#include "etk-x11.h"
#include "./../support/Autolock.h"
#include "./../support/ClassInfo.h"


EXGraphicsDrawable::EXGraphicsDrawable(EXGraphicsEngine *x11Engine, euint32 w, euint32 h)
	: EGraphicsDrawable(), fEngine(NULL)
{
	if(w == E_MAXUINT32 || h == E_MAXUINT32)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return;
	}

	fEngine = x11Engine;
	if(fEngine == NULL) return;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) {fEngine = NULL; return;}

	xPixmap = XCreatePixmap(fEngine->xDisplay, fEngine->xRootWindow, w + 1, h + 1, fEngine->xDepth);
#ifdef HAVE_XFT
	xDraw = XftDrawCreate(fEngine->xDisplay, xPixmap, fEngine->xVisual, fEngine->xColormap);
#endif

	e_rgb_color whiteColor = e_make_rgb_color(255, 255, 255, 255);
	EGraphicsDrawable::SetBackgroundColor(whiteColor);
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
		EAutolock <EXGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK)
			ETK_ERROR("[GRAPHICS]: %s --- Invalid graphics engine.", __PRETTY_FUNCTION__);

		XFreePixmap(fEngine->xDisplay, xPixmap);
		XFreeGC(fEngine->xDisplay, xGC);

		if(xBackgroundAlloced) EXGraphicsContext::FreeXColor(fEngine, xBackground);

#ifdef HAVE_XFT
		if(xDraw) XftDrawDestroy(xDraw);
#endif
	}
}


e_status_t
EXGraphicsDrawable::SetBackgroundColor(e_rgb_color bkColor)
{
	if(fEngine == NULL) return E_ERROR;

	e_rgb_color color = BackgroundColor();
	bkColor.alpha = color.alpha = 255;

	if(bkColor == color) return E_OK;

	do {
		EAutolock <EXGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

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
			if(EXGraphicsContext::AllocXColor(fEngine, bkColor, &p) == false) return E_ERROR;
			if(xBackgroundAlloced) EXGraphicsContext::FreeXColor(fEngine, xBackground);
			xBackground = p;
			xBackgroundAlloced = true;
		}

		// TODO: clear content of pixmap
	} while(false);

	EGraphicsDrawable::SetBackgroundColor(bkColor);

	return E_OK;
}


e_status_t
EXGraphicsDrawable::ResizeTo(euint32 w, euint32 h)
{
	if(w == E_MAXUINT32 || h == E_MAXUINT32)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

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

	return E_OK;
}


e_status_t
EXGraphicsDrawable::CopyTo(EGraphicsContext *_dc_,
			   EGraphicsDrawable *dstDrawable,
			   eint32 x, eint32 y, euint32 w, euint32 h,
			   eint32 dstX, eint32 dstY, euint32 dstW, euint32 dstH)
{
	if(w != dstW || h != dstH)
	{
		// TODO
		ETK_DEBUG("[GRAPHICS]: %s --- FIXME: (w != dstW || h != dstY).", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	if(w == E_MAXUINT32 || h == E_MAXUINT32 || dstW == E_MAXUINT32 || dstH == E_MAXUINT32)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	if(fEngine == NULL || dstDrawable == NULL) return E_ERROR;

	EAutolock <EXGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	EXGraphicsContext *dc = e_cast_as(_dc_, EXGraphicsContext);
	if(dc == NULL || dc->fEngine != fEngine) return E_ERROR;
	if(dc->DrawingMode() != E_OP_COPY)
	{
		// TODO
		ETK_DEBUG("[GRAPHICS]: %s --- FIXME: unsupported drawing mode.", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	EXGraphicsWindow *win = NULL;
	EXGraphicsDrawable *pix = NULL;

	e_status_t retVal = E_OK;

	if((win = e_cast_as(dstDrawable, EXGraphicsWindow)) != NULL)
		XCopyArea(fEngine->xDisplay, xPixmap, win->xWindow, xGC, x, y, w + 1, h + 1, dstX, dstY);
	else if((pix = e_cast_as(dstDrawable, EXGraphicsDrawable)) != NULL)
		XCopyArea(fEngine->xDisplay, xPixmap, pix->xPixmap, xGC, x, y, w + 1, h + 1, dstX, dstY);
	else
		retVal = E_ERROR;

	XFlush(fEngine->xDisplay);

	return retVal;
}

#endif /* LINUX */
