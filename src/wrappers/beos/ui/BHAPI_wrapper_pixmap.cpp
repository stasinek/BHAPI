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
#ifdef BHAPI_OS_BEOS


#include <be/interface/View.h>
#include <be/interface/Region.h>
#include <be/interface/Bitmap.h>

#include <BHAPI_wrapper_beos-graphics.h>

#include <kits/support/ClassInfo.h>
#include <kits/support/Autolock.h>
#include <kits/render/Pixmap.h>


EBeBitmapPriv::EBeBitmapPriv(uint32 w,  uint32 h)
	: BBitmap(BRect(0, 0, w, h), B_RGB32, true)
{
	if((fView = new BView(BRect(0, 0, w, h), NULL, B_FOLLOW_ALL, 0)) != NULL)
	{
		Lock();
		AddChild(fView);
		fView->SetLineMode(B_BUTT_CAP, B_MITER_JOIN);
		fView->SetViewColor(255, 255, 255);
		fView->SetHighColor(255, 255, 255);
		fView->SetDrawingMode(B_OP_COPY);
		fView->FillRect(fView->Bounds(), B_SOLID_HIGH);
		Unlock();
	}
}


EBeBitmapPriv::~EBeBitmapPriv()
{
}


EBeGraphicsDrawable::EBeGraphicsDrawable(EBeGraphicsEngine *beEngine,  uint32 w,  uint32 h)
	: BGraphicsDrawable(), beBitmap(NULL), fEngine(NULL)
{
	if(w == B_MAXUINT32 || h == B_MAXUINT32)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return;
	}

	fEngine = beEngine;
	if(fEngine == NULL) return;

	if((beBitmap = new EBeBitmapPriv(w, h)) == NULL || beBitmap->fView == NULL)
	{
		if(beBitmap != NULL) {delete beBitmap; beBitmap = NULL;}
		fEngine = NULL;
		return;
	}

	bhapi::rgb_color whiteColor = {255, 255, 255, 255};
	BGraphicsDrawable::SetBackgroundColor(whiteColor);
}


EBeGraphicsDrawable::~EBeGraphicsDrawable()
{
	if(beBitmap != NULL) delete beBitmap;
}


status_t 
EBeGraphicsDrawable::SetBackgroundColor(bhapi::rgb_color bkColor)
{
	bhapi::rgb_color color = BackgroundColor();
	bkColor.alpha = color.alpha = 255;
	if(bkColor == color) return B_OK;

	if(fEngine == NULL) return B_ERROR;

	BAutolock <EBeGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	if(beBitmap == NULL || beBitmap->fView == NULL) return B_ERROR;

	beBitmap->Lock();
	beBitmap->fView->SetDrawingMode(B_OP_COPY);
	beBitmap->fView->SetPenSize(0);
	beBitmap->fView->SetViewColor(*((rgb_color*)&bkColor));
	beBitmap->fView->SetHighColor(*((rgb_color*)&bkColor));
	beBitmap->fView->FillRect(beBitmap->fView->Bounds());
	beBitmap->Unlock();

	BGraphicsDrawable::SetBackgroundColor(bkColor);

	return B_OK;
}


status_t 
EBeGraphicsDrawable::ResizeTo(uint32 w,  uint32 h)
{
	if(w == B_MAXUINT32 || h == B_MAXUINT32)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	if(fEngine == NULL) return B_ERROR;

	BAutolock <EBeGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return B_ERROR;

	EBeBitmapPriv *newBitmap = new EBeBitmapPriv(w, h);
	if(newBitmap == NULL || newBitmap->fView == NULL)
	{
		if(newBitmap != NULL) delete newBitmap;
		return B_ERROR;
	}
	if(beBitmap != NULL) delete beBitmap;
	beBitmap = newBitmap;

	return B_OK;
}


static void __bhapi_convert_region(const BRegion *region, BRegion *beRegion, BRect maxRect)
{
	if(beRegion == NULL) return;

	if(region != NULL)
	{
		for(int32 i = 0; i < region->CountRects(); i++)
		{
			BRect r = region->RectAt(i);
			BRect bRect(r.left, r.top, r.right, r.bottom);
			beRegion->Include(bRect);
		}
	}
	else
	{
		beRegion->Set(maxRect);
	}
}


