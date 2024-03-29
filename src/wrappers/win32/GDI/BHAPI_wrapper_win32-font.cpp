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
 * File: BHAPI_wrapper_win32-font.cpp
 *
 * --------------------------------------------------------------------------*/
#ifdef WIN32

#include <BHAPI_wrapper_win32gdi.h>

#include <kits/add-ons/font/FontEngine.h>
#include <kits/support/ClassInfo.h>
#include <kits/support/Autolock.h>
#include <kits/support/StringArray.h>
#include <kits/interface/Window.h>
#include <kits/interface/View.h>
#include <kits/debug/Errors.h>
#include <os/debug.h>


BOOL CALLBACK _etkEnumHeightCallBack_(ENUMLOGFONTEX *lplfe, NEWTEXTMETRICEX *lpntme, DWORD FontType, LPARAM lParam)
{
	BMessage *fontMsg = (BMessage*)lParam;
	if(fontMsg == NULL || lplfe == NULL) return FALSE;

	BString queryStyle;
	fontMsg->FindString("style", &queryStyle);
	if(queryStyle.Length() <= 0) return FALSE;

    const char *fontStyle = (FontType & TRUETYPE_FONTTYPE ? (const char*)lplfe->elfStyle : "Unknown");
	if(queryStyle != fontStyle) return TRUE;

	BOOL retVal = TRUE;

    if(FontType & TRUETYPE_FONTTYPE)
	{
		fontMsg->AddBool("scalable", true);
		retVal = FALSE;
	}
	else if(FontType & RASTER_FONTTYPE)
	{
		fontMsg->AddInt32("height", lplfe->elfLogFont.lfHeight);
	}

	return retVal;
}


class BFontWin32 : public BFontEngine {
public:
	BFontWin32(EWin32GraphicsEngine *win32Engine, const char *wFontname, const char *wFontStyle);
	virtual ~BFontWin32();

	virtual bool IsValid() const;
	virtual bool IsScalable() const;
	virtual void ForceFontAliasing(bool enable);

	virtual float StringWidth(const char *string, float size, float spacing, float shear, bool bold,  int32_t length) const;
	virtual void GetHeight(bhapi::font_height *height, float size, float shear, bool bold) const;
	virtual BRect RenderString(BHandler *view, const char *string, float size, float spacing, float shear, bool bold,  int32_t length);

	virtual bool Detach(bhapi::font_detach_callback *callback);

private:
	EWin32GraphicsEngine *fEngine;

	BString fLocalFontFamily;
	BString fLocalFontStyle;

	HDC fTmpDC;
	HFONT fCacheFont;
	uint32_t fCacheFontHeight;

	bool fForceAliasing;
	bool fScalable;

	static status_t _CreateFont(EWin32GraphicsEngine *win32Engine, const char *family, const char *style,
				       uint32_t height, bool aliasing, HFONT *font);
	static status_t _DestroyFont(EWin32GraphicsEngine *win32Engine, HFONT *font);
	static status_t _StringWidth(EWin32GraphicsEngine *win32Engine, const char *string,
				        uint32_t spacing,  uint32_t *w, HDC *hdc, HFONT *font);
	static status_t _GetHeight(EWin32GraphicsEngine *win32Engine,  uint32_t *leading,  uint32_t *ascent,  uint32_t *descent,
				     HDC *hdc, HFONT *font);
	static status_t _RenderString(EWin32GraphicsEngine *win32Engine, EWin32GraphicsDrawable *pixmap, BGraphicsContext *dc,
					const char *string,  uint32_t spacing,
					int32_t x,  int32_t y,  uint32_t *w,  uint32_t *h,  uint32_t *ascent, HFONT *font);
	static status_t _CreateTmpDC(EWin32GraphicsEngine *win32Engine, HDC *hdc);
	static status_t _DestroyTmpDC(EWin32GraphicsEngine *win32Engine, HDC *hdc);
};


