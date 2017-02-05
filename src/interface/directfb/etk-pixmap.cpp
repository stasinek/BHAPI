/* --------------------------------------------------------------------------
 * 
 * DirectFB Graphics Add-on for ETK++
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
#ifdef DIRECTFB

#include "./../../support/Autolock.h"
#include "./../../support/ClassInfo.h"

#include "etk-dfb.h"


EDFBGraphicsDrawable::EDFBGraphicsDrawable(EDFBGraphicsEngine *dfbEngine, euint32 w, euint32 h)
	: EGraphicsDrawable(), fEngine(NULL)
{
	if(w >= E_MAXINT32 || h >= E_MAXINT32)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return;
	}

	fEngine = dfbEngine;
	if(fEngine == NULL) return;

	e_rgb_color whiteColor = e_make_rgb_color(255, 255, 255, 255);
	EGraphicsDrawable::SetBackgroundColor(whiteColor);

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) {fEngine = NULL; return;}

	DFBSurfaceDescription desc;
	desc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT);
	desc.caps = DSCAPS_SYSTEMONLY;
	desc.pixelformat = DSPF_ARGB;
	desc.width = w + 1;
	desc.height = h + 1;

	if(fEngine->dfbDisplay->CreateSurface(fEngine->dfbDisplay, &desc, &dfbSurface) != DFB_OK)
	{
		fEngine = NULL;
		return;
	}

#if 0
	DFBSurfacePixelFormat pixel_format;
	dfbSurface->GetPixelFormat(dfbSurface, &pixel_format);
	ETK_DEBUG("[GRAPHICS]: DFBSurface created (PixelFormat: 0x%x).", pixel_format);
#endif

	fWidth = w;
	fHeight = h;

	dfbSurface->Clear(dfbSurface, 255, 255, 255, 255);
}


EDFBGraphicsDrawable::~EDFBGraphicsDrawable()
{
	if(fEngine != NULL)
	{
		EAutolock <EDFBGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK)
			ETK_ERROR("[GRAPHICS]: %s --- Invalid graphics engine.", __PRETTY_FUNCTION__);

		dfbSurface->Release(dfbSurface);
	}
}


e_status_t
EDFBGraphicsDrawable::SetBackgroundColor(e_rgb_color bkColor)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	e_rgb_color c = BackgroundColor();
	if(c != bkColor)
	{
		EGraphicsDrawable::SetBackgroundColor(c);
		dfbSurface->Clear(dfbSurface, c.red, c.green, c.blue, 255);
	}

	return E_OK;
}


e_status_t
EDFBGraphicsDrawable::ResizeTo(euint32 w, euint32 h)
{
	if(w >= E_MAXINT32 || h >= E_MAXINT32)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	DFBSurfaceDescription desc;
	desc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT);
	desc.caps = DSCAPS_SYSTEMONLY;
	desc.width = w + 1;
	desc.height = h + 1;

	IDirectFBSurface *newSurface;
	if(fEngine->dfbDisplay->CreateSurface(fEngine->dfbDisplay, &desc, &newSurface) != DFB_OK) return E_ERROR;

	dfbSurface->Release(dfbSurface);
	dfbSurface = newSurface;

	e_rgb_color c = BackgroundColor();
	dfbSurface->Clear(dfbSurface, c.red, c.green, c.blue, 255);

	return E_OK;
}


e_status_t
EDFBGraphicsDrawable::CopyTo(EGraphicsContext *dc,
			     EGraphicsDrawable *dstDrawable,
			     eint32 x, eint32 y, euint32 w, euint32 h,
			     eint32 dstX, eint32 dstY, euint32 dstW, euint32 dstH)
{
	if(w >= E_MAXINT32 || h >= E_MAXINT32 || dstW >= E_MAXINT32 || dstH >= E_MAXINT32)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	if(fEngine == NULL || dc == NULL || dstDrawable == NULL) return E_ERROR;

	if(dc->DrawingMode() != E_OP_COPY)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- FIXME: unsupported drawing mode.", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	EDFBGraphicsWindow *win = NULL;
	EDFBGraphicsDrawable *pix = NULL;
	IDirectFBSurface *destSurface = NULL;
	ERect margins(0, 0, 0, 0);

	if((win = e_cast_as(dstDrawable, EDFBGraphicsWindow)) != NULL) {destSurface = win->dfbSurface; margins = win->fMargins;}
	else if((pix = e_cast_as(dstDrawable, EDFBGraphicsDrawable)) != NULL) destSurface = pix->dfbSurface;

	if(destSurface == NULL) return E_ERROR;

	DFBRegion *dfbRegions = NULL;
	int nRegions = 0;

	if(fEngine->ConvertRegion(dc->Clipping(), &dfbRegions, &nRegions) == false) return E_ERROR;

	destSurface->SetBlittingFlags(destSurface, DSBLIT_NOFX);

	for(int i = 0; i < nRegions; i++)
	{
		destSurface->SetClip(destSurface, dfbRegions + i);

		DFBRectangle srcRect, destRect;
		srcRect.x = (int)x;
		srcRect.y = (int)y;
		srcRect.w = (int)w + 1;
		srcRect.h = (int)h + 1;
		destRect.x = (int)dstX + (int)margins.left;
		destRect.y = (int)dstY + (int)margins.top;
		destRect.w = (int)dstW + 1;
		destRect.h = (int)dstH + 1;

		if(dstW == w && dstH == h)
			destSurface->Blit(destSurface, dfbSurface, &srcRect, destRect.x, destRect.y);
		else
			destSurface->StretchBlit(destSurface, dfbSurface, &srcRect, &destRect);
	}

	if(win != NULL) destSurface->Flip(destSurface, NULL, DSFLIP_WAITFORSYNC);

	free(dfbRegions);

	return E_OK;
}


e_status_t
EDFBGraphicsDrawable::DrawPixmap(EGraphicsContext *dc, const EPixmap *pix,
				 eint32 x, eint32 y, euint32 w, euint32 h,
				 eint32 dstX, eint32 dstY, euint32 dstW, euint32 dstH)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_draw_epixmap(dfbSurface, dc, pix, x, y, w, h, dstX, dstY, dstW, dstH);
}


e_status_t
EDFBGraphicsDrawable::StrokePoint(EGraphicsContext *dc,
				  eint32 x, eint32 y)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_stroke_point(dfbSurface, dc, x, y);
}


e_status_t
EDFBGraphicsDrawable::StrokePoints(EGraphicsContext *dc,
				   const eint32 *pts, eint32 count)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_stroke_points(dfbSurface, dc, pts, count);
}


e_status_t
EDFBGraphicsDrawable::StrokePoints_Colors(EGraphicsContext *dc,
					  const EList *ptsArrayLists, eint32 arrayCount,
					  const e_rgb_color *highColors)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_stroke_points_color(dfbSurface, dc, ptsArrayLists, arrayCount, highColors);
}


e_status_t
EDFBGraphicsDrawable::StrokePoints_Alphas(EGraphicsContext *dc,
					  const eint32 *pts, const euint8 *alpha, eint32 count)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_stroke_points_alphas(dfbSurface, dc, pts, alpha, count);
}


e_status_t
EDFBGraphicsDrawable::StrokeLine(EGraphicsContext *dc,
				 eint32 x0, eint32 y0, eint32 x1, eint32 y1)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_stroke_line(dfbSurface, dc, x0, y0, x1, y1);
}


e_status_t
EDFBGraphicsDrawable::StrokePolygon(EGraphicsContext *dc,
				    const eint32 *pts, eint32 count, bool closed)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_stroke_polygon(dfbSurface, dc, pts, count, closed);
}


e_status_t
EDFBGraphicsDrawable::FillPolygon(EGraphicsContext *dc,
				  const eint32 *pts, eint32 count)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_fill_polygon(dfbSurface, dc, pts, count);
}


e_status_t
EDFBGraphicsDrawable::StrokeRect(EGraphicsContext *dc,
				 eint32 x, eint32 y, euint32 w, euint32 h)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_stroke_rect(dfbSurface, dc, x, y, w, h);
}


e_status_t
EDFBGraphicsDrawable::FillRect(EGraphicsContext *dc,
			       eint32 x, eint32 y, euint32 w, euint32 h)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_fill_rect(dfbSurface, dc, x, y, w, h);
}


e_status_t
EDFBGraphicsDrawable::StrokeRects(EGraphicsContext *dc,
				  const eint32 *rects, eint32 count)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_stroke_rects(dfbSurface, dc, rects, count);
}


e_status_t
EDFBGraphicsDrawable::FillRects(EGraphicsContext *dc,
			        const eint32 *rects, eint32 count)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_fill_rects(dfbSurface, dc, rects, count);
}


e_status_t
EDFBGraphicsDrawable::FillRegion(EGraphicsContext *dc,
				 const ERegion &region)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_fill_region(dfbSurface, dc, region);
}


e_status_t
EDFBGraphicsDrawable::StrokeRoundRect(EGraphicsContext *dc,
				      eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_stroke_round_rect(dfbSurface, dc, x, y, w, h, xRadius, yRadius);
}


e_status_t
EDFBGraphicsDrawable::FillRoundRect(EGraphicsContext *dc,
				    eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_fill_round_rect(dfbSurface, dc, x, y, w, h, xRadius, yRadius);
}


e_status_t
EDFBGraphicsDrawable::StrokeArc(EGraphicsContext *dc,
				eint32 x, eint32 y, euint32 w, euint32 h, float startAngle, float endAngle)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_stroke_arc(dfbSurface, dc, x, y, w, h, startAngle, endAngle);
}


e_status_t
EDFBGraphicsDrawable::FillArc(EGraphicsContext *dc,
			      eint32 x, eint32 y, euint32 w, euint32 h, float startAngle, float endAngle)
{
	if(fEngine == NULL) return E_ERROR;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return E_ERROR;

	return etk_dfb_fill_arc(dfbSurface, dc, x, y, w, h, startAngle, endAngle);
}

#endif /* DIRECTFB */
#endif /* LINUX */
