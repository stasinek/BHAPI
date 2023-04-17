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
 * File: BHAPI_wrapper_drawing.cpp
 *
 * --------------------------------------------------------------------------*/
#ifdef LINUX
#ifdef DIRECTFB

#include <kits/render/Render.h>
#include <kits/render/Pixmap.h>

#include <BHAPI_wrapper_dfb.h>

class BHAPI_LOCAL EDFBRender : public BRender {
public:
	EDFBRender();

	void SetSurface(IDirectFBSurface *surface, BRect *margin = NULL);
	void SetClipping(const BRegion *clipping);
	void PrepareForDrawing(BGraphicsContext *dc);

private:
	IDirectFBSurface *fSurface;
	BRegion fClipping;
	BRect fMargins;

	virtual status_t InitCheck() const;
	virtual void GetFrame(int32_t *originX,  int32_t *originY,  uint32_t *width,  uint32_t *height) const;
	virtual void GetPixel(int32_t x,  int32_t y, bhapi::rgb_color &color) const;
	virtual void PutPixel(int32_t x,  int32_t y, bhapi::rgb_color color);
	virtual void PutRect(int32_t x,  int32_t y,  uint32_t width,  uint32_t height, bhapi::rgb_color color);
};


EDFBRender::EDFBRender()
	: BRender(), fSurface(NULL)
{
}


void 
EDFBRender::SetSurface(IDirectFBSurface *surface, BRect *margin)
{
	fSurface = surface;
	if(fSurface) fSurface->SetClip(fSurface, NULL);
	if(margin) fMargins = *margin;
	else fMargins.Set(0, 0, 0, 0);
}


void 
EDFBRender::SetClipping(const BRegion *clipping)
{
	fClipping.MakeEmpty();
	if(clipping != NULL)
	{
		for(int32_t i = 0; i < clipping->CountRects(); i++)
		{
			BRect rect = clipping->RectAt(i).FloorCopy();
			fClipping.Include(rect);
		}
	}
}


void 
EDFBRender::PrepareForDrawing(BGraphicsContext *dc)
{
	if(dc == NULL) return;
	SetDrawingMode(dc->DrawingMode());
	SetHighColor(dc->HighColor());
	SetLowColor(dc->LowColor());
	SetPenSize((float)dc->PenSize());
	SetClipping(dc->Clipping());
	SetSquarePointStyle(dc->IsSquarePointStyle());
}


status_t 
EDFBRender::InitCheck() const
{
	return(fSurface ? B_OK : B_NO_INIT);
}


void 
EDFBRender::GetFrame(int32_t *originX,  int32_t *originY,  uint32_t *width,  uint32_t *height) const
{
	int w = 0, h = 0;
	if(fSurface) fSurface->GetSize(fSurface, &w, &h);

	if(originX) *originX = 0;
	if(originY) *originY = 0;
	if(width) *width = (uint32_t)w - (uint32_t)fMargins.left - (uint32_t)fMargins.top;
	if(height) *height = (uint32_t)h - (uint32_t)fMargins.top - (uint32_t)fMargins.bottom;
}