status_t 
EBeGraphicsDrawable::CopyTo(BGraphicsContext *dc,
			    BGraphicsDrawable *dstDrawable,
			     int32 x,  int32 y,  uint32 w,  uint32 h,
			     int32 dstX,  int32 dstY,  uint32 dstW,  uint32 dstH)
{
	if(w == B_MAXUINT32 || h == B_MAXUINT32 || dstW == B_MAXUINT32 || dstH == B_MAXUINT32)
	{
		BHAPI_DEBUG("[GRAPHICS]: %s --- Either width or height is so large.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	if(fEngine == NULL || dc == NULL || dstDrawable == NULL || fEngine->Lock() == false) return B_ERROR;

	if(dc->DrawingMode() != B_OP_COPY)
	{
		// TODO
		fEngine->Unlock();
		BHAPI_DEBUG("[GRAPHICS]: %s --- FIXME: unsupported drawing mode.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	if(fEngine->InitCheck() != B_OK || beBitmap == NULL || beBitmap->fView == NULL)
	{
		fEngine->Unlock();
		return B_ERROR;
	}

	EBeGraphicsWindow *win = NULL;
	EBeGraphicsDrawable *pix = NULL;

	status_t retVal = B_ERROR;

	if((win = cast_as(dstDrawable, EBeGraphicsWindow)) != NULL)
	{
		if(win->beWinMsgr.IsValid())
		{
			beBitmap->Lock();
			beBitmap->fView->Sync();
			beBitmap->Unlock();

			BMessage msg('b_');
			msg.AddInt32("BHAPI:what", BHAPI_BEOS_DRAW_BITMAP);
			msg.AddPointer("bitmap", beBitmap);
			msg.AddRect("src", BRect(x, y, x + w, y + h));
			msg.AddRect("dest", BRect(dstX, dstY, dstX + dstW, dstY + dstH));
			if(dc->Clipping() != NULL) msg.AddPointer("clipping", (const void*)dc->Clipping());

			win->beWinMsgr.SendMessage(&msg, &msg);
		}
	}
	else if((pix = cast_as(dstDrawable, EBeGraphicsDrawable)) != NULL)
	{
		if(pix->beBitmap != NULL)
		{
			beBitmap->Lock();
			beBitmap->fView->Sync();
			beBitmap->Unlock();

			pix->beBitmap->Lock();

			BRegion beRegion;
			__bhapi_convert_region(dc->Clipping(), &beRegion, pix->beBitmap->Bounds());
			pix->beBitmap->fView->ConstrainClippingRegion(&beRegion);
			pix->beBitmap->fView->DrawBitmap(beBitmap, BRect(x, y, x + w, y + h), BRect(dstX, dstY, dstX + dstW, dstY + dstH));

			pix->beBitmap->Unlock();
		}
	}

	fEngine->Unlock();

	return retVal;
}


bool __bhapi_prepare_beview(BView *view, BGraphicsContext *dc)
{
	if(view == NULL || dc == NULL || dc->Clipping() == NULL || dc->Clipping()->CountRects() <= 0) return false;

	if(dc->DrawingMode() == B_OP_XOR)
	{
		BHAPI_WARNING("[GRAPHICS]: %s --- drawing mode 'E_OP_XOR' not supported.", __PRETTY_FUNCTION__);
		return false;
	}

	view->SetDrawingMode((drawing_mode)((int32)dc->DrawingMode() - (dc->DrawingMode() == B_OP_COPY ? 0 : 1)));
	view->SetPenSize((float)(dc->PenSize()));

	bhapi::rgb_color color = dc->HighColor();
	view->SetHighColor(*((rgb_color*)&color));

	color = dc->LowColor();
	view->SetLowColor(*((rgb_color*)&color));

	BRegion beRegion;
	__bhapi_convert_region(dc->Clipping(), &beRegion, view->Bounds());
	view->ConstrainClippingRegion(&beRegion);

	return true;
}


status_t 
EBeGraphicsDrawable::DrawPixmap(BGraphicsContext *dc, const BPixmap *epixmap,
				int32 x,  int32 y,  uint32 w,  uint32 h,
				int32 dstX,  int32 dstY,  uint32 dstW,  uint32 dstH)
{
	if(fEngine == NULL || epixmap == NULL || epixmap->IsValid() == false) return B_ERROR;

	BAutolock <EBeGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK || beBitmap == NULL) return B_ERROR;

	BAutolock <EBeBitmapPriv> bitmap_autolock(beBitmap);

	BView *bView = beBitmap->fView;
	if(bView == NULL || __bhapi_prepare_beview(bView, dc) == false) return B_ERROR;

	void *bits = NULL;
	int32 bitsLen = 0;

	switch(epixmap->ColorSpace())
	{
		case B_RGB24_BIG:
			bitsLen = (int32)epixmap->BitsLength();
			bits = (void*)epixmap->Bits();
			break;

		case B_RGB24:
			bitsLen = (int32)epixmap->BitsLength();
			if((bits = malloc((size_t)bitsLen)) != NULL)
			{
				uint8 *tmp = (uint8*)bits;
				const  uint8 *src = (const  uint8*)epixmap->Bits();
				for(uint32 i = 0; i < epixmap->BitsLength(); i += 3, tmp += 3, src += 3)
				{
					tmp[0] = src[2];
					tmp[1] = src[1];
					tmp[2] = src[0];
				}
			}
			break;

		case B_RGB32:
		case B_RGBA32:
			bitsLen = ((int32)epixmap->Bounds().Width() + 1) * ((int32)epixmap->Bounds().Height() + 1) * 3;
			if((bits = malloc((size_t)bitsLen)) != NULL)
			{
				uint8 *tmp = (uint8*)bits;
				const  uint8 *src = (const  uint8*)epixmap->Bits();
				for(uint32 i = 0; i < epixmap->BitsLength(); i += 4, tmp += 3, src += 4)
				{
					tmp[0] = src[2];
					tmp[1] = src[1];
					tmp[2] = src[0];
				}
			}
			break;

		default:
			break;
	}

	if(bits == NULL)
	{
			BHAPI_WARNING("[GRAPHICS]: %s --- Unsupported color space (0x%x).", __PRETTY_FUNCTION__, epixmap->ColorSpace());
			return B_ERROR;
	}

	BRect r = epixmap->Bounds();
	BRect bRect(r.left, r.top, r.right, r.bottom);
	BBitmap *tmp = new BBitmap(bRect, B_RGB32);
	tmp->SetBits(bits, bitsLen, 0, B_RGB32);
	if(tmp->IsValid()) bView->DrawBitmap(tmp, BRect(x, y, x + w, y + h), BRect(dstX, dstY, dstX + dstW, dstY + dstH));
	bView->Sync();
	delete tmp;

	if(bits != NULL && bits != (void*)epixmap->Bits()) free(bits);

	return B_OK;
}


status_t 
EBeGraphicsDrawable::StrokePoint(BGraphicsContext *dc,
				  int32 x,  int32 y)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EBeGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK || beBitmap == NULL) return B_ERROR;

	BAutolock <EBeBitmapPriv> bitmap_autolock(beBitmap);

	BView *bView = beBitmap->fView;
	if(bView == NULL || __bhapi_prepare_beview(bView, dc) == false) return B_ERROR;

	bhapi::pattern pat = dc->Pattern();

	if(dc->PenSize() <= 1)
	{
		bView->FillRect(BRect(x, y, x, y), *((pattern*)&pat));
	}
	else
	{
		BRect rect((float)x + 0.5, (float)y + 0.5,
			   (float)x + 0.5 + dc->PenSize(), (float)y + 0.5 + dc->PenSize());
		rect.OffsetBy((float)dc->PenSize() / -2.f, (float)dc->PenSize() / -2.f);

		if(dc->IsSquarePointStyle())
			bView->FillRect(rect, *((pattern*)&pat));
		else
			bView->FillEllipse(rect, *((pattern*)&pat));
	}

	return B_OK;
}


status_t 
EBeGraphicsDrawable::StrokePoints(BGraphicsContext *dc,
				  const  int32 *pts,  int32 count)
{
	if(fEngine == NULL) return B_ERROR;
	if(pts == NULL || count <= 0) return B_ERROR;

	BAutolock <EBeGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK || beBitmap == NULL) return B_ERROR;

	BAutolock <EBeBitmapPriv> bitmap_autolock(beBitmap);

	BView *bView = beBitmap->fView;
	if(bView == NULL || __bhapi_prepare_beview(bView, dc) == false) return B_ERROR;

	bhapi::pattern pat = dc->Pattern();

	if(dc->PenSize() <= 1)
	{
		for(int32 i = 0; i < count; i++)
		{
			int32 x = *pts++;
			int32 y = *pts++;
			bView->FillRect(BRect(x, y, x, y), *((pattern*)&pat));
		}
	}
	else
	{
		for(int32 i = 0; i < count; i++)
		{
			int32 x = *pts++;
			int32 y = *pts++;

			BRect rect((float)x + 0.5, (float)y + 0.5,
				   (float)x + 0.5 + dc->PenSize(), (float)y + 0.5 + dc->PenSize());
			rect.OffsetBy((float)dc->PenSize() / -2.f, (float)dc->PenSize() / -2.f);

			if(dc->IsSquarePointStyle())
				bView->FillRect(rect, *((pattern*)&pat));
			else
				bView->FillEllipse(rect, *((pattern*)&pat));
		}
	}

	return B_OK;
}


