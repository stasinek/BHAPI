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
 * File: etk-win32-font.cpp
 *
 * --------------------------------------------------------------------------*/
#ifdef WIN32

#include "etk-win32gdi.h"

#include "./../../add-ons/font/FontEngine.h"
#include "./../../support/ClassInfo.h"
#include "./../../support/Autolock.h"
#include "./../../support/StringArray.h"
#include "./../../interface/Window.h"
#include "./../../interface/View.h"


BOOL CALLBACK _etkEnumHeightCallBack_(ENUMLOGFONTEX *lplfe, NEWTEXTMETRICEX *lpntme, DWORD FontType, LPARAM lParam)
{
	EMessage *fontMsg = (EMessage*)lParam;
	if(fontMsg == NULL || lplfe == NULL) return FALSE;

	EString queryStyle;
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


class EFontWin32 : public EFontEngine {
public:
	EFontWin32(EWin32GraphicsEngine *win32Engine, const char *wFontname, const char *wFontStyle);
	virtual ~EFontWin32();

	virtual bool IsValid() const;
	virtual bool IsScalable() const;
	virtual void ForceFontAliasing(bool enable);

	virtual float StringWidth(const char *string, float size, float spacing, float shear, bool bold, eint32 length) const;
	virtual void GetHeight(e_font_height *height, float size, float shear, bool bold) const;
	virtual ERect RenderString(EHandler *view, const char *string, float size, float spacing, float shear, bool bold, eint32 length);

	virtual bool Detach(e_font_detach_callback *callback);

private:
	EWin32GraphicsEngine *fEngine;

	EString fLocalFontFamily;
	EString fLocalFontStyle;

	HDC fTmpDC;
	HFONT fCacheFont;
	euint32 fCacheFontHeight;

	bool fForceAliasing;
	bool fScalable;

	static e_status_t _CreateFont(EWin32GraphicsEngine *win32Engine, const char *family, const char *style,
				      euint32 height, bool aliasing, HFONT *font);
	static e_status_t _DestroyFont(EWin32GraphicsEngine *win32Engine, HFONT *font);
	static e_status_t _StringWidth(EWin32GraphicsEngine *win32Engine, const char *string,
				       euint32 spacing, euint32 *w, HDC *hdc, HFONT *font);
	static e_status_t _GetHeight(EWin32GraphicsEngine *win32Engine, euint32 *leading, euint32 *ascent, euint32 *descent,
				     HDC *hdc, HFONT *font);
	static e_status_t _RenderString(EWin32GraphicsEngine *win32Engine, EWin32GraphicsDrawable *pixmap, EGraphicsContext *dc,
					const char *string, euint32 spacing,
					eint32 x, eint32 y, euint32 *w, euint32 *h, euint32 *ascent, HFONT *font);
	static e_status_t _CreateTmpDC(EWin32GraphicsEngine *win32Engine, HDC *hdc);
	static e_status_t _DestroyTmpDC(EWin32GraphicsEngine *win32Engine, HDC *hdc);
};


e_status_t
EFontWin32::_CreateFont(EWin32GraphicsEngine *win32Engine, const char *family, const char *style,
			euint32 height, bool aliasing, HFONT *font)
{
	if(win32Engine == NULL || font == NULL ||
	   family == NULL || *family == 0 || style == NULL || *style == 0) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_CREATE_FONT;
	callback.fontFamily = family;
	callback.fontString = style;
	callback.h = height;
	callback.fontAliasing = aliasing;
	callback.font = font;

	if(SendMessageA(win32Engine->win32RequestAsyncWin, win32Engine->WM_ETK_MESSAGE,
			WM_ETK_MESSAGE_FONT, (LPARAM)&callback) != (LRESULT)TRUE) return E_ERROR;

	if(*font == NULL) return E_ERROR;
	return E_OK;
}


LRESULT _etk_create_font(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_ETK_MESSAGE_CREATE_FONT ||
	   callback->fontFamily == NULL || *(callback->fontFamily) == 0 ||
	   callback->fontString == NULL || *(callback->fontString) == 0 ||
	   callback->h == 0 || callback->font == NULL) return FALSE;

	EAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != E_OK) return FALSE;

	HFONT newFont = CreateFont((int)callback->h, 0, 0, 0, FW_DONTCARE,
				   FALSE, FALSE, FALSE, DEFAULT_CHARSET,
				   OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
				   (callback->fontAliasing ? NONANTIALIASED_QUALITY : DEFAULT_QUALITY),
				   DEFAULT_PITCH, callback->fontFamily);
	if(newFont == NULL) return FALSE;

	if(*(callback->font) != NULL) DeleteObject(*(callback->font));
	*(callback->font) = newFont;

	return TRUE;
}


