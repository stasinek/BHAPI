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
 * File: BHAPI_wrapper_dfb-font.cpp
 *
 * --------------------------------------------------------------------------*/
#ifdef LINUX
#ifdef DIRECTFB

#include "BHAPI_wrapper_dfb.h"

#include "../../add-ons/font/FontEngine.h"
#include "../../support/ClassInfo.h"
#include "../../support/Autolock.h"
#include "../../interface/Window.h"
#include "../../interface/View.h"
#include "../../storage/Directory.h"
#include "../../storage/Path.h"

#define BHAPI_DIRECTFONT_DEFAULT_SIZE	12


class EDFBFont : public BFontEngine {
public:
	EDFBFont(EDFBGraphicsEngine *dfbEngine, const char *filename);
	virtual ~EDFBFont();

	virtual bool IsValid() const;
	virtual bool IsScalable() const;
	virtual void ForceFontAliasing(bool enable);

	virtual float StringWidth(const char *string, float size, float spacing, float shear, bool bold,  int32 length) const;
	virtual void GetHeight(bhapi::font_height *height, float size, float shear, bool bold) const;
	virtual BRect RenderString(BHandler *view, const char *string, float size, float spacing, float shear, bool bold,  int32 length);

	virtual bhapi::font_detach_callback* Attach(void (*callback)(void*), void *data);
	virtual bool Detach(bhapi::font_detach_callback *callback);

private:
	bool fScalable;
	bool fForceFontAliasing;
	IDirectFBFont *fDFBFont;
	char *fFilename;

	EDFBGraphicsEngine *fEngine;
};


EDFBFont::EDFBFont(EDFBGraphicsEngine *dfbEngine, const char *filename)
	: BFontEngine(), fScalable(false), fForceFontAliasing(false), fFilename(NULL), fEngine(NULL)
{
	if(dfbEngine == NULL) return;

	fEngine = dfbEngine;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) {fEngine = NULL; return;}

	DFBFontDescription fontdesc;
	fontdesc.flags = (DFBFontDescriptionFlags)(DFDESC_ATTRIBUTES | DFDESC_HEIGHT);
	fontdesc.attributes = DFFA_NONE;
	fontdesc.height = BHAPI_DIRECTFONT_DEFAULT_SIZE;

	if(fEngine->dfbDisplay->CreateFont(fEngine->dfbDisplay, filename, &fontdesc, &fDFBFont) != DFB_OK)
	{
		BHAPI_DEBUG("[FONT]: CreateFont(%s) failed.", filename);
		fEngine = NULL;
		return;
	}

	BPath aPath(filename);
	if(aPath.Path() != NULL) fFilename = bhapi::strdup(aPath.Path());
	SetFamily(aPath.Leaf() ? aPath.Leaf() : "DFB-Default");
	SetStyle("Regular");

	int height = 0;
	fDFBFont->GetHeight(fDFBFont, &height);
	float sizes = (float)height;
	SetFixedSize(&sizes, 1);

	SetRenderMode(B_FONT_RENDER_DIRECTLY);
}


EDFBFont::~EDFBFont()
{
	if(fEngine != NULL)
	{
		BAutolock <EDFBGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK)
			BHAPI_ERROR("[FONT]: %s --- Invalid graphics engine.", __PRETTY_FUNCTION__);

		fDFBFont->Release(fDFBFont);
	}

	if(fFilename) delete[] fFilename;
}


bhapi::font_detach_callback*
EDFBFont::Attach(void (*callback)(void*), void *data)
{
	if(fEngine == NULL) return NULL;
	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return NULL;
	return BFontEngine::Attach(callback, data);
}


bool 
EDFBFont::Detach(bhapi::font_detach_callback *callback)
{
	if(fEngine == NULL) return false;
	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false) return false;
	if(!BFontEngine::Detach(callback)) return false;
	return true;
}


bool 
EDFBFont::IsValid() const
{
	return(fEngine != NULL && fDFBFont != NULL && Family() != NULL && Style() != NULL);
}


bool 
EDFBFont::IsScalable() const
{
	return fScalable;
}


void 
EDFBFont::ForceFontAliasing(bool enable)
{
	if(fEngine == NULL) return;
	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return;

	if(fForceFontAliasing != enable)
	{
		fForceFontAliasing = enable;

		DFBFontDescription fontdesc;
		fontdesc.flags = (DFBFontDescriptionFlags)(DFDESC_ATTRIBUTES | DFDESC_HEIGHT);
		fontdesc.attributes = (fForceFontAliasing ? DFFA_MONOCHROME : DFFA_NONE);
		fontdesc.height = BHAPI_DIRECTFONT_DEFAULT_SIZE;

		IDirectFBFont *newFont = NULL;
		fEngine->dfbDisplay->CreateFont(fEngine->dfbDisplay, fFilename, &fontdesc, &newFont);
		if(newFont)
		{
			fDFBFont->Release(fDFBFont);
			fDFBFont = newFont;

			int height = 0;
			fDFBFont->GetHeight(fDFBFont, &height);
			float sizes = (float)height;
			SetFixedSize(&sizes, 1);
		}
	}
}


