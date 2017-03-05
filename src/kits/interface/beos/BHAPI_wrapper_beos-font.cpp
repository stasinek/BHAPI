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
 * File: BHAPI_wrapper_beos-font.cpp
 *
 * --------------------------------------------------------------------------*/
#ifdef BHAPI_OS_BEOS


#include <be/app/AppDefs.h>
#include <be/interface/Font.h>

#include "BHAPI_wrapper_beos-graphics.h"

#include "../../add-ons/font/FontEngine.h"
#include "../../support/ClassInfo.h"
#include "../../support/Autolock.h"
#include "../../interface/Window.h"
#include "../../interface/View.h"


class BFontBe : public BFontEngine {
public:
	BFontBe(EBeGraphicsEngine *beEngine, const font_family family, const font_style style);
	virtual ~BFontBe();

	virtual bool IsValid() const;
	virtual bool IsScalable() const;
	virtual void ForceFontAliasing(bool enable);

	virtual float StringWidth(const char *string, float size, float spacing, float shear, bool bold, b_int32 length) const;
	virtual void GetHeight(bhapi::font_height *height, float size, float shear, bool bold) const;
	virtual BRect RenderString(BHandler *view, const char *string, float size, float spacing, float shear, bool bold, b_int32 length);

	virtual bhapi::font_detach_callback* Attach(void (*callback)(void*), void *data);
	virtual bool Detach(bhapi::font_detach_callback *callback);

private:
	bool fScalable;
	bool fForceFontAliasing;
	BFont fBeFont;

	EBeGraphicsEngine *fBeEngine;
};


BFontBe::BFontBe(EBeGraphicsEngine *beEngine, const font_family family, const font_style style)
	: BFontEngine(), fScalable(false), fForceFontAliasing(false), fBeEngine(NULL)
{
	if(beEngine == NULL || family[0] == 0 || style[0] == 0) return;

	fBeEngine = beEngine;

	BAutolock <EBeGraphicsEngine> autolock(fBeEngine);
	if(autolock.IsLocked() == false || fBeEngine->InitCheck() != B_OK) {fBeEngine = NULL; return;}
	if(fBeFont.SetFamilyAndStyle(family, style) != B_OK) {fBeEngine = NULL; return;}

	fScalable = true;
/*
	if(fBeFont.FileFormat() == B_TRUETYPE_WINDOWS) fScalable = true;

	int32 nTuned = fBeFont.CountTuned();

	float *fontSizes = (nTuned > 0 ? new float[nTuned] : NULL);
	b_int32 nFontSizes = 0;

	for(int32 i = 0; i < nTuned && fontSizes != NULL; i++)
	{
		tuned_font_info fontInfo;
		fBeFont.GetTunedInfo(i, &fontInfo);

		fBeFont.SetSize(fontInfo.size);

		bool isFixed = fBeFont.IsFixed();
		for(b_int32 j = 0; j < nFontSizes; j++)
			if(fontSizes[j] == fontInfo.size) {isFixed = false; break;}
		if(isFixed == false) continue;

		fontSizes[nFontSizes++] = fontInfo.size;
	}

	if(fontSizes)
	{
		SetFixedSize(fontSizes, nFontSizes);
		delete[] fontSizes;
	}

	if(nFontSizes <= 0 && fScalable == false) {fBeEngine = NULL; return;}
*/

	SetFamily(family);
	SetStyle(style);
	SetRenderMode(B_FONT_RENDER_DIRECTLY);
}


BFontBe::~BFontBe()
{
}


bhapi::font_detach_callback*
BFontBe::Attach(void (*callback)(void*), void *data)
{
	if(fBeEngine == NULL) return NULL;
	BAutolock <EBeGraphicsEngine> autolock(fBeEngine);
	if(autolock.IsLocked() == false || fBeEngine->InitCheck() != B_OK) return NULL;
	return BFontEngine::Attach(callback, data);
}


