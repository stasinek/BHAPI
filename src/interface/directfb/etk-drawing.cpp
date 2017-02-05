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
 * File: etk-drawing.cpp
 * 
 * --------------------------------------------------------------------------*/

#include "./../render/Render.h>
#include "./../render/Pixmap.h>

#include "etk-dfb.h"

class _LOCAL EDFBRender : public ERender {
public:
	EDFBRender();

	void SetSurface(IDirectFBSurface *surface, ERect *margin = NULL);
	void SetClipping(const ERegion *clipping);
	void PrepareForDrawing(EGraphicsContext *dc);

private:
	IDirectFBSurface *fSurface;
	ERegion fClipping;
	ERect fMargins;

	virtual e_status_t InitCheck() const;
	virtual void GetFrame(eint32 *originX, eint32 *originY, euint32 *width, euint32 *height) const;
	virtual void GetPixel(eint32 x, eint32 y, e_rgb_color &color) const;
	virtual void PutPixel(eint32 x, eint32 y, e_rgb_color color);
	virtual void PutRect(eint32 x, eint32 y, euint32 width, euint32 height, e_rgb_color color);
};


EDFBRender::EDFBRender()
	: ERender(), fSurface(NULL)
{
}


void
EDFBRender::SetSurface(IDirectFBSurface *surface, ERect *margin)
{
	fSurface = surface;
	if(fSurface) fSurface->SetClip(fSurface, NULL);
	if(margin) fMargins = *margin;
	else fMargins.Set(0, 0, 0, 0);
}


void
EDFBRender::SetClipping(const ERegion *clipping)
{
	fClipping.MakeEmpty();
	if(clipping != NULL)
	{
		for(eint32 i = 0; i < clipping->CountRects(); i++)
		{
			ERect rect = clipping->RectAt(i).FloorCopy();
			fClipping.Include(rect);
		}
	}
}


void
EDFBRender::PrepareForDrawing(EGraphicsContext *dc)
{
	if(dc == NULL) return;
	SetDrawingMode(dc->DrawingMode());
	SetHighColor(dc->HighColor());
	SetLowColor(dc->LowColor());
	SetPenSize((float)dc->PenSize());
	SetClipping(dc->Clipping());
	SetSquarePointStyle(dc->IsSquarePointStyle());
}


e_status_t
EDFBRender::InitCheck() const
{
	return(fSurface ? E_OK : E_NO_INIT);
}


void
EDFBRender::GetFrame(eint32 *originX, eint32 *originY, euint32 *width, euint32 *height) const
{
	int w = 0, h = 0;
	if(fSurface) fSurface->GetSize(fSurface, &w, &h);

	if(originX) *originX = 0;
	if(originY) *originY = 0;
	if(width) *width = (euint32)w - (euint32)fMargins.left - (euint32)fMargins.top;
	if(height) *height = (euint32)h - (euint32)fMargins.top - (euint32)fMargins.bottom;
}


