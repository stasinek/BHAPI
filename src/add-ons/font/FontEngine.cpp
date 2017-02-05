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
 * File: FontEngine.cpp
 *
 * --------------------------------------------------------------------------*/

#include <string.h>

#ifdef __BEOS__
#include "./../../app/AppDefs.h"
#include "./../../interface/Font.h"
#endif

#include "FontEngine.h"

#include "./../../support/SupportDefs.h"
#include "./../../support/Locker.h"
#include "./../../support/Autolock.h"
#include "./../../app/Application.h"
#include "./../../add-ons/graphics/GraphicsEngine.h"

static EFont* _etk_plain_font = NULL;
static EFont* _etk_bold_font = NULL;
static EFont* _etk_fixed_font = NULL;

static ELocker etk_font_locker;
static bool _etk_font_initialized_ = false;
static bool _etk_font_canceling_ = false;
static EStringArray etk_font_families;

#ifdef HAVE_FT2
extern bool etk_font_freetype2_init(void);
extern bool etk_font_freetype2_is_valid(void);
extern void etk_font_freetype2_cancel(void);
extern bool etk_update_freetype2_font_families(bool check_only);
#endif // HAVE_FT2


_IMPEXP_ETK bool etk_font_add(const char *family, const char *style, EFontEngine *engine)
{
	if(family == NULL || *family == 0 || style == NULL || *style == 0 || engine == NULL) return false;

	EAutolock <ELocker> autolock(&etk_font_locker);
	if(!_etk_font_initialized_ || engine->fServing != NULL) return false;

	EStringArray *styles = NULL;
	etk_font_families.ItemAt(etk_font_families.FindString(family), (void**)&styles);
	if(styles ? styles->FindString(style) >= 0 : false)
	{
//		ETK_DEBUG("[FONT]: %s --- style[%s] of family[%s] already exists.", __PRETTY_FUNCTION__, style, family);
		return false;
	}

	if(!styles)
	{
		styles = new EStringArray;
		if(!styles || !styles->AddItem(style, (void*)engine) ||
		   !etk_font_families.AddItem(family, (void*)styles))
		{
			ETK_DEBUG("[FONT]: %s --- Add style[%s] of family[%s] failed.", __PRETTY_FUNCTION__, style, family);
			if(styles) delete styles;
			return false;
		}
	}
	else
	{
		if(!styles->AddItem(style, (void*)engine))
		{
			ETK_DEBUG("[FONT]: %s --- Add style[%s] of family[%s] failed.", __PRETTY_FUNCTION__, style, family);
			return false;
		}
	}

	engine->fServing = styles;

	return true;
}


EFontEngine::EFontEngine()
	: fFamily(NULL), fStyle(NULL), fFixedSize(NULL), nFixedSize(0), fRenderMode(E_FONT_RENDER_UNKNOWN), fServing(NULL)
{
}


EFontEngine::EFontEngine(const char *family, const char *style)
	: fFamily(NULL), fStyle(NULL), fFixedSize(NULL), nFixedSize(0), fRenderMode(E_FONT_RENDER_UNKNOWN), fServing(NULL)
{
	SetFamily(family);
	SetStyle(style);
}


bool
EFontEngine::InServing() const
{
	EAutolock <ELocker> autolock(&etk_font_locker);
	return(fServing != NULL);
}


void
EFontEngine::OutOfServing()
{
	EAutolock <ELocker> autolock(&etk_font_locker);
	if(_etk_font_canceling_ || fServing == NULL) return;

	EFontEngine *engine = NULL;
	for(eint32 i = 0; i < fServing->CountItems(); i++)
	{
		fServing->ItemAt(i, (void**)&engine);
		if(engine != this) continue;
		if(fServing->RemoveItem(i))
		{
			if(fServing->CountItems() <= 0)
			{
				EStringArray *styles = NULL;
				for(eint32 j = 0; j < etk_font_families.CountItems(); j++)
				{
					etk_font_families.ItemAt(j, (void**)&styles);
					if(styles != fServing) continue;
					if(etk_font_families.RemoveItem(j)) delete fServing;
					break;
				}
			}
			fServing = NULL;
		}
		break;
	}
}