status_t 
BFontWin32::_CreateFont(EWin32GraphicsEngine *win32Engine, const char *family, const char *style,
			uint32_t height, bool aliasing, HFONT *font)
{
	if(win32Engine == NULL || font == NULL ||
	   family == NULL || *family == 0 || style == NULL || *style == 0) return B_ERROR;

    bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_CREATB_FONT;
	callback.fontFamily = family;
	callback.fontString = style;
	callback.h = height;
	callback.fontAliasing = aliasing;
	callback.font = font;

	if(SendMessageA(win32Engine->win32RequestAsyncWin, win32Engine->WMBHAPI_MESSAGE,
			WMBHAPI_MESSAGB_FONT, (LPARAM)&callback) != (LRESULT)TRUE) return B_ERROR;

	if(*font == NULL) return B_ERROR;
	return B_OK;
}


LRESULT _bhapi_create_font(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WMBHAPI_MESSAGE_CREATB_FONT ||
	   callback->fontFamily == NULL || *(callback->fontFamily) == 0 ||
	   callback->fontString == NULL || *(callback->fontString) == 0 ||
	   callback->h == 0 || callback->font == NULL) return FALSE;

	BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK) return FALSE;

    HFONT newFont = CreateFontA((int)callback->h, 0, 0, 0, FW_DONTCARE,
				   FALSE, FALSE, FALSE, DEFAULT_CHARSET,
				   OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
				   (callback->fontAliasing ? NONANTIALIASED_QUALITY : DEFAULT_QUALITY),
				   DEFAULT_PITCH, callback->fontFamily);
	if(newFont == NULL) return FALSE;

	if(*(callback->font) != NULL) DeleteObject(*(callback->font));
	*(callback->font) = newFont;

	return TRUE;
}


status_t 
BFontWin32::_DestroyFont(EWin32GraphicsEngine *win32Engine, HFONT *font)
{
	if(win32Engine == NULL || font == NULL) return B_ERROR;

    bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_DESTROY_FONT;
	callback.font = font;

	if(SendMessageA(win32Engine->win32RequestAsyncWin, win32Engine->WMBHAPI_MESSAGE,
			WMBHAPI_MESSAGB_FONT, (LPARAM)&callback) != (LRESULT)TRUE) return B_ERROR;

	if(*font != NULL) return B_ERROR;
	return B_OK;
}


LRESULT _bhapi_destroy_font(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WMBHAPI_MESSAGE_DESTROY_FONT ||
	   callback->font == NULL || *(callback->font) == NULL) return FALSE;

	BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK) return FALSE;

	DeleteObject(*(callback->font));
	*(callback->font) = NULL;

	return TRUE;
}


status_t 
BFontWin32::_StringWidth(EWin32GraphicsEngine *win32Engine, const char *string,  uint32_t spacing,  uint32_t *w, HDC *hdc, HFONT *font)
{
	if(win32Engine == NULL || string == NULL || *string == 0 || w == NULL ||
	   hdc == NULL || *hdc == NULL || font == NULL || *font == NULL) return B_ERROR;

    bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGB_FONT_STRING_WIDTH;
	callback.fontTmpDC = hdc;
	callback.font = font;
	callback.fontString = string;
	callback.fontSpacing = spacing;

	if(SendMessageA(win32Engine->win32RequestAsyncWin, win32Engine->WMBHAPI_MESSAGE,
			WMBHAPI_MESSAGB_FONT, (LPARAM)&callback) != (LRESULT)TRUE) return B_ERROR;

	if(callback.w == 0) return B_ERROR;
	*w = callback.w;
	return B_OK;
}