void 
EDFBRender::GetPixel(int32_t x,  int32_t y, bhapi::rgb_color &color) const
{
	DFBSurfacePixelFormat pixel_format;
	void *ptr;
	int pitch;
	uint32_t dfbColor = 0;

	x += (int32_t)fMargins.left;
	y += (int32_t)fMargins.top;

	if(fSurface == NULL) return;
	if(fSurface->GetPixelFormat(fSurface, &pixel_format) != DFB_OK)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- fSurface->GetPixelFormat() failed.", __PRETTY_FUNCTION__);
		return;
	}

	if(fSurface->Lock(fSurface, DSLF_READ, &ptr, &pitch) != DFB_OK)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- fSurface->Lock() failed.", __PRETTY_FUNCTION__);
		return;
	}

	switch(DFB_BYTES_PER_PIXEL(pixel_format))
	{
		case 1: // 8-bpp
			dfbColor = (uint32_t)(*((uint8*)ptr + y * pitch + x));
			break;

		case 2: // 15-bpp or 16-bpp
			dfbColor = (uint32_t)(*((uint16*)ptr + y * pitch / 2 + x));
			break;

		case 3: // 24-bpp
			{
				uint8 *bufp;
				bufp = (uint8*)ptr + y * pitch + x * 3;
#ifdef BHAPI_BIG_ENDIAN
				dfbColor = ((uint32_t)bufp[0] << 16) | ((uint32_t)bufp[1] << 8) | (uint32_t)bufp[2];
#else
				dfbColor = ((uint32_t)bufp[2] << 16) | ((uint32_t)bufp[1] << 8) | (uint32_t)bufp[0];
#endif
			}
			break;

		case 4: // 32-bpp
			dfbColor = *((uint32_t*)ptr + y * pitch / 4 + x);
			break;

		default:
//			BHAPI_DEBUG("[GRAPHICS]: %s --- Unsupported pixel format.", __PRETTY_FUNCTION__);
			fSurface->Unlock(fSurface);
			return;
	}

	fSurface->Unlock(fSurface);

	if(!DFB_PIXELFORMAT_IS_INDEXED(pixel_format))
	{
		switch(pixel_format)
		{
			case DSPF_RGB332:
				color.set_to((dfbColor & 0xe0) | 0x1f,
					     ((dfbColor & 0x1c) << 3) | 0x1f,
					     ((dfbColor & 0x03) << 6) | 0x3f);
				break;
#if 0
			case DSPF_RGB15:
				color.set_to(((dfbColor & 0x7c00) >> 7) | 0x0007,
					     ((dfbColor & 0x03e0) >> 2) | 0x0007,
					     ((dfbColor & 0x001f) << 3) | 0x0007);
				break;
#endif
			case DSPF_RGB16:
				color.set_to(((dfbColor & 0xf800) >> 8) | 0x0007,
					     ((dfbColor & 0x07e0) >> 3) | 0x0003,
					     ((dfbColor & 0x001f) << 3) | 0x0007);
				break;

			case DSPF_RGB24:
			case DSPF_RGB32:
			case DSPF_ARGB:
				color.set_to((dfbColor >> 16) & 0xff, (dfbColor >> 8) & 0xff, dfbColor & 0xff);
				break;

			default:
//				BHAPI_DEBUG("[GRAPHICS]: %s --- Unsupported pixel format.", __PRETTY_FUNCTION__);
				break;
		}
	}
	else
	{
		IDirectFBPalette *pal = NULL;
		DFBColor c;

		if(fSurface->GetPalette(fSurface, &pal) != DFB_OK)
		{
			BHAPI_DEBUG("[GRAPHICS]: %s --- fSurface->GetPalette() failed.", __PRETTY_FUNCTION__);
			return;
		}
		if(pal->GetEntries(pal, &c, 1, (unsigned int)dfbColor) != DFB_OK)
		{
			BHAPI_DEBUG("[GRAPHICS]: %s --- pal->GetEntries() failed.", __PRETTY_FUNCTION__);
			return;
		}

		color.set_to(c.r, c.g, c.b, 0xff);
	}

//	BHAPI_DEBUG("[GRAPHICS]: %s --- Pixel Format: 0x%x, Color 0x%x, R %I8u, G %I8u, B %I8u",
//		  __PRETTY_FUNCTION__, pixel_format, dfbColor, color.red, color.green, color.blue);
}


void 
EDFBRender::PutPixel(int32_t x,  int32_t y, bhapi::rgb_color color)
{
	if(fSurface == NULL) return;
	if(fClipping.Contains(BPoint((float)x, (float)y)) == false) return;

	x += (int32_t)fMargins.left;
	y += (int32_t)fMargins.top;

	fSurface->SetDrawingFlags(fSurface, DSDRAW_NOFX);
	fSurface->SetColor(fSurface, color.red, color.green, color.blue, 255);
	fSurface->FillRectangle(fSurface, x, y, 1, 1);
}