e_status_t
EFontWin32::_DestroyFont(EWin32GraphicsEngine *win32Engine, HFONT *font)
{
	if(win32Engine == NULL || font == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_DESTROY_FONT;
	callback.font = font;

	if(SendMessageA(win32Engine->win32RequestAsyncWin, win32Engine->WM_ETK_MESSAGE,
			WM_ETK_MESSAGE_FONT, (LPARAM)&callback) != (LRESULT)TRUE) return E_ERROR;

	if(*font != NULL) return E_ERROR;
	return E_OK;
}


LRESULT _etk_destroy_font(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_ETK_MESSAGE_DESTROY_FONT ||
	   callback->font == NULL || *(callback->font) == NULL) return FALSE;

	EAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != E_OK) return FALSE;

	DeleteObject(*(callback->font));
	*(callback->font) = NULL;

	return TRUE;
}


e_status_t
EFontWin32::_StringWidth(EWin32GraphicsEngine *win32Engine, const char *string, euint32 spacing, euint32 *w, HDC *hdc, HFONT *font)
{
	if(win32Engine == NULL || string == NULL || *string == 0 || w == NULL ||
	   hdc == NULL || *hdc == NULL || font == NULL || *font == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_FONT_STRING_WIDTH;
	callback.fontTmpDC = hdc;
	callback.font = font;
	callback.fontString = string;
	callback.fontSpacing = spacing;

	if(SendMessageA(win32Engine->win32RequestAsyncWin, win32Engine->WM_ETK_MESSAGE,
			WM_ETK_MESSAGE_FONT, (LPARAM)&callback) != (LRESULT)TRUE) return E_ERROR;

	if(callback.w == 0) return E_ERROR;
	*w = callback.w;
	return E_OK;
}


LRESULT _etk_font_string_width(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_ETK_MESSAGE_FONT_STRING_WIDTH ||
	   callback->fontString == NULL || *(callback->fontString) == 0 ||
	   callback->fontTmpDC == NULL || *(callback->fontTmpDC) == NULL ||
	   callback->font == NULL || *(callback->font) == NULL) return FALSE;

	EAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != E_OK) return FALSE;

	TEXTMETRIC ptm;
	if(SelectObject(*(callback->fontTmpDC), *(callback->font)) == 0 || GetTextMetrics(*(callback->fontTmpDC), &ptm) == 0)
	{
		SelectObject(*(callback->fontTmpDC), GetStockObject(DEFAULT_GUI_FONT));
		return FALSE;
	}

	euint32 width = 0;
	euint32 height = (euint32)ptm.tmHeight;
	euint32 delta = callback->fontSpacing;

	if(GetVersion() < 0x80000000) // Windows NT/2000/XP
	{
		eunichar32 *utf32 = e_utf8_convert_to_utf32(callback->fontString, -1);
		const eunichar32 *tmp = e_utf32_at(utf32, 0);

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

			width += (cWidth > 0 ? (euint32)cWidth : height) + (tmp == (const eunichar32*)utf32 ? 0 : delta);
			tmp = e_utf32_next(tmp);
		}

		if(utf32 != NULL) free(utf32);
	}
	else // Windows 95/98
	{
		euint8 uLen = 0;
		const char *uStr = e_utf8_at(callback->fontString, 0, &uLen);
		const char *tmp = uStr;

		while(!(!tmp || *tmp == 0))
		{
			eint32 cWidth = -1;

			char *aStr = etk_win32_convert_utf8_to_active(tmp, (eint32)uLen);
			if(aStr != NULL)
			{
				SIZE sz;
				if(GetTextExtentPoint32(*(callback->fontTmpDC), aStr, strlen(aStr), &sz)) cWidth = (eint32)sz.cx;
				free(aStr);
			}

			width += (cWidth > 0 ? (euint32)cWidth : height) + (tmp == uStr ? 0 : delta);

			tmp = e_utf8_next(tmp, &uLen);
		}
	}

	SelectObject(*(callback->fontTmpDC), GetStockObject(DEFAULT_GUI_FONT));

	callback->w = width;

	return TRUE;
}


