/* --------------------------------------------------------------------------
 *
 * BHAPI++ previously named ETK++, The Easy Toolkit for C++ programing
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
 * File: etk-win32-font.cpp
 *
 * --------------------------------------------------------------------------*/
#ifdef WIN32

#include "etk-win32gdi.h"

#include "../../add-ons/font/FontEngine.h"
#include "../../support/ClassInfo.h"
#include "../../support/Autolock.h"
#include "../../support/StringArray.h"
#include "../../interface/Window.h"
#include "../../interface/View.h"
#include "../../support/Errors.h"
#include "../../kernel/Debug.h"


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

	virtual float StringWidth(const char *string, float size, float spacing, float shear, bool bold, b_int32 length) const;
	virtual void GetHeight(bhapi::font_height *height, float size, float shear, bool bold) const;
	virtual BRect RenderString(BHandler *view, const char *string, float size, float spacing, float shear, bool bold, b_int32 length);

	virtual bool Detach(bhapi::font_detach_callback *callback);

private:
	EWin32GraphicsEngine *fEngine;

	BString fLocalFontFamily;
	BString fLocalFontStyle;

	HDC fTmpDC;
	HFONT fCacheFont;
	b_uint32 fCacheFontHeight;

	bool fForceAliasing;
	bool fScalable;

	static b_status_t _CreateFont(EWin32GraphicsEngine *win32Engine, const char *family, const char *style,
				      b_uint32 height, bool aliasing, HFONT *font);
	static b_status_t _DestroyFont(EWin32GraphicsEngine *win32Engine, HFONT *font);
	static b_status_t _StringWidth(EWin32GraphicsEngine *win32Engine, const char *string,
				       b_uint32 spacing, b_uint32 *w, HDC *hdc, HFONT *font);
	static b_status_t _GetHeight(EWin32GraphicsEngine *win32Engine, b_uint32 *leading, b_uint32 *ascent, b_uint32 *descent,
				     HDC *hdc, HFONT *font);
	static b_status_t _RenderString(EWin32GraphicsEngine *win32Engine, EWin32GraphicsDrawable *pixmap, BGraphicsContext *dc,
					const char *string, b_uint32 spacing,
					b_int32 x, b_int32 y, b_uint32 *w, b_uint32 *h, b_uint32 *ascent, HFONT *font);
	static b_status_t _CreateTmpDC(EWin32GraphicsEngine *win32Engine, HDC *hdc);
	static b_status_t _DestroyTmpDC(EWin32GraphicsEngine *win32Engine, HDC *hdc);
};


b_status_t
BFontWin32::_CreateFont(EWin32GraphicsEngine *win32Engine, const char *family, const char *style,
			b_uint32 height, bool aliasing, HFONT *font)
{
	if(win32Engine == NULL || font == NULL ||
	   family == NULL || *family == 0 || style == NULL || *style == 0) return B_ERROR;

    bhapi::win32_gdi_callback_t callback;
	callback.command = WM_BHAPI_MESSAGE_CREATB_FONT;
	callback.fontFamily = family;
	callback.fontString = style;
	callback.h = height;
	callback.fontAliasing = aliasing;
	callback.font = font;

	if(SendMessageA(win32Engine->win32RequestAsyncWin, win32Engine->WM_BHAPI_MESSAGE,
			WM_BHAPI_MESSAGB_FONT, (LPARAM)&callback) != (LRESULT)TRUE) return B_ERROR;

	if(*font == NULL) return B_ERROR;
	return B_OK;
}


LRESULT _bhapi_create_font(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_BHAPI_MESSAGE_CREATB_FONT ||
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


b_status_t
BFontWin32::_DestroyFont(EWin32GraphicsEngine *win32Engine, HFONT *font)
{
	if(win32Engine == NULL || font == NULL) return B_ERROR;

    bhapi::win32_gdi_callback_t callback;
	callback.command = WM_BHAPI_MESSAGE_DESTROY_FONT;
	callback.font = font;

	if(SendMessageA(win32Engine->win32RequestAsyncWin, win32Engine->WM_BHAPI_MESSAGE,
			WM_BHAPI_MESSAGB_FONT, (LPARAM)&callback) != (LRESULT)TRUE) return B_ERROR;

	if(*font != NULL) return B_ERROR;
	return B_OK;
}


LRESULT _bhapi_destroy_font(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_BHAPI_MESSAGE_DESTROY_FONT ||
	   callback->font == NULL || *(callback->font) == NULL) return FALSE;

	BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK) return FALSE;

	DeleteObject(*(callback->font));
	*(callback->font) = NULL;

	return TRUE;
}