bool
BFontBe::Detach(bhapi::font_detach_callback *callback)
{
	if(fBeEngine == NULL) return false;
	BAutolock <EBeGraphicsEngine> autolock(fBeEngine);
	if(autolock.IsLocked() == false) return false;
	if(!BFontEngine::Detach(callback)) return false;
	return true;
}


bool
BFontBe::IsValid() const
{
	return(fBeEngine != NULL && Family() != NULL && Style() != NULL);
}


bool
BFontBe::IsScalable() const
{
	return fScalable;
}


void
BFontBe::ForceFontAliasing(bool enable)
{
	if(fBeEngine == NULL) return;
	BAutolock <EBeGraphicsEngine> autolock(fBeEngine);
	if(autolock.IsLocked() == false || fBeEngine->InitCheck() != B_OK) return;
	fForceFontAliasing = enable;
}


float
BFontBe::StringWidth(const char *string, float size, float spacing, float shear, bool bold, b_int32 length) const
{
	if(fBeEngine == NULL) return 0;
	if((int)size <= 0 || string == NULL || *string == 0 || length == 0 || !IsAttached()) return 0;

	BAutolock <EBeGraphicsEngine> autolock(fBeEngine);
	if(autolock.IsLocked() == false || fBeEngine->InitCheck() != B_OK) return 0;

	BFont aFont(fBeFont);
	aFont.SetSize(size);
	aFont.SetShear(shear);
	aFont.SetFace(bold ? B_BOLD_FACE : B_REGULAR_FACE);

	if(aFont.Size() != size) return 0;

	font_height fontHeight;
	aFont.GetHeight(&fontHeight);

	float width = 0;
	if(length < 0 || (size_t)length > strlen(string)) length = (b_int32)strlen(string);
	float height = fontHeight.ascent + fontHeight.descent;
	float delta = (float)ceil((double)(spacing * size));

	b_uint8 bytes = 0;
	const char *str = bhapi::utf8_at(string, 0, &bytes);
	const char *tmp = str;
	while(!(tmp == NULL || bytes == 0 || (size_t)(tmp - string) > (size_t)length - (size_t)bytes))
	{
		BString aStr(tmp, (b_int32)bytes);
		float bWidth = aFont.StringWidth(aStr.String());
		width += (bWidth > 0 ? bWidth : height) + (tmp == str ? 0.f : delta);
		tmp = bhapi::utf8_next(tmp, &bytes);
	}

	return width;
}


void
BFontBe::GetHeight(bhapi::font_height *height, float size, float shear, bool bold) const
{
	if(fBeEngine == NULL || height == NULL) return;

	bzero(height, sizeof(bhapi::font_height));

	BAutolock <EBeGraphicsEngine> autolock(fBeEngine);
	if(autolock.IsLocked() == false || fBeEngine->InitCheck() != B_OK) return;

	BFont aFont(fBeFont);
	aFont.SetSize(size);
	aFont.SetShear(shear);
	aFont.SetFace(bold ? B_BOLD_FACE : B_REGULAR_FACE);

	if(aFont.Size() != size) return;

	font_height fontHeight;
	aFont.GetHeight(&fontHeight);

	height->ascent = fontHeight.ascent;
	height->descent = fontHeight.descent;
	height->leading = fontHeight.leading;

	return;
}