LRESULT _bhapi_font_string_width(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WMBHAPI_MESSAGB_FONT_STRING_WIDTH ||
	   callback->fontString == NULL || *(callback->fontString) == 0 ||
	   callback->fontTmpDC == NULL || *(callback->fontTmpDC) == NULL ||
	   callback->font == NULL || *(callback->font) == NULL) return FALSE;

	BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK) return FALSE;

	TEXTMETRIC ptm;
	if(SelectObject(*(callback->fontTmpDC), *(callback->font)) == 0 || GetTextMetrics(*(callback->fontTmpDC), &ptm) == 0)
	{
		SelectObject(*(callback->fontTmpDC), GetStockObject(DEFAULT_GUI_FONT));
		return FALSE;
	}

	uint32_t width = 0;
	uint32_t height = (uint32_t)ptm.tmHeight;
	uint32_t delta = callback->fontSpacing;

	if(GetVersion() < 0x80000000) // Windows NT/2000/XP
	{
		unichar32 *utf32 = bhapi::utf8_convert_to_utf32(callback->fontString, -1);
		const unichar32 *tmp = bhapi::utf32_at(utf32, 0);

		while(!(!tmp || *tmp == 0))
		{
			int cWidth = -1;
			if(ptm.tmPitchAndFamily & TMPF_TRUETYPE) // TrueType
			{
				ABC pAbc;
				if(GetCharABCWidthsW(*(callback->fontTmpDC), *tmp, *tmp, &pAbc) != 0)
					cWidth = (pAbc.abcA + (int)pAbc.abcB + pAbc.abcC);
			}
			else
			{
				GetCharWidthW(*(callback->fontTmpDC), *tmp, *tmp, &cWidth);
			}

			width += (cWidth > 0 ? (uint32_t)cWidth : height) + (tmp == (const unichar32*)utf32 ? 0 : delta);
			tmp = bhapi::utf32_next(tmp);
		}

		if(utf32 != NULL) free(utf32);
	}
	else // Windows 95/98
	{
		uint8 uLen = 0;
		const char *uStr = bhapi::utf8_at(callback->fontString, 0, &uLen);
		const char *tmp = uStr;

		while(!(!tmp || *tmp == 0))
		{
			int32_t cWidth = -1;

            char *aStr = bhapi::win32_convert_utf8_to_active(tmp, (int32_t)uLen);
			if(aStr != NULL)
			{
				SIZE sz;
                if(GetTextExtentPoint32_tA(*(callback->fontTmpDC), aStr, strlen(aStr), &sz)) cWidth = (int32_t)sz.cx;
				free(aStr);
			}

			width += (cWidth > 0 ? (uint32_t)cWidth : height) + (tmp == uStr ? 0 : delta);

			tmp = bhapi::utf8_next(tmp, &uLen);
		}
	}

	SelectObject(*(callback->fontTmpDC), GetStockObject(DEFAULT_GUI_FONT));

	callback->w = width;

	return TRUE;
}


status_t 
BFontWin32::_GetHeight(EWin32GraphicsEngine *win32Engine,  uint32_t *leading,  uint32_t *ascent,  uint32_t *descent, HDC *hdc, HFONT *font)
{
	if(win32Engine == NULL || leading == NULL || ascent == NULL || descent == NULL ||
	   hdc == NULL || *hdc == NULL || font == NULL || *font == NULL) return B_ERROR;

    bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGB_FONT_GET_HEIGHT;
	callback.fontTmpDC = hdc;
	callback.font = font;

	if(SendMessageA(win32Engine->win32RequestAsyncWin, win32Engine->WMBHAPI_MESSAGE,
			WMBHAPI_MESSAGB_FONT, (LPARAM)&callback) != (LRESULT)TRUE) return B_ERROR;

	*leading = callback.ww;
	*ascent = callback.wh;
	*descent = callback.h;

	return B_OK;
}


LRESULT _bhapi_font_get_height(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WMBHAPI_MESSAGB_FONT_GET_HEIGHT ||
	   callback->fontTmpDC == NULL || *(callback->fontTmpDC) == NULL ||
	   callback->font == NULL || *(callback->font) == NULL) return FALSE;

	BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK) return FALSE;

	TEXTMETRIC ptm;
	if(SelectObject(*(callback->fontTmpDC), *(callback->font)) == 0 || GetTextMetrics(*(callback->fontTmpDC), &ptm) == 0)
	{
		SelectObject(*(callback->fontTmpDC), GetStockObject(DEFAULT_GUI_FONT));
		return FALSE;
	}

	callback->ww = (uint32_t)ptm.tmInternalLeading;
	callback->wh = (uint32_t)ptm.tmAscent;
	callback->h = (uint32_t)ptm.tmDescent;

	SelectObject(*(callback->fontTmpDC), GetStockObject(DEFAULT_GUI_FONT));

	return TRUE;
}