e_status_t
EFontWin32::_GetHeight(EWin32GraphicsEngine *win32Engine, euint32 *leading, euint32 *ascent, euint32 *descent, HDC *hdc, HFONT *font)
{
	if(win32Engine == NULL || leading == NULL || ascent == NULL || descent == NULL ||
	   hdc == NULL || *hdc == NULL || font == NULL || *font == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_FONT_GET_HEIGHT;
	callback.fontTmpDC = hdc;
	callback.font = font;

	if(SendMessageA(win32Engine->win32RequestAsyncWin, win32Engine->WM_ETK_MESSAGE,
			WM_ETK_MESSAGE_FONT, (LPARAM)&callback) != (LRESULT)TRUE) return E_ERROR;

	*leading = callback.ww;
	*ascent = callback.wh;
	*descent = callback.h;

	return E_OK;
}


LRESULT _etk_font_get_height(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_ETK_MESSAGE_FONT_GET_HEIGHT ||
	   callback->fontTmpDC == NULL || *(callback->fontTmpDC) == NULL ||
	   callback->font == NULL || *(callback->font) == NULL) return FALSE;

	EAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != E_OK) return FALSE;

	TEXTMETRIC ptm;
	if(SelectObject(*(callback->fontTmpDC), *(callback->font)) == 0 || GetTextMetrics(*(callback->fontTmpDC), &ptm) == 0)
	{
		SelectObject(*(callback->fontTmpDC), GetStockObject(DEFAULT_GUI_FONT));
		return FALSE;
	}

	callback->ww = (euint32)ptm.tmInternalLeading;
	callback->wh = (euint32)ptm.tmAscent;
	callback->h = (euint32)ptm.tmDescent;

	SelectObject(*(callback->fontTmpDC), GetStockObject(DEFAULT_GUI_FONT));

	return TRUE;
}