void 
EDFBRender::PutRect(int32_t x,  int32_t y,  uint32_t width,  uint32_t height, bhapi::rgb_color color)
{
	if(fSurface == NULL || width == 0 || height == 0) return;

	BRegion aRegion(fClipping);
	aRegion &= BRect((float)x, (float)y, (float)x + (float)width - 1.f, (float)y + (float)height - 1.f);
	if(aRegion.CountRects() <= 0) return;
	aRegion.OffsetBy(fMargins.left, fMargins.top);

	fSurface->SetDrawingFlags(fSurface, DSDRAW_NOFX);
	fSurface->SetColor(fSurface, color.red, color.green, color.blue, 255);

#ifdef DFB_HAVE_FILLRECTANGLES
	DFBRectangle *dfbRects = (DFBRectangle*)malloc(sizeof(DFBRectangle) * (size_t)aRegion.CountRects());
#endif
	for(int32_t i = 0; i < aRegion.CountRects(); i++)
	{
		BRect r = aRegion.RectAt(i).FloorCopy();
#ifdef DFB_HAVE_FILLRECTANGLES
		if(dfbRects == NULL)
		{
#endif
			fSurface->FillRectangle(fSurface, (int)r.left, (int)r.top, (int)r.Width() + 1, (int)r.Height() + 1);
#ifdef DFB_HAVE_FILLRECTANGLES
		}
		else
		{
			dfbRects[i].x = (int)r.left;
			dfbRects[i].y = (int)r.top;
			dfbRects[i].w = (int)r.Width() + 1;
			dfbRects[i].h = (int)r.Height() + 1;
		}
#endif
	}

#ifdef DFB_HAVE_FILLRECTANGLES
	if(dfbRects)
	{
		fSurface->FillRectangles(fSurface, dfbRects, (unsigned int)aRegion.CountRects());
		free(dfbRects);
	}
#endif
}


static EDFBRender b_dfb_render;


status_t b_dfbhapi::stroke_point(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
				int32_t x,  int32_t y, BRect *margins)
{
	b_dfb_render.SetSurface(dfbSurface, margins);
	b_dfb_render.PrepareForDrawing(dc);

	b_dfb_render.StrokePoint(x, y, dc->Pattern());

	b_dfb_render.SetSurface(NULL);

	return B_OK;
}


status_t b_dfbhapi::stroke_points(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
				 const  int32_t *pts,  int32_t count, BRect *margins)
{
	b_dfb_render.SetSurface(dfbSurface, margins);
	b_dfb_render.PrepareForDrawing(dc);

	for(int32_t i = 0; i < count; i++)
	{
		int32_t x = *pts++;
		int32_t y = *pts++;
		b_dfb_render.StrokePoint(x, y, dc->Pattern());
	}

	b_dfb_render.SetSurface(NULL);

	return B_OK;
}


status_t b_dfbhapi::stroke_points_color(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
				       const BList *ptsArrayLists,  int32_t arrayCount, const bhapi::rgb_color *high_colors, BRect *margins)
{
	b_dfb_render.SetSurface(dfbSurface, margins);
	b_dfb_render.PrepareForDrawing(dc);

	bhapi::rgb_color oldColor = dc->HighColor();

	for(int32_t k = 0; k < arrayCount; k++, ptsArrayLists++)
	{
		if(ptsArrayLists == NULL) break;

		bhapi::rgb_color color = (high_colors == NULL ? oldColor : *high_colors++);

		int32_t count = ptsArrayLists->CountItems();
		if(count <= 0) continue;

		b_dfb_render.SetHighColor(color);

		for(int32_t i = 0; i < count; i++)
		{
			const  int32_t *pt = (const  int32_t*)ptsArrayLists->ItemAt(i);
			if(!pt) continue;

			int32_t x = *pt++;
			int32_t y = *pt++;
			b_dfb_render.StrokePoint(x, y, dc->Pattern());
		}
	}

	b_dfb_render.SetSurface(NULL);

	return B_OK;
}