status_t 
BFontWin32::_RenderString(EWin32GraphicsEngine *win32Engine, EWin32GraphicsDrawable *pixmap, BGraphicsContext *dc,
			  const char *string,  uint32_t spacing,
			   int32_t x,  int32_t y,  uint32_t *w,  uint32_t *h,  uint32_t *ascent, HFONT *font)
{
	if(win32Engine == NULL || pixmap == NULL || dc == NULL ||
	   string == NULL || *string == 0 ||
	   w == NULL || h == NULL || ascent == NULL ||
	   font == NULL || *font == NULL) return B_ERROR;

    bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGB_FONT_RENDER_STRING;
	callback.font = font;
	callback.pixmap = pixmap;
	callback.dc = dc;
	callback.fontString = string;
	callback.fontSpacing = spacing;
	callback.x = x;
	callback.y = y;

	if(SendMessageA(win32Engine->win32RequestAsyncWin, win32Engine->WMBHAPI_MESSAGE,
			WMBHAPI_MESSAGB_FONT, (LPARAM)&callback) != (LRESULT)TRUE) return B_ERROR;

	if(callback.w == 0 || callback.h == 0) return B_ERROR;

	*w = callback.w;
	*h = callback.h;
	*ascent = callback.wh;

	return B_OK;
}


LRESULT _bhapi_font_render_string(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WMBHAPI_MESSAGB_FONT_RENDER_STRING ||
	   callback->fontString == NULL || *(callback->fontString) == 0 ||
	   callback->font == NULL || *(callback->font) == NULL ||
	   callback->pixmap == NULL || callback->pixmap->win32Pixmap == NULL || callback->pixmap->win32HDC == NULL ||
	   callback->dc == NULL) return FALSE;

	BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK) return FALSE;

	TEXTMETRIC ptm;
	if(SelectObject(callback->pixmap->win32HDC, *(callback->font)) == 0 || GetTextMetrics(callback->pixmap->win32HDC, &ptm) == 0)
	{
		SelectObject(callback->pixmap->win32HDC, GetStockObject(DEFAULT_GUI_FONT));
		return FALSE;
	}

	if(win32Engine->PrepareContext(callback->pixmap, callback->dc, true, false) == false) return FALSE;

	int x = callback->x;
	int y = callback->y - (int)ptm.tmAscent - 1;

	uint32_t width = 0;
	uint32_t height = (uint32_t)ptm.tmHeight;
	uint32_t delta = callback->fontSpacing;

	if(GetVersion() < 0x80000000) // Windows NT/2000/XP
	{
		unichar32 *utf32 = bhapi::utf8_convert_to_utf32(callback->fontString, -1);
		const unichar32 *tmp = bhapi::utf32_at(utf32, 0);

		while(!(!tmp || *tmp == 0))
		{
			int cWidth = -1;
			if(ptm.tmPitchAndFamily & TMPF_TRUETYPE) // TrueType
			{
				ABC pAbc;
				if(GetCharABCWidthsW(callback->pixmap->win32HDC, *tmp, *tmp, &pAbc) != 0)
					cWidth = (pAbc.abcA + (int)pAbc.abcB + pAbc.abcC);
			}
			else
			{
				GetCharWidthW(callback->pixmap->win32HDC, *tmp, *tmp, &cWidth);
			}

			if(cWidth > 0)
			{
				unichar16*unicode = bhapi::utf32_convert_to_unicode(tmp, 1);
				if(unicode != NULL)
				{
					ExtTextOutW(callback->pixmap->win32HDC, x, y, 0, NULL,
						    (WCHAR*)unicode, bhapi::unicode_strlen_etc(unicode, -1, false), NULL);
					free(unicode);
				}
				else
				{
					cWidth = -1;
				}
			}
			else
			{
				Rectangle(callback->pixmap->win32HDC, x + 2, y + 2,
					  x + 3 + (int)(max_c(height, 4) - 4), y + 3 + (int)(max_c(height, 4) - 4));
			}

			x += (cWidth > 0 ? cWidth : (int)height) + (int)delta;
			width += (cWidth > 0 ? (uint32_t)cWidth : height) + (tmp == (const unichar32*)utf32 ? 0 : delta);
			tmp = bhapi::utf32_next(tmp);
		}

		if(utf32 != NULL) free(utf32);
	}
	else // Windows 95/98
	{
		uint8 uLen = 0;
		const char *uStr = bhapi::utf8_at(callback->fontString, 0, &uLen);
		const char *tmp = uStr;

		while(!(!tmp || *tmp == 0))
		{
			int32_t cWidth = -1;

            char *aStr = bhapi::win32_convert_utf8_to_active(tmp, (int32_t)uLen);
			if(aStr != NULL)
			{
				SIZE sz;
                if(GetTextExtentPoint32_tA(callback->pixmap->win32HDC, aStr, strlen(aStr), &sz)) cWidth = (int32_t)sz.cx;
			}

			if(cWidth > 0 && aStr != NULL)
                ExtTextOutA(callback->pixmap->win32HDC, x, y, 0, NULL, aStr, strlen(aStr), NULL);
			else
				Rectangle(callback->pixmap->win32HDC, x + 2, y + 2,
					  x + 3 + (int)(max_c(height, 4) - 4), y + 3 + (int)(max_c(height, 4) - 4));

			if(aStr != NULL) free(aStr);

			x += (cWidth > 0 ? cWidth : (int)height) + (int)delta;
			width += (cWidth > 0 ? (uint32_t)cWidth : height) + (tmp == uStr ? 0 : delta);

			tmp = bhapi::utf8_next(tmp, &uLen);
		}
	}

	SelectObject(callback->pixmap->win32HDC, GetStockObject(DEFAULT_GUI_FONT));

	callback->w = width;
	callback->h = height;
	callback->wh = (uint32_t)ptm.tmAscent;

	return TRUE;
}