BRect
BFontBe::RenderString(BHandler *_view, const char *string, float size, float spacing, float shear, bool bold, b_int32 length)
{
	if(fBeEngine == NULL || (int)size <= 0 || string == NULL || *string == 0 || length == 0) return BRect();

	BView *view = cast_as(_view, BView);
	if(view == NULL || view->Window() == NULL || view->IsPrinting()) return BRect();

	BRegion viewClipping;
	view->GetClippingRegion(&viewClipping);
	if(viewClipping.CountRects() <= 0) return BRect();

	BAutolock <EBeGraphicsEngine> autolock(fBeEngine);
	if(autolock.IsLocked() == false || fBeEngine->InitCheck() != B_OK) return BRect();

	EBeGraphicsDrawable *pix = cast_as(BGraphicsEngine::GetPixmap(view->Window()), EBeGraphicsDrawable);
	BGraphicsContext *dc = BGraphicsEngine::GetContext(view);
	if(pix == NULL || pix->beBitmap == NULL || dc == NULL) return BRect();

	if(!IsAttached()) return BRect();

	fBeFont.SetSize(size);
	fBeFont.SetShear(shear);
	fBeFont.SetFace(bold ? B_BOLD_FACE : B_REGULAR_FACE);

	if(fBeFont.Size() != size) return BRect();

	BAutolock <EBeBitmapPriv> bitmap_autolock(pix->beBitmap);

	BView *bView = pix->beBitmap->fView;
	if(bView == NULL || __bhapi_prepare_beview(bView, dc) == false) return BRect();

	bView->SetFont(&fBeFont, B_FONT_ALL);
	bView->ForceFontAliasing(fForceFontAliasing);

	font_height fontHeight;
	fBeFont.GetHeight(&fontHeight);

	float width = 0;
	if(length < 0 || (size_t)length > strlen(string)) length = (b_int32)strlen(string);
	float height = fontHeight.ascent + fontHeight.descent;
	float delta = (float)ceil((double)(spacing * size));
	BPoint pt = view->ConvertToWindow(view->PenLocation());
	BPoint bPt(pt.x, pt.y);

	b_uint8 bytes = 0;
	const char *str = bhapi::utf8_at(string, 0, &bytes);
	const char *tmp = str;
	while(!(tmp == NULL || bytes == 0 || (size_t)(tmp - string) > (size_t)length - (size_t)bytes))
	{
		BString aStr(tmp, (b_int32)bytes);
		float bWidth = fBeFont.StringWidth(aStr.String());

		if(bWidth > 0) bView->DrawString(aStr.String(), bPt);

		bPt.x += (bWidth > 0 ? bWidth : height) + delta;
		width += (bWidth > 0 ? bWidth : height) + (tmp == str ? 0.f : delta);

		tmp = bhapi::utf8_next(tmp, &bytes);
	}

	BRect updateRect;
	updateRect.left = pt.x;
	updateRect.right = pt.x + width;
	updateRect.top = pt.y - (float)(fontHeight.ascent + 1);
	updateRect.bottom = updateRect.top + height;
	view->ConvertFromWindow(&updateRect);
	updateRect &= viewClipping.Frame();

	return updateRect;
}


b_status_t
EBeGraphicsEngine::InitalizeFonts()
{
	BAutolock <EBeGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false) return B_ERROR;

	return InitCheck();
}


void
EBeGraphicsEngine::DestroyFonts()
{
}


b_status_t
EBeGraphicsEngine::UpdateFonts(bool check_only)
{
	BAutolock <EBeGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false || InitCheck() != B_OK) return B_ERROR;

	if(check_only)
	{
		BHAPI_WARNING("[GRAPHICS]: %s --- check_only not implement yet.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	b_status_t retVal = B_ERROR;

	BHAPI_DEBUG("[GRAPHICS]: Updating BeOS fonts ...");

	for(int32 i = 0; i < count_font_families(); i++)
	{
		font_family family;
		if(get_font_family(i, &family) != B_OK) continue;

		int32 nStyles = count_font_styles(family);
		for(int32 j = 0; j < nStyles; j++)
		{
			font_style style;
			if(get_font_style(family, j, &style) != B_OK) continue;

			BFontBe *engine = new BFontBe(this, family, style);
			if(!engine || !engine->IsValid())
			{
				if(engine) delete engine;
				continue;
			}

			if(bhapi::font_add(engine->Family(), engine->Style(), engine))
				retVal = B_OK;
			else
				delete engine;
		}
	}

	BHAPI_DEBUG("[GRAPHICS]: BeOS fonts updated.");

	return retVal;
}
#endif /* BEOS */