EFontEngine::~EFontEngine()
{
	if(InServing()) OutOfServing();

	for(eint32 i = 0; i < fAttached.CountItems(); i++)
	{
		e_font_detach_callback *eCallback = (e_font_detach_callback*)fAttached.ItemAt(i);
		if(!eCallback) continue;
		if(eCallback->callback != NULL) eCallback->callback(eCallback->data);
		delete eCallback;
	}
	fAttached.MakeEmpty();

	if(fFamily) delete[] fFamily;
	if(fStyle) delete[] fStyle;
	if(fFixedSize) delete[] fFixedSize;
}


e_font_detach_callback*
EFontEngine::Attach(void (*callback)(void*), void *data)
{
	e_font_detach_callback *eCallback = new e_font_detach_callback;
	if(!eCallback) return NULL;

	if(!fAttached.AddItem(eCallback))
	{
		delete eCallback;
		return NULL;
	}

	eCallback->callback = callback;
	eCallback->data = data;

	return eCallback;
}


bool
EFontEngine::IsAttached() const
{
	return(fAttached.CountItems() > 0);
}


bool
EFontEngine::Detach(e_font_detach_callback *callback)
{
	if(!callback) return false;
	for(eint32 i = fAttached.CountItems() - 1; i >= 0; i--)
	{
		e_font_detach_callback *eCallback = (e_font_detach_callback*)fAttached.ItemAt(i);
		if(eCallback != callback) continue;
		if(fAttached.RemoveItem(i))
		{
			if(eCallback->callback != NULL) eCallback->callback(eCallback->data);
			delete eCallback;
			return true;
		}
		break;
	}
	return false;
}


bool
EFontEngine::Lock()
{
	return fLocker.Lock();
}


void
EFontEngine::Unlock()
{
	fLocker.Unlock();
}


const char*
EFontEngine::Family() const
{
	return fFamily;
}


const char*
EFontEngine::Style() const
{
	return fStyle;
}


e_font_render_mode
EFontEngine::RenderMode() const
{
	return fRenderMode;
}


void
EFontEngine::SetRenderMode(e_font_render_mode rmode)
{
	fRenderMode = rmode;
}


bool
EFontEngine::HasFixedSize(eint32 *count) const
{
	if(nFixedSize > 0 && fFixedSize != NULL)
	{
		if(count) *count = nFixedSize;
		return true;
	}

	return false;
}


bool
EFontEngine::GetFixedSize(float *size, eint32 index) const
{
	if(size == NULL || index < 0) return false;

	if(nFixedSize > index && fFixedSize != NULL)
	{
		*size = fFixedSize[index];
		return true;
	}

	return false;
}


void
EFontEngine::SetFamily(const char *family)
{
	if(fFamily) delete[] fFamily;
	fFamily = NULL;
	if(family) fFamily = EStrdup(family);
}


void
EFontEngine::SetStyle(const char *style)
{
	if(fStyle) delete[] fStyle;
	fStyle = NULL;
	if(style) fStyle = EStrdup(style);
}


void
EFontEngine::SetFixedSize(float *sizes, eint32 count)
{
	if(fFixedSize)
	{
		delete[] fFixedSize;
		fFixedSize = NULL;
	}

	nFixedSize = 0;

	if(sizes && count > 0)
	{
		fFixedSize = new float[count];
		memcpy(fFixedSize, sizes, sizeof(float) * (size_t)count);
		nFixedSize = count;
	}
}


bool
EFontEngine::IsValid() const
{
	return false;
}


bool
EFontEngine::IsScalable() const
{
	return false;
}


void
EFontEngine::ForceFontAliasing(bool enable)
{
}


float
EFontEngine::StringWidth(const char *string, float size, float spacing, float shear, bool bold, eint32 length) const
{
	return 0;
}


void
EFontEngine::GetHeight(e_font_height *height, float size, float shear, bool bold) const
{
	if(height) bzero(height, sizeof(e_font_height));
}