e_status_t
EFontWin32::_RenderString(EWin32GraphicsEngine *win32Engine, EWin32GraphicsDrawable *pixmap, EGraphicsContext *dc,
			  const char *string, euint32 spacing,
			  eint32 x, eint32 y, euint32 *w, euint32 *h, euint32 *ascent, HFONT *font)
{
	if(win32Engine == NULL || pixmap == NULL || dc == NULL ||
	   string == NULL || *string == 0 ||
	   w == NULL || h == NULL || ascent == NULL ||
	   font == NULL || *font == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_FONT_RENDER_STRING;
	callback.font = font;
	callback.pixmap = pixmap;
	callback.dc = dc;
	callback.fontString = string;
	callback.fontSpacing = spacing;
	callback.x = x;
	callback.y = y;

	if(SendMessageA(win32Engine->win32RequestAsyncWin, win32Engine->WM_ETK_MESSAGE,
			WM_ETK_MESSAGE_FONT, (LPARAM)&callback) != (LRESULT)TRUE) return E_ERROR;

	if(callback.w == 0 || callback.h == 0) return E_ERROR;

	*w = callback.w;
	*h = callback.h;
	*ascent = callback.wh;

	return E_OK;
}


LRESULT _etk_font_render_string(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_ETK_MESSAGE_FONT_RENDER_STRING ||
	   callback->fontString == NULL || *(callback->fontString) == 0 ||
	   callback->font == NULL || *(callback->font) == NULL ||
	   callback->pixmap == NULL || callback->pixmap->win32Pixmap == NULL || callback->pixmap->win32HDC == NULL ||
	   callback->dc == NULL) return FALSE;

	EAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != E_OK) return FALSE;

	TEXTMETRIC ptm;
	if(SelectObject(callback->pixmap->win32HDC, *(callback->font)) == 0 || GetTextMetrics(callback->pixmap->win32HDC, &ptm) == 0)
	{
		SelectObject(callback->pixmap->win32HDC, GetStockObject(DEFAULT_GUI_FONT));
		return FALSE;
	}

	if(win32Engine->PrepareContext(callback->pixmap, callback->dc, true, false) == false) return FALSE;

	int x = callback->x;
	int y = callback->y - (int)ptm.tmAscent - 1;

	euint32 width = 0;
	euint32 height = (euint32)ptm.tmHeight;
	euint32 delta = callback->fontSpacing;

	if(GetVersion() < 0x80000000) // Windows NT/2000/XP
	{
		eunichar32 *utf32 = e_utf8_convert_to_utf32(callback->fontString, -1);
		const eunichar32 *tmp = e_utf32_at(utf32, 0);

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
				eunichar *unicode = e_utf32_convert_to_unicode(tmp, 1);
				if(unicode != NULL)
				{
					ExtTextOutW(callback->pixmap->win32HDC, x, y, 0, NULL,
						    (WCHAR*)unicode, e_unicode_strlen_etc(unicode, -1, false), NULL);
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
			width += (cWidth > 0 ? (euint32)cWidth : height) + (tmp == (const eunichar32*)utf32 ? 0 : delta);
			tmp = e_utf32_next(tmp);
		}

		if(utf32 != NULL) free(utf32);
	}
	else // Windows 95/98
	{
		euint8 uLen = 0;
		const char *uStr = e_utf8_at(callback->fontString, 0, &uLen);
		const char *tmp = uStr;

		while(!(!tmp || *tmp == 0))
		{
			eint32 cWidth = -1;

			char *aStr = etk_win32_convert_utf8_to_active(tmp, (eint32)uLen);
			if(aStr != NULL)
			{
				SIZE sz;
				if(GetTextExtentPoint32(callback->pixmap->win32HDC, aStr, strlen(aStr), &sz)) cWidth = (eint32)sz.cx;
			}

			if(cWidth > 0 && aStr != NULL)
				ExtTextOut(callback->pixmap->win32HDC, x, y, 0, NULL, aStr, strlen(aStr), NULL);
			else
				Rectangle(callback->pixmap->win32HDC, x + 2, y + 2,
					  x + 3 + (int)(max_c(height, 4) - 4), y + 3 + (int)(max_c(height, 4) - 4));

			if(aStr != NULL) free(aStr);

			x += (cWidth > 0 ? cWidth : (int)height) + (int)delta;
			width += (cWidth > 0 ? (euint32)cWidth : height) + (tmp == uStr ? 0 : delta);

			tmp = e_utf8_next(tmp, &uLen);
		}
	}

	SelectObject(callback->pixmap->win32HDC, GetStockObject(DEFAULT_GUI_FONT));

	callback->w = width;
	callback->h = height;
	callback->wh = (euint32)ptm.tmAscent;

	return TRUE;
}


e_status_t
EFontWin32::_CreateTmpDC(EWin32GraphicsEngine *win32Engine, HDC *hdc)
{
	if(win32Engine == NULL || hdc == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_CREATE_FONT_TMP_DC;
	callback.fontTmpDC = hdc;

	if(SendMessageA(win32Engine->win32RequestAsyncWin, win32Engine->WM_ETK_MESSAGE,
			WM_ETK_MESSAGE_FONT, (LPARAM)&callback) != (LRESULT)TRUE) return E_ERROR;

	if(*hdc == NULL) return E_ERROR;
	return E_OK;
}


LRESULT _etk_create_font_tmp_dc(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_ETK_MESSAGE_CREATE_FONT_TMP_DC || callback->fontTmpDC == NULL) return FALSE;

	EAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != E_OK) return FALSE;

	HDC newDC = CreateCompatibleDC(win32Engine->win32ScreenHDC);
	if(newDC == NULL) return FALSE;

	if(*(callback->fontTmpDC) != NULL) DeleteDC(*(callback->fontTmpDC));
	*(callback->fontTmpDC) = newDC;

	return TRUE;
}