status_t b_dfbhapi::stroke_points_alphas(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
					const  int32_t *pts, const  uint8 *alpha,  int32_t count, BRect *margins)
{
	b_dfb_render.SetSurface(dfbSurface, margins);
	b_dfb_render.PrepareForDrawing(dc);
	b_dfb_render.SetDrawingMode(B_OP_ALPHA);

	bhapi::rgb_color c = dc->HighColor();

	for(int32_t i = 0; i < count; i++)
	{
		int32_t x = *pts++;
		int32_t y = *pts++;
		c.alpha = *alpha++;

		b_dfb_render.SetHighColor(c);
		b_dfb_render.StrokePoint(x, y, B_SOLID_HIGH);
	}

	b_dfb_render.SetSurface(NULL);

	return B_OK;
}


status_t b_dfbhapi::stroke_line(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
			        int32_t x0,  int32_t y0,  int32_t x1,  int32_t y1, BRect *margins)
{
	b_dfb_render.SetSurface(dfbSurface, margins);
	b_dfb_render.PrepareForDrawing(dc);

	status_t retVal = B_ERROR;

	if(dc->PenSize() <= 1)
	{
		b_dfb_render.StrokeLine(x0, y0, x1, y1, dc->Pattern());
		retVal = B_OK;
	}
	else
	{
		BHAPI_WARNING("[GRAPHICS]: %s --- Wide-line not supported yet.", __PRETTY_FUNCTION__);
	}

	b_dfb_render.SetSurface(NULL);

	return retVal;
}


status_t b_dfbhapi::stroke_rect(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
			        int32_t x,  int32_t y,  uint32_t w,  uint32_t h, BRect *margins)
{
	b_dfb_render.SetSurface(dfbSurface, margins);
	b_dfb_render.PrepareForDrawing(dc);

	status_t retVal = B_ERROR;

	if(dc->PenSize() <= 1)
	{
		b_dfb_render.StrokeLine(x, y, x + (int32_t)w, y, dc->Pattern());
		if(h > 0) b_dfb_render.StrokeLine(x, y + (int32_t)h, x + (int32_t)w, y + (int32_t)h, dc->Pattern());
		if(h > 1)
		{
			b_dfb_render.StrokeLine(x, y + 1, x, y + (int32_t)h - 1, dc->Pattern());
			b_dfb_render.StrokeLine(x + (int32_t)w, y + 1, x + (int32_t)w, y + (int32_t)h - 1, dc->Pattern());
		}
		retVal = B_OK;
	}
	else
	{
		BHAPI_WARNING("[GRAPHICS]: %s --- Wide-line not supported yet.", __PRETTY_FUNCTION__);
	}

	b_dfb_render.SetSurface(NULL);

	return retVal;
}


status_t b_dfb_fill_rect(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
			      int32_t x,  int32_t y,  uint32_t w,  uint32_t h, BRect *margins)
{
	b_dfb_render.SetSurface(dfbSurface, margins);
	b_dfb_render.PrepareForDrawing(dc);

	b_dfb_render.FillRect(x, y, w + 1, h + 1, dc->Pattern());

	b_dfb_render.SetSurface(NULL);

	return B_OK;
}


