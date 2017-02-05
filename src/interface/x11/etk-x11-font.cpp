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
 * File: etk-x11-font.cpp
 *
 * --------------------------------------------------------------------------*/

#include "etk-x11.h"

#if 1
// "Xutf8*" functions don't work well enough
#undef X_HAVE_UTF8_STRING
#endif

#include "etk-x11.h"

#include "./../add-ons/font/FontEngine.h>
#include "./../support/ClassInfo.h>
#include "./../support/List.h>
#include "./../support/Autolock.h>
#include "./../interface/Window.h>
#include "./../interface/View.h>


class EFontX11 : public EFontEngine {
public:
	EFontX11(EXGraphicsEngine *x11Engine, const char *xFontname);
#ifdef HAVE_XFT
	EFontX11(EXGraphicsEngine *x11Engine, const char *xftFontFamily, const char *xftFontStyle);
#endif
	virtual ~EFontX11();

	virtual bool IsValid() const;

	virtual bool IsScalable() const;

#ifdef HAVE_XFT
	virtual void ForceFontAliasing(bool enable);
#endif

	virtual float StringWidth(const char *string, float size, float spacing, float shear, bool bold, eint32 length) const;
	virtual void GetHeight(e_font_height *height, float size, float shear, bool bold) const;
	virtual ERect RenderString(EHandler *view, const char *string, float size, float spacing, float shear, bool bold, eint32 length);

	virtual e_font_detach_callback* Attach(void (*callback)(void*), void *data);
	virtual bool Detach(e_font_detach_callback *callback);

private:
	typedef struct _font_data_ {
#ifdef HAVE_XFT
		bool xft;
#endif
		int xFontsize;
		void *xFontset;
	} _font_data_;

	EStringArray fFonts;
	bool fScalable;
#ifdef HAVE_XFT
	bool fForceFontAliasing;
#endif

	EFontX11::_font_data_ *fCurFontData;

	EXGraphicsEngine *fX11Engine;
};


static EFontEngine* _etk_get_x_font_engine(EXGraphicsEngine *x11Engine, const EString &family, const EString &style)
{
	if(x11Engine == NULL) return NULL;

	EAutolock <EXGraphicsEngine> autolock(x11Engine);
	if(autolock.IsLocked() == false || x11Engine->InitCheck() != E_OK) return NULL;

	eint32 index = x11Engine->xFontEngines.FindString(family, x11Engine->xFontEngines.CountItems() - 1, true, true);
	if(index < 0) return NULL;

	EList* engine_list = NULL;
	x11Engine->xFontEngines.ItemAt(index, (void**)&engine_list);
	if(!engine_list) return NULL;

	for(eint32 i = engine_list->CountItems() - 1; i >= 0; i--)
	{
		EFontX11* engine = (EFontX11*)engine_list->ItemAt(i);
		if(family != engine->Family() || style != engine->Style()) continue;
		return engine;
	}

	return NULL;
}


static bool _etk_add_x_font_engine(EXGraphicsEngine *x11Engine, EFontX11 *engine)
{
	if(x11Engine == NULL || engine == NULL || engine->Family() == NULL || engine->Style() == NULL) return false;

	EAutolock <EXGraphicsEngine> autolock(x11Engine);
	if(autolock.IsLocked() == false || x11Engine->InitCheck() != E_OK) return false;

	eint32 index = x11Engine->xFontEngines.FindString(engine->Family(), x11Engine->xFontEngines.CountItems() - 1, true, true);
	if(index >= 0)
	{
		EList* engine_list = NULL;
		x11Engine->xFontEngines.ItemAt(index, (void**)&engine_list);
		if(!engine_list) return false;

		if(engine_list->AddItem(engine) == false) return false;
		if(etk_font_add(engine->Family(), engine->Style(), engine)) return true;
		engine_list->RemoveItem(engine_list->CountItems() - 1);
		return false;
	}

	EList *engine_list = new EList;
	if(!engine_list) return false;

	if(x11Engine->xFontEngines.AddItem(engine->Family(), engine_list) == false ||
	   engine_list->AddItem(engine) == false ||
	   etk_font_add(engine->Family(), engine->Style(), engine) == false)
	{
		if(engine_list->CountItems() > 0) x11Engine->xFontEngines.RemoveItem(x11Engine->xFontEngines.CountItems() - 1);
		delete engine_list;
		return false;
	}

	return true;
}


