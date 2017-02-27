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
 * File: etk-x11-font.cpp
 *
 * --------------------------------------------------------------------------*/
#ifdef LINUX

#include "etk-x11.h"

#if 1
// "Xutf8*" functions don't work well enough
#undef X_HAVE_UTF8_STRING
#endif

#include "etk-x11.h"

#include "../add-ons/font/FontEngine.h"
#include "../support/ClassInfo.h"
#include "../support/List.h"
#include "../support/Autolock.h"
#include "../interface/Window.h"
#include "../interface/View.h"


class BFontX11 : public BFontEngine {
public:
	BFontX11(EXGraphicsEngine *x11Engine, const char *xFontname);
#ifdef HAVE_XFT
	BFontX11(EXGraphicsEngine *x11Engine, const char *xftFontFamily, const char *xftFontStyle);
#endif
	virtual ~BFontX11();

	virtual bool IsValid() const;

	virtual bool IsScalable() const;

#ifdef HAVE_XFT
	virtual void ForceFontAliasing(bool enable);
#endif

	virtual float StringWidth(const char *string, float size, float spacing, float shear, bool bold, b_int32 length) const;
	virtual void GetHeight(bhapi::font_height *height, float size, float shear, bool bold) const;
	virtual BRect RenderString(BHandler *view, const char *string, float size, float spacing, float shear, bool bold, b_int32 length);

	virtual bhapi::font_detach_callback* Attach(void (*callback)(void*), void *data);
	virtual bool Detach(bhapi::font_detach_callback *callback);

private:
	typedef struct _font_data_ {
#ifdef HAVE_XFT
		bool xft;
#endif
		int xFontsize;
		void *xFontset;
	} _font_data_;

	BStringArray fFonts;
	bool fScalable;
#ifdef HAVE_XFT
	bool fForceFontAliasing;
#endif

	BFontX11::_font_data_ *fCurFontData;

	EXGraphicsEngine *fX11Engine;
};


static BFontEngine* _bhapi_get_x_font_engine(EXGraphicsEngine *x11Engine, const BString &family, const BString &style)
{
	if(x11Engine == NULL) return NULL;

	BAutolock <EXGraphicsEngine> autolock(x11Engine);
	if(autolock.IsLocked() == false || x11Engine->InitCheck() != B_OK) return NULL;

	b_int32 index = x11Engine->xFontEngines.FindString(family, x11Engine->xFontEngines.CountItems() - 1, true, true);
	if(index < 0) return NULL;

	BList* engine_list = NULL;
	x11Engine->xFontEngines.ItemAt(index, (void**)&engine_list);
	if(!engine_list) return NULL;

	for(b_int32 i = engine_list->CountItems() - 1; i >= 0; i--)
	{
		BFontX11* engine = (BFontX11*)engine_list->ItemAt(i);
		if(family != engine->Family() || style != engine->Style()) continue;
		return engine;
	}

	return NULL;
}


static bool _bhapi_add_x_font_engine(EXGraphicsEngine *x11Engine, BFontX11 *engine)
{
	if(x11Engine == NULL || engine == NULL || engine->Family() == NULL || engine->Style() == NULL) return false;

	BAutolock <EXGraphicsEngine> autolock(x11Engine);
	if(autolock.IsLocked() == false || x11Engine->InitCheck() != B_OK) return false;

	b_int32 index = x11Engine->xFontEngines.FindString(engine->Family(), x11Engine->xFontEngines.CountItems() - 1, true, true);
	if(index >= 0)
	{
		BList* engine_list = NULL;
		x11Engine->xFontEngines.ItemAt(index, (void**)&engine_list);
		if(!engine_list) return false;

		if(engine_list->AddItem(engine) == false) return false;
		if(bhapi::font_add(engine->Family(), engine->Style(), engine)) return true;
		engine_list->RemoveItem(engine_list->CountItems() - 1);
		return false;
	}

	BList *engine_list = new BList;
	if(!engine_list) return false;

	if(x11Engine->xFontEngines.AddItem(engine->Family(), engine_list) == false ||
	   engine_list->AddItem(engine) == false ||
	   bhapi::font_add(engine->Family(), engine->Style(), engine) == false)
	{
		if(engine_list->CountItems() > 0) x11Engine->xFontEngines.RemoveItem(x11Engine->xFontEngines.CountItems() - 1);
		delete engine_list;
		return false;
	}

	return true;
}