float
EFontEngine::StringWidth(const EString &str, float size, float spacing, float shear, bool bold, eint32 length) const
{
	return StringWidth(str.String(), size, spacing, shear, bold, length);
}


ERect
EFontEngine::RenderString(EHandler *view, const char *string,
			  float size, float spacing,
			  float shear, bool bold, eint32 length)
{
	return ERect();
}


ERect
EFontEngine::RenderString(EHandler *view, const EString &str,
			  float size, float spacing,
			  float shear, bool bold, eint32 length)
{
	return RenderString(view, str.String(), size, spacing, shear, bold, length);
}


euint8*
EFontEngine::RenderString(const char *string, eint32 *width, eint32 *height, bool *is_mono,
			  float size, float spacing,
			  float shear, bool bold, eint32 length)
{
	return NULL;
}


euint8*
EFontEngine::RenderString(const EString &str, eint32 *width, eint32 *height, bool *is_mono,
			  float size, float spacing,
			  float shear, bool bold, eint32 length)
{
	return RenderString(str.String(), width, height, is_mono, size, spacing, shear, bold, length);
}


_IMPEXP_ETK eint32 etk_count_font_families(void)
{
	EAutolock <ELocker> autolock(&etk_font_locker);

	return etk_font_families.CountItems();
}


_IMPEXP_ETK e_status_t etk_get_font_family(eint32 index, const char **name)
{
	if(!name) return E_BAD_VALUE;

	EAutolock <ELocker> autolock(&etk_font_locker);

	const EString *str = etk_font_families.ItemAt(index);
	if(!str) return E_ERROR;

	*name = str->String();
	return E_OK;
}


_IMPEXP_ETK eint32 etk_get_font_family_index(const char *name)
{
	if(!name) return -1;

	EAutolock <ELocker> autolock(&etk_font_locker);

	eint32 fIndex = etk_font_families.FindString(name);
	return fIndex;
}


_IMPEXP_ETK eint32 etk_get_font_style_index(const char *family, const char *name)
{
	if(!family || !name) return -1;

	EAutolock <ELocker> autolock(&etk_font_locker);

	eint32 index = etk_font_families.FindString(family);
	if(index < 0) return -1;

	EStringArray *styles = NULL;
	etk_font_families.ItemAt(index, (void**)&styles);
	if(!styles) return -1;

	index = styles->FindString(name);
	return index;
}


_IMPEXP_ETK eint32 etk_count_font_styles(const char *name)
{
	EAutolock <ELocker> autolock(&etk_font_locker);

	return etk_count_font_styles(etk_font_families.FindString(name));
}


_IMPEXP_ETK eint32 etk_count_font_styles(eint32 index)
{
	if(index < 0) return -1;

	EAutolock <ELocker> autolock(&etk_font_locker);

	EStringArray *styles = NULL;
	etk_font_families.ItemAt(index, (void**)&styles);

	return(styles ? styles->CountItems() : -1);
}


_IMPEXP_ETK e_status_t etk_get_font_style(const char *family, eint32 index, const char **name)
{
	if(!family || !name) return E_BAD_VALUE;

	EAutolock <ELocker> autolock(&etk_font_locker);

	eint32 fIndex = etk_font_families.FindString(family);
	if(fIndex < 0) return E_ERROR;

	EStringArray *styles = NULL;
	etk_font_families.ItemAt(fIndex, (void**)&styles);
	if(!styles) return E_ERROR;

	const EString *str = styles->ItemAt(index);
	if(!str) return E_ERROR;

	*name = str->String();
	return E_OK;
}


_IMPEXP_ETK EFontEngine* etk_get_font_engine(const char *family, const char *style)
{
	if(!family || !style) return NULL;

	EAutolock <ELocker> autolock(&etk_font_locker);

	EStringArray *styles = NULL;
	etk_font_families.ItemAt(etk_font_families.FindString(family), (void**)&styles);
	if(!styles) return NULL;

	EFontEngine *engine = NULL;
	styles->ItemAt(styles->FindString(style), (void**)&engine);

	return engine;
}