status_t b_dfbhapi::stroke_rects(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
				const  int32_t *rects,  int32_t count, BRect *margins)
{
	b_dfb_render.SetSurface(dfbSurface, margins);
	b_dfb_render.PrepareForDrawing(dc);

	status_t retVal = B_ERROR;

	if(dc->PenSize() <= 1)
	{
		for(int32_t i = 0; i < count; i++)
		{
			int32_t x = *rects++;  int32_t y = *rects++;  uint32_t w = (uint32_t)(*rects++);  uint32_t h = (uint32_t)(*rects++);

			b_dfb_render.StrokeLine(x, y, x + (int32_t)w, y, dc->Pattern());
			if(h > 0) b_dfb_render.StrokeLine(x, y + (int32_t)h, x + (int32_t)w, y + (int32_t)h, dc->Pattern());
			if(h > 1)
			{
				b_dfb_render.StrokeLine(x, y + 1, x, y + (int32_t)h - 1, dc->Pattern());
				b_dfb_render.StrokeLine(x + (int32_t)w, y + 1, x + (int32_t)w, y + (int32_t)h - 1, dc->Pattern());
			}
		}

		retVal = B_OK;
	}
	else
	{
		BHAPI_WARNING("[GRAPHICS]: %s --- Wide-line not supported yet.", __PRETTY_FUNCTION__);
	}

	b_dfb_render.SetSurface(NULL);

	return retVal;
}


status_t b_dfb_fill_rects(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
			      const  int32_t *rects,  int32_t count, BRect *margins)
{
	b_dfb_render.SetSurface(dfbSurface, margins);
	b_dfb_render.PrepareForDrawing(dc);

	for(int32_t i = 0; i < count; i++)
	{
		int32_t x = *rects++;  int32_t y = *rects++;  uint32_t w = (uint32_t)(*rects++);  uint32_t h = (uint32_t)(*rects++);
		b_dfb_render.FillRect(x, y, w + 1, h + 1, dc->Pattern());
	}

	b_dfb_render.SetSurface(NULL);

	return B_OK;
}


status_t b_dfb_fill_region(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
			       const BRegion &region, BRect *margins)
{
	BRegion aRegion;
	if(dc->Clipping()) aRegion = *(dc->Clipping());
	aRegion &= region;

	if(aRegion.CountRects() <= 0) return B_ERROR;

	b_dfb_render.SetSurface(dfbSurface, margins);
	b_dfb_render.PrepareForDrawing(dc);
	b_dfb_render.SetClipping(&aRegion);

	BRect rect = aRegion.Frame().FloorCopy();
	b_dfb_render.FillRect(rect, dc->Pattern());

	b_dfb_render.SetSurface(NULL);

	return B_OK;
}


status_t b_dfbhapi::stroke_arc(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
			       int32_t x,  int32_t y,  uint32_t w,  uint32_t h, float startAngle, float endAngle, BRect *margins)
{
	b_dfb_render.SetSurface(dfbSurface, margins);
	b_dfb_render.PrepareForDrawing(dc);

	status_t retVal = B_ERROR;

	if(dc->PenSize() <= 1)
	{
		if(endAngle - startAngle >= 360.f)
			b_dfb_render.StrokeEllipse(x, y, w, h, dc->Pattern());
		else
			b_dfb_render.StrokeArc(x, y, w, h, (int32_t)(startAngle * 64.f), (int32_t)(endAngle * 64.f), dc->Pattern());
		retVal = B_OK;
	}
	else
	{
		BHAPI_WARNING("[GRAPHICS]: %s --- Wide-line not supported yet.", __PRETTY_FUNCTION__);
	}

	b_dfb_render.SetSurface(NULL);

	return retVal;
}


status_t b_dfb_fill_arc(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
			     int32_t x,  int32_t y,  uint32_t w,  uint32_t h, float startAngle, float endAngle, BRect *margins)
{
	b_dfb_render.SetSurface(dfbSurface, margins);
	b_dfb_render.PrepareForDrawing(dc);

	status_t retVal = B_ERROR;

	if(endAngle - startAngle >= 360.f)
	{
		b_dfb_render.FillEllipse(x, y, w, h, true, dc->Pattern());
		retVal = B_OK;
	}
	else
	{
		BHAPI_WARNING("[GRAPHICS]: %s --- not supported yet.", __PRETTY_FUNCTION__);
	}

	b_dfb_render.SetSurface(NULL);

	return retVal;
}