EFontX11::EFontX11(EXGraphicsEngine *x11Engine, const char *xFontname)
	: EFontEngine(), fScalable(false), fCurFontData(NULL), fX11Engine(NULL)
{
	if(x11Engine == NULL || xFontname == NULL) return;

	fX11Engine = x11Engine;

	EAutolock <EXGraphicsEngine> autolock(fX11Engine);
	if(autolock.IsLocked() == false || fX11Engine->InitCheck() != E_OK) {fX11Engine = NULL; return;}

	EString strFontname(xFontname);
	EStringArray *array = strFontname.Split('-');
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

	EString family, style;
	family.Append(array->ItemAt(2)->String() ? array->ItemAt(2)->String() : "Unknown");
	style.Append(array->ItemAt(3)->String() ? array->ItemAt(3)->String() : "Unknown");

	delete array;

	EFontX11 *engine = e_cast_as(_etk_get_x_font_engine(fX11Engine, family, style), EFontX11);
	if(engine)
	{
		if(pxlsz == 0 && engine->fScalable) return;

		EFontX11::_font_data_ *font_data = new EFontX11::_font_data_;
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
					eint32 nSizes = 0;
					for(eint32 i = 0; i < engine->fFonts.CountItems(); i++)
					{
						EFontX11::_font_data_ *_font_data = NULL;
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

	EFontX11::_font_data_ *font_data = new EFontX11::_font_data_;
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

	SetRenderMode(E_FONT_RENDER_DIRECTLY);
}


#ifdef HAVE_XFT
EFontX11::EFontX11(EXGraphicsEngine *x11Engine, const char *xftFontFamily, const char *xftFontStyle)
	: EFontEngine(), fScalable(false), fForceFontAliasing(false), fCurFontData(NULL), fX11Engine(NULL)
{
	if(x11Engine == NULL || xftFontFamily == NULL || xftFontStyle == NULL) return;
	if(*xftFontFamily == 0 || *xftFontStyle == 0) return;

	fX11Engine = x11Engine;

	EAutolock <EXGraphicsEngine> autolock(fX11Engine);
	if(autolock.IsLocked() == false || fX11Engine->InitCheck() != E_OK) {fX11Engine = NULL; return;}

	XftFontSet *fs = XftListFonts(fX11Engine->xDisplay, fX11Engine->xScreen,
				      XFT_FAMILY, XftTypeString, xftFontFamily, XFT_STYLE, XftTypeString, xftFontStyle, 0,
				      XFT_PIXEL_SIZE, 0);
	if(fs == NULL || fs->nfont <= 0 || fs->fonts == NULL)
	{
		if(fs) XftFontSetDestroy(fs);
		return;
	}

	EFontX11 *engine = e_cast_as(_etk_get_x_font_engine(fX11Engine, xftFontFamily, xftFontStyle), EFontX11);
	if(engine)
	{
		for(int i = 0; i < fs->nfont; i++)
		{
			double fontSize = 0;
			XftPatternGetDouble(fs->fonts[i], XFT_PIXEL_SIZE, 0, &fontSize);

			if((int)fontSize <= 0 && engine->fScalable) continue;
			if((int)fontSize <= 0) fontSize = 0;

			EFontX11::_font_data_ *font_data = new EFontX11::_font_data_;
			EString strFontname;
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
						eint32 nSizes = 0;
						for(eint32 j = 0; j < engine->fFonts.CountItems(); j++)
						{
							EFontX11::_font_data_ *_font_data = NULL;
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
	eint32 nSizes = 0;
	for(int i = 0; i < fs->nfont; i++)
	{
		double fontSize = 0;
		XftPatternGetDouble(fs->fonts[i], XFT_PIXEL_SIZE, 0, &fontSize);

		if((int)fontSize <= 0 && fScalable) continue;
		if((int)fontSize <= 0) fontSize = 0;

		EFontX11::_font_data_ *font_data = new EFontX11::_font_data_;
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

	SetRenderMode(E_FONT_RENDER_DIRECTLY);

	return;
}
#endif


EFontX11::~EFontX11()
{
	if(fX11Engine != NULL)
	{
		EAutolock <EXGraphicsEngine> autolock(fX11Engine);
		if(autolock.IsLocked() == false) ETK_ERROR("[GRAPHICS]: %s --- Invalid graphics engine.", __PRETTY_FUNCTION__);

		bool xRunning = (fX11Engine->InitCheck() == E_OK);

		for(eint32 i = 0; i < fFonts.CountItems(); i++)
		{
			EFontX11::_font_data_ *font_data = NULL;
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


e_font_detach_callback*
EFontX11::Attach(void (*callback)(void*), void *data)
{
	if(fX11Engine == NULL) return NULL;

	EAutolock <EXGraphicsEngine> autolock(fX11Engine);
	if(autolock.IsLocked() == false || fX11Engine->InitCheck() != E_OK) return NULL;

	if(fFonts.CountItems() <= 0) return NULL;

	return EFontEngine::Attach(callback, data);
}


bool
EFontX11::Detach(e_font_detach_callback *callback)
{
	if(fX11Engine == NULL) return false;

	EAutolock <EXGraphicsEngine> autolock(fX11Engine);
	if(autolock.IsLocked() == false) return false;

	if(!EFontEngine::Detach(callback)) return false;

	if(!IsAttached() && fFonts.CountItems() > 0)
	{
		bool xRunning = (fX11Engine->InitCheck() == E_OK);

		for(eint32 i = 0; i < fFonts.CountItems(); i++)
		{
			EFontX11::_font_data_ *font_data = NULL;
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
EFontX11::IsValid() const
{
	return(fX11Engine != NULL && fFonts.CountItems() > 0 && Family() != NULL && Style() != NULL);
}


bool
EFontX11::IsScalable() const
{
	return fScalable;
}


#ifdef HAVE_XFT
void
EFontX11::ForceFontAliasing(bool enable)
{
	if(fX11Engine == NULL) return;

	EAutolock <EXGraphicsEngine> autolock(fX11Engine);
	if(autolock.IsLocked() == false || fX11Engine->InitCheck() != E_OK) return;

	if(fForceFontAliasing != enable)
	{
		fForceFontAliasing = enable;

		EFontX11::_font_data_ *font_data = fCurFontData;
		if(font_data == NULL || font_data->xft == false || font_data->xFontset == NULL) return;

		double size = (double)font_data->xFontsize;
		if(size < 0) size = -size;
		XftFont *newFont = XftFontOpen(fX11Engine->xDisplay, fX11Engine->xScreen,
					       XFT_FAMILY, XftTypeString, Family(),
					       XFT_STYLE, XftTypeString, Style(),
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
EFontX11::StringWidth(const char *string, float size, float spacing, float shear, bool bold, eint32 length) const
{
	if(fX11Engine == NULL) return 0;
	if((int)size <= 0 || string == NULL || *string == 0 || length == 0 || !IsAttached() || fFonts.CountItems() <= 0) return 0;

	EAutolock <EXGraphicsEngine> autolock(fX11Engine);
	if(autolock.IsLocked() == false || fX11Engine->InitCheck() != E_OK) return 0;

	EFontX11::_font_data_ *curFont = NULL;
	if(fCurFontData == NULL || fCurFontData->xFontset == NULL ||
	   !(fCurFontData->xFontsize == (int)size || (fScalable && -(fCurFontData->xFontsize) == (int)size)))
	{
		eint32 scalable_index = -1;
		for(eint32 i = 0; i < fFonts.CountItems(); i++)
		{
			EFontX11::_font_data_ *font_data = NULL;
			const EString *str = fFonts.ItemAt(i, (void**)&font_data);
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
											 XFT_FAMILY, XftTypeString, Family(),
											 XFT_STYLE, XftTypeString, Style(),
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
						if(lists != NULL) XFreeStringList(lists);
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
			EFontX11::_font_data_ *font_data = NULL;
			const EString *str = fFonts.ItemAt(scalable_index, (void**)&font_data);
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
											 XFT_FAMILY, XftTypeString, Family(),
											 XFT_STYLE, XftTypeString, Style(),
											 XFT_PIXEL_SIZE, XftTypeDouble, (double)size,
											 XFT_CORE, XftTypeBool, True,
											 XFT_ANTIALIAS, XftTypeBool, (Bool)!fForceFontAliasing, 0);
					}
					else
					{
#endif
						EString aStr = str->String();
						EString bStr = "-";
						bStr << (int)size;
						aStr.ReplaceLast("-0", bStr.String());
						char **lists = NULL;
						int count = 0;
						if(font_data->xFontset) XFreeFontSet(fX11Engine->xDisplay, (XFontSet)(font_data->xFontset));
						font_data->xFontset = (void*)XCreateFontSet(fX11Engine->xDisplay, aStr.String(), &lists, &count, NULL);
						if(lists != NULL) XFreeStringList(lists);
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
		if(length < 0 || (size_t)length > strlen(string)) length = (eint32)strlen(string);

		euint8 len = 0;
		const char *tmp = e_utf8_at(string, 0, &len);
		float height = (float)(xfs->ascent + xfs->descent);
		float delta = (float)ceil((double)(spacing * size));
		while(!(len == 0 || !tmp || *tmp == 0 || tmp - string > length - (eint32)len))
		{
			XGlyphInfo glyph;
			XftTextExtentsUtf8(fX11Engine->xDisplay, xfs, (XftChar8*)tmp, (int)len, &glyph);
			int cWidth = (int)(glyph.xOff);
			width += (float)(cWidth > 0 ? cWidth : height) + (tmp == string ? 0.f : delta);
			tmp = e_utf8_next(tmp, &len);
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
	if(length < 0 || (size_t)length > strlen(string)) length = (eint32)strlen(string);
	float height = (float)(ascent + descent);
	float delta = (float)ceil((double)(spacing * size));

#ifdef X_HAVE_UTF8_STRING
	euint8 len = 0;
	const char *tmp = e_utf8_at(string, 0, &len);
	while(!(len == 0 || !tmp || *tmp == 0 || tmp - string > length - (eint32)len))
	{
		int cWidth = Xutf8TextEscapement(xFontset, (char*)tmp, (int)len);
		width += (float)(cWidth > 0 ? cWidth : (int)height) + (tmp == string ? 0.f : delta);
		tmp = e_utf8_next(tmp, &len);
	}
#else
	eunichar32 *utf32 = e_utf8_convert_to_utf32(string, length);
	if(utf32)
	{
		const eunichar32 *tmp = e_utf32_at(utf32, 0);
		while(!(!tmp || *tmp == 0))
		{
			int cWidth = XwcTextEscapement(xFontset, (wchar_t*)tmp, 1);
			width += (float)(cWidth > 0 ? cWidth : (int)height) + (tmp == (const eunichar32*)utf32 ? 0.f : delta);
			tmp = e_utf32_next(tmp);
		}
		free(utf32);
	}
#endif

	return width;
}


void
EFontX11::GetHeight(e_font_height *height, float size, float shear, bool bold) const
{
	if(fX11Engine == NULL || height == NULL) return;

	bzero(height, sizeof(e_font_height));

	EAutolock <EXGraphicsEngine> autolock(fX11Engine);
	if(autolock.IsLocked() == false || fX11Engine->InitCheck() != E_OK) return;

	if((int)size <= 0 || !IsAttached() || fFonts.CountItems() <= 0) return;

	EFontX11::_font_data_ *curFont = NULL;
	if(fCurFontData == NULL || fCurFontData->xFontset == NULL ||
	   !(fCurFontData->xFontsize == (int)size || (fScalable && -(fCurFontData->xFontsize) == (int)size)))
	{
		eint32 scalable_index = -1;
		for(eint32 i = 0; i < fFonts.CountItems(); i++)
		{
			EFontX11::_font_data_ *font_data = NULL;
			const EString *str = fFonts.ItemAt(i, (void**)&font_data);
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
											 XFT_FAMILY, XftTypeString, Family(),
											 XFT_STYLE, XftTypeString, Style(),
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
						if(lists != NULL) XFreeStringList(lists);
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
			EFontX11::_font_data_ *font_data = NULL;
			const EString *str = fFonts.ItemAt(scalable_index, (void**)&font_data);
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
											 XFT_FAMILY, XftTypeString, Family(),
											 XFT_STYLE, XftTypeString, Style(),
											 XFT_PIXEL_SIZE, XftTypeDouble, (double)size,
											 XFT_CORE, XftTypeBool, True,
											 XFT_ANTIALIAS, XftTypeBool, (Bool)!fForceFontAliasing, 0);
					}
					else
					{
#endif
						EString aStr = str->String();
						EString bStr = "-";
						bStr << (int)size;
						aStr.ReplaceLast("-0", bStr.String());
						char **lists = NULL;
						int count = 0;
						if(font_data->xFontset) XFreeFontSet(fX11Engine->xDisplay, (XFontSet)(font_data->xFontset));
						font_data->xFontset = (void*)XCreateFontSet(fX11Engine->xDisplay, aStr.String(), &lists, &count, NULL);
						if(lists != NULL) XFreeStringList(lists);
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
ERect
EFontX11::RenderString(EHandler *_view, const char *string, float size, float spacing, float shear, bool bold, eint32 length)
{
	if(fX11Engine == NULL || (int)size <= 0 || string == NULL || *string == 0 || length == 0) return ERect();

	EView *view = e_cast_as(_view, EView);
	if(view == NULL || view->Window() == NULL || view->IsPrinting()) return ERect();

	ERegion viewClipping;
	view->GetClippingRegion(&viewClipping);
	if(viewClipping.CountRects() <= 0) return ERect();

	EAutolock <EXGraphicsEngine> autolock(fX11Engine);
	if(autolock.IsLocked() == false || fX11Engine->InitCheck() != E_OK) return ERect();

	EXGraphicsDrawable *pix = e_cast_as(EGraphicsEngine::GetPixmap(view->Window()), EXGraphicsDrawable);
	EXGraphicsContext *dc = e_cast_as(EGraphicsEngine::GetContext(view), EXGraphicsContext);
	if(pix == NULL || dc == NULL) return ERect();

	if(!IsAttached() || fFonts.CountItems() <= 0) return ERect();

	EFontX11::_font_data_ *curFont = NULL;
	if(fCurFontData == NULL || fCurFontData->xFontset == NULL ||
	   !(fCurFontData->xFontsize == (int)size || (fScalable && -(fCurFontData->xFontsize) == (int)size)))
	{
		eint32 scalable_index = -1;
		for(eint32 i = 0; i < fFonts.CountItems(); i++)
		{
			EFontX11::_font_data_ *font_data = NULL;
			const EString *str = fFonts.ItemAt(i, (void**)&font_data);
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
											 XFT_FAMILY, XftTypeString, Family(),
											 XFT_STYLE, XftTypeString, Style(),
											 XFT_PIXEL_SIZE, XftTypeDouble, (double)size,
											 XFT_CORE, XftTypeBool, True,
											 XFT_ANTIALIAS, XftTypeBool, (Bool)!fForceFontAliasing, 0);
						if(font_data->xFontset == NULL)
							ETK_WARNING("[GRAPHICS]: XftFontOpen \"%s-%s-%d\" failed.",
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
							ETK_WARNING("[GRAPHICS]: XCreateFontSet \"%s\" failed.", str->String());
						if(lists != NULL) XFreeStringList(lists);
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
			EFontX11::_font_data_ *font_data = NULL;
			const EString *str = fFonts.ItemAt(scalable_index, (void**)&font_data);
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
											 XFT_FAMILY, XftTypeString, Family(),
											 XFT_STYLE, XftTypeString, Style(),
											 XFT_PIXEL_SIZE, XftTypeDouble, (double)size,
											 XFT_CORE, XftTypeBool, True,
											 XFT_ANTIALIAS, XftTypeBool, (Bool)!fForceFontAliasing, 0);
						if(font_data->xFontset == NULL)
							ETK_WARNING("[GRAPHICS]: XftFontOpen \"%s-%s-%d\" failed.",
								    Family(), Style(), (int)size);
					}
					else
					{
#endif
						EString aStr = str->String();
						EString bStr = "-";
						bStr << (int)size;
						aStr.ReplaceLast("-0", bStr.String());
						char **lists = NULL;
						int count = 0;
						if(font_data->xFontset) XFreeFontSet(fX11Engine->xDisplay, (XFontSet)(font_data->xFontset));
						font_data->xFontset = (void*)XCreateFontSet(fX11Engine->xDisplay, aStr.String(),
											    &lists, &count, NULL);
						if(font_data->xFontset == NULL)
							ETK_WARNING("[GRAPHICS]: XCreateFontSet \"%s\" failed.", str->String());
						if(lists != NULL) XFreeStringList(lists);
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

	if(!curFont) return ERect();

	dc->PrepareXColor();

#ifdef HAVE_XFT
	if(curFont->xft)
	{
		XftFont *xfs = (XftFont*)curFont->xFontset;
		if(xfs == NULL || pix->xDraw == NULL) return ERect();

		if(length < 0 || (size_t)length > strlen(string)) length = (eint32)strlen(string);

		EPoint pt = view->ConvertToWindow(view->PenLocation()).FloorSelf();
		euint8 len = 0;
		const char *tmp = e_utf8_at(string, 0, &len);
		int x = (int)pt.x;
		int y = (int)pt.y;
		float width = 0;
		float height = (float)(xfs->ascent + xfs->descent);
		int delta = (int)ceil((double)(spacing * size));

		XftColor xftcolor;
		Region xRegion = NULL;
		if(dc->GetXClipping(&xRegion) == E_OK) XftDrawSetClip(pix->xDraw, xRegion);

		e_rgb_color color = dc->HighColor();

		dc->GetXHighColor(&(xftcolor.pixel));
		xftcolor.color.red = (unsigned short)color.red * 257;
		xftcolor.color.green = (unsigned short)color.green * 257;
		xftcolor.color.blue = (unsigned short)color.blue * 257;
		xftcolor.color.alpha = E_MAXUSHORT;

		while(!(len == 0 || !tmp || *tmp == 0 || tmp - string > length - (eint32)len))
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
			tmp = e_utf8_next(tmp, &len);
		}

//		XFlush(fX11Engine->xDisplay);

		ERect updateRect;
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
	if(xFontset == NULL) return ERect();

	XFontStruct **xFontStructs = NULL;
	char **names = NULL;
	int nFonts = 0;
	if((nFonts = XFontsOfFontSet(xFontset, &xFontStructs, &names)) <= 0 || xFontStructs == NULL) return ERect();

	int ascent = 0, descent = 0;
	for(int i = 0; i < nFonts; i++)
	{
		ascent = max_c(xFontStructs[i]->max_bounds.ascent, ascent);
		descent = max_c(xFontStructs[i]->max_bounds.descent, descent);
	}

	if(length < 0 || (size_t)length > strlen(string)) length = (eint32)strlen(string);

	EPoint pt = view->ConvertToWindow(view->PenLocation()).FloorSelf();
	int x = (int)pt.x;
	int y = (int)pt.y;
	float width = 0;
	float height = (float)(ascent + descent);
	int delta = (int)ceil((double)(spacing * size));

#ifdef X_HAVE_UTF8_STRING
	euint8 len = 0;
	const char *tmp = e_utf8_at(string, 0, &len);
	while(!(len == 0 || !tmp || *tmp == 0 || tmp - string > length - (eint32)len))
	{
		int cWidth = Xutf8TextEscapement(xFontset, (char*)tmp, (int)len);
		if(cWidth > 0)
			Xutf8DrawString(fX11Engine->xDisplay, pix->xPixmap, xFontset, dc->xGC, x, y, (char*)tmp, (int)len);
		else
			XDrawRectangle(fX11Engine->xDisplay, pix->xPixmap, dc->xGC, x + 2, y - ascent + 1,
				       (unsigned int)(max_c(height, 4) - 4), (unsigned int)(max_c(height, 4) - 4));

		x += (cWidth > 0 ? cWidth : (int)height) + delta;
		width += (float)(cWidth > 0 ? cWidth : (int)height) + (float)(tmp == string ? 0 : delta);
		tmp = e_utf8_next(tmp, &len);
	}
#else
	eunichar32 *utf32 = e_utf8_convert_to_utf32(string, length);
	if(utf32)
	{
		const eunichar32 *tmp = e_utf32_at(utf32, 0);
		while(!(!tmp || *tmp == 0))
		{
			int cWidth = XwcTextEscapement(xFontset, (wchar_t*)tmp, 1);
			if(cWidth > 0)
				XwcDrawString(fX11Engine->xDisplay, pix->xPixmap, xFontset, dc->xGC, x, y, (wchar_t*)tmp, 1);
			else
				XDrawRectangle(fX11Engine->xDisplay, pix->xPixmap, dc->xGC, x + 2, y - ascent + 1,
					       (unsigned int)(max_c(height, 4) - 4), (unsigned int)(max_c(height, 4) - 4));

			x += (cWidth > 0 ? cWidth : (int)height) + delta;
			width += (float)(cWidth > 0 ? cWidth : (int)height) + (float)(tmp == (const eunichar32*)utf32 ? 0 : delta);
			tmp = e_utf32_next(tmp);
		}
		free(utf32);
	}
#endif
//	XFlush(fX11Engine->xDisplay);

	ERect updateRect;
	updateRect.left = pt.x;
	updateRect.right = pt.x + width;
	updateRect.top = pt.y - (float)(ascent + 1);
	updateRect.bottom = updateRect.top + height;
	view->ConvertFromWindow(&updateRect);
	updateRect &= viewClipping.Frame();

	return updateRect;
}


e_status_t
EXGraphicsEngine::InitalizeFonts()
{
	EAutolock <EXGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false) return E_ERROR;

	return InitCheck();
}


void
EXGraphicsEngine::DestroyFonts()
{
	EAutolock <EXGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false) return;

	for(eint32 i = 0; i < xFontEngines.CountItems(); i++)
	{
		EList* engine_list = NULL;
		xFontEngines.ItemAt(i, (void**)&engine_list);
		if(!engine_list) continue;
		for(eint32 j = 0; j < engine_list->CountItems(); j++)
		{
			EFontX11 *engine = (EFontX11*)engine_list->ItemAt(j);
			if(!engine) continue;
			delete engine;
		}
		delete engine_list;
	}
	xFontEngines.MakeEmpty();
}


e_status_t
EXGraphicsEngine::UpdateFonts(bool check_only)
{
	EAutolock <EXGraphicsEngine> autolock(this);
	if(autolock.IsLocked() == false || InitCheck() != E_OK) return E_ERROR;

	if(check_only)
	{
		ETK_WARNING("[GRAPHICS]: %s --- check_only not implement yet.", __PRETTY_FUNCTION__);
		return E_ERROR;
	}

	e_status_t retVal = E_ERROR;

#ifdef HAVE_XFT
	ETK_DEBUG("[GRAPHICS]: Updating Xft fonts ...");

	XftFontSet *fs = XftListFonts(xDisplay, xScreen, 0, XFT_FAMILY, 0);
	if(!(fs == NULL || fs->nfont <= 0 || fs->fonts == NULL))
	{
		for(int i = 0; i < fs->nfont; i++)
		{
			char *fontName = NULL;
			if(XftPatternGetString(fs->fonts[i], XFT_FAMILY, 0, &fontName) != XftResultMatch || fontName == NULL) continue;

			XftFontSet *tfs = XftListFonts(xDisplay, xScreen, XFT_FAMILY, XftTypeString, fontName, 0, XFT_STYLE, 0);
			if(!(tfs == NULL || tfs->nfont <= 0 || tfs->fonts == NULL))
			{
				for(int j = 0; j < tfs->nfont; j++)
				{
					char *fontStyle = NULL;
					if(XftPatternGetString(tfs->fonts[j], XFT_STYLE, 0, &fontStyle) != XftResultMatch) continue;
					if(!fontStyle) continue;

					EFontX11 *engine = new EFontX11(this, fontName, fontStyle);
					if(!engine || !engine->IsValid())
					{
						if(engine) delete engine;
						continue;
					}
					if(!_etk_add_x_font_engine(this, engine)) delete engine;
				}
			}
			if(tfs) XftFontSetDestroy(tfs);
		}

		retVal = E_OK;
	}
	if(fs) XftFontSetDestroy(fs);

	ETK_DEBUG("[GRAPHICS]: Xft fonts updated.");
#endif

	ETK_DEBUG("[GRAPHICS]: Updating X11 core fonts ...");

	int count = 0;
	char **fontNames = XListFonts(xDisplay, "-*-*-*-r-*-*-*-*-*-*-*-*-*-*", E_MAXINT, &count);

	if(fontNames && count > 0)
	{
		for(int i = 0; i < count; i++)
		{
			EFontX11 *engine = new EFontX11(this, fontNames[i]);
			if(!engine || !engine->IsValid())
			{
				if(engine) delete engine;
				continue;
			}
			if(!_etk_add_x_font_engine(this, engine)) delete engine;
		}

		retVal = E_OK;
	}

	if(fontNames) XFreeFontNames(fontNames);

	ETK_DEBUG("[GRAPHICS]: X11 core fonts updated.");

	return retVal;
}