BFontX11::BFontX11(EXGraphicsEngine *x11Engine, const char *xFontname)
	: BFontEngine(), fScalable(false), fCurFontData(NULL), fX11Engine(NULL)
{
	if(x11Engine == NULL || xFontname == NULL) return;

	fX11Engine = x11Engine;

	BAutolock <EXGraphicsEngine> autolock(fX11Engine);
	if(autolock.IsLocked() == false || fX11Engine->InitCheck() != B_OK) {fX11Engine = NULL; return;}

	BString strFontname(xFontname);
	BStringArray *array = strFontname.Split('-');
	int pxlsz = 0;
#if 0
	if(!array || array->CountItems() != 15 || array->ItemAt(7)->GetInteger(&pxlsz) == false || pxlsz < 0)
#else
	if(!array || array->CountItems() != 15 || array->ItemAt(7)->GetInteger(&pxlsz) == false || pxlsz <= 0)
#endif
	{
		if(array) delete array;
		return;
	}
	strFontname.Remove(0, strFontname.FindFirst("-", 1));
	strFontname.Remove(strFontname.FindLast("-"), -1);
	strFontname.Remove(strFontname.FindLast("-"), -1);
	strFontname.Remove(strFontname.FindLast("-"), -1);
	strFontname.Remove(strFontname.FindLast("-"), -1);
	strFontname.Remove(strFontname.FindLast("-"), -1);
	strFontname.Remove(strFontname.FindLast("-"), -1);
	strFontname.Remove(strFontname.FindLast("-"), -1);
	strFontname.Prepend("-*");
	strFontname.Append("-*-*-*-*-*-*-*");

	BString family, style;
	family.Append(array->ItemAt(2)->String() ? array->ItemAt(2)->String() : "Unknown");
	style.Append(array->ItemAt(3)->String() ? array->ItemAt(3)->String() : "Unknown");

	delete array;

	BFontX11 *engine = b_cast_as(_bhapi_get_x_font_engine(fX11Engine, family, style), BFontX11);
	if(engine)
	{
		if(pxlsz == 0 && engine->fScalable) return;

		BFontX11::_font_data_ *font_data = new BFontX11::_font_data_;
		if(!(!font_data || !engine->fFonts.AddItem(strFontname, (void*)font_data)))
		{
#ifdef HAVE_XFT
			font_data->xft = false;
#endif
			font_data->xFontsize = pxlsz;
			font_data->xFontset = NULL;

			if(pxlsz == 0)
			{
				engine->fScalable = true;
			}
			else
			{
				float *font_sizes = new float[engine->fFonts.CountItems()];
				if(font_sizes)
				{
					b_int32 nSizes = 0;
					for(b_int32 i = 0; i < engine->fFonts.CountItems(); i++)
					{
						BFontX11::_font_data_ *_font_data = NULL;
						engine->fFonts.ItemAt(i, (void**)&_font_data);
						if(_font_data == NULL) continue;
						if(_font_data->xFontsize <= 0) continue;
						if(_font_data->xFontsize == pxlsz && i != engine->fFonts.CountItems() - 1)
						{
							delete[] font_sizes;
							engine->fFonts.RemoveItem(engine->fFonts.CountItems() - 1);
							delete font_data;
							return;
						}
						font_sizes[nSizes++] = (float)_font_data->xFontsize;
					}

					engine->SetFixedSize(font_sizes, nSizes);
					delete[] font_sizes;
				}
				else
				{
					engine->fFonts.RemoveItem(engine->fFonts.CountItems() - 1);
					delete font_data;
				}
			}
		}
		else
		{
			if(font_data) delete font_data;
		}

		return;
	}

	BFontX11::_font_data_ *font_data = new BFontX11::_font_data_;
	if(!font_data) return;

#ifdef HAVE_XFT
	font_data->xft = false;
#endif
	font_data->xFontsize = pxlsz;
	font_data->xFontset = NULL;
	if(!fFonts.AddItem(strFontname, (void*)font_data))
	{
		delete font_data;
		return;
	}

	SetFamily(family.String());
	SetStyle(style.String());

	if(pxlsz == 0)
	{
		fScalable = true;
	}
	else
	{
		float fontSize = (float)pxlsz;
		SetFixedSize(&fontSize, 1);
	}

#ifdef HAVE_XFT
	fForceFontAliasing = false;
#endif

	SetRenderMode(B_FONT_RENDER_DIRECTLY);
}


#ifdef HAVE_XFT
BFontX11::BFontX11(EXGraphicsEngine *x11Engine, const char *xftFontFamily, const char *xftFontStyle)
	: BFontEngine(), fScalable(false), fForceFontAliasing(false), fCurFontData(NULL), fX11Engine(NULL)
{
	if(x11Engine == NULL || xftFontFamily == NULL || xftFontStyle == NULL) return;
	if(*xftFontFamily == 0 || *xftFontStyle == 0) return;

	fX11Engine = x11Engine;

	BAutolock <EXGraphicsEngine> autolock(fX11Engine);
	if(autolock.IsLocked() == false || fX11Engine->InitCheck() != B_OK) {fX11Engine = NULL; return;}

	XftFontSet *fs = XftListFonts(fX11Engine->xDisplay, fX11Engine->xScreen,
				      XFT_FAMILY, XftTypString, xftFontFamily, XFT_STYLE, XftTypString, xftFontStyle, 0,
				      XFT_PIXEL_SIZE, 0);
	if(fs == NULL || fs->nfont <= 0 || fs->fonts == NULL)
	{
		if(fs) XftFontSetDestroy(fs);
		return;
	}

	BFontX11 *engine = b_cast_as(_bhapi_get_x_font_engine(fX11Engine, xftFontFamily, xftFontStyle), BFontX11);
	if(engine)
	{
		for(int i = 0; i < fs->nfont; i++)
		{
			double fontSize = 0;
			XftPatternGetDouble(fs->fonts[i], XFT_PIXEL_SIZE, 0, &fontSize);

			if((int)fontSize <= 0 && engine->fScalable) continue;
			if((int)fontSize <= 0) fontSize = 0;

			BFontX11::_font_data_ *font_data = new BFontX11::_font_data_;
			BString strFontname;
			if(!(!font_data || !engine->fFonts.AddItem(NULL, (void*)font_data)))
			{
				font_data->xft = true;
				font_data->xFontsize = (int)fontSize;
				font_data->xFontset = NULL;

				if((int)fontSize <= 0)
				{
					engine->fScalable = true;
				}
				else
				{
					float *font_sizes = new float[engine->fFonts.CountItems()];
					if(font_sizes)
					{
						b_int32 nSizes = 0;
						for(b_int32 j = 0; j < engine->fFonts.CountItems(); j++)
						{
							BFontX11::_font_data_ *_font_data = NULL;
							engine->fFonts.ItemAt(j, (void**)&_font_data);
							if(_font_data == NULL) continue;
							if(_font_data->xFontsize <= 0) continue;
							if(_font_data->xFontsize == (int)fontSize && j != engine->fFonts.CountItems() - 1)
							{
								delete[] font_sizes;
								engine->fFonts.RemoveItem(engine->fFonts.CountItems() - 1);
								delete font_data;
								font_data = NULL;
								break;
							}
							font_sizes[nSizes++] = (float)_font_data->xFontsize;
						}

						if(!font_data) continue;

						engine->SetFixedSize(font_sizes, nSizes);
						delete[] font_sizes;
					}
					else
					{
						engine->fFonts.RemoveItem(engine->fFonts.CountItems() - 1);
						delete font_data;
					}
				}
			}
			else
			{
				if(font_data) delete font_data;
			}
		}

		XftFontSetDestroy(fs);
		return;
	}

	float fontSizes[fs->nfont];
	b_int32 nSizes = 0;
	for(int i = 0; i < fs->nfont; i++)
	{
		double fontSize = 0;
		XftPatternGetDouble(fs->fonts[i], XFT_PIXEL_SIZE, 0, &fontSize);

		if((int)fontSize <= 0 && fScalable) continue;
		if((int)fontSize <= 0) fontSize = 0;

		BFontX11::_font_data_ *font_data = new BFontX11::_font_data_;
		if(!(!font_data || !fFonts.AddItem(NULL, (void*)font_data)))
		{
			font_data->xft = true;
			font_data->xFontsize = (int)fontSize;
			font_data->xFontset = NULL;

			if((int)fontSize <= 0)
				fScalable = true;
			else
				fontSizes[nSizes++] = (float)((int)fontSize);
		}
		else
		{
			if(font_data) delete font_data;
		}
	}

	XftFontSetDestroy(fs);

	SetFamily(xftFontFamily);
	SetStyle(xftFontStyle);
	if(nSizes > 0) SetFixedSize(fontSizes, nSizes);

	SetRenderMode(B_FONT_RENDER_DIRECTLY);

	return;
}
#endif