status_t 
EBeGraphicsDrawable::StrokePoints_Colors(BGraphicsContext *dc,
					 const BList *ptsArrayLists,  int32 arrayCount,
					 const bhapi::rgb_color *highColors)
{
	if(fEngine == NULL) return B_ERROR;
	if(ptsArrayLists == NULL || arrayCount <= 0) return B_ERROR;

	BAutolock <EBeGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK || beBitmap == NULL) return B_ERROR;

	BAutolock <EBeBitmapPriv> bitmap_autolock(beBitmap);

	BView *bView = beBitmap->fView;
	if(bView == NULL || __bhapi_prepare_beview(bView, dc) == false) return B_ERROR;

	bhapi::rgb_color oldColor = dc->HighColor();
	for(int32 k = 0; k < arrayCount; k++, ptsArrayLists++)
	{
		if(ptsArrayLists == NULL) break;

		bhapi::rgb_color color = (highColors == NULL ? oldColor : *highColors++);

		int32 count = ptsArrayLists->CountItems();
		if(count <= 0) continue;

		bView->SetHighColor(*((rgb_color*)&color));

		if(dc->PenSize() <= 1)
		{
			for(int32 i = 0; i < count; i++)
			{
				const  int32 *pt = (const  int32*)ptsArrayLists->ItemAt(i);
				if(!pt) continue;

				int32 x = *pt++;
				int32 y = *pt++;
				bView->FillRect(BRect(x, y, x, y), B_SOLID_HIGH);
			}
		}
		else
		{
			for(int32 i = 0; i < count; i++)
			{
				const  int32 *pt = (const  int32*)ptsArrayLists->ItemAt(i);
				if(!pt) continue;

				int32 x = *pt++;
				int32 y = *pt++;

				BRect rect((float)x + 0.5, (float)y + 0.5,
					   (float)x + 0.5 + dc->PenSize(), (float)y + 0.5 + dc->PenSize());
				rect.OffsetBy((float)dc->PenSize() / -2.f, (float)dc->PenSize() / -2.f);

				if(dc->IsSquarePointStyle())
					bView->FillRect(rect, B_SOLID_HIGH);
				else
					bView->FillEllipse(rect, B_SOLID_HIGH);
			}
		}
	}

	return B_OK;
}


