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
 * File: etk-beos-font.cpp
 *
 * --------------------------------------------------------------------------*/
#ifdef BEOS

#include <be/app/AppDefs.h>
#include <be/interface/Font.h>

#include "etk-beos-graphics.h"

#include "./../../add-ons/font/FontEngine.h"
#include "./../../support/ClassInfo.h"
#include "./../../support/Autolock.h"
#include "./../../interface/Window.h"
#include "./../../interface/View.h"


class EFontBe : public EFontEngine {
public:
	EFontBe(EBeGraphicsEngine *beEngine, const font_family family, const font_style style);
	virtual ~EFontBe();

	virtual bool IsValid() const;
	virtual bool IsScalable() const;
	virtual void ForceFontAliasing(bool enable);

	virtual float StringWidth(const char *string, float size, float spacing, float shear, bool bold, eint32 length) const;
	virtual void GetHeight(e_font_height *height, float size, float shear, bool bold) const;
	virtual ERect RenderString(EHandler *view, const char *string, float size, float spacing, float shear, bool bold, eint32 length);

	virtual e_font_detach_callback* Attach(void (*callback)(void*), void *data);
	virtual bool Detach(e_font_detach_callback *callback);

private:
	bool fScalable;
	bool fForceFontAliasing;
	BFont fBeFont;

	EBeGraphicsEngine *fBeEngine;
};