BFontX11::~BFontX11()
{
	if(fX11Engine != NULL)
	{
		BAutolock <EXGraphicsEngine> autolock(fX11Engine);
		if(autolock.IsLocked() == false) BHAPI_ERROR("[GRAPHICS]: %s --- Invalid graphics engine.", __PRETTY_FUNCTION__);

		bool xRunning = (fX11Engine->InitCheck() == B_OK);

		for(b_int32 i = 0; i < fFonts.CountItems(); i++)
		{
			BFontX11::_font_data_ *font_data = NULL;
			fFonts.ItemAt(i, (void**)&font_data);
			if(!(!font_data || font_data->xFontset == NULL) && xRunning)
			{
#ifdef HAVE_XFT
				if(font_data->xft) XftFontClose(fX11Engine->xDisplay, (XftFont*)(font_data->xFontset));
				else XFreeFontSet(fX11Engine->xDisplay, (XFontSet)(font_data->xFontset));
#else
				XFreeFontSet(fX11Engine->xDisplay, (XFontSet)(font_data->xFontset));
#endif
			}
			delete font_data;
		}

		fFonts.MakeEmpty();

		// TODO: remove self from xFontEngines
	}
}


bhapi::font_detach_callback*
BFontX11::Attach(void (*callback)(void*), void *data)
{
	if(fX11Engine == NULL) return NULL;

	BAutolock <EXGraphicsEngine> autolock(fX11Engine);
	if(autolock.IsLocked() == false || fX11Engine->InitCheck() != B_OK) return NULL;

	if(fFonts.CountItems() <= 0) return NULL;

	return BFontEngine::Attach(callback, data);
}


bool
BFontX11::Detach(bhapi::font_detach_callback *callback)
{
	if(fX11Engine == NULL) return false;

	BAutolock <EXGraphicsEngine> autolock(fX11Engine);
	if(autolock.IsLocked() == false) return false;

	if(!BFontEngine::Detach(callback)) return false;

	if(!IsAttached() && fFonts.CountItems() > 0)
	{
		bool xRunning = (fX11Engine->InitCheck() == B_OK);

		for(b_int32 i = 0; i < fFonts.CountItems(); i++)
		{
			BFontX11::_font_data_ *font_data = NULL;
			fFonts.ItemAt(i, (void**)&font_data);
			if(!font_data) continue;

			if(font_data->xFontset != NULL && xRunning)
			{
#ifdef HAVE_XFT
				if(font_data->xft) XftFontClose(fX11Engine->xDisplay, (XftFont*)(font_data->xFontset));
				else XFreeFontSet(fX11Engine->xDisplay, (XFontSet)(font_data->xFontset));
#else
				XFreeFontSet(fX11Engine->xDisplay, (XFontSet)(font_data->xFontset));
#endif
			}
			font_data->xFontset = NULL;
		}
	}

	return true;
}


bool
BFontX11::IsValid() const
{
	return(fX11Engine != NULL && fFonts.CountItems() > 0 && Family() != NULL && Style() != NULL);
}


bool
BFontX11::IsScalable() const
{
	return fScalable;
}


#ifdef HAVE_XFT
void
BFontX11::ForceFontAliasing(bool enable)
{
	if(fX11Engine == NULL) return;

	BAutolock <EXGraphicsEngine> autolock(fX11Engine);
	if(autolock.IsLocked() == false || fX11Engine->InitCheck() != B_OK) return;

	if(fForceFontAliasing != enable)
	{
		fForceFontAliasing = enable;

		BFontX11::_font_data_ *font_data = fCurFontData;
		if(font_data == NULL || font_data->xft == false || font_data->xFontset == NULL) return;

		double size = (double)font_data->xFontsize;
		if(size < 0) size = -size;
		XftFont *newFont = XftFontOpen(fX11Engine->xDisplay, fX11Engine->xScreen,
					       XFT_FAMILY, XftTypString, Family(),
					       XFT_STYLE, XftTypString, Style(),
					       XFT_PIXEL_SIZE, XftTypeDouble, size,
					       XFT_CORE, XftTypeBool, True,
					       XFT_ANTIALIAS, XftTypeBool, (Bool)!fForceFontAliasing, 0);
		if(newFont != NULL)
		{
			XftFontClose(fX11Engine->xDisplay, (XftFont*)font_data->xFontset);
			font_data->xFontset = (XftFont*)newFont;
		}
	}
}
#endif