_IMPEXP_ETK EFontEngine* etk_get_font_engine(eint32 familyIndex, eint32 styleIndex)
{
	EAutolock <ELocker> autolock(&etk_font_locker);

	EStringArray *styles = NULL;
	etk_font_families.ItemAt(familyIndex, (void**)&styles);
	if(!styles) return NULL;

	EFontEngine *engine = NULL;
	styles->ItemAt(styleIndex, (void**)&engine);

	return engine;
}


static bool etk_font_other_init()
{
	// TODO
	return true;
}


static void etk_font_other_cancel()
{
	// TODO
}


static bool etk_update_other_font_families(bool check_only)
{
	// TODO
	return(check_only ? false : true);
}


_IMPEXP_ETK bool etk_update_font_families(bool check_only)
{
	EAutolock <ELocker> autolock(&etk_font_locker);

	if(!_etk_font_initialized_) return false;

	if(!check_only)
	{
		EStringArray *styles;
		for(eint32 i = 0; i < etk_font_families.CountItems(); i++)
		{
			styles = NULL;
			etk_font_families.ItemAt(i, (void**)&styles);
			if(styles)
			{
				EFontEngine *engine;
				for(eint32 j = 0; j < styles->CountItems(); j++)
				{
					engine = NULL;
					styles->ItemAt(j, (void**)&engine);
					if(engine) delete engine;
				}
				styles->MakeEmpty();
				delete styles;
			}
		}
		etk_font_families.MakeEmpty();
	}

	bool updateFailed = false;

	if(!(etk_app == NULL || etk_app->fGraphicsEngine == NULL)) etk_app->fGraphicsEngine->UpdateFonts(check_only);

	// TODO: fix the return value style
#ifdef HAVE_FT2
	if(etk_font_freetype2_is_valid())
	{
		if(etk_update_freetype2_font_families(check_only))
		{
			if(check_only) return true;
		}
		else if(!updateFailed && !check_only)
		{
			updateFailed = true;
		}
	}
#endif

	if(etk_update_other_font_families(check_only))
	{
		if(check_only) return true;
	}
	else if(!updateFailed && !check_only)
	{
		updateFailed = true;
	}

	return(check_only ? false : !updateFailed);
}