status_t b_dfbhapi::stroke_polygon(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
				  const  int32_t *pts,  int32_t count, bool closed, BRect *margins)
{
	BPolygon aPolygon;
	BPoint aPt;

	for(int32_t i = 0; i < count; i++)
	{
		aPt.x = (float)(*pts++) + 0.5f;
		aPt.y = (float)(*pts++) + 0.5f;
		aPolygon.AddPoints(&aPt, 1);
	}

	if(aPolygon.CountPoints() <= 0) return B_ERROR;

	b_dfb_render.SetSurface(dfbSurface, margins);
	b_dfb_render.PrepareForDrawing(dc);

	b_dfb_render.StrokePolygon(aPolygon.Points(), aPolygon.CountPoints(), closed, dc->Pattern());

	b_dfb_render.SetSurface(NULL);

	return B_OK;
}


status_t b_dfb_fill_polygon(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
				const  int32_t *pts,  int32_t count, BRect *margins)
{
	BPolygon aPolygon;
	BPoint aPt;

	for(int32_t i = 0; i < count; i++)
	{
		aPt.x = (float)(*pts++) + 0.5f;
		aPt.y = (float)(*pts++) + 0.5f;
		aPolygon.AddPoints(&aPt, 1);
	}

	if(aPolygon.CountPoints() <= 0) return B_ERROR;

	b_dfb_render.SetSurface(dfbSurface, margins);
	b_dfb_render.PrepareForDrawing(dc);

	b_dfb_render.FillPolygon(aPolygon.Points(), aPolygon.CountPoints(), true, dc->Pattern());

	b_dfb_render.SetSurface(NULL);

	return B_OK;
}


status_t b_dfbhapi::stroke_round_rect(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
				      int32_t x,  int32_t y,  uint32_t w,  uint32_t h,  uint32_t xRadius,  uint32_t yRadius, BRect *margins)
{
	// TODO
	return B_ERROR;
}


status_t b_dfb_fill_round_rect(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
				    int32_t x,  int32_t y,  uint32_t w,  uint32_t h,  uint32_t xRadius,  uint32_t yRadius, BRect *margins)
{
	// TODO
	return B_ERROR;
}


status_t b_dfb_draw_epixmap(IDirectFBSurface *dfbSurface, BGraphicsContext *dc, const BPixmap *pix,
				int32_t x,  int32_t y,  uint32_t w,  uint32_t h,
				int32_t dstX,  int32_t dstY,  uint32_t dstW,  uint32_t dstH, BRect *margins)
{
	int maxX = 0, maxY = 0;

	if(w != dstW || h != dstH)
	{
		// TODO
		BHAPI_DEBUG("[GRAPHICS]: %s --- FIXME: (w != dstW || h != dstY).", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	dfbSurface->GetSize(dfbSurface, &maxX, &maxY);
	maxX--; maxY--;

	if(dstX > maxX || dstY > maxY) return B_ERROR;

	b_dfb_render.SetSurface(dfbSurface, margins);
	b_dfb_render.PrepareForDrawing(dc);
	b_dfb_render.SetPenSize(0);

	for(int32_t j = 0; j <= (int32_t)h; j++)
	{
		int32_t srcY = y + j;
		if(srcY < 0 || dstY + j < 0) continue;
		if(srcY > (int32_t)pix->Bounds().Height() || dstY + j > maxY) break;

		for(int32_t i = 0; i <= (int32_t)w; i++)
		{
			int32_t srcX = x + i;
			if(srcX < 0 || dstX + i < 0) continue;
			if(srcX > (int32_t)pix->Bounds().Width() || dstX + i > maxX) break;

			b_dfb_render.SetHighColor(pix->GetPixel(x + i, y + j));
			b_dfb_render.StrokePoint(dstX + i, dstY + j, B_SOLID_HIGH);
		}
	}

	b_dfb_render.SetSurface(NULL);

	return B_OK;
}

#endif /* DIRECTFB */
#endif /* LINUX */