status_t 
EBeGraphicsDrawable::StrokePoints_Alphas(BGraphicsContext *dc,
					 const  int32 *pts, const  uint8 *alpha,  int32 count)
{
	if(fEngine == NULL) return B_ERROR;
	if(pts == NULL || count <= 0) return B_ERROR;

	BAutolock <EBeGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK || beBitmap == NULL) return B_ERROR;

	BAutolock <EBeBitmapPriv> bitmap_autolock(beBitmap);

	BView *bView = beBitmap->fView;
	if(bView == NULL || __bhapi_prepare_beview(bView, dc) == false) return B_ERROR;

	bhapi::rgb_color color = dc->HighColor();
	color.alpha = 255;
	bView->SetDrawingMode(B_OP_ALPHA);

	if(dc->PenSize() <= 1)
	{
		for(int32 i = 0; i < count; i++)
		{
			int32 x = *pts++;
			int32 y = *pts++;
			color.alpha = (alpha == NULL ? 255 : *alpha++);

			if(color.alpha == 0) continue;
			bView->SetHighColor(*((rgb_color*)&color));
			bView->FillRect(BRect(x, y, x, y), B_SOLID_HIGH);
		}
	}
	else
	{
		for(int32 i = 0; i < count; i++)
		{
			int32 x = *pts++;
			int32 y = *pts++;
			color.alpha = (alpha == NULL ? 255 : *alpha++);

			BRect rect((float)x + 0.5, (float)y + 0.5,
				   (float)x + 0.5 + dc->PenSize(), (float)y + 0.5 + dc->PenSize());
			rect.OffsetBy((float)dc->PenSize() / -2.f, (float)dc->PenSize() / -2.f);

			if(color.alpha == 0) continue;
			bView->SetHighColor(*((rgb_color*)&color));
			if(dc->IsSquarePointStyle())
				bView->FillRect(rect, B_SOLID_HIGH);
			else
				bView->FillEllipse(rect, B_SOLID_HIGH);
		}
	}

	return B_OK;
}