float
BFontX11::StringWidth(const char *string, float size, float spacing, float shear, bool bold, b_int32 length) const
{
	if(fX11Engine == NULL) return 0;
	if((int)size <= 0 || string == NULL || *string == 0 || length == 0 || !IsAttached() || fFonts.CountItems() <= 0) return 0;

	BAutolock <EXGraphicsEngine> autolock(fX11Engine);
	if(autolock.IsLocked() == false || fX11Engine->InitCheck() != B_OK) return 0;

	BFontX11::_font_data_ *curFont = NULL;
	if(fCurFontData == NULL || fCurFontData->xFontset == NULL ||
	   !(fCurFontData->xFontsize == (int)size || (fScalable && -(fCurFontData->xFontsize) == (int)size)))
	{
		b_int32 scalable_index = -1;
		for(b_int32 i = 0; i < fFonts.CountItems(); i++)
		{
			BFontX11::_font_data_ *font_data = NULL;
			const BString *str = fFonts.ItemAt(i, (void**)&font_data);
			if(!font_data) continue;
#ifdef HAVE_XFT
			if((str == NULL || str->Length() <= 0) && font_data->xft == false) continue;
#else
			if(str == NULL || str->Length() <= 0) continue;
#endif

			if(font_data->xFontsize <= 0)
			{
				scalable_index = i;
				continue;
			}

			if(font_data->xFontsize == (int)size)
			{
				if(font_data->xFontset == NULL)
				{
#ifdef HAVE_XFT
					if(font_data->xft)
					{
						font_data->xFontset = (void*)XftFontOpen(fX11Engine->xDisplay, fX11Engine->xScreen,
											 XFT_FAMILY, XftTypString, Family(),
											 XFT_STYLE, XftTypString, Style(),
											 XFT_PIXEL_SIZE, XftTypeDouble, (double)size,
											 XFT_CORE, XftTypeBool, True,
											 XFT_ANTIALIAS, XftTypeBool, (Bool)!fForceFontAliasing, 0);
					}
					else
					{
#endif
						char **lists = NULL;
						int count = 0;
						font_data->xFontset = (void*)XCreateFontSet(fX11Engine->xDisplay, str->String(),
											    &lists, &count, NULL);
						if(lists != NULL) XFreStringList(lists);
#ifdef HAVE_XFT
					}
#endif
				}
				curFont = font_data;
				break;
			}
		}
		if(curFont == NULL && scalable_index >= 0)
		{
			BFontX11::_font_data_ *font_data = NULL;
			const BString *str = fFonts.ItemAt(scalable_index, (void**)&font_data);
#ifdef HAVE_XFT
			if(!(font_data == NULL || (font_data->xft == false && (str == NULL || str->Length() <= 0))))
#else
			if(!(font_data == NULL || str == NULL || str->Length() <= 0))
#endif
			{
				if(-(font_data->xFontsize) != (int)size || font_data->xFontset == NULL)
				{
					font_data->xFontsize = -((int)size);
#ifdef HAVE_XFT
					if(font_data->xft)
					{
						if(font_data->xFontset) XftFontClose(fX11Engine->xDisplay, (XftFont*)font_data->xFontset);
						font_data->xFontset = (void*)XftFontOpen(fX11Engine->xDisplay, fX11Engine->xScreen,
											 XFT_FAMILY, XftTypString, Family(),
											 XFT_STYLE, XftTypString, Style(),
											 XFT_PIXEL_SIZE, XftTypeDouble, (double)size,
											 XFT_CORE, XftTypeBool, True,
											 XFT_ANTIALIAS, XftTypeBool, (Bool)!fForceFontAliasing, 0);
					}
					else
					{
#endif
						BString aStr = str->String();
						BString bStr = "-";
						bStr << (int)size;
						aStr.ReplaceLast("-0", bStr.String());
						char **lists = NULL;
						int count = 0;
						if(font_data->xFontset) XFreeFontSet(fX11Engine->xDisplay, (XFontSet)(font_data->xFontset));
						font_data->xFontset = (void*)XCreateFontSet(fX11Engine->xDisplay, aStr.String(), &lists, &count, NULL);
						if(lists != NULL) XFreStringList(lists);
#ifdef HAVE_XFT
					}
#endif
				}
				curFont = font_data;
			}
		}
	}
	else
	{
		curFont = fCurFontData;
	}

	if(!curFont) return 0;

#ifdef HAVE_XFT
	if(curFont->xft)
	{
		XftFont *xfs = (XftFont*)curFont->xFontset;
		if(xfs == NULL) return 0;

		float width = 0;
		if(length < 0 || (size_t)length > strlen(string)) length = (b_int32)strlen(string);

		b_uint8 len = 0;
		const char *tmp = bhapi::utf8_at(string, 0, &len);
		float height = (float)(xfs->ascent + xfs->descent);
		float delta = (float)ceil((double)(spacing * size));
		while(!(len == 0 || !tmp || *tmp == 0 || tmp - string > length - (b_int32)len))
		{
			XGlyphInfo glyph;
			XftTextExtentsUtf8(fX11Engine->xDisplay, xfs, (XftChar8*)tmp, (int)len, &glyph);
			int cWidth = (int)(glyph.xOff);
			width += (float)(cWidth > 0 ? cWidth : height) + (tmp == string ? 0.f : delta);
			tmp = bhapi::utf8_next(tmp, &len);
		}

		return width;
	}
#endif

	XFontSet xFontset = (XFontSet)curFont->xFontset;
	if(xFontset == NULL) return 0;

	XFontStruct **xFontStructs = NULL;
	char **names = NULL;
	int nFonts = 0;
	if((nFonts = XFontsOfFontSet(xFontset, &xFontStructs, &names)) <= 0 || xFontStructs == NULL) return 0;

	int ascent = 0, descent = 0;
	for(int i = 0; i < nFonts; i++)
	{
		ascent = max_c(xFontStructs[i]->max_bounds.ascent, ascent);
		descent = max_c(xFontStructs[i]->max_bounds.descent, descent);
	}

	float width = 0;
	if(length < 0 || (size_t)length > strlen(string)) length = (b_int32)strlen(string);
	float height = (float)(ascent + descent);
	float delta = (float)ceil((double)(spacing * size));

#ifdef X_HAVE_UTF8_STRING
	b_uint8 len = 0;
	const char *tmp = bhapi::utf8_at(string, 0, &len);
	while(!(len == 0 || !tmp || *tmp == 0 || tmp - string > length - (b_int32)len))
	{
		int cWidth = Xutf8TextEscapement(xFontset, (char*)tmp, (int)len);
		width += (float)(cWidth > 0 ? cWidth : (int)height) + (tmp == string ? 0.f : delta);
		tmp = bhapi::utf8_next(tmp, &len);
	}
#else
	b_unichar32 *utf32 = bhapi::utf8_convert_to_utf32(string, length);
	if(utf32)
	{
		const b_unichar32 *tmp = bhapi::utf32_at(utf32, 0);
		while(!(!tmp || *tmp == 0))
		{
			int cWidth = XwcTextEscapement(xFontset, (wchar_t*)tmp, 1);
			width += (float)(cWidth > 0 ? cWidth : (int)height) + (tmp == (const b_unichar32*)utf32 ? 0.f : delta);
			tmp = bhapi::utf32_next(tmp);
		}
		free(utf32);
	}
#endif

	return width;
}