status_t 
BFontWin32::_CreateTmpDC(EWin32GraphicsEngine *win32Engine, HDC *hdc)
{
	if(win32Engine == NULL || hdc == NULL) return B_ERROR;

    bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_CREATB_FONT_TMP_DC;
	callback.fontTmpDC = hdc;

	if(SendMessageA(win32Engine->win32RequestAsyncWin, win32Engine->WMBHAPI_MESSAGE,
			WMBHAPI_MESSAGB_FONT, (LPARAM)&callback) != (LRESULT)TRUE) return B_ERROR;

	if(*hdc == NULL) return B_ERROR;
	return B_OK;
}


LRESULT _bhapi_creatbhapi::font_tmp_dc(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WMBHAPI_MESSAGE_CREATB_FONT_TMP_DC || callback->fontTmpDC == NULL) return FALSE;

	BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK) return FALSE;

	HDC newDC = CreateCompatibleDC(win32Engine->win32ScreenHDC);
	if(newDC == NULL) return FALSE;

	if(*(callback->fontTmpDC) != NULL) DeleteDC(*(callback->fontTmpDC));
	*(callback->fontTmpDC) = newDC;

	return TRUE;
}


status_t 
BFontWin32::_DestroyTmpDC(EWin32GraphicsEngine *win32Engine, HDC *hdc)
{
	if(win32Engine == NULL || hdc == NULL) return B_ERROR;

    bhapi::win32_gdi_callback_t callback;
	callback.command = WMBHAPI_MESSAGE_DESTROY_FONT_TMP_DC;
	callback.fontTmpDC = hdc;

	if(SendMessageA(win32Engine->win32RequestAsyncWin, win32Engine->WMBHAPI_MESSAGE,
			WMBHAPI_MESSAGB_FONT, (LPARAM)&callback) != (LRESULT)TRUE) return B_ERROR;

	if(*hdc != NULL) return B_ERROR;
	return B_OK;
}