b_status_t
BFontWin32::_StringWidth(EWin32GraphicsEngine *win32Engine, const char *string, b_uint32 spacing, b_uint32 *w, HDC *hdc, HFONT *font)
{
	if(win32Engine == NULL || string == NULL || *string == 0 || w == NULL ||
	   hdc == NULL || *hdc == NULL || font == NULL || *font == NULL) return B_ERROR;

    bhapi::win32_gdi_callback_t callback;
	callback.command = WM_BHAPI_MESSAGB_FONT_STRING_WIDTH;
	callback.fontTmpDC = hdc;
	callback.font = font;
	callback.fontString = string;
	callback.fontSpacing = spacing;

	if(SendMessageA(win32Engine->win32RequestAsyncWin, win32Engine->WM_BHAPI_MESSAGE,
			WM_BHAPI_MESSAGB_FONT, (LPARAM)&callback) != (LRESULT)TRUE) return B_ERROR;

	if(callback.w == 0) return B_ERROR;
	*w = callback.w;
	return B_OK;
}


LRESULT _bhapi_font_string_width(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_BHAPI_MESSAGB_FONT_STRING_WIDTH ||
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

	b_uint32 width = 0;
	b_uint32 height = (b_uint32)ptm.tmHeight;
	b_uint32 delta = callback->fontSpacing;

	if(GetVersion() < 0x80000000) // Windows NT/2000/XP
	{
		b_unichar32 *utf32 = bhapi::utf8_convert_to_utf32(callback->fontString, -1);
		const b_unichar32 *tmp = bhapi::utf32_at(utf32, 0);

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

			width += (cWidth > 0 ? (b_uint32)cWidth : height) + (tmp == (const b_unichar32*)utf32 ? 0 : delta);
			tmp = bhapi::utf32_next(tmp);
		}

		if(utf32 != NULL) free(utf32);
	}
	else // Windows 95/98
	{
		b_uint8 uLen = 0;
		const char *uStr = bhapi::utf8_at(callback->fontString, 0, &uLen);
		const char *tmp = uStr;

		while(!(!tmp || *tmp == 0))
		{
			b_int32 cWidth = -1;

            char *aStr = bhapi::win32_convert_utf8_to_active(tmp, (b_int32)uLen);
			if(aStr != NULL)
			{
				SIZE sz;
                if(GetTextExtentPoint32A(*(callback->fontTmpDC), aStr, strlen(aStr), &sz)) cWidth = (b_int32)sz.cx;
				free(aStr);
			}

			width += (cWidth > 0 ? (b_uint32)cWidth : height) + (tmp == uStr ? 0 : delta);

			tmp = bhapi::utf8_next(tmp, &uLen);
		}
	}

	SelectObject(*(callback->fontTmpDC), GetStockObject(DEFAULT_GUI_FONT));

	callback->w = width;

	return TRUE;
}


b_status_t
BFontWin32::_GetHeight(EWin32GraphicsEngine *win32Engine, b_uint32 *leading, b_uint32 *ascent, b_uint32 *descent, HDC *hdc, HFONT *font)
{
	if(win32Engine == NULL || leading == NULL || ascent == NULL || descent == NULL ||
	   hdc == NULL || *hdc == NULL || font == NULL || *font == NULL) return B_ERROR;

    bhapi::win32_gdi_callback_t callback;
	callback.command = WM_BHAPI_MESSAGB_FONT_GET_HEIGHT;
	callback.fontTmpDC = hdc;
	callback.font = font;

	if(SendMessageA(win32Engine->win32RequestAsyncWin, win32Engine->WM_BHAPI_MESSAGE,
			WM_BHAPI_MESSAGB_FONT, (LPARAM)&callback) != (LRESULT)TRUE) return B_ERROR;

	*leading = callback.ww;
	*ascent = callback.wh;
	*descent = callback.h;

	return B_OK;
}


LRESULT _bhapi_font_get_height(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_BHAPI_MESSAGB_FONT_GET_HEIGHT ||
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

	callback->ww = (b_uint32)ptm.tmInternalLeading;
	callback->wh = (b_uint32)ptm.tmAscent;
	callback->h = (b_uint32)ptm.tmDescent;

	SelectObject(*(callback->fontTmpDC), GetStockObject(DEFAULT_GUI_FONT));

	return TRUE;
}