float
EDFBFont::StringWidth(const char *string, float size, float spacing, float shear, bool bold,  int32 length) const
{
	if(fEngine == NULL) return 0;
	if((int)size <= 0 || string == NULL || *string == 0 || length == 0 || !IsAttached()) return 0;

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return 0;

	int height = 0;
	fDFBFont->GetHeight(fDFBFont, &height);

	if(size != (float)height) return 0;

	float width = 0;
	if(length < 0 || (size_t)length > strlen(string)) length = (int32)strlen(string);
	float delta = (float)ceil((double)(spacing * size));

	uint8 bytes = 0;
	const char *str = bhapi::utf8_at(string, 0, &bytes);
	const char *tmp = str;
	while(!(tmp == NULL || bytes == 0 || (size_t)(tmp - string) > (size_t)length - (size_t)bytes))
	{
		BString aStr(tmp, (int32)bytes);

		int bWidth = -1;
		fDFBFont->GetStringWidth(fDFBFont, aStr.String(), aStr.Length(), &bWidth);

		width += (float)(bWidth > 0 ? bWidth : height) + (tmp == str ? 0.f : delta);
		tmp = bhapi::utf8_next(tmp, &bytes);
	}

	return width;
}


void 
EDFBFont::GetHeight(bhapi::font_height *height, float size, float shear, bool bold) const
{
	if(fEngine == NULL || height == NULL) return;

	bzero(height, sizeof(bhapi::font_height));

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return;

	int dfbHeight = 0;
	fDFBFont->GetHeight(fDFBFont, &dfbHeight);

	if(size != (float)dfbHeight) return;

	int ascent = 0, descent = 0;
	fDFBFont->GetAscender(fDFBFont, &ascent);
	fDFBFont->GetDescender(fDFBFont, &descent);

	height->ascent = (float)(dfbHeight - ascent + descent) / 2.f + (float)ascent;
	height->descent = (float)dfbHeight - height->ascent;
	height->leading = 0;

	return;
}


BRect
EDFBFont::RenderString(BHandler *_view, const char *string, float size, float spacing, float shear, bool bold,  int32 length)
{
	if(fEngine == NULL || (int)size <= 0 || string == NULL || *string == 0 || length == 0) return BRect();

	BView *view = cast_as(_view, BView);
	if(view == NULL || view->Window() == NULL || view->IsPrinting()) return BRect();

	BRegion viewClipping;
	view->GetClippingRegion(&viewClipping);
	if(viewClipping.CountRects() <= 0) return BRect();

	BAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != B_OK) return BRect();

	EDFBGraphicsDrawable *pix = cast_as(BGraphicsEngine::GetPixmap(view->Window()), EDFBGraphicsDrawable);
	BGraphicsContext *dc = BGraphicsEngine::GetContext(view);
	if(pix == NULL || pix->dfbSurface == NULL || dc == NULL) return BRect();

	if(!IsAttached()) return BRect();

	int height = 0, ascent = 0, descent = 0;
	fDFBFont->GetHeight(fDFBFont, &height);
	fDFBFont->GetAscender(fDFBFont, &ascent);
	fDFBFont->GetDescender(fDFBFont, &descent);

	if(size != (float)height) return BRect();

	float width = 0;
	if(length < 0 || (size_t)length > strlen(string)) length = (int32)strlen(string);
	float delta = (float)ceil((double)(spacing * size));

	BPoint pt = view->ConvertToWindow(view->PenLocation());
	pt.y -= (float)(ascent + 1);

	bhapi::rgb_color c = dc->HighColor();
	pix->dfbSurface->SetColor(pix->dfbSurface, c.red, c.green, c.blue, 255);
	pix->dfbSurface->SetDrawingFlags(pix->dfbSurface, DSDRAW_NOFX);
	pix->dfbSurface->SetFont(pix->dfbSurface, fDFBFont);

	uint8 bytes = 0;
	const char *str = bhapi::utf8_at(string, 0, &bytes);
	const char *tmp = str;
	while(!(tmp == NULL || bytes == 0 || (size_t)(tmp - string) > (size_t)length - (size_t)bytes))
	{
		BString aStr(tmp, (int32)bytes);

		int bWidth = -1;
		fDFBFont->GetStringWidth(fDFBFont, aStr.String(), aStr.Length(), &bWidth);

		for(int32 i = 0; i < dc->Clipping()->CountRects(); i++)
		{
			BRect rect = dc->Clipping()->RectAt(i).FloorCopy();

			DFBRegion clipping;
			clipping.x1 = (int)rect.left;
			clipping.y1 = (int)rect.top;
			clipping.x2 = (int)rect.right;
			clipping.y2 = (int)rect.bottom;

			pix->dfbSurface->SetClip(pix->dfbSurface, &clipping);

			if(bWidth > 0)
				pix->dfbSurface->DrawString(pix->dfbSurface, aStr.String(), aStr.Length(),
							    (int)pt.x, (int)pt.y, (DFBSurfaceTextFlags)(DSTF_LEFT | DSTF_TOP));
//			else
//				pix->dfbSurface->FillRectangle(pix->dfbSurface, (int)pt.x + 3, (int)pt.y + 3, height - 6, height - 6);
		}

		pt.x += (float)(bWidth > 0 ? bWidth : height) + delta;
		width += (float)(bWidth > 0 ? bWidth : height) + (tmp == str ? 0.f : delta);

		tmp = bhapi::utf8_next(tmp, &bytes);
	}

	pix->dfbSurface->SetFont(pix->dfbSurface, NULL);

	BRect updateRect;
	updateRect.left = pt.x;
	updateRect.right = pt.x + width;
	updateRect.top = pt.y;
	updateRect.bottom = updateRect.top + (float)height;
	view->ConvertFromWindow(&updateRect);
	updateRect &= viewClipping.Frame();

	return updateRect;
}