void
BFontX11::GetHeight(bhapi::font_height *height, float size, float shear, bool bold) const
{
	if(fX11Engine == NULL || height == NULL) return;

	bzero(height, sizeof(bhapi::font_height));

	BAutolock <EXGraphicsEngine> autolock(fX11Engine);
	if(autolock.IsLocked() == false || fX11Engine->InitCheck() != B_OK) return;

	if((int)size <= 0 || !IsAttached() || fFonts.CountItems() <= 0) return;

	BFontX11::_font_data_ *curFont = NULL;
	if(fCurFontData == NULL || fCurFontData->xFontset == NULL ||
	   !(fCurFontData->xFontsize == (int)size || (fScalable && -(fCurFontData->xFontsize) == (int)size)))
	{
		b_int32 scalable_index = -1;
		for(b_int32 i = 0; i < fFonts.CountItems(); i++)
		{
			BFontX11::_font_data_ *font_data = NULL;
			const BString *str = fFonts.ItemAt(i, (void**)&font_data);
			if(!font_data) continue;
#ifdef HAVE_XFT
			if((str == NULL || str->Length() <= 0) && font_data->xft == false) continue;
#else
			if(str == NULL || str->Length() <= 0) continue;
#endif

			if(font_data->xFontsize <= 0)
			{
				scalable_index = i;
				continue;
			}

			if(font_data->xFontsize == (int)size)
			{
				if(font_data->xFontset == NULL)
				{
#ifdef HAVE_XFT
					if(font_data->xft)
					{
						font_data->xFontset = (void*)XftFontOpen(fX11Engine->xDisplay, fX11Engine->xScreen,
											 XFT_FAMILY, XftTypString, Family(),
											 XFT_STYLE, XftTypString, Style(),
											 XFT_PIXEL_SIZE, XftTypeDouble, (double)size,
											 XFT_CORE, XftTypeBool, True,
											 XFT_ANTIALIAS, XftTypeBool, (Bool)!fForceFontAliasing, 0);
					}
					else
					{
#endif
						char **lists = NULL;
						int count = 0;
						font_data->xFontset = (void*)XCreateFontSet(fX11Engine->xDisplay, str->String(),
											    &lists, &count, NULL);
						if(lists != NULL) XFreStringList(lists);
#ifdef HAVE_XFT
					}
#endif
				}
				curFont = font_data;
				break;
			}
		}
		if(curFont == NULL && scalable_index >= 0)
		{
			BFontX11::_font_data_ *font_data = NULL;
			const BString *str = fFonts.ItemAt(scalable_index, (void**)&font_data);
#ifdef HAVE_XFT
			if(!(font_data == NULL || (font_data->xft == false && (str == NULL || str->Length() <= 0))))
#else
			if(!(font_data == NULL || str == NULL || str->Length() <= 0))
#endif
			{
				if(-(font_data->xFontsize) != (int)size || font_data->xFontset == NULL)
				{
					font_data->xFontsize = -((int)size);
#ifdef HAVE_XFT
					if(font_data->xft)
					{
						if(font_data->xFontset) XftFontClose(fX11Engine->xDisplay, (XftFont*)font_data->xFontset);
						font_data->xFontset = (void*)XftFontOpen(fX11Engine->xDisplay, fX11Engine->xScreen,
											 XFT_FAMILY, XftTypString, Family(),
											 XFT_STYLE, XftTypString, Style(),
											 XFT_PIXEL_SIZE, XftTypeDouble, (double)size,
											 XFT_CORE, XftTypeBool, True,
											 XFT_ANTIALIAS, XftTypeBool, (Bool)!fForceFontAliasing, 0);
					}
					else
					{
#endif
						BString aStr = str->String();
						BString bStr = "-";
						bStr << (int)size;
						aStr.ReplaceLast("-0", bStr.String());
						char **lists = NULL;
						int count = 0;
						if(font_data->xFontset) XFreeFontSet(fX11Engine->xDisplay, (XFontSet)(font_data->xFontset));
						font_data->xFontset = (void*)XCreateFontSet(fX11Engine->xDisplay, aStr.String(), &lists, &count, NULL);
						if(lists != NULL) XFreStringList(lists);
#ifdef HAVE_XFT
					}
#endif
				}
				curFont = font_data;
			}
		}
	}
	else
	{
		curFont = fCurFontData;
	}

	if(!curFont) return;

#ifdef HAVE_XFT
	if(curFont->xft)
	{
		XftFont *xfs = (XftFont*)curFont->xFontset;
		if(xfs == NULL) return;
		height->ascent = (float)xfs->ascent;
		height->descent = (float)xfs->descent;
		height->leading = 0;
		return;
	}
#endif

	XFontSet xFontset = (XFontSet)curFont->xFontset;
	if(xFontset == NULL) return;

	XFontStruct **xFontStructs = NULL;
	char **names = NULL;
	int nFonts = 0;
	if(!((nFonts = XFontsOfFontSet(xFontset, &xFontStructs, &names)) <= 0 || xFontStructs == NULL))
	{
		for(int i = 0; i < nFonts; i++)
		{
			height->ascent = max_c((float)(xFontStructs[i]->max_bounds.ascent), height->ascent);
			height->descent = max_c((float)(xFontStructs[i]->max_bounds.descent), height->descent);
		}
		height->leading = 0;
	}
}