status_t 
EBeGraphicsDrawable::StrokeLine(BGraphicsContext *dc,
				int32 x0,  int32 y0,  int32 x1,  int32 y1)
{
	if(x0 == x1 && y0 == y1) return StrokePoint(dc, x0, y0);

	if(fEngine == NULL) return B_ERROR;

	BAutolock <EBeGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK || beBitmap == NULL) return B_ERROR;

	BAutolock <EBeBitmapPriv> bitmap_autolock(beBitmap);

	BView *bView = beBitmap->fView;
	if(bView == NULL || __bhapi_prepare_beview(bView, dc) == false) return B_ERROR;

	bhapi::pattern pat = dc->Pattern();

	bView->StrokeLine(BPoint(x0, y0), BPoint(x1, y1), *((pattern*)&pat));

	return B_OK;
}


status_t 
EBeGraphicsDrawable::StrokePolygon(BGraphicsContext *dc,
				   const  int32 *pts,  int32 count, bool closed)
{
	if(fEngine == NULL || dc == NULL || pts == NULL || count <= 0 || count >= B_MAXINT32) return B_ERROR;

	if(count == 1) return StrokePoint(dc, pts[0], pts[1]);
	else if(count == 2) return StrokeLine(dc, pts[0], pts[1], pts[2], pts[3]);

	BAutolock <EBeGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK || beBitmap == NULL) return B_ERROR;

	BAutolock <EBeBitmapPriv> bitmap_autolock(beBitmap);

	BView *bView = beBitmap->fView;
	if(bView == NULL || __bhapi_prepare_beview(bView, dc) == false) return B_ERROR;

	bhapi::pattern pat = dc->Pattern();

	BPoint *bPts = new BPoint[count];
	if(!bPts) return B_ERROR;
	for(int32 i = 0; i < count; i++) {bPts[i].x = *pts++; bPts[i].y = *pts++;}

	bView->StrokePolygon(bPts, count, closed, *((pattern*)&pat));

	delete[] bPts;

	return B_OK;
}


status_t 
EBeGraphicsDrawable::FillPolygon(BGraphicsContext *dc,
				 const  int32 *pts,  int32 count)
{
	if(fEngine == NULL || dc == NULL || pts == NULL || count <= 0 || count >= B_MAXINT32) return B_ERROR;

	if(count == 1) return FillRect(dc, pts[0], pts[1], 0, 0);

	BAutolock <EBeGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK || beBitmap == NULL) return B_ERROR;

	BAutolock <EBeBitmapPriv> bitmap_autolock(beBitmap);

	BView *bView = beBitmap->fView;
	if(bView == NULL || __bhapi_prepare_beview(bView, dc) == false) return B_ERROR;

	bhapi::pattern pat = dc->Pattern();

	BPoint *bPts = new BPoint[count];
	if(!bPts) return B_ERROR;
	for(int32 i = 0; i < count; i++) {bPts[i].x = *pts++; bPts[i].y = *pts++;}

	bView->FillPolygon(bPts, count, *((pattern*)&pat));

	delete[] bPts;

	return B_OK;
}


