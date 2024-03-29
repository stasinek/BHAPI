/* --------------------------------------------------------------------------
 *
 * DirectFB Graphics Add-on for BHAPI++
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
#ifdef DIRECTFB

#include <kits/support/Autolock.h>
#include <kits/support/ClassInfo.h>

#include <BHAPI_wrapper_dfb.h>


EDFBGraphicsDrawable::EDFBGraphicsDrawable(EDFBGraphicsEngine *dfbEngine,  uint32_t w,  uint32_t h)
	: BGraphicsDrawable(), fEngine(NULL)
{
	if(w >= B_MAXINT32 || h >= B_MAXINT32)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return;
	}

	fEngine = dfbEngine;
	if(fEngine == NULL) return;

	bhapi::rgb_color whiteColor = bhapi::make_rgb_color(255, 255, 255, 255);
	BGraphicsDrawable::SetBackgroundColor(whiteColor);

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) {fEngine = NULL; return;}

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
	BHAPI_DEBUG("[GRAPHICS]: DFBSurface created (PixelFormat: 0x%x).", pixel_format);
#endif

	fWidth = w;
	fHeight = h;

	dfbSurface->Clear(dfbSurface, 255, 255, 255, 255);
}


EDFBGraphicsDrawable::~EDFBGraphicsDrawable()
{
	if(fEngine != NULL)
	{
		BAutolock <EDFBGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK)
			BHAPI_ERROR("[GRAPHICS]: %s --- Invalid graphics engine.", __PRETTY_FUNCTION__);

		dfbSurface->Release(dfbSurface);
	}
}


status_t 
EDFBGraphicsDrawable::SetBackgroundColor(bhapi::rgb_color bkColor)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	bhapi::rgb_color c = BackgroundColor();
	if(c != bkColor)
	{
		BGraphicsDrawable::SetBackgroundColor(c);
		dfbSurface->Clear(dfbSurface, c.red, c.green, c.blue, 255);
	}

	return B_OK;
}


status_t 
EDFBGraphicsDrawable::ResizeTo(uint32_t w,  uint32_t h)
{
	if(w >= B_MAXINT32 || h >= B_MAXINT32)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	DFBSurfaceDescription desc;
	desc.flags = (DFBSurfaceDescriptionFlags)(DSDESC_CAPS | DSDESC_WIDTH | DSDESC_HEIGHT);
	desc.caps = DSCAPS_SYSTEMONLY;
	desc.width = w + 1;
	desc.height = h + 1;

	IDirectFBSurface *newSurface;
	if(fEngine->dfbDisplay->CreateSurface(fEngine->dfbDisplay, &desc, &newSurface) != DFB_OK) return B_ERROR;

	dfbSurface->Release(dfbSurface);
	dfbSurface = newSurface;

	bhapi::rgb_color c = BackgroundColor();
	dfbSurface->Clear(dfbSurface, c.red, c.green, c.blue, 255);

	return B_OK;
}


status_t 
EDFBGraphicsDrawable::CopyTo(BGraphicsContext *dc,
			     BGraphicsDrawable *dstDrawable,
			      int32_t x,  int32_t y,  uint32_t w,  uint32_t h,
			      int32_t dstX,  int32_t dstY,  uint32_t dstW,  uint32_t dstH)
{
	if(w >= B_MAXINT32 || h >= B_MAXINT32 || dstW >= B_MAXINT32 || dstH >= B_MAXINT32)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	if(fEngine == NULL || dc == NULL || dstDrawable == NULL) return B_ERROR;

	if(dc->DrawingMode() != B_OP_COPY)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- FIXME: unsupported drawing mode.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	EDFBGraphicsWindow *win = NULL;
	EDFBGraphicsDrawable *pix = NULL;
	IDirectFBSurface *destSurface = NULL;
	BRect margins(0, 0, 0, 0);

	if((win = cast_as(dstDrawable, EDFBGraphicsWindow)) != NULL) {destSurface = win->dfbSurface; margins = win->fMargins;}
	else if((pix = cast_as(dstDrawable, EDFBGraphicsDrawable)) != NULL) destSurface = pix->dfbSurface;

	if(destSurface == NULL) return B_ERROR;

	DFBRegion *dfbRegions = NULL;
	int nRegions = 0;

	if(fEngine->ConvertRegion(dc->Clipping(), &dfbRegions, &nRegions) == false) return B_ERROR;

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

	return B_OK;
}


status_t 
EDFBGraphicsDrawable::DrawPixmap(BGraphicsContext *dc, const BPixmap *pix,
				  int32_t x,  int32_t y,  uint32_t w,  uint32_t h,
				  int32_t dstX,  int32_t dstY,  uint32_t dstW,  uint32_t dstH)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfb_draw_epixmap(dfbSurface, dc, pix, x, y, w, h, dstX, dstY, dstW, dstH);
}


status_t 
EDFBGraphicsDrawable::StrokePoint(BGraphicsContext *dc,
				   int32_t x,  int32_t y)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfbhapi::stroke_point(dfbSurface, dc, x, y);
}


status_t 
EDFBGraphicsDrawable::StrokePoints(BGraphicsContext *dc,
				   const  int32_t *pts,  int32_t count)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfbhapi::stroke_points(dfbSurface, dc, pts, count);
}


status_t 
EDFBGraphicsDrawable::StrokePoints_Colors(BGraphicsContext *dc,
					  const BList *ptsArrayLists,  int32_t arrayCount,
					  const bhapi::rgb_color *highColors)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfbhapi::stroke_points_color(dfbSurface, dc, ptsArrayLists, arrayCount, highColors);
}


status_t 
EDFBGraphicsDrawable::StrokePoints_Alphas(BGraphicsContext *dc,
					  const  int32_t *pts, const  uint8 *alpha,  int32_t count)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfbhapi::stroke_points_alphas(dfbSurface, dc, pts, alpha, count);
}


status_t 
EDFBGraphicsDrawable::StrokeLine(BGraphicsContext *dc,
				  int32_t x0,  int32_t y0,  int32_t x1,  int32_t y1)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfbhapi::stroke_line(dfbSurface, dc, x0, y0, x1, y1);
}


status_t 
EDFBGraphicsDrawable::StrokePolygon(BGraphicsContext *dc,
				    const  int32_t *pts,  int32_t count, bool closed)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfbhapi::stroke_polygon(dfbSurface, dc, pts, count, closed);
}


status_t 
EDFBGraphicsDrawable::FillPolygon(BGraphicsContext *dc,
				  const  int32_t *pts,  int32_t count)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfb_fill_polygon(dfbSurface, dc, pts, count);
}


status_t 
EDFBGraphicsDrawable::StrokeRect(BGraphicsContext *dc,
				  int32_t x,  int32_t y,  uint32_t w,  uint32_t h)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfbhapi::stroke_rect(dfbSurface, dc, x, y, w, h);
}


status_t 
EDFBGraphicsDrawable::FillRect(BGraphicsContext *dc,
			        int32_t x,  int32_t y,  uint32_t w,  uint32_t h)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfb_fill_rect(dfbSurface, dc, x, y, w, h);
}


status_t 
EDFBGraphicsDrawable::StrokeRects(BGraphicsContext *dc,
				  const  int32_t *rects,  int32_t count)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfbhapi::stroke_rects(dfbSurface, dc, rects, count);
}


status_t 
EDFBGraphicsDrawable::FillRects(BGraphicsContext *dc,
			        const  int32_t *rects,  int32_t count)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfb_fill_rects(dfbSurface, dc, rects, count);
}


status_t 
EDFBGraphicsDrawable::FillRegion(BGraphicsContext *dc,
				 const BRegion &region)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfb_fill_region(dfbSurface, dc, region);
}


status_t 
EDFBGraphicsDrawable::StrokeRoundRect(BGraphicsContext *dc,
				       int32_t x,  int32_t y,  uint32_t w,  uint32_t h,  uint32_t xRadius,  uint32_t yRadius)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfbhapi::stroke_round_rect(dfbSurface, dc, x, y, w, h, xRadius, yRadius);
}


status_t 
EDFBGraphicsDrawable::FillRoundRect(BGraphicsContext *dc,
				     int32_t x,  int32_t y,  uint32_t w,  uint32_t h,  uint32_t xRadius,  uint32_t yRadius)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfb_fill_round_rect(dfbSurface, dc, x, y, w, h, xRadius, yRadius);
}


status_t 
EDFBGraphicsDrawable::StrokeArc(BGraphicsContext *dc,
				int32_t x,  int32_t y,  uint32_t w,  uint32_t h, float startAngle, float endAngle)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfbhapi::stroke_arc(dfbSurface, dc, x, y, w, h, startAngle, endAngle);
}


status_t 
EDFBGraphicsDrawable::FillArc(BGraphicsContext *dc,
			       int32_t x,  int32_t y,  uint32_t w,  uint32_t h, float startAngle, float endAngle)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	return b_dfb_fill_arc(dfbSurface, dc, x, y, w, h, startAngle, endAngle);
}

#endif /* DIRECTFB */
#endif /* LINUX */