void
EDFBRender::GetPixel(eint32 x, eint32 y, e_rgb_color &color) const
{
	DFBSurfacePixelFormat pixel_format;
	void *ptr;
	int pitch;
	euint32 dfbColor = 0;

	x += (eint32)fMargins.left;
	y += (eint32)fMargins.top;

	if(fSurface == NULL) return;
	if(fSurface->GetPixelFormat(fSurface, &pixel_format) != DFB_OK)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- fSurface->GetPixelFormat() failed.", __PRETTY_FUNCTION__);
		return;
	}

	if(fSurface->Lock(fSurface, DSLF_READ, &ptr, &pitch) != DFB_OK)
	{
		ETK_DEBUG("[GRAPHICS]: %s --- fSurface->Lock() failed.", __PRETTY_FUNCTION__);
		return;
	}

	switch(DFB_BYTES_PER_PIXEL(pixel_format))
	{
		case 1: // 8-bpp
			dfbColor = (euint32)(*((euint8*)ptr + y * pitch + x));
			break;

		case 2: // 15-bpp or 16-bpp
			dfbColor = (euint32)(*((euint16*)ptr + y * pitch / 2 + x));
			break;

		case 3: // 24-bpp
			{
				euint8 *bufp;
				bufp = (euint8*)ptr + y * pitch + x * 3;
#ifdef ETK_BIG_ENDIAN
				dfbColor = ((euint32)bufp[0] << 16) | ((euint32)bufp[1] << 8) | (euint32)bufp[2];
#else
				dfbColor = ((euint32)bufp[2] << 16) | ((euint32)bufp[1] << 8) | (euint32)bufp[0];
#endif
			}
			break;

		case 4: // 32-bpp
			dfbColor = *((euint32*)ptr + y * pitch / 4 + x);
			break;

		default:
//			ETK_DEBUG("[GRAPHICS]: %s --- Unsupported pixel format.", __PRETTY_FUNCTION__);
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
//				ETK_DEBUG("[GRAPHICS]: %s --- Unsupported pixel format.", __PRETTY_FUNCTION__);
				break;
		}
	}
	else
	{
		IDirectFBPalette *pal = NULL;
		DFBColor c;

		if(fSurface->GetPalette(fSurface, &pal) != DFB_OK)
		{
			ETK_DEBUG("[GRAPHICS]: %s --- fSurface->GetPalette() failed.", __PRETTY_FUNCTION__);
			return;
		}
		if(pal->GetEntries(pal, &c, 1, (unsigned int)dfbColor) != DFB_OK)
		{
			ETK_DEBUG("[GRAPHICS]: %s --- pal->GetEntries() failed.", __PRETTY_FUNCTION__);
			return;
		}

		color.set_to(c.r, c.g, c.b, 0xff);
	}

//	ETK_DEBUG("[GRAPHICS]: %s --- Pixel Format: 0x%x, Color 0x%x, R %I8u, G %I8u, B %I8u",
//		  __PRETTY_FUNCTION__, pixel_format, dfbColor, color.red, color.green, color.blue);
}


void
EDFBRender::PutPixel(eint32 x, eint32 y, e_rgb_color color)
{
	if(fSurface == NULL) return;
	if(fClipping.Contains(EPoint((float)x, (float)y)) == false) return;

	x += (eint32)fMargins.left;
	y += (eint32)fMargins.top;

	fSurface->SetDrawingFlags(fSurface, DSDRAW_NOFX);
	fSurface->SetColor(fSurface, color.red, color.green, color.blue, 255);
	fSurface->FillRectangle(fSurface, x, y, 1, 1);
}