// TODO: print, drawing op when use xft
BRect
BFontX11::RenderString(BHandler *_view, const char *string, float size, float spacing, float shear, bool bold, b_int32 length)
{
	if(fX11Engine == NULL || (int)size <= 0 || string == NULL || *string == 0 || length == 0) return BRect();

	BView *view = b_cast_as(_view, BView);
	if(view == NULL || view->Window() == NULL || view->IsPrinting()) return BRect();

	BRegion viewClipping;
	view->GetClippingRegion(&viewClipping);
	if(viewClipping.CountRects() <= 0) return BRect();

	BAutolock <EXGraphicsEngine> autolock(fX11Engine);
	if(autolock.IsLocked() == false || fX11Engine->InitCheck() != B_OK) return BRect();

	EXGraphicsDrawable *pix = b_cast_as(BGraphicsEngine::GetPixmap(view->Window()), EXGraphicsDrawable);
	EXGraphicsContext *dc = b_cast_as(BGraphicsEngine::GetContext(view), EXGraphicsContext);
	if(pix == NULL || dc == NULL) return BRect();

	if(!IsAttached() || fFonts.CountItems() <= 0) return BRect();

	BFontX11::_font_data_ *curFont = NULL;
	if(fCurFontData == NULL || fCurFontData->xFontset == NULL ||
	   !(fCurFontData->xFontsize == (int)size || (fScalable && -(fCurFontData->xFontsize) == (int)size)))
	{
		b_int32 scalable_index = -1;
		for(b_int32 i = 0; i < fFonts.CountItems(); i++)
		{
			BFontX11::_font_data_ *font_data = NULL;
			const BString *str = fFonts.ItemAt(i, (void**)&font_data);
			if(!font_data) continue;
#ifdef HAVE_XFT
			if((str == NULL || str->Length() <= 0) && font_data->xft == false) continue;
#else
			if(str == NULL || str->Length() <= 0) continue;
#endif

			if(font_data->xFontsize <= 0)
			{
				scalable_index = i;
				continue;
			}

			if(font_data->xFontsize == (int)size)
			{
				if(font_data->xFontset == NULL)
				{
#ifdef HAVE_XFT
					if(font_data->xft)
					{
						font_data->xFontset = (void*)XftFontOpen(fX11Engine->xDisplay, fX11Engine->xScreen,
											 XFT_FAMILY, XftTypString, Family(),
											 XFT_STYLE, XftTypString, Style(),
											 XFT_PIXEL_SIZE, XftTypeDouble, (double)size,
											 XFT_CORE, XftTypeBool, True,
											 XFT_ANTIALIAS, XftTypeBool, (Bool)!fForceFontAliasing, 0);
						if(font_data->xFontset == NULL)
							BHAPI_WARNING("[GRAPHICS]: XftFontOpen \"%s-%s-%d\" failed.",
								    Family(), Style(), (int)size);
					}
					else
					{
#endif
						char **lists = NULL;
						int count = 0;
						font_data->xFontset = (void*)XCreateFontSet(fX11Engine->xDisplay, str->String(),
											    &lists, &count, NULL);
						if(font_data->xFontset == NULL)
							BHAPI_WARNING("[GRAPHICS]: XCreateFontSet \"%s\" failed.", str->String());
						if(lists != NULL) XFreStringList(lists);
#ifdef HAVE_XFT
					}
#endif
				}
				fCurFontData = font_data;
				curFont = fCurFontData;
				break;
			}
		}
		if(curFont == NULL && scalable_index >= 0)
		{
			BFontX11::_font_data_ *font_data = NULL;
			const BString *str = fFonts.ItemAt(scalable_index, (void**)&font_data);
#ifdef HAVE_XFT
			if(!(font_data == NULL || (font_data->xft == false && (str == NULL || str->Length() <= 0))))
#else
			if(!(font_data == NULL || str == NULL || str->Length() <= 0))
#endif
			{
				if(-(font_data->xFontsize) != (int)size || font_data->xFontset == NULL)
				{
					font_data->xFontsize = -((int)size);
#ifdef HAVE_XFT
					if(font_data->xft)
					{
						if(font_data->xFontset) XftFontClose(fX11Engine->xDisplay, (XftFont*)font_data->xFontset);
						font_data->xFontset = (void*)XftFontOpen(fX11Engine->xDisplay, fX11Engine->xScreen,
											 XFT_FAMILY, XftTypString, Family(),
											 XFT_STYLE, XftTypString, Style(),
											 XFT_PIXEL_SIZE, XftTypeDouble, (double)size,
											 XFT_CORE, XftTypeBool, True,
											 XFT_ANTIALIAS, XftTypeBool, (Bool)!fForceFontAliasing, 0);
						if(font_data->xFontset == NULL)
							BHAPI_WARNING("[GRAPHICS]: XftFontOpen \"%s-%s-%d\" failed.",
								    Family(), Style(), (int)size);
					}
					else
					{
#endif
						BString aStr = str->String();
						BString bStr = "-";
						bStr << (int)size;
						aStr.ReplaceLast("-0", bStr.String());
						char **lists = NULL;
						int count = 0;
						if(font_data->xFontset) XFreeFontSet(fX11Engine->xDisplay, (XFontSet)(font_data->xFontset));
						font_data->xFontset = (void*)XCreateFontSet(fX11Engine->xDisplay, aStr.String(),
											    &lists, &count, NULL);
						if(font_data->xFontset == NULL)
							BHAPI_WARNING("[GRAPHICS]: XCreateFontSet \"%s\" failed.", str->String());
						if(lists != NULL) XFreStringList(lists);
#ifdef HAVE_XFT
					}
#endif
				}
				fCurFontData = font_data;
				curFont = fCurFontData;
			}
		}
	}
	else
	{
		curFont = fCurFontData;
	}

	if(!curFont) return BRect();

	dc->PrepareXColor();

#ifdef HAVE_XFT
	if(curFont->xft)
	{
		XftFont *xfs = (XftFont*)curFont->xFontset;
		if(xfs == NULL || pix->xDraw == NULL) return BRect();

		if(length < 0 || (size_t)length > strlen(string)) length = (b_int32)strlen(string);

		BPoint pt = view->ConvertToWindow(view->PenLocation()).FloorSelf();
		b_uint8 len = 0;
		const char *tmp = bhapi::utf8_at(string, 0, &len);
		int x = (int)pt.x;
		int y = (int)pt.y;
		float width = 0;
		float height = (float)(xfs->ascent + xfs->descent);
		int delta = (int)ceil((double)(spacing * size));

		XftColor xftcolor;
		Region xRegion = NULL;
		if(dc->GetXClipping(&xRegion) == B_OK) XftDrawSetClip(pix->xDraw, xRegion);

		b_rgb_color color = dc->HighColor();

		dc->GetXHighColor(&(xftcolor.pixel));
		xftcolor.color.red = (unsigned short)color.red * 257;
		xftcolor.color.green = (unsigned short)color.green * 257;
		xftcolor.color.blue = (unsigned short)color.blue * 257;
		xftcolor.color.alpha = B_MAXUSHORT;

		while(!(len == 0 || !tmp || *tmp == 0 || tmp - string > length - (b_int32)len))
		{
			XGlyphInfo glyph;
			XftTextExtentsUtf8(fX11Engine->xDisplay, xfs, (XftChar8*)tmp, (int)len, &glyph);
			int cWidth = (int)(glyph.xOff);

			if(cWidth > 0)
				XftDrawStringUtf8(pix->xDraw, &xftcolor, xfs, x, y, (XftChar8*)tmp, (int)len);
			else
				XDrawRectangle(fX11Engine->xDisplay, pix->xPixmap, dc->xGC, x + 2, y - xfs->ascent + 1,
					       (unsigned int)(max_c(height, 4) - 4), (unsigned int)(max_c(height, 4) - 4));

			x += (cWidth > 0 ? cWidth : (int)height) + delta;
			width += (float)(cWidth > 0 ? cWidth : (int)height) + (float)(tmp == string ? 0 : delta);
			tmp = bhapi::utf8_next(tmp, &len);
		}

//		XFlush(fX11Engine->xDisplay);

		BRect updateRect;
		updateRect.left = pt.x;
		updateRect.right = pt.x + width;
		updateRect.top = pt.y - (float)(xfs->ascent + 1);
		updateRect.bottom = updateRect.top + height;
		view->ConvertFromWindow(&updateRect);
		updateRect &= viewClipping.Frame();

		return updateRect;
	}
#endif

	XFontSet xFontset = (XFontSet)curFont->xFontset;
	if(xFontset == NULL) return BRect();

	XFontStruct **xFontStructs = NULL;
	char **names = NULL;
	int nFonts = 0;
	if((nFonts = XFontsOfFontSet(xFontset, &xFontStructs, &names)) <= 0 || xFontStructs == NULL) return BRect();

	int ascent = 0, descent = 0;
	for(int i = 0; i < nFonts; i++)
	{
		ascent = max_c(xFontStructs[i]->max_bounds.ascent, ascent);
		descent = max_c(xFontStructs[i]->max_bounds.descent, descent);
	}

	if(length < 0 || (size_t)length > strlen(string)) length = (b_int32)strlen(string);

	BPoint pt = view->ConvertToWindow(view->PenLocation()).FloorSelf();
	int x = (int)pt.x;
	int y = (int)pt.y;
	float width = 0;
	float height = (float)(ascent + descent);
	int delta = (int)ceil((double)(spacing * size));

#ifdef X_HAVE_UTF8_STRING
	b_uint8 len = 0;
	const char *tmp = bhapi::utf8_at(string, 0, &len);
	while(!(len == 0 || !tmp || *tmp == 0 || tmp - string > length - (b_int32)len))
	{
		int cWidth = Xutf8TextEscapement(xFontset, (char*)tmp, (int)len);
		if(cWidth > 0)
			Xutf8DrawString(fX11Engine->xDisplay, pix->xPixmap, xFontset, dc->xGC, x, y, (char*)tmp, (int)len);
		else
			XDrawRectangle(fX11Engine->xDisplay, pix->xPixmap, dc->xGC, x + 2, y - ascent + 1,
				       (unsigned int)(max_c(height, 4) - 4), (unsigned int)(max_c(height, 4) - 4));

		x += (cWidth > 0 ? cWidth : (int)height) + delta;
		width += (float)(cWidth > 0 ? cWidth : (int)height) + (float)(tmp == string ? 0 : delta);
		tmp = bhapi::utf8_next(tmp, &len);
	}
#else
	b_unichar32 *utf32 = bhapi::utf8_convert_to_utf32(string, length);
	if(utf32)
	{
		const b_unichar32 *tmp = bhapi::utf32_at(utf32, 0);
		while(!(!tmp || *tmp == 0))
		{
			int cWidth = XwcTextEscapement(xFontset, (wchar_t*)tmp, 1);
			if(cWidth > 0)
				XwcDrawString(fX11Engine->xDisplay, pix->xPixmap, xFontset, dc->xGC, x, y, (wchar_t*)tmp, 1);
			else
				XDrawRectangle(fX11Engine->xDisplay, pix->xPixmap, dc->xGC, x + 2, y - ascent + 1,
					       (unsigned int)(max_c(height, 4) - 4), (unsigned int)(max_c(height, 4) - 4));

			x += (cWidth > 0 ? cWidth : (int)height) + delta;
			width += (float)(cWidth > 0 ? cWidth : (int)height) + (float)(tmp == (const b_unichar32*)utf32 ? 0 : delta);
			tmp = bhapi::utf32_next(tmp);
		}
		free(utf32);
	}
#endif
//	XFlush(fX11Engine->xDisplay);

	BRect updateRect;
	updateRect.left = pt.x;
	updateRect.right = pt.x + width;
	updateRect.top = pt.y - (float)(ascent + 1);
	updateRect.bottom = updateRect.top + height;
	view->ConvertFromWindow(&updateRect);
	updateRect &= viewClipping.Frame();

	return updateRect;
}