LRESULT _bhapi_destroy_font_tmp_dc(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WMBHAPI_MESSAGE_DESTROY_FONT_TMP_DC ||
	   callback->fontTmpDC == NULL || *(callback->fontTmpDC) == NULL) return FALSE;

	BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK) return FALSE;

	DeleteDC(*(callback->fontTmpDC));
	*(callback->fontTmpDC) = NULL;

	return TRUE;
}


BFontWin32::BFontWin32(EWin32GraphicsEngine *win32Engine, const char *wFontname, const char *wFontStyle)
	: BFontEngine(), fEngine(NULL), fTmpDC(NULL), fCacheFont(NULL), fCacheFontHeight(0), fForceAliasing(false), fScalable(false)
{
	if(win32Engine == NULL || wFontname == NULL || *wFontname == 0 || strlen(wFontname) >= LF_FACESIZE ||
	   wFontStyle == NULL || *wFontStyle == 0 || strlen(wFontStyle) >= LF_FACESIZE) return;

    LOGFONTA logFont;
	logFont.lfCharSet = DEFAULT_CHARSET;
	bzero(logFont.lfFaceName, LF_FACESIZE);
	strncpy(logFont.lfFaceName, wFontname, LF_FACESIZE - 1);
	logFont.lfPitchAndFamily = 0;

	BMessage fontMsg;
	fontMsg.AddString("family", wFontname);
	fontMsg.AddString("style", wFontStyle);

	win32Engine->Lock();
    EnumFontFamiliesExA(win32Engine->win32ScreenHDC, &logFont, (FONTENUMPROCA)_etkEnumHeightCallBack_, (LPARAM)&fontMsg, 0);
	win32Engine->Unlock();

	if(fontMsg.HasBool("scalable"))
	{
		fScalable = true;
	}
	else if(fontMsg.HasInt32("height"))
	{
		int32_t nSizes = fontMsg.CountItems("height", B_INT32_TYPE);
		if(nSizes <= 0) return;
		float *font_sizes = new float[nSizes];
		if(font_sizes == NULL) return;
		for(int32_t i = 0; i < nSizes; i++)
		{
			int32_t val = 0;
			fontMsg.FindInt32("height", i, &val);
			font_sizes[i] = (float)val;
		}
		SetFixedSize(font_sizes, nSizes);
		delete[] font_sizes;
	}
	else
	{
		return;
	}

	fLocalFontFamily = wFontname;
	fLocalFontStyle = wFontStyle;

    char *sFontFamily = bhapi::win32_convert_active_to_utf8(wFontname, -1);
    char *sFontStyle = bhapi::win32_convert_active_to_utf8(wFontStyle, -1);

	SetFamily(sFontFamily == NULL ? wFontname : sFontFamily);
	SetStyle(sFontStyle == NULL ? wFontStyle : sFontStyle);

	if(sFontFamily) free(sFontFamily);
	if(sFontStyle) free(sFontStyle);

	fEngine = win32Engine;
	_CreateTmpDC(fEngine, &fTmpDC);

	SetRenderMode(B_FONT_RENDER_DIRECTLY);
}


BFontWin32::~BFontWin32()
{
	_DestroyFont(fEngine, &fCacheFont);
	_DestroyTmpDC(fEngine, &fTmpDC);
	if(fCacheFont != NULL) DeleteObject(fCacheFont);
	if(fTmpDC != NULL) DeleteDC(fTmpDC);
}


bool 
BFontWin32::IsValid() const
{
	return(fEngine != NULL && fTmpDC != NULL && Family() != NULL && Style() != NULL &&
	       fLocalFontFamily.Length() > 0 && fLocalFontStyle.Length() > 0);
}


bool 
BFontWin32::IsScalable() const
{
	return fScalable;
}