void
EDFBRender::PutRect(eint32 x, eint32 y, euint32 width, euint32 height, e_rgb_color color)
{
	if(fSurface == NULL || width == 0 || height == 0) return;

	ERegion aRegion(fClipping);
	aRegion &= ERect((float)x, (float)y, (float)x + (float)width - 1.f, (float)y + (float)height - 1.f);
	if(aRegion.CountRects() <= 0) return;
	aRegion.OffsetBy(fMargins.left, fMargins.top);

	fSurface->SetDrawingFlags(fSurface, DSDRAW_NOFX);
	fSurface->SetColor(fSurface, color.red, color.green, color.blue, 255);

#ifdef DFB_HAVE_FILLRECTANGLES
	DFBRectangle *dfbRects = (DFBRectangle*)malloc(sizeof(DFBRectangle) * (size_t)aRegion.CountRects());
#endif
	for(eint32 i = 0; i < aRegion.CountRects(); i++)
	{
		ERect r = aRegion.RectAt(i).FloorCopy();
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


static EDFBRender etk_dfb_render;


e_status_t etk_dfb_stroke_point(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
				eint32 x, eint32 y, ERect *margins)
{
	etk_dfb_render.SetSurface(dfbSurface, margins);
	etk_dfb_render.PrepareForDrawing(dc);

	etk_dfb_render.StrokePoint(x, y, dc->Pattern());

	etk_dfb_render.SetSurface(NULL);

	return E_OK;
}


e_status_t etk_dfb_stroke_points(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
				 const eint32 *pts, eint32 count, ERect *margins)
{
	etk_dfb_render.SetSurface(dfbSurface, margins);
	etk_dfb_render.PrepareForDrawing(dc);

	for(eint32 i = 0; i < count; i++)
	{
		eint32 x = *pts++;
		eint32 y = *pts++;
		etk_dfb_render.StrokePoint(x, y, dc->Pattern());
	}

	etk_dfb_render.SetSurface(NULL);

	return E_OK;
}


e_status_t etk_dfb_stroke_points_color(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
				       const EList *ptsArrayLists, eint32 arrayCount, const e_rgb_color *high_colors, ERect *margins)
{
	etk_dfb_render.SetSurface(dfbSurface, margins);
	etk_dfb_render.PrepareForDrawing(dc);

	e_rgb_color oldColor = dc->HighColor();

	for(eint32 k = 0; k < arrayCount; k++, ptsArrayLists++)
	{
		if(ptsArrayLists == NULL) break;

		e_rgb_color color = (high_colors == NULL ? oldColor : *high_colors++);

		eint32 count = ptsArrayLists->CountItems();
		if(count <= 0) continue;

		etk_dfb_render.SetHighColor(color);

		for(eint32 i = 0; i < count; i++)
		{
			const eint32 *pt = (const eint32*)ptsArrayLists->ItemAt(i);
			if(!pt) continue;

			eint32 x = *pt++;
			eint32 y = *pt++;
			etk_dfb_render.StrokePoint(x, y, dc->Pattern());
		}
	}

	etk_dfb_render.SetSurface(NULL);

	return E_OK;
}


e_status_t etk_dfb_stroke_points_alphas(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
					const eint32 *pts, const euint8 *alpha, eint32 count, ERect *margins)
{
	etk_dfb_render.SetSurface(dfbSurface, margins);
	etk_dfb_render.PrepareForDrawing(dc);
	etk_dfb_render.SetDrawingMode(E_OP_ALPHA);

	e_rgb_color c = dc->HighColor();

	for(eint32 i = 0; i < count; i++)
	{
		eint32 x = *pts++;
		eint32 y = *pts++;
		c.alpha = *alpha++;

		etk_dfb_render.SetHighColor(c);
		etk_dfb_render.StrokePoint(x, y, E_SOLID_HIGH);
	}

	etk_dfb_render.SetSurface(NULL);

	return E_OK;
}


e_status_t etk_dfb_stroke_line(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
			       eint32 x0, eint32 y0, eint32 x1, eint32 y1, ERect *margins)
{
	etk_dfb_render.SetSurface(dfbSurface, margins);
	etk_dfb_render.PrepareForDrawing(dc);

	e_status_t retVal = E_ERROR;

	if(dc->PenSize() <= 1)
	{
		etk_dfb_render.StrokeLine(x0, y0, x1, y1, dc->Pattern());
		retVal = E_OK;
	}
	else
	{
		ETK_WARNING("[GRAPHICS]: %s --- Wide-line not supported yet.", __PRETTY_FUNCTION__);
	}

	etk_dfb_render.SetSurface(NULL);

	return retVal;
}


e_status_t etk_dfb_stroke_rect(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
			       eint32 x, eint32 y, euint32 w, euint32 h, ERect *margins)
{
	etk_dfb_render.SetSurface(dfbSurface, margins);
	etk_dfb_render.PrepareForDrawing(dc);

	e_status_t retVal = E_ERROR;

	if(dc->PenSize() <= 1)
	{
		etk_dfb_render.StrokeLine(x, y, x + (eint32)w, y, dc->Pattern());
		if(h > 0) etk_dfb_render.StrokeLine(x, y + (eint32)h, x + (eint32)w, y + (eint32)h, dc->Pattern());
		if(h > 1)
		{
			etk_dfb_render.StrokeLine(x, y + 1, x, y + (eint32)h - 1, dc->Pattern());
			etk_dfb_render.StrokeLine(x + (eint32)w, y + 1, x + (eint32)w, y + (eint32)h - 1, dc->Pattern());
		}
		retVal = E_OK;
	}
	else
	{
		ETK_WARNING("[GRAPHICS]: %s --- Wide-line not supported yet.", __PRETTY_FUNCTION__);
	}

	etk_dfb_render.SetSurface(NULL);

	return retVal;
}


e_status_t etk_dfb_fill_rect(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
			     eint32 x, eint32 y, euint32 w, euint32 h, ERect *margins)
{
	etk_dfb_render.SetSurface(dfbSurface, margins);
	etk_dfb_render.PrepareForDrawing(dc);

	etk_dfb_render.FillRect(x, y, w + 1, h + 1, dc->Pattern());

	etk_dfb_render.SetSurface(NULL);

	return E_OK;
}


e_status_t etk_dfb_stroke_rects(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
				const eint32 *rects, eint32 count, ERect *margins)
{
	etk_dfb_render.SetSurface(dfbSurface, margins);
	etk_dfb_render.PrepareForDrawing(dc);

	e_status_t retVal = E_ERROR;

	if(dc->PenSize() <= 1)
	{
		for(eint32 i = 0; i < count; i++)
		{
			eint32 x = *rects++; eint32 y = *rects++; euint32 w = (euint32)(*rects++); euint32 h = (euint32)(*rects++);

			etk_dfb_render.StrokeLine(x, y, x + (eint32)w, y, dc->Pattern());
			if(h > 0) etk_dfb_render.StrokeLine(x, y + (eint32)h, x + (eint32)w, y + (eint32)h, dc->Pattern());
			if(h > 1)
			{
				etk_dfb_render.StrokeLine(x, y + 1, x, y + (eint32)h - 1, dc->Pattern());
				etk_dfb_render.StrokeLine(x + (eint32)w, y + 1, x + (eint32)w, y + (eint32)h - 1, dc->Pattern());
			}
		}

		retVal = E_OK;
	}
	else
	{
		ETK_WARNING("[GRAPHICS]: %s --- Wide-line not supported yet.", __PRETTY_FUNCTION__);
	}

	etk_dfb_render.SetSurface(NULL);

	return retVal;
}


e_status_t etk_dfb_fill_rects(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
			      const eint32 *rects, eint32 count, ERect *margins)
{
	etk_dfb_render.SetSurface(dfbSurface, margins);
	etk_dfb_render.PrepareForDrawing(dc);

	for(eint32 i = 0; i < count; i++)
	{
		eint32 x = *rects++; eint32 y = *rects++; euint32 w = (euint32)(*rects++); euint32 h = (euint32)(*rects++);
		etk_dfb_render.FillRect(x, y, w + 1, h + 1, dc->Pattern());
	}

	etk_dfb_render.SetSurface(NULL);

	return E_OK;
}


e_status_t etk_dfb_fill_region(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
			       const ERegion &region, ERect *margins)
{
	ERegion aRegion;
	if(dc->Clipping()) aRegion = *(dc->Clipping());
	aRegion &= region;

	if(aRegion.CountRects() <= 0) return E_ERROR;

	etk_dfb_render.SetSurface(dfbSurface, margins);
	etk_dfb_render.PrepareForDrawing(dc);
	etk_dfb_render.SetClipping(&aRegion);

	ERect rect = aRegion.Frame().FloorCopy();
	etk_dfb_render.FillRect(rect, dc->Pattern());

	etk_dfb_render.SetSurface(NULL);

	return E_OK;
}


e_status_t etk_dfb_stroke_arc(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
			      eint32 x, eint32 y, euint32 w, euint32 h, float startAngle, float endAngle, ERect *margins)
{
	etk_dfb_render.SetSurface(dfbSurface, margins);
	etk_dfb_render.PrepareForDrawing(dc);

	e_status_t retVal = E_ERROR;

	if(dc->PenSize() <= 1)
	{
		if(endAngle - startAngle >= 360.f)
			etk_dfb_render.StrokeEllipse(x, y, w, h, dc->Pattern());
		else
			etk_dfb_render.StrokeArc(x, y, w, h, (eint32)(startAngle * 64.f), (eint32)(endAngle * 64.f), dc->Pattern());
		retVal = E_OK;
	}
	else
	{
		ETK_WARNING("[GRAPHICS]: %s --- Wide-line not supported yet.", __PRETTY_FUNCTION__);
	}

	etk_dfb_render.SetSurface(NULL);

	return retVal;
}


e_status_t etk_dfb_fill_arc(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
			    eint32 x, eint32 y, euint32 w, euint32 h, float startAngle, float endAngle, ERect *margins)
{
	etk_dfb_render.SetSurface(dfbSurface, margins);
	etk_dfb_render.PrepareForDrawing(dc);

	e_status_t retVal = E_ERROR;

	if(endAngle - startAngle >= 360.f)
	{
		etk_dfb_render.FillEllipse(x, y, w, h, true, dc->Pattern());
		retVal = E_OK;
	}
	else
	{
		ETK_WARNING("[GRAPHICS]: %s --- not supported yet.", __PRETTY_FUNCTION__);
	}

	etk_dfb_render.SetSurface(NULL);

	return retVal;
}


e_status_t etk_dfb_stroke_polygon(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
				  const eint32 *pts, eint32 count, bool closed, ERect *margins)
{
	EPolygon aPolygon;
	EPoint aPt;

	for(eint32 i = 0; i < count; i++)
	{
		aPt.x = (float)(*pts++) + 0.5f;
		aPt.y = (float)(*pts++) + 0.5f;
		aPolygon.AddPoints(&aPt, 1);
	}

	if(aPolygon.CountPoints() <= 0) return E_ERROR;

	etk_dfb_render.SetSurface(dfbSurface, margins);
	etk_dfb_render.PrepareForDrawing(dc);

	etk_dfb_render.StrokePolygon(aPolygon.Points(), aPolygon.CountPoints(), closed, dc->Pattern());

	etk_dfb_render.SetSurface(NULL);

	return E_OK;
}


e_status_t etk_dfb_fill_polygon(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
				const eint32 *pts, eint32 count, ERect *margins)
{
	EPolygon aPolygon;
	EPoint aPt;

	for(eint32 i = 0; i < count; i++)
	{
		aPt.x = (float)(*pts++) + 0.5f;
		aPt.y = (float)(*pts++) + 0.5f;
		aPolygon.AddPoints(&aPt, 1);
	}

	if(aPolygon.CountPoints() <= 0) return E_ERROR;

	etk_dfb_render.SetSurface(dfbSurface, margins);
	etk_dfb_render.PrepareForDrawing(dc);

	etk_dfb_render.FillPolygon(aPolygon.Points(), aPolygon.CountPoints(), true, dc->Pattern());

	etk_dfb_render.SetSurface(NULL);

	return E_OK;
}


e_status_t etk_dfb_stroke_round_rect(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
				     eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius, ERect *margins)
{
	// TODO
	return E_ERROR;
}


e_status_t etk_dfb_fill_round_rect(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
				   eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius, ERect *margins)
{
	// TODO
	return E_ERROR;
}


e_status_t etk_dfb_draw_epixmap(IDirectFBSurface *dfbSurface, EGraphicsContext *dc, const EPixmap *pix,
				eint32 x, eint32 y, euint32 w, euint32 h,
				eint32 dstX, eint32 dstY, euint32 dstW, euint32 dstH, ERect *margins)
{
	int maxX = 0, maxY = 0;

	if(w != dstW || h != dstH)
	{
		// TODO
		ETK_DEBUG("[GRAPHICS]: %s --- FIXME: (w != dstW || h != dstY).", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	dfbSurface->GetSize(dfbSurface, &maxX, &maxY);
	maxX--; maxY--;

	if(dstX > maxX || dstY > maxY) return E_ERROR;

	etk_dfb_render.SetSurface(dfbSurface, margins);
	etk_dfb_render.PrepareForDrawing(dc);
	etk_dfb_render.SetPenSize(0);

	for(eint32 j = 0; j <= (eint32)h; j++)
	{
		eint32 srcY = y + j;
		if(srcY < 0 || dstY + j < 0) continue;
		if(srcY > (eint32)pix->Bounds().Height() || dstY + j > maxY) break;

		for(eint32 i = 0; i <= (eint32)w; i++)
		{
			eint32 srcX = x + i;
			if(srcX < 0 || dstX + i < 0) continue;
			if(srcX > (eint32)pix->Bounds().Width() || dstX + i > maxX) break;

			etk_dfb_render.SetHighColor(pix->GetPixel(x + i, y + j));
			etk_dfb_render.StrokePoint(dstX + i, dstY + j, E_SOLID_HIGH);
		}
	}

	etk_dfb_render.SetSurface(NULL);

	return E_OK;
}