status_t 
EBeGraphicsDrawable::StrokeRect(BGraphicsContext *dc,
				int32 x,  int32 y,  uint32 w,  uint32 h)
{
	if(w == 0 && h == 0)
		return StrokePoint(dc, x, y);
	else if(w == 0 || h == 0)
		return StrokeLine(dc, x, y, x + (int32)w, y + (int32)h);

	if(fEngine == NULL) return B_ERROR;

	BAutolock <EBeGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK || beBitmap == NULL) return B_ERROR;

	BAutolock <EBeBitmapPriv> bitmap_autolock(beBitmap);

	BView *bView = beBitmap->fView;
	if(bView == NULL || __bhapi_prepare_beview(bView, dc) == false) return B_ERROR;

	bhapi::pattern pat = dc->Pattern();

	bView->StrokeRect(BRect(x, y, x + w, y + h), *((pattern*)&pat));

	return B_OK;
}


status_t 
EBeGraphicsDrawable::FillRect(BGraphicsContext *dc,
			       int32 x,  int32 y,  uint32 w,  uint32 h)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EBeGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK || beBitmap == NULL) return B_ERROR;

	BAutolock <EBeBitmapPriv> bitmap_autolock(beBitmap);

	BView *bView = beBitmap->fView;
	if(bView == NULL || __bhapi_prepare_beview(bView, dc) == false) return B_ERROR;

	bhapi::pattern pat = dc->Pattern();

	bView->FillRect(BRect(x, y, x + w, y + h), *((pattern*)&pat));

	return B_OK;
}


status_t 
EBeGraphicsDrawable::StrokeRects(BGraphicsContext *dc,
				 const  int32 *rects,  int32 count)
{
	if(fEngine == NULL) return B_ERROR;
	if(rects == NULL || count <= 0) return B_ERROR;

	BAutolock <EBeGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK || beBitmap == NULL) return B_ERROR;

	BAutolock <EBeBitmapPriv> bitmap_autolock(beBitmap);

	BView *bView = beBitmap->fView;
	if(bView == NULL || __bhapi_prepare_beview(bView, dc) == false) return B_ERROR;

	bhapi::pattern pat = dc->Pattern();

	for(int32 i = 0; i < count; i++)
	{
		int32 x = *rects++;  int32 y = *rects++;  uint32 w = (uint32)(*rects++);  uint32 h = (uint32)(*rects++);

		if(w == 0 && h == 0)
		{
			if(dc->PenSize() <= 1)
			{
				bView->FillRect(BRect(x, y, x, y), *((pattern*)&pat));
			}
			else
			{
				BRect rect((float)x + 0.5, (float)y + 0.5,
					   (float)x + 0.5 + dc->PenSize(), (float)y + 0.5 + dc->PenSize());
				rect.OffsetBy(dc->PenSize() / -2, dc->PenSize() / -2);

				if(dc->IsSquarePointStyle())
					bView->FillRect(rect, *((pattern*)&pat));
				else
					bView->FillEllipse(rect, *((pattern*)&pat));
			}
		}
		else if(w == 0 || h == 0)
		{
			bView->StrokeLine(BPoint(x, y), BPoint(x + w, y + h), *((pattern*)&pat));
		}
		else
		{
			bView->StrokeRect(BRect(x, y, x + w, y + h), *((pattern*)&pat));
		}
	}

	return B_OK;
}


status_t 
EBeGraphicsDrawable::FillRects(BGraphicsContext *dc,
			       const  int32 *rects,  int32 count)
{
	if(fEngine == NULL) return B_ERROR;
	if(rects == NULL || count <= 0) return B_ERROR;

	BAutolock <EBeGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK || beBitmap == NULL) return B_ERROR;

	BAutolock <EBeBitmapPriv> bitmap_autolock(beBitmap);

	BView *bView = beBitmap->fView;
	if(bView == NULL || __bhapi_prepare_beview(bView, dc) == false) return B_ERROR;

	bhapi::pattern pat = dc->Pattern();

	for(int32 i = 0; i < count; i++)
	{
		int32 x = *rects++;  int32 y = *rects++;  uint32 w = (uint32)(*rects++);  uint32 h = (uint32)(*rects++);
		bView->FillRect(BRect(x, y, x + w, y + h), *((pattern*)&pat));
	}

	return B_OK;
}