e_status_t
EFontWin32::_DestroyTmpDC(EWin32GraphicsEngine *win32Engine, HDC *hdc)
{
	if(win32Engine == NULL || hdc == NULL) return E_ERROR;

	etk_win32_gdi_callback_t callback;
	callback.command = WM_ETK_MESSAGE_DESTROY_FONT_TMP_DC;
	callback.fontTmpDC = hdc;

	if(SendMessageA(win32Engine->win32RequestAsyncWin, win32Engine->WM_ETK_MESSAGE,
			WM_ETK_MESSAGE_FONT, (LPARAM)&callback) != (LRESULT)TRUE) return E_ERROR;

	if(*hdc != NULL) return E_ERROR;
	return E_OK;
}


LRESULT _etk_destroy_font_tmp_dc(EWin32GraphicsEngine *win32Engine, etk_win32_gdi_callback_t *callback)
{
	if(win32Engine == NULL || callback == NULL ||
	   callback->command != WM_ETK_MESSAGE_DESTROY_FONT_TMP_DC ||
	   callback->fontTmpDC == NULL || *(callback->fontTmpDC) == NULL) return FALSE;

	EAutolock <EWin32GraphicsEngine> autolock(win32Engine);
	if(autolock.IsLocked() == false || win32Engine->InitCheck() != E_OK) return FALSE;

	DeleteDC(*(callback->fontTmpDC));
	*(callback->fontTmpDC) = NULL;

	return TRUE;
}