b_status_t
EXGraphicsEngine::InitalizeFonts()
{
	BAutolock <EXGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false) return B_ERROR;

	return InitCheck();
}


void
EXGraphicsEngine::DestroyFonts()
{
	BAutolock <EXGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false) return;

	for(b_int32 i = 0; i < xFontEngines.CountItems(); i++)
	{
		BList* engine_list = NULL;
		xFontEngines.ItemAt(i, (void**)&engine_list);
		if(!engine_list) continue;
		for(b_int32 j = 0; j < engine_list->CountItems(); j++)
		{
			BFontX11 *engine = (BFontX11*)engine_list->ItemAt(j);
			if(!engine) continue;
			delete engine;
		}
		delete engine_list;
	}
	xFontEngines.MakeEmpty();
}


b_status_t
EXGraphicsEngine::UpdateFonts(bool check_only)
{
	BAutolock <EXGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false || InitCheck() != B_OK) return B_ERROR;

	if(check_only)
	{
		BHAPI_WARNING("[GRAPHICS]: %s --- check_only not implement yet.", __PRETTY_FUNCTION__);
		return B_ERROR;
	}

	b_status_t retVal = B_ERROR;

#ifdef HAVE_XFT
	BHAPI_DEBUG("[GRAPHICS]: Updating Xft fonts ...");

	XftFontSet *fs = XftListFonts(xDisplay, xScreen, 0, XFT_FAMILY, 0);
	if(!(fs == NULL || fs->nfont <= 0 || fs->fonts == NULL))
	{
		for(int i = 0; i < fs->nfont; i++)
		{
			char *fontName = NULL;
			if(XftPatternGetString(fs->fonts[i], XFT_FAMILY, 0, &fontName) != XftResultMatch || fontName == NULL) continue;

			XftFontSet *tfs = XftListFonts(xDisplay, xScreen, XFT_FAMILY, XftTypString, fontName, 0, XFT_STYLE, 0);
			if(!(tfs == NULL || tfs->nfont <= 0 || tfs->fonts == NULL))
			{
				for(int j = 0; j < tfs->nfont; j++)
				{
					char *fontStyle = NULL;
					if(XftPatternGetString(tfs->fonts[j], XFT_STYLE, 0, &fontStyle) != XftResultMatch) continue;
					if(!fontStyle) continue;

					BFontX11 *engine = new BFontX11(this, fontName, fontStyle);
					if(!engine || !engine->IsValid())
					{
						if(engine) delete engine;
						continue;
					}
					if(!_bhapi_add_x_font_engine(this, engine)) delete engine;
				}
			}
			if(tfs) XftFontSetDestroy(tfs);
		}

		retVal = B_OK;
	}
	if(fs) XftFontSetDestroy(fs);

	BHAPI_DEBUG("[GRAPHICS]: Xft fonts updated.");
#endif

	BHAPI_DEBUG("[GRAPHICS]: Updating X11 core fonts ...");

	int count = 0;
	char **fontNames = XListFonts(xDisplay, "-*-*-*-r-*-*-*-*-*-*-*-*-*-*", B_MAXINT, &count);

	if(fontNames && count > 0)
	{
		for(int i = 0; i < count; i++)
		{
			BFontX11 *engine = new BFontX11(this, fontNames[i]);
			if(!engine || !engine->IsValid())
			{
				if(engine) delete engine;
				continue;
			}
			if(!_bhapi_add_x_font_engine(this, engine)) delete engine;
		}

		retVal = B_OK;
	}

	if(fontNames) XFreeFontNames(fontNames);

	BHAPI_DEBUG("[GRAPHICS]: X11 core fonts updated.");

	return retVal;
}

#endif /* LINUX */