b_status_t
BFontWin32::_RenderString(EWin32GraphicsEngine *win32Engine, EWin32GraphicsDrawable *pixmap, BGraphicsContext *dc,
			  const char *string, b_uint32 spacing,
			  b_int32 x, b_int32 y, b_uint32 *w, b_uint32 *h, b_uint32 *ascent, HFONT *font)
{
	if(win32Engine == NULL || pixmap == NULL || dc == NULL ||
	   string == NULL || *string == 0 ||
	   w == NULL || h == NULL || ascent == NULL ||
	   font == NULL || *font == NULL) return B_ERROR;

    bhapi::win32_gdi_callback_t callback;
	callback.command = WM_BHAPI_MESSAGB_FONT_RENDER_STRING;
	callback.font = font;
	callback.pixmap = pixmap;
	callback.dc = dc;
	callback.fontString = string;
	callback.fontSpacing = spacing;
	callback.x = x;
	callback.y = y;

	if(SendMessageA(win32Engine->win32RequestAsyncWin, win32Engine->WM_BHAPI_MESSAGE,
			WM_BHAPI_MESSAGB_FONT, (LPARAM)&callback) != (LRESULT)TRUE) return B_ERROR;

	if(callback.w == 0 || callback.h == 0) return B_ERROR;

	*w = callback.w;
	*h = callback.h;
	*ascent = callback.wh;

	return B_OK;
}


LRESULT _bhapi_font_render_string(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_BHAPI_MESSAGB_FONT_RENDER_STRING ||
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

	b_uint32 width = 0;
	b_uint32 height = (b_uint32)ptm.tmHeight;
	b_uint32 delta = callback->fontSpacing;

	if(GetVersion() < 0x80000000) // Windows NT/2000/XP
	{
		b_unichar32 *utf32 = bhapi::utf8_convert_to_utf32(callback->fontString, -1);
		const b_unichar32 *tmp = bhapi::utf32_at(utf32, 0);

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
				b_unichar16*unicode = bhapi::utf32_convert_to_unicode(tmp, 1);
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
			width += (cWidth > 0 ? (b_uint32)cWidth : height) + (tmp == (const b_unichar32*)utf32 ? 0 : delta);
			tmp = bhapi::utf32_next(tmp);
		}

		if(utf32 != NULL) free(utf32);
	}
	else // Windows 95/98
	{
		b_uint8 uLen = 0;
		const char *uStr = bhapi::utf8_at(callback->fontString, 0, &uLen);
		const char *tmp = uStr;

		while(!(!tmp || *tmp == 0))
		{
			b_int32 cWidth = -1;

            char *aStr = bhapi::win32_convert_utf8_to_active(tmp, (b_int32)uLen);
			if(aStr != NULL)
			{
				SIZE sz;
                if(GetTextExtentPoint32A(callback->pixmap->win32HDC, aStr, strlen(aStr), &sz)) cWidth = (b_int32)sz.cx;
			}

			if(cWidth > 0 && aStr != NULL)
                ExtTextOutA(callback->pixmap->win32HDC, x, y, 0, NULL, aStr, strlen(aStr), NULL);
			else
				Rectangle(callback->pixmap->win32HDC, x + 2, y + 2,
					  x + 3 + (int)(max_c(height, 4) - 4), y + 3 + (int)(max_c(height, 4) - 4));

			if(aStr != NULL) free(aStr);

			x += (cWidth > 0 ? cWidth : (int)height) + (int)delta;
			width += (cWidth > 0 ? (b_uint32)cWidth : height) + (tmp == uStr ? 0 : delta);

			tmp = bhapi::utf8_next(tmp, &uLen);
		}
	}

	SelectObject(callback->pixmap->win32HDC, GetStockObject(DEFAULT_GUI_FONT));

	callback->w = width;
	callback->h = height;
	callback->wh = (b_uint32)ptm.tmAscent;

	return TRUE;
}


b_status_t
BFontWin32::_CreateTmpDC(EWin32GraphicsEngine *win32Engine, HDC *hdc)
{
	if(win32Engine == NULL || hdc == NULL) return B_ERROR;

    bhapi::win32_gdi_callback_t callback;
	callback.command = WM_BHAPI_MESSAGE_CREATB_FONT_TMP_DC;
	callback.fontTmpDC = hdc;

	if(SendMessageA(win32Engine->win32RequestAsyncWin, win32Engine->WM_BHAPI_MESSAGE,
			WM_BHAPI_MESSAGB_FONT, (LPARAM)&callback) != (LRESULT)TRUE) return B_ERROR;

	if(*hdc == NULL) return B_ERROR;
	return B_OK;
}


LRESULT _bhapi_creatbhapi::font_tmp_dc(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_BHAPI_MESSAGE_CREATB_FONT_TMP_DC || callback->fontTmpDC == NULL) return FALSE;

	BAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != B_OK) return FALSE;

	HDC newDC = CreateCompatibleDC(win32Engine->win32ScreenHDC);
	if(newDC == NULL) return FALSE;

	if(*(callback->fontTmpDC) != NULL) DeleteDC(*(callback->fontTmpDC));
	*(callback->fontTmpDC) = newDC;

	return TRUE;
}