status_t 
EBeGraphicsDrawable::FillRegion(BGraphicsContext *dc,
				const BRegion &region)
{
	if(fEngine == NULL || dc->Clipping() == NULL) return B_ERROR;
	if(region.CountRects() <= 0) return B_ERROR;

	BAutolock <EBeGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK || beBitmap == NULL) return B_ERROR;

	BAutolock <EBeBitmapPriv> bitmap_autolock(beBitmap);

	BView *bView = beBitmap->fView;
	if(bView == NULL || __bhapi_prepare_beview(bView, dc) == false) return B_ERROR;

	bhapi::pattern pat = dc->Pattern();

	BRegion aRegion(region);
	aRegion &= *(dc->Clipping());
	if(aRegion.CountRects() <= 0) return B_ERROR;

	BRegion beRegion;
	for(int32 i = 0; i < aRegion.CountRects(); i++)
	{
		BRect r = aRegion.RectAt(i).FloorCopy();

		clipping_rect bRect;
		bRect.left = (int32)r.left;
		bRect.top = (int32)r.top;
		bRect.right = (int32)r.right;
		bRect.bottom = (int32)r.bottom;

		beRegion.Include(bRect);
	}

	bView->ConstrainClippingRegion(&beRegion);
	bView->FillRect(beRegion.Frame(), *((pattern*)&pat));

	return B_OK;
}


status_t 
EBeGraphicsDrawable::StrokeRoundRect(BGraphicsContext *dc,
				      int32 x,  int32 y,  uint32 w,  uint32 h,  uint32 xRadius,  uint32 yRadius)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EBeGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK || beBitmap == NULL) return B_ERROR;

	BAutolock <EBeBitmapPriv> bitmap_autolock(beBitmap);

	BView *bView = beBitmap->fView;
	if(bView == NULL || __bhapi_prepare_beview(bView, dc) == false) return B_ERROR;

	bhapi::pattern pat = dc->Pattern();

	BRect rect(x, y, x + w, y + h);
	bView->StrokeRoundRect(rect, (float)xRadius, (float)yRadius, *((pattern*)&pat));

	return B_OK;
}


status_t 
EBeGraphicsDrawable::FillRoundRect(BGraphicsContext *dc,
				    int32 x,  int32 y,  uint32 w,  uint32 h,  uint32 xRadius,  uint32 yRadius)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EBeGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK || beBitmap == NULL) return B_ERROR;

	BAutolock <EBeBitmapPriv> bitmap_autolock(beBitmap);

	BView *bView = beBitmap->fView;
	if(bView == NULL || __bhapi_prepare_beview(bView, dc) == false) return B_ERROR;

	bhapi::pattern pat = dc->Pattern();

	BRect rect(x, y, x + w, y + h);
	bView->FillRoundRect(rect, (float)xRadius, (float)yRadius, *((pattern*)&pat));

	return B_OK;
}


status_t 
EBeGraphicsDrawable::StrokeArc(BGraphicsContext *dc,
			        int32 x,  int32 y,  uint32 w,  uint32 h, float startAngle, float endAngle)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EBeGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK || beBitmap == NULL) return B_ERROR;

	BAutolock <EBeBitmapPriv> bitmap_autolock(beBitmap);

	BView *bView = beBitmap->fView;
	if(bView == NULL || __bhapi_prepare_beview(bView, dc) == false) return B_ERROR;

	bhapi::pattern pat = dc->Pattern();

	bView->StrokeArc(BRect(x, y, x + w, y + h), startAngle, endAngle - startAngle, *((pattern*)&pat));

	return B_OK;
}


status_t 
EBeGraphicsDrawable::FillArc(BGraphicsContext *dc,
			      int32 x,  int32 y,  uint32 w,  uint32 h, float startAngle, float endAngle)
{
	if(fEngine == NULL) return B_ERROR;

	BAutolock <EBeGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK || beBitmap == NULL) return B_ERROR;

	BAutolock <EBeBitmapPriv> bitmap_autolock(beBitmap);

	BView *bView = beBitmap->fView;
	if(bView == NULL || __bhapi_prepare_beview(bView, dc) == false) return B_ERROR;

	bhapi::pattern pat = dc->Pattern();

	bView->FillArc(BRect(x, y, x + w, y + h), startAngle, endAngle - startAngle, *((pattern*)&pat));

	return B_OK;
}
#endif /* BEOS */