EFontBe::EFontBe(EBeGraphicsEngine *beEngine, const font_family family, const font_style style)
	: EFontEngine(), fScalable(false), fForceFontAliasing(false), fBeEngine(NULL)
{
	if(beEngine == NULL || family[0] == 0 || style[0] == 0) return;

	fBeEngine = beEngine;

	EAutolock <EBeGraphicsEngine> autolock(fBeEngine);
	if(autolock.IsLocked() == false || fBeEngine->InitCheck() != E_OK) {fBeEngine = NULL; return;}
	if(fBeFont.SetFamilyAndStyle(family, style) != B_OK) {fBeEngine = NULL; return;}

	fScalable = true;
/*
	if(fBeFont.FileFormat() == B_TRUETYPE_WINDOWS) fScalable = true;

	int32 nTuned = fBeFont.CountTuned();

	float *fontSizes = (nTuned > 0 ? new float[nTuned] : NULL);
	eint32 nFontSizes = 0;

	for(int32 i = 0; i < nTuned && fontSizes != NULL; i++)
	{
		tuned_font_info fontInfo;
		fBeFont.GetTunedInfo(i, &fontInfo);

		fBeFont.SetSize(fontInfo.size);

		bool isFixed = fBeFont.IsFixed();
		for(eint32 j = 0; j < nFontSizes; j++)
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
	SetRenderMode(E_FONT_RENDER_DIRECTLY);
}


EFontBe::~EFontBe()
{
}


e_font_detach_callback*
EFontBe::Attach(void (*callback)(void*), void *data)
{
	if(fBeEngine == NULL) return NULL;
	EAutolock <EBeGraphicsEngine> autolock(fBeEngine);
	if(autolock.IsLocked() == false || fBeEngine->InitCheck() != E_OK) return NULL;
	return EFontEngine::Attach(callback, data);
}


bool
EFontBe::Detach(e_font_detach_callback *callback)
{
	if(fBeEngine == NULL) return false;
	EAutolock <EBeGraphicsEngine> autolock(fBeEngine);
	if(autolock.IsLocked() == false) return false;
	if(!EFontEngine::Detach(callback)) return false;
	return true;
}


bool
EFontBe::IsValid() const
{
	return(fBeEngine != NULL && Family() != NULL && Style() != NULL);
}


bool
EFontBe::IsScalable() const
{
	return fScalable;
}


void
EFontBe::ForceFontAliasing(bool enable)
{
	if(fBeEngine == NULL) return;
	EAutolock <EBeGraphicsEngine> autolock(fBeEngine);
	if(autolock.IsLocked() == false || fBeEngine->InitCheck() != E_OK) return;
	fForceFontAliasing = enable;
}


float
EFontBe::StringWidth(const char *string, float size, float spacing, float shear, bool bold, eint32 length) const
{
	if(fBeEngine == NULL) return 0;
	if((int)size <= 0 || string == NULL || *string == 0 || length == 0 || !IsAttached()) return 0;

	EAutolock <EBeGraphicsEngine> autolock(fBeEngine);
	if(autolock.IsLocked() == false || fBeEngine->InitCheck() != E_OK) return 0;

	BFont aFont(fBeFont);
	aFont.SetSize(size);
	aFont.SetShear(shear);
	aFont.SetFace(bold ? B_BOLD_FACE : B_REGULAR_FACE);

	if(aFont.Size() != size) return 0;

	font_height fontHeight;
	aFont.GetHeight(&fontHeight);

	float width = 0;
	if(length < 0 || (size_t)length > strlen(string)) length = (eint32)strlen(string);
	float height = fontHeight.ascent + fontHeight.descent;
	float delta = (float)ceil((double)(spacing * size));

	euint8 bytes = 0;
	const char *str = e_utf8_at(string, 0, &bytes);
	const char *tmp = str;
	while(!(tmp == NULL || bytes == 0 || (size_t)(tmp - string) > (size_t)length - (size_t)bytes))
	{
		EString aStr(tmp, (eint32)bytes);
		float bWidth = aFont.StringWidth(aStr.String());
		width += (bWidth > 0 ? bWidth : height) + (tmp == str ? 0.f : delta);
		tmp = e_utf8_next(tmp, &bytes);
	}

	return width;
}


void
EFontBe::GetHeight(e_font_height *height, float size, float shear, bool bold) const
{
	if(fBeEngine == NULL || height == NULL) return;

	bzero(height, sizeof(e_font_height));

	EAutolock <EBeGraphicsEngine> autolock(fBeEngine);
	if(autolock.IsLocked() == false || fBeEngine->InitCheck() != E_OK) return;

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


ERect
EFontBe::RenderString(EHandler *_view, const char *string, float size, float spacing, float shear, bool bold, eint32 length)
{
	if(fBeEngine == NULL || (int)size <= 0 || string == NULL || *string == 0 || length == 0) return ERect();

	EView *view = e_cast_as(_view, EView);
	if(view == NULL || view->Window() == NULL || view->IsPrinting()) return ERect();

	ERegion viewClipping;
	view->GetClippingRegion(&viewClipping);
	if(viewClipping.CountRects() <= 0) return ERect();

	EAutolock <EBeGraphicsEngine> autolock(fBeEngine);
	if(autolock.IsLocked() == false || fBeEngine->InitCheck() != E_OK) return ERect();

	EBeGraphicsDrawable *pix = e_cast_as(EGraphicsEngine::GetPixmap(view->Window()), EBeGraphicsDrawable);
	EGraphicsContext *dc = EGraphicsEngine::GetContext(view);
	if(pix == NULL || pix->beBitmap == NULL || dc == NULL) return ERect();

	if(!IsAttached()) return ERect();

	fBeFont.SetSize(size);
	fBeFont.SetShear(shear);
	fBeFont.SetFace(bold ? B_BOLD_FACE : B_REGULAR_FACE);

	if(fBeFont.Size() != size) return ERect();

	EAutolock <EBeBitmapPriv> bitmap_autolock(pix->beBitmap);

	BView *bView = pix->beBitmap->fView;
	if(bView == NULL || __etk_prepare_beview(bView, dc) == false) return ERect();

	bView->SetFont(&fBeFont, B_FONT_ALL);
	bView->ForceFontAliasing(fForceFontAliasing);

	font_height fontHeight;
	fBeFont.GetHeight(&fontHeight);

	float width = 0;
	if(length < 0 || (size_t)length > strlen(string)) length = (eint32)strlen(string);
	float height = fontHeight.ascent + fontHeight.descent;
	float delta = (float)ceil((double)(spacing * size));
	EPoint pt = view->ConvertToWindow(view->PenLocation());
	BPoint bPt(pt.x, pt.y);

	euint8 bytes = 0;
	const char *str = e_utf8_at(string, 0, &bytes);
	const char *tmp = str;
	while(!(tmp == NULL || bytes == 0 || (size_t)(tmp - string) > (size_t)length - (size_t)bytes))
	{
		EString aStr(tmp, (eint32)bytes);
		float bWidth = fBeFont.StringWidth(aStr.String());

		if(bWidth > 0) bView->DrawString(aStr.String(), bPt);

		bPt.x += (bWidth > 0 ? bWidth : height) + delta;
		width += (bWidth > 0 ? bWidth : height) + (tmp == str ? 0.f : delta);

		tmp = e_utf8_next(tmp, &bytes);
	}

	ERect updateRect;
	updateRect.left = pt.x;
	updateRect.right = pt.x + width;
	updateRect.top = pt.y - (float)(fontHeight.ascent + 1);
	updateRect.bottom = updateRect.top + height;
	view->ConvertFromWindow(&updateRect);
	updateRect &= viewClipping.Frame();

	return updateRect;
}


e_status_t
EBeGraphicsEngine::InitalizeFonts()
{
	EAutolock <EBeGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false) return E_ERROR;

	return InitCheck();
}


void
EBeGraphicsEngine::DestroyFonts()
{
}


e_status_t
EBeGraphicsEngine::UpdateFonts(bool check_only)
{
	EAutolock <EBeGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false || InitCheck() != E_OK) return E_ERROR;

	if(check_only)
	{
		ETK_WARNING("[GRAPHICS]: %s --- check_only not implement yet.", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	e_status_t retVal = E_ERROR;

	ETK_DEBUG("[GRAPHICS]: Updating BeOS fonts ...");

	for(int32 i = 0; i < count_font_families(); i++)
	{
		font_family family;
		if(get_font_family(i, &family) != B_OK) continue;

		int32 nStyles = count_font_styles(family);
		for(int32 j = 0; j < nStyles; j++)
		{
			font_style style;
			if(get_font_style(family, j, &style) != B_OK) continue;

			EFontBe *engine = new EFontBe(this, family, style);
			if(!engine || !engine->IsValid())
			{
				if(engine) delete engine;
				continue;
			}

			if(etk_font_add(engine->Family(), engine->Style(), engine))
				retVal = E_OK;
			else
				delete engine;
		}
	}

	ETK_DEBUG("[GRAPHICS]: BeOS fonts updated.");

	return retVal;
}
#endif /* BEOS */