void 
BFontWin32::ForceFontAliasing(bool enable)
{
	if(fForceAliasing == enable) return;
	fForceAliasing = enable;
	if(fCacheFontHeight > 0)
		_CreateFont(fEngine, fLocalFontFamily.String(), fLocalFontStyle.String(), fCacheFontHeight, fForceAliasing, &fCacheFont);
}


float
BFontWin32::StringWidth(const char *string, float size, float spacing, float shear, bool bold,  int32_t length) const
{
	if(fEngine == NULL || fTmpDC == NULL ||
	   (int)size <= 0 || string == NULL || *string == 0 || length == 0 || !IsAttached()) return 0;

	HFONT curFont = NULL;
	if(fCacheFont == NULL || fCacheFontHeight != (uint32_t)size)
	{
		_CreateFont(fEngine, fLocalFontFamily.String(), fLocalFontStyle.String(), (uint32_t)size, fForceAliasing, &curFont);
	}
	else
	{
		curFont = fCacheFont;
	}

	if(curFont == NULL) return 0;

	BString str;
	str.Append(string, length);

	uint32_t width = 0;
	_StringWidth(fEngine, str.String(), (uint32_t)ceil((double)(spacing * size)), &width, (HDC*)&fTmpDC, &curFont);

	if(curFont != fCacheFont)
	{
		if(_DestroyFont(fEngine, &curFont) != B_OK) DeleteObject(curFont);
	}

	return (float)width;
}


void 
BFontWin32::GetHeight(bhapi::font_height *height, float size, float shear, bool bold) const
{
	if(!height) return;
	bzero(height, sizeof(bhapi::font_height));

	if(fEngine == NULL || fTmpDC == NULL || (int)size <= 0 || !IsAttached()) return;

	HFONT curFont = NULL;
	if(fCacheFont == NULL || fCacheFontHeight != (uint32_t)size)
	{
		_CreateFont(fEngine, fLocalFontFamily.String(), fLocalFontStyle.String(), (uint32_t)size, fForceAliasing, &curFont);
	}
	else
	{
		curFont = fCacheFont;
	}

	if(curFont == NULL) return;

	uint32_t leading = 0, ascent = 0, descent = 0;
	_GetHeight(fEngine, &leading, &ascent, &descent, (HDC*)&fTmpDC, &curFont);

	if(curFont != fCacheFont)
	{
		if(_DestroyFont(fEngine, &curFont) != B_OK) DeleteObject(curFont);
	}

	height->leading = (float)leading;
	height->ascent = (float)ascent;
	height->descent = (float)descent;
}


BRect
BFontWin32::RenderString(BHandler *_view, const char *string, float size, float spacing, float shear, bool bold,  int32_t length)
{
	if(fEngine == NULL || (int)size <= 0 || string == NULL || *string == 0 || length == 0 || !IsAttached()) return BRect();

	BView *view = cast_as(_view, BView);
	if(view == NULL || view->Window() == NULL || view->IsPrinting()) return BRect();

	BRegion viewClipping;
	view->GetClippingRegion(&viewClipping);
	if(viewClipping.CountRects() <= 0) return BRect();

	EWin32GraphicsDrawable *pix = cast_as(fEngine->GetPixmap(view->Window()), EWin32GraphicsDrawable);
	BGraphicsContext *dc = fEngine->GetContext(view);
	if(pix == NULL || dc == NULL) return BRect();

	if(fCacheFont == NULL || fCacheFontHeight != (uint32_t)size)
	{
		if(_CreateFont(fEngine, fLocalFontFamily.String(), fLocalFontStyle.String(),
			       (uint32_t)size, fForceAliasing, &fCacheFont) != B_OK) return BRect();
		fCacheFontHeight = (uint32_t)size;
	}

	BPoint pt = view->ConvertToWindow(view->PenLocation()).FloorSelf();
	BString str;
	str.Append(string, length);

	uint32_t w, h, ascent;
	if(_RenderString(fEngine, pix, dc, str.String(), (uint32_t)ceil((double)(spacing * size)),
			 (int32_t)pt.x, (int32_t)pt.y, &w, &h, &ascent, &fCacheFont) != B_OK) return BRect();

	BRect updateRect;
	updateRect.left = pt.x;
	updateRect.right = pt.x + (float)w;
	updateRect.top = pt.y - (float)(ascent + 1);
	updateRect.bottom = updateRect.top + (float)h;
	view->ConvertFromWindow(&updateRect);
	updateRect &= viewClipping.Frame();

	return updateRect;
}


