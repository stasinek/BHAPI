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
 * File: etk-dfb-font.cpp
 * 
 * --------------------------------------------------------------------------*/

#include "etk-dfb.h"

#include "./../add-ons/font/FontEngine.h>
#include "./../support/ClassInfo.h>
#include "./../support/Autolock.h>
#include "./../interface/Window.h>
#include "./../interface/View.h>
#include "./../storage/Directory.h>
#include "./../storage/Path.h>

#define ETK_DIRECTFONT_DEFAULT_SIZE	12


class EDFBFont : public EFontEngine {
public:
	EDFBFont(EDFBGraphicsEngine *dfbEngine, const char *filename);
	virtual ~EDFBFont();

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
	IDirectFBFont *fDFBFont;
	char *fFilename;

	EDFBGraphicsEngine *fEngine;
};


EDFBFont::EDFBFont(EDFBGraphicsEngine *dfbEngine, const char *filename)
	: EFontEngine(), fScalable(false), fForceFontAliasing(false), fFilename(NULL), fEngine(NULL)
{
	if(dfbEngine == NULL) return;

	fEngine = dfbEngine;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) {fEngine = NULL; return;}

	DFBFontDescription fontdesc;
	fontdesc.flags = (DFBFontDescriptionFlags)(DFDESC_ATTRIBUTES | DFDESC_HEIGHT);
	fontdesc.attributes = DFFA_NONE;
	fontdesc.height = ETK_DIRECTFONT_DEFAULT_SIZE;

	if(fEngine->dfbDisplay->CreateFont(fEngine->dfbDisplay, filename, &fontdesc, &fDFBFont) != DFB_OK)
	{
		ETK_DEBUG("[FONT]: CreateFont(%s) failed.", filename);
		fEngine = NULL;
		return;
	}

	EPath aPath(filename);
	if(aPath.Path() != NULL) fFilename = EStrdup(aPath.Path());
	SetFamily(aPath.Leaf() ? aPath.Leaf() : "DFB-Default");
	SetStyle("Regular");

	int height = 0;
	fDFBFont->GetHeight(fDFBFont, &height);
	float sizes = (float)height;
	SetFixedSize(&sizes, 1);

	SetRenderMode(E_FONT_RENDER_DIRECTLY);
}


EDFBFont::~EDFBFont()
{
	if(fEngine != NULL)
	{
		EAutolock <EDFBGraphicsEngine> autolock(fEngine);
		if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK)
			ETK_ERROR("[FONT]: %s --- Invalid graphics engine.", __PRETTY_FUNCTION__);

		fDFBFont->Release(fDFBFont);
	}

	if(fFilename) delete[] fFilename;
}


e_font_detach_callback*
EDFBFont::Attach(void (*callback)(void*), void *data)
{
	if(fEngine == NULL) return NULL;
	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return NULL;
	return EFontEngine::Attach(callback, data);
}