status_t 
EDFBGraphicsEngine::InitalizeFonts()
{
	BAutolock <EDFBGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false) return B_ERROR;

	return InitCheck();
}


void 
EDFBGraphicsEngine::DestroyFonts()
{
}


status_t 
EDFBGraphicsEngine::UpdateFonts(bool check_only)
{
	BString fonts_dirs;

	const char *dirs = getenv("DIRECTFB_FONTS_DIR");
	if(dirs) fonts_dirs += dirs;

	if(fonts_dirs.Length() <= 0)
		BHAPI_WARNING("[FONT]: you can set the environment \"DIRECTFB_FONTS_DIR\" to match the correct dirs.");

	BAutolock <EDFBGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false || InitCheck() != B_OK) return B_ERROR;

	if(check_only)
	{
		BHAPI_WARNING("[GRAPHICS]: %s --- check_only not implement yet.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	BHAPI_DEBUG("[GRAPHICS]: Updating DirectFB fonts ...");

	BStringArray *fonts_dirs_array = fonts_dirs.Split(":");
	BHAPI_DEBUG("[FONT]: Fonts directory number: %d", (fonts_dirs_array ? fonts_dirs_array->CountItems() : 0));

	int32 count = 0;
	const BString *_fonts_dir;
	for(int32 m = 0; (_fonts_dir = (fonts_dirs_array ? fonts_dirs_array->ItemAt(m) : NULL)) != NULL; m++)
	{
		BDirectory directory(_fonts_dir->String());
		if(directory.InitCheck() != B_OK)
		{
			BHAPI_WARNING("[FONT]: CAN NOT open fonts directory - \"%s\"!", _fonts_dir->String());
			continue;
		}
		BHAPI_DEBUG("[FONT]: Opening font directory \"%s\"...", _fonts_dir->String());

		BEntry aEntry;
		while(directory.GetNextEntry(&aEntry, true) == B_OK)
		{
			BPath aPath;
			if(aEntry.GetPath(&aPath) != B_OK) continue;
			BString filename = aPath.Leaf();

			if(filename.Length() <= 0 || filename == "." || filename == "..") continue;

			BHAPI_DEBUG("[FONT]: Reading font file \"%s\" ...", aPath.Path());

			EDFBFont *engine = new EDFBFont(this, aPath.Path());
			if(engine == NULL || engine->IsValid() == false ||
			   bhapi::font_add(engine->Family(), engine->Style(), engine) == false)
			{
				if(engine) delete engine;
				continue;
			}

			count++;
		}
	}
	if(fonts_dirs_array) delete fonts_dirs_array;

	if(count == 0)
	{
		EDFBFont *engine = new EDFBFont(this, NULL);
		if(engine == NULL || engine->IsValid() == false ||
		   bhapi::font_add(engine->Family(), engine->Style(), engine) == false)
		{
			if(engine) delete engine;
		}
	}

	BHAPI_DEBUG("[GRAPHICS]: DirectFB fonts updated.");

	return B_OK;
}

#endif /* DIRECTFB */
#endif /* LINUX */