EFontWin32::EFontWin32(EWin32GraphicsEngine *win32Engine, const char *wFontname, const char *wFontStyle)
	: EFontEngine(), fEngine(NULL), fTmpDC(NULL), fCacheFont(NULL), fCacheFontHeight(0), fForceAliasing(false), fScalable(false)
{
	if(win32Engine == NULL || wFontname == NULL || *wFontname == 0 || strlen(wFontname) >= LF_FACESIZE ||
	   wFontStyle == NULL || *wFontStyle == 0 || strlen(wFontStyle) >= LF_FACESIZE) return;

	LOGFONT logFont;
	logFont.lfCharSet = DEFAULT_CHARSET;
	bzero(logFont.lfFaceName, LF_FACESIZE);
	strncpy(logFont.lfFaceName, wFontname, LF_FACESIZE - 1);
	logFont.lfPitchAndFamily = 0;

	EMessage fontMsg;
	fontMsg.AddString("family", wFontname);
	fontMsg.AddString("style", wFontStyle);

	win32Engine->Lock();
	EnumFontFamiliesEx(win32Engine->win32ScreenHDC, &logFont, (FONTENUMPROC)_etkEnumHeightCallBack_, (LPARAM)&fontMsg, 0);
	win32Engine->Unlock();

	if(fontMsg.HasBool("scalable"))
	{
		fScalable = true;
	}
	else if(fontMsg.HasInt32("height"))
	{
		eint32 nSizes = fontMsg.CountItems("height", E_INT32_TYPE);
		if(nSizes <= 0) return;
		float *font_sizes = new float[nSizes];
		if(font_sizes == NULL) return;
		for(eint32 i = 0; i < nSizes; i++)
		{
			eint32 val = 0;
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

	char *sFontFamily = etk_win32_convert_active_to_utf8(wFontname, -1);
	char *sFontStyle = etk_win32_convert_active_to_utf8(wFontStyle, -1);

	SetFamily(sFontFamily == NULL ? wFontname : sFontFamily);
	SetStyle(sFontStyle == NULL ? wFontStyle : sFontStyle);

	if(sFontFamily) free(sFontFamily);
	if(sFontStyle) free(sFontStyle);

	fEngine = win32Engine;
	_CreateTmpDC(fEngine, &fTmpDC);

	SetRenderMode(E_FONT_RENDER_DIRECTLY);
}


EFontWin32::~EFontWin32()
{
	_DestroyFont(fEngine, &fCacheFont);
	_DestroyTmpDC(fEngine, &fTmpDC);
	if(fCacheFont != NULL) DeleteObject(fCacheFont);
	if(fTmpDC != NULL) DeleteDC(fTmpDC);
}


bool
EFontWin32::IsValid() const
{
	return(fEngine != NULL && fTmpDC != NULL && Family() != NULL && Style() != NULL &&
	       fLocalFontFamily.Length() > 0 && fLocalFontStyle.Length() > 0);
}


bool
EFontWin32::IsScalable() const
{
	return fScalable;
}


void
EFontWin32::ForceFontAliasing(bool enable)
{
	if(fForceAliasing == enable) return;
	fForceAliasing = enable;
	if(fCacheFontHeight > 0)
		_CreateFont(fEngine, fLocalFontFamily.String(), fLocalFontStyle.String(), fCacheFontHeight, fForceAliasing, &fCacheFont);
}


float
EFontWin32::StringWidth(const char *string, float size, float spacing, float shear, bool bold, eint32 length) const
{
	if(fEngine == NULL || fTmpDC == NULL ||
	   (int)size <= 0 || string == NULL || *string == 0 || length == 0 || !IsAttached()) return 0;

	HFONT curFont = NULL;
	if(fCacheFont == NULL || fCacheFontHeight != (euint32)size)
	{
		_CreateFont(fEngine, fLocalFontFamily.String(), fLocalFontStyle.String(), (euint32)size, fForceAliasing, &curFont);
	}
	else
	{
		curFont = fCacheFont;
	}

	if(curFont == NULL) return 0;

	EString str;
	str.Append(string, length);

	euint32 width = 0;
	_StringWidth(fEngine, str.String(), (euint32)ceil((double)(spacing * size)), &width, (HDC*)&fTmpDC, &curFont);

	if(curFont != fCacheFont)
	{
		if(_DestroyFont(fEngine, &curFont) != E_OK) DeleteObject(curFont);
	}

	return (float)width;
}


void
EFontWin32::GetHeight(e_font_height *height, float size, float shear, bool bold) const
{
	if(!height) return;
	bzero(height, sizeof(e_font_height));

	if(fEngine == NULL || fTmpDC == NULL || (int)size <= 0 || !IsAttached()) return;

	HFONT curFont = NULL;
	if(fCacheFont == NULL || fCacheFontHeight != (euint32)size)
	{
		_CreateFont(fEngine, fLocalFontFamily.String(), fLocalFontStyle.String(), (euint32)size, fForceAliasing, &curFont);
	}
	else
	{
		curFont = fCacheFont;
	}

	if(curFont == NULL) return;

	euint32 leading = 0, ascent = 0, descent = 0;
	_GetHeight(fEngine, &leading, &ascent, &descent, (HDC*)&fTmpDC, &curFont);

	if(curFont != fCacheFont)
	{
		if(_DestroyFont(fEngine, &curFont) != E_OK) DeleteObject(curFont);
	}

	height->leading = (float)leading;
	height->ascent = (float)ascent;
	height->descent = (float)descent;
}


ERect
EFontWin32::RenderString(EHandler *_view, const char *string, float size, float spacing, float shear, bool bold, eint32 length)
{
	if(fEngine == NULL || (int)size <= 0 || string == NULL || *string == 0 || length == 0 || !IsAttached()) return ERect();

	EView *view = e_cast_as(_view, EView);
	if(view == NULL || view->Window() == NULL || view->IsPrinting()) return ERect();

	ERegion viewClipping;
	view->GetClippingRegion(&viewClipping);
	if(viewClipping.CountRects() <= 0) return ERect();

	EWin32GraphicsDrawable *pix = e_cast_as(fEngine->GetPixmap(view->Window()), EWin32GraphicsDrawable);
	EGraphicsContext *dc = fEngine->GetContext(view);
	if(pix == NULL || dc == NULL) return ERect();

	if(fCacheFont == NULL || fCacheFontHeight != (euint32)size)
	{
		if(_CreateFont(fEngine, fLocalFontFamily.String(), fLocalFontStyle.String(),
			       (euint32)size, fForceAliasing, &fCacheFont) != E_OK) return ERect();
		fCacheFontHeight = (euint32)size;
	}

	EPoint pt = view->ConvertToWindow(view->PenLocation()).FloorSelf();
	EString str;
	str.Append(string, length);

	euint32 w, h, ascent;
	if(_RenderString(fEngine, pix, dc, str.String(), (euint32)ceil((double)(spacing * size)),
			 (eint32)pt.x, (eint32)pt.y, &w, &h, &ascent, &fCacheFont) != E_OK) return ERect();

	ERect updateRect;
	updateRect.left = pt.x;
	updateRect.right = pt.x + (float)w;
	updateRect.top = pt.y - (float)(ascent + 1);
	updateRect.bottom = updateRect.top + (float)h;
	view->ConvertFromWindow(&updateRect);
	updateRect &= viewClipping.Frame();

	return updateRect;
}


bool
EFontWin32::Detach(e_font_detach_callback *callback)
{
	if(!EFontEngine::Detach(callback)) return false;
	if(!IsAttached()) _DestroyFont(fEngine, &fCacheFont);

	return true;
}


e_status_t
EWin32GraphicsEngine::InitalizeFonts()
{
	EAutolock <EWin32GraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false) return E_ERROR;

	return InitCheck();
}


void
EWin32GraphicsEngine::DestroyFonts()
{
	// TODO
}


BOOL CALLBACK _etkEnumFamAndStyleCallBack_(ENUMLOGFONTEX *lplfe, NEWTEXTMETRICEX *lpntme, DWORD FontType, LPARAM lParam)
{
	EMessage *fontMsg = (EMessage*)lParam;
	if(fontMsg == NULL || lplfe == NULL) return FALSE;

	const char *fontFamily = lplfe->elfLogFont.lfFaceName;
	if(*fontFamily == 0) return TRUE;

	const char *fontStyle = NULL;
	if(FontType & TRUETYPE_FONTTYPE) fontStyle = (const char*)lplfe->elfStyle;
	if(fontStyle == NULL || *fontStyle == 0) fontStyle = "Unknown";

	EString str;
	str << fontFamily << "\t" << fontStyle;

	if(fontMsg->HasString(str.String()) == false)
	{
		fontMsg->AddString(str.String(), (FontType & TRUETYPE_FONTTYPE ? "Scalable" : "Fixed"));
		fontMsg->AddString("etk:font", str);
	}

	return TRUE;
}


e_status_t
EWin32GraphicsEngine::UpdateFonts(bool check_only)
{
	Lock();
	if(InitCheck() != E_OK)
	{
		Unlock();
		return E_ERROR;
	}

	if(check_only)
	{
		ETK_WARNING("[GRAPHICS]: %s --- check_only not implement yet.", __PRETTY_FUNCTION__);
		Unlock();
		return E_ERROR;
	}

	e_status_t retVal = E_ERROR;

	ETK_DEBUG("[GRAPHICS]: Updating GDI32 core fonts ...");

	LOGFONT logFont;
	logFont.lfCharSet = DEFAULT_CHARSET;
	logFont.lfFaceName[0] = '\0';
	logFont.lfPitchAndFamily = 0;

	EMessage fontMsg;
	EnumFontFamiliesEx(win32ScreenHDC, &logFont, (FONTENUMPROC)_etkEnumFamAndStyleCallBack_, (LPARAM)&fontMsg, 0);

	Unlock();

	for(eint32 i = 0; i < fontMsg.CountItems("etk:font", E_STRING_TYPE); i++)
	{
		EString str;
		fontMsg.FindString("etk:font", i, &str);

		eint32 escapeIndex = str.FindFirst("\t");
		if(escapeIndex <= 0 || escapeIndex >= str.Length() - 1) continue;

		EString fontFamily;
		EString fontStyle;

		str.CopyInto(fontFamily, 0, escapeIndex);
		str.CopyInto(fontStyle, escapeIndex + 1, -1);

		EFontWin32 *engine = new EFontWin32(this, fontFamily.String(), fontStyle.String());
		if(engine == NULL || engine->IsValid() == false)
		{
			if(engine) delete engine;
		}
		else
		{
			if(etk_font_add(engine->Family(), engine->Style(), engine))
				retVal = E_OK;
			else
				delete engine;
		}
	}

	ETK_DEBUG("[GRAPHICS]: GDI32 core fonts updated.");

	return retVal;
}

#endif /* WIN32 */