bool
EDFBFont::Detach(e_font_detach_callback *callback)
{
	if(fEngine == NULL) return false;
	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false) return false;
	if(!EFontEngine::Detach(callback)) return false;
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
	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return;

	if(fForceFontAliasing != enable)
	{
		fForceFontAliasing = enable;

		DFBFontDescription fontdesc;
		fontdesc.flags = (DFBFontDescriptionFlags)(DFDESC_ATTRIBUTES | DFDESC_HEIGHT);
		fontdesc.attributes = (fForceFontAliasing ? DFFA_MONOCHROME : DFFA_NONE);
		fontdesc.height = ETK_DIRECTFONT_DEFAULT_SIZE;

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
EDFBFont::StringWidth(const char *string, float size, float spacing, float shear, bool bold, eint32 length) const
{
	if(fEngine == NULL) return 0;
	if((int)size <= 0 || string == NULL || *string == 0 || length == 0 || !IsAttached()) return 0;

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return 0;

	int height = 0;
	fDFBFont->GetHeight(fDFBFont, &height);

	if(size != (float)height) return 0;

	float width = 0;
	if(length < 0 || (size_t)length > strlen(string)) length = (eint32)strlen(string);
	float delta = (float)ceil((double)(spacing * size));

	euint8 bytes = 0;
	const char *str = e_utf8_at(string, 0, &bytes);
	const char *tmp = str;
	while(!(tmp == NULL || bytes == 0 || (size_t)(tmp - string) > (size_t)length - (size_t)bytes))
	{
		EString aStr(tmp, (eint32)bytes);

		int bWidth = -1;
		fDFBFont->GetStringWidth(fDFBFont, aStr.String(), aStr.Length(), &bWidth);

		width += (float)(bWidth > 0 ? bWidth : height) + (tmp == str ? 0.f : delta);
		tmp = e_utf8_next(tmp, &bytes);
	}

	return width;
}


void
EDFBFont::GetHeight(e_font_height *height, float size, float shear, bool bold) const
{
	if(fEngine == NULL || height == NULL) return;

	bzero(height, sizeof(e_font_height));

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return;

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


ERect
EDFBFont::RenderString(EHandler *_view, const char *string, float size, float spacing, float shear, bool bold, eint32 length)
{
	if(fEngine == NULL || (int)size <= 0 || string == NULL || *string == 0 || length == 0) return ERect();

	EView *view = e_cast_as(_view, EView);
	if(view == NULL || view->Window() == NULL || view->IsPrinting()) return ERect();

	ERegion viewClipping;
	view->GetClippingRegion(&viewClipping);
	if(viewClipping.CountRects() <= 0) return ERect();

	EAutolock <EDFBGraphicsEngine> autolock(fEngine);
	if(autolock.IsLocked() == false || fEngine->InitCheck() != E_OK) return ERect();

	EDFBGraphicsDrawable *pix = e_cast_as(EGraphicsEngine::GetPixmap(view->Window()), EDFBGraphicsDrawable);
	EGraphicsContext *dc = EGraphicsEngine::GetContext(view);
	if(pix == NULL || pix->dfbSurface == NULL || dc == NULL) return ERect();

	if(!IsAttached()) return ERect();

	int height = 0, ascent = 0, descent = 0;
	fDFBFont->GetHeight(fDFBFont, &height);
	fDFBFont->GetAscender(fDFBFont, &ascent);
	fDFBFont->GetDescender(fDFBFont, &descent);

	if(size != (float)height) return ERect();

	float width = 0;
	if(length < 0 || (size_t)length > strlen(string)) length = (eint32)strlen(string);
	float delta = (float)ceil((double)(spacing * size));

	EPoint pt = view->ConvertToWindow(view->PenLocation());
	pt.y -= (float)(ascent + 1);

	e_rgb_color c = dc->HighColor();
	pix->dfbSurface->SetColor(pix->dfbSurface, c.red, c.green, c.blue, 255);
	pix->dfbSurface->SetDrawingFlags(pix->dfbSurface, DSDRAW_NOFX);
	pix->dfbSurface->SetFont(pix->dfbSurface, fDFBFont);

	euint8 bytes = 0;
	const char *str = e_utf8_at(string, 0, &bytes);
	const char *tmp = str;
	while(!(tmp == NULL || bytes == 0 || (size_t)(tmp - string) > (size_t)length - (size_t)bytes))
	{
		EString aStr(tmp, (eint32)bytes);

		int bWidth = -1;
		fDFBFont->GetStringWidth(fDFBFont, aStr.String(), aStr.Length(), &bWidth);

		for(eint32 i = 0; i < dc->Clipping()->CountRects(); i++)
		{
			ERect rect = dc->Clipping()->RectAt(i).FloorCopy();

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

		tmp = e_utf8_next(tmp, &bytes);
	}

	pix->dfbSurface->SetFont(pix->dfbSurface, NULL);

	ERect updateRect;
	updateRect.left = pt.x;
	updateRect.right = pt.x + width;
	updateRect.top = pt.y;
	updateRect.bottom = updateRect.top + (float)height;
	view->ConvertFromWindow(&updateRect);
	updateRect &= viewClipping.Frame();

	return updateRect;
}


e_status_t
EDFBGraphicsEngine::InitalizeFonts()
{
	EAutolock <EDFBGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false) return E_ERROR;

	return InitCheck();
}


void
EDFBGraphicsEngine::DestroyFonts()
{
}


e_status_t
EDFBGraphicsEngine::UpdateFonts(bool check_only)
{
	EString fonts_dirs;

	const char *dirs = getenv("DIRECTFB_FONTS_DIR");
	if(dirs) fonts_dirs += dirs;

	if(fonts_dirs.Length() <= 0)
		ETK_WARNING("[FONT]: you can set the environment \"DIRECTFB_FONTS_DIR\" to match the correct dirs.");

	EAutolock <EDFBGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false || InitCheck() != E_OK) return E_ERROR;

	if(check_only)
	{
		ETK_WARNING("[GRAPHICS]: %s --- check_only not implement yet.", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	ETK_DEBUG("[GRAPHICS]: Updating DirectFB fonts ...");

	EStringArray *fonts_dirs_array = fonts_dirs.Split(":");
	ETK_DEBUG("[FONT]: Fonts directory number: %d", (fonts_dirs_array ? fonts_dirs_array->CountItems() : 0));

	eint32 count = 0;
	const EString *_fonts_dir;
	for(eint32 m = 0; (_fonts_dir = (fonts_dirs_array ? fonts_dirs_array->ItemAt(m) : NULL)) != NULL; m++)
	{
		EDirectory directory(_fonts_dir->String());
		if(directory.InitCheck() != E_OK)
		{
			ETK_WARNING("[FONT]: CAN NOT open fonts directory - \"%s\"!", _fonts_dir->String());
			continue;
		}
		ETK_DEBUG("[FONT]: Opening font directory \"%s\"...", _fonts_dir->String());

		EEntry aEntry;
		while(directory.GetNextEntry(&aEntry, true) == E_OK)
		{
			EPath aPath;
			if(aEntry.GetPath(&aPath) != E_OK) continue;
			EString filename = aPath.Leaf();

			if(filename.Length() <= 0 || filename == "." || filename == "..") continue;

			ETK_DEBUG("[FONT]: Reading font file \"%s\" ...", aPath.Path());

			EDFBFont *engine = new EDFBFont(this, aPath.Path());
			if(engine == NULL || engine->IsValid() == false ||
			   etk_font_add(engine->Family(), engine->Style(), engine) == false)
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
		   etk_font_add(engine->Family(), engine->Style(), engine) == false)
		{
			if(engine) delete engine;
		}
	}

	ETK_DEBUG("[GRAPHICS]: DirectFB fonts updated.");

	return E_OK;
}