_LOCAL bool etk_font_init(void)
{
	EAutolock <ELocker> autolock(&etk_font_locker);

	if(!_etk_font_initialized_)
	{
		ETK_DEBUG("[FONT]: Initalizing fonts ...");

#ifdef HAVE_FT2
		etk_font_freetype2_init();
#endif
		etk_font_other_init();
		_etk_font_initialized_ = true;
		etk_update_font_families(false);

		// TODO
		_etk_plain_font = new EFont();
		if(_etk_plain_font)
		{
#ifdef ETK_GRAPHICS_BEOS_BUILT_IN
			font_family bFamily;
			font_style bStyle;
			be_plain_font->GetFamilyAndStyle(&bFamily, &bStyle);
			if(_etk_plain_font->SetFamilyAndStyle(bFamily, bStyle) == E_OK)
			{
				_etk_plain_font->SetSize(be_plain_font->Size());
			}
			else
			{
#endif // ETK_GRAPHICS_BEOS_BUILT_IN
			const char *family = getenv("ETK_PLAIN_FONT_FAMILY");
			const char *style = getenv("ETK_PLAIN_FONT_STYLE");
			if(((family == NULL || *family == 0 || style == NULL || *style == 0) ?
				(ETK_WARNING("[FONT]: $ETK_PLAIN_FONT_FAMILY = 0 || $ETK_PLAIN_FONT_STYLE = 0"),E_ERROR) :
				_etk_plain_font->SetFamilyAndStyle(family, style)) != E_OK)
#ifdef ETK_GRAPHICS_WIN32_BUILT_IN
			if(_etk_plain_font->SetFamilyAndStyle("SimSun", "Regular") != E_OK)
			if(_etk_plain_font->SetFamilyAndStyle("宋体", "Regular") != E_OK)
			if(_etk_plain_font->SetFamilyAndStyle("SimHei", "Regular") != E_OK)
			if(_etk_plain_font->SetFamilyAndStyle("黑体", "Regular") != E_OK)
			if(_etk_plain_font->SetFamilyAndStyle("Tahoma", "Regular") != E_OK)
#endif // ETK_GRAPHICS_WIN32_BUILT_IN
			if(_etk_plain_font->SetFamilyAndStyle("helvetica", "medium") != E_OK) _etk_plain_font->SetFamilyAndStyle(0);

			float fsize = 12;
			const char *fontSize = getenv("ETK_PLAIN_FONT_SIZE");
			if(!(fontSize == NULL || *fontSize == 0)) fsize = (float)atoi(fontSize);
			else ETK_WARNING("[FONT]: $ETK_PLAIN_FONT_SIZE = 0");

			if(_etk_plain_font->IsScalable() == false)
			{
				_etk_plain_font->GetFixedSize(&fsize);
				_etk_plain_font->SetSize(fsize);
			}
			else
			{
				_etk_plain_font->SetSize(fsize);
			}
#ifdef ETK_GRAPHICS_BEOS_BUILT_IN
			}
#endif // ETK_GRAPHICS_BEOS_BUILT_IN
			_etk_plain_font->SetSpacing(0.05f);
			_etk_plain_font->SetShear(90);
		}
		_etk_bold_font = new EFont();
		if(_etk_bold_font)
		{
#ifdef ETK_GRAPHICS_BEOS_BUILT_IN
			font_family bFamily;
			font_style bStyle;
			be_bold_font->GetFamilyAndStyle(&bFamily, &bStyle);
			if(_etk_bold_font->SetFamilyAndStyle(bFamily, bStyle) == E_OK)
			{
				_etk_bold_font->SetSize(be_bold_font->Size());
			}
			else
			{
#endif // ETK_GRAPHICS_BEOS_BUILT_IN
			const char *family = getenv("ETK_BOLD_FONT_FAMILY");
			const char *style = getenv("ETK_BOLD_FONT_STYLE");
			if(((family == NULL || *family == 0 || style == NULL || *style == 0) ?
				(ETK_WARNING("[FONT]: $ETK_BOLD_FONT_FAMILY = 0 || $ETK_BOLD_FONT_STYLE = 0"),E_ERROR) :
				_etk_bold_font->SetFamilyAndStyle(family, style)) != E_OK)
#ifdef ETK_GRAPHICS_WIN32_BUILT_IN
			if(_etk_bold_font->SetFamilyAndStyle("SimHei", "Regular") != E_OK)
			if(_etk_bold_font->SetFamilyAndStyle("黑体", "Regular") != E_OK)
			if(_etk_bold_font->SetFamilyAndStyle("Tahoma", "Regular") != E_OK)
#endif // ETK_GRAPHICS_WIN32_BUILT_IN
			if(_etk_bold_font->SetFamilyAndStyle("helvetica", "bold") != E_OK) _etk_bold_font->SetFamilyAndStyle(0);

			float fsize = 12;
			const char *fontSize = getenv("ETK_BOLD_FONT_SIZE");
			if(!(fontSize == NULL || *fontSize == 0)) fsize = (float)atoi(fontSize);
			else ETK_WARNING("[FONT]: $ETK_BOLD_FONT_SIZE = 0");

			if(_etk_bold_font->IsScalable() == false)
			{
				_etk_bold_font->GetFixedSize(&fsize);
				_etk_bold_font->SetSize(fsize);
			}
			else
			{
				_etk_bold_font->SetSize(fsize);
			}
#ifdef ETK_GRAPHICS_BEOS_BUILT_IN
			}
#endif // ETK_GRAPHICS_BEOS_BUILT_IN
			_etk_bold_font->SetSpacing(0.05f);
			_etk_bold_font->SetShear(90);
		}
		_etk_fixed_font = new EFont();
		if(_etk_fixed_font)
		{
#ifdef ETK_GRAPHICS_BEOS_BUILT_IN
			font_family bFamily;
			font_style bStyle;
			be_fixed_font->GetFamilyAndStyle(&bFamily, &bStyle);
			if(_etk_fixed_font->SetFamilyAndStyle(bFamily, bStyle) == E_OK)
			{
				_etk_fixed_font->SetSize(be_fixed_font->Size());
			}
			else
			{
#endif // ETK_GRAPHICS_BEOS_BUILT_IN
			const char *family = getenv("ETK_FIXED_FONT_FAMILY");
			const char *style = getenv("ETK_FIXED_FONT_STYLE");
			if(((family == NULL || *family == 0 || style == NULL || *style == 0) ?
				(ETK_WARNING("[FONT]: $ETK_FIXED_FONT_FAMILY = 0 || $ETK_FIXED_FONT_STYLE = 0"),E_ERROR) :
				_etk_fixed_font->SetFamilyAndStyle(family, style)) != E_OK)
#ifdef ETK_GRAPHICS_WIN32_BUILT_IN
			if(_etk_fixed_font->SetFamilyAndStyle("SimHei", "Regular") != E_OK)
			if(_etk_fixed_font->SetFamilyAndStyle("黑体", "Regular") != E_OK)
			if(_etk_fixed_font->SetFamilyAndStyle("Tahoma", "Regular") != E_OK)
#endif // ETK_GRAPHICS_WIN32_BUILT_IN
			if(_etk_fixed_font->SetFamilyAndStyle("times", "medium") != E_OK) _etk_fixed_font->SetFamilyAndStyle(0);

			float fsize = 10;
			const char *fontSize = getenv("ETK_FIXED_FONT_SIZE");
			if(!(fontSize == NULL || *fontSize == 0)) fsize = (float)atoi(fontSize);
			else ETK_WARNING("[FONT]: $ETK_FIXED_FONT_SIZE = 0");

			if(_etk_fixed_font->IsScalable() == false)
			{
				_etk_fixed_font->GetFixedSize(&fsize);
				_etk_fixed_font->SetSize(fsize);
			}
			else
			{
				_etk_fixed_font->SetSize(fsize);
			}
#ifdef ETK_GRAPHICS_BEOS_BUILT_IN
			}
#endif // ETK_GRAPHICS_BEOS_BUILT_IN
			_etk_fixed_font->SetSpacing(0.05f);
			_etk_fixed_font->SetShear(70);
		}

		etk_plain_font = _etk_plain_font;
		etk_bold_font = _etk_bold_font;
		etk_fixed_font = _etk_fixed_font;

		ETK_DEBUG("[FONT]: Fonts initalized.");
	}

	return(etk_font_families.CountItems() > 0);
}