bool 
BFontWin32::Detach(bhapi::font_detach_callback *callback)
{
	if(!BFontEngine::Detach(callback)) return false;
	if(!IsAttached()) _DestroyFont(fEngine, &fCacheFont);

	return true;
}


status_t 
EWin32GraphicsEngine::InitalizeFonts()
{
	BAutolock <EWin32GraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false) return B_ERROR;

	return InitCheck();
}


void 
EWin32GraphicsEngine::DestroyFonts()
{
	// TODO
}


BOOL CALLBACK _etkEnumFamAndStyleCallBack_(ENUMLOGFONTEXA *lplfe, NEWTEXTMETRICEX *lpntme, DWORD FontType, LPARAM lParam)
{
	BMessage *fontMsg = (BMessage*)lParam;
	if(fontMsg == NULL || lplfe == NULL) return FALSE;

	const char *fontFamily = lplfe->elfLogFont.lfFaceName;
	if(*fontFamily == 0) return TRUE;

	const char *fontStyle = NULL;
    if(FontType & TRUETYPE_FONTTYPE) fontStyle = (const char*)lplfe->elfStyle;
	if(fontStyle == NULL || *fontStyle == 0) fontStyle = "Unknown";

	BString str;
	str << fontFamily << "\t" << fontStyle;

	if(fontMsg->HasString(str.String()) == false)
	{
        fontMsg->AddString(str.String(), (FontType & TRUETYPE_FONTTYPE ? "Scalable" : "Fixed"));
		fontMsg->AddString("BHAPI:font", str);
	}

	return TRUE;
}


status_t 
EWin32GraphicsEngine::UpdateFonts(bool check_only)
{
	Lock();
	if(InitCheck() != B_OK)
	{
		Unlock();
		return B_ERROR;
	}

	if(check_only)
	{
		BHAPI_WARNING("[GRAPHICS]: %s --- check_only not implement yet.", __PRETTY_FUNCTION__);
		Unlock();
		return B_ERROR;
	}

	status_t retVal = B_ERROR;

	BHAPI_DEBUG("[GRAPHICS]: Updating GDI32 core fonts ...");

	LOGFONT logFont;
	logFont.lfCharSet = DEFAULT_CHARSET;
	logFont.lfFaceName[0] = '\0';
	logFont.lfPitchAndFamily = 0;

	BMessage fontMsg;
	EnumFontFamiliesEx(win32ScreenHDC, &logFont, (FONTENUMPROC)_etkEnumFamAndStyleCallBack_, (LPARAM)&fontMsg, 0);

	Unlock();

	for(int32_t i = 0; i < fontMsg.CountItems("BHAPI:font", B_STRING_TYPE); i++)
	{
		BString str;
		fontMsg.FindString("BHAPI:font", i, &str);

		int32_t escapeIndex = str.FindFirst("\t");
		if(escapeIndex <= 0 || escapeIndex >= str.Length() - 1) continue;

		BString fontFamily;
		BString fontStyle;

		str.CopyInto(fontFamily, 0, escapeIndex);
		str.CopyInto(fontStyle, escapeIndex + 1, -1);

		BFontWin32 *engine = new BFontWin32(this, fontFamily.String(), fontStyle.String());
		if(engine == NULL || engine->IsValid() == false)
		{
			if(engine) delete engine;
		}
		else
		{
			if(bhapi::font_add(engine->Family(), engine->Style(), engine))
				retVal = B_OK;
			else
				delete engine;
		}
	}

	BHAPI_DEBUG("[GRAPHICS]: GDI32 core fonts updated.");

	return retVal;
}

#endif /* WIN32 */