b_status_t
BFontWin32::_DestroyTmpDC(EWin32GraphicsEngine *win32Engine, HDC *hdc)
{
	if(win32Engine == NULL || hdc == NULL) return B_ERROR;

    bhapi::win32_gdi_callback_t callback;
	callback.command = WM_BHAPI_MESSAGE_DESTROY_FONT_TMP_DC;
	callback.fontTmpDC = hdc;

	if(SendMessageA(win32Engine->win32RequestAsyncWin, win32Engine->WM_BHAPI_MESSAGE,
			WM_BHAPI_MESSAGB_FONT, (LPARAM)&callback) != (LRESULT)TRUE) return B_ERROR;

	if(*hdc != NULL) return B_ERROR;
	return B_OK;
}


LRESULT _bhapi_destroy_font_tmp_dc(EWin32GraphicsEngine *win32Engine, bhapi::win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_BHAPI_MESSAGE_DESTROY_FONT_TMP_DC ||
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
		b_int32 nSizes = fontMsg.CountItems("height", B_INT32_TYPE);
		if(nSizes <= 0) return;
		float *font_sizes = new float[nSizes];
		if(font_sizes == NULL) return;
		for(b_int32 i = 0; i < nSizes; i++)
		{
			b_int32 val = 0;
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
BFontWin32::StringWidth(const char *string, float size, float spacing, float shear, bool bold, b_int32 length) const
{
	if(fEngine == NULL || fTmpDC == NULL ||
	   (int)size <= 0 || string == NULL || *string == 0 || length == 0 || !IsAttached()) return 0;

	HFONT curFont = NULL;
	if(fCacheFont == NULL || fCacheFontHeight != (b_uint32)size)
	{
		_CreateFont(fEngine, fLocalFontFamily.String(), fLocalFontStyle.String(), (b_uint32)size, fForceAliasing, &curFont);
	}
	else
	{
		curFont = fCacheFont;
	}

	if(curFont == NULL) return 0;

	BString str;
	str.Append(string, length);

	b_uint32 width = 0;
	_StringWidth(fEngine, str.String(), (b_uint32)ceil((double)(spacing * size)), &width, (HDC*)&fTmpDC, &curFont);

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
	if(fCacheFont == NULL || fCacheFontHeight != (b_uint32)size)
	{
		_CreateFont(fEngine, fLocalFontFamily.String(), fLocalFontStyle.String(), (b_uint32)size, fForceAliasing, &curFont);
	}
	else
	{
		curFont = fCacheFont;
	}

	if(curFont == NULL) return;

	b_uint32 leading = 0, ascent = 0, descent = 0;
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
BFontWin32::RenderString(BHandler *_view, const char *string, float size, float spacing, float shear, bool bold, b_int32 length)
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

	if(fCacheFont == NULL || fCacheFontHeight != (b_uint32)size)
	{
		if(_CreateFont(fEngine, fLocalFontFamily.String(), fLocalFontStyle.String(),
			       (b_uint32)size, fForceAliasing, &fCacheFont) != B_OK) return BRect();
		fCacheFontHeight = (b_uint32)size;
	}

	BPoint pt = view->ConvertToWindow(view->PenLocation()).FloorSelf();
	BString str;
	str.Append(string, length);

	b_uint32 w, h, ascent;
	if(_RenderString(fEngine, pix, dc, str.String(), (b_uint32)ceil((double)(spacing * size)),
			 (b_int32)pt.x, (b_int32)pt.y, &w, &h, &ascent, &fCacheFont) != B_OK) return BRect();

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


b_status_t
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


b_status_t
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

	b_status_t retVal = B_ERROR;

	BHAPI_DEBUG("[GRAPHICS]: Updating GDI32 core fonts ...");

	LOGFONT logFont;
	logFont.lfCharSet = DEFAULT_CHARSET;
	logFont.lfFaceName[0] = '\0';
	logFont.lfPitchAndFamily = 0;

	BMessage fontMsg;
	EnumFontFamiliesEx(win32ScreenHDC, &logFont, (FONTENUMPROC)_etkEnumFamAndStyleCallBack_, (LPARAM)&fontMsg, 0);

	Unlock();

	for(b_int32 i = 0; i < fontMsg.CountItems("BHAPI:font", B_STRING_TYPE); i++)
	{
		BString str;
		fontMsg.FindString("BHAPI:font", i, &str);

		b_int32 escapeIndex = str.FindFirst("\t");
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