_LOCAL void etk_font_cancel(void)
{
	EAutolock <ELocker> autolock(&etk_font_locker);

	if(_etk_font_initialized_)
	{
		_etk_font_canceling_ = true;

		if(_etk_plain_font) delete _etk_plain_font;
		if(_etk_bold_font) delete _etk_bold_font;
		if(_etk_fixed_font) delete _etk_fixed_font;
		etk_plain_font = _etk_plain_font = NULL;
		etk_bold_font = _etk_bold_font = NULL;
		etk_fixed_font = _etk_fixed_font = NULL;

#ifdef HAVE_FT2
		etk_font_freetype2_cancel();
#endif
		etk_font_other_cancel();

		EStringArray *styles;
		for(eint32 i = 0; i < etk_font_families.CountItems(); i++)
		{
			styles = NULL;
			etk_font_families.ItemAt(i, (void**)&styles);
			if(styles)
			{
				EFontEngine *engine;
				for(eint32 j = 0; j < styles->CountItems(); j++)
				{
					engine = NULL;
					styles->ItemAt(j, (void**)&engine);
					if(engine) delete engine;
				}
				styles->MakeEmpty();
				delete styles;
			}
		}
		etk_font_families.MakeEmpty();

		_etk_font_canceling_ = false;
		_etk_font_initialized_ = false;
	}
}


_LOCAL bool etk_font_lock(void)
{
	return etk_font_locker.Lock();
}


_LOCAL void etk_font_unlock(void)
{
	etk_font_locker.Unlock();
}

