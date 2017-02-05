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
 * File: Font.cpp
 *
 * --------------------------------------------------------------------------*/

#include <string.h>

#include "./../config.h>
#include "./../add-ons/font/FontEngine.h>
#include "./../support/Autolock.h>

#include "Font.h"


extern _IMPEXP_ETK const EFont* etk_plain_font = NULL;
extern _IMPEXP_ETK const EFont* etk_bold_font = NULL;
extern _IMPEXP_ETK const EFont* etk_fixed_font = NULL;

static ELocker etk_font_info_locker;

typedef struct etk_font_info {
	EFontEngine *engine;
	e_font_detach_callback *detach_callback;
	float size;
	float spacing;
	float shear;
	bool bold;
	eint32 family_index;
	eint32 style_index;

	etk_font_info()
	{
		engine = NULL;
		detach_callback = NULL;
		size = 10.f;
		spacing = 0.1f;
		shear = 90.f;
		bold = false;
		family_index = -1;
		style_index = -1;
	}

	static void _detach_callback_(etk_font_info *info)
	{
		if(!info) return;
		etk_font_info_locker.Lock();
		info->engine = NULL;
		info->detach_callback = NULL;
		etk_font_info_locker.Unlock();
	}

	etk_font_info& operator=(const etk_font_info &info)
	{
		etk_font_info_locker.Lock();

		EFontEngine *_engine_ = engine;
		if(_engine_)
		{
			_engine_->Lock();
			if(_engine_->Detach(detach_callback) == false)
			{
				_engine_->Unlock();
				etk_font_info_locker.Unlock();
				return *this;
			}
			_engine_->Unlock();
		}
		engine = NULL;
		detach_callback = NULL;

		if(info.engine)
		{
			info.engine->Lock();
			if((detach_callback = info.engine->Attach((void(*)(void*))_detach_callback_, this)) == NULL)
			{
				info.engine->Unlock();
				etk_font_info_locker.Unlock();
				return *this;
			}
			info.engine->Unlock();
		}

		engine = info.engine;
		family_index = info.family_index;
		style_index = info.style_index;
		size = info.size;
		spacing = info.spacing;
		shear = info.shear;
		bold = info.bold;

		etk_font_info_locker.Unlock();

		return *this;
	}

	bool operator==(const etk_font_info &info)
	{
		EAutolock <ELocker> autolock(&etk_font_info_locker);
		return(engine == info.engine && size == info.size && spacing == info.spacing && shear == info.shear && bold == info.bold ? true : false);
	}

	bool operator!=(const etk_font_info &info)
	{
		EAutolock <ELocker> autolock(&etk_font_info_locker);
		return(engine == info.engine && size == info.size && spacing == info.spacing && shear == info.shear && bold == info.bold ? false : true);
	}

	bool SetEngine(EFontEngine *fengine)
	{
		etk_font_info_locker.Lock();

		EFontEngine *_engine_ = engine;
		if(_engine_)
		{
			_engine_->Lock();
			if(_engine_->Detach(detach_callback) == false)
			{
				_engine_->Unlock();
				etk_font_info_locker.Unlock();
				return false;
			}
			_engine_->Unlock();
		}
		engine = NULL;
		detach_callback = NULL;

		if(fengine)
		{
			fengine->Lock();
			if((detach_callback = fengine->Attach((void(*)(void*))_detach_callback_, this)) == NULL)
			{
				fengine->Unlock();
				etk_font_info_locker.Unlock();
				return false;
			}
			fengine->Unlock();
		}

		engine = fengine;
		family_index = engine ? etk_get_font_family_index(engine->Family()) : -1;
		style_index = engine ? etk_get_font_style_index(engine->Family(), engine->Style()) : -1;

		etk_font_info_locker.Unlock();

		return true;
	}

	euint32 FamilyAndStyle() const
	{
		if(family_index < 0 || style_index < 0) return E_MAXUINT32;
		euint32 fIndex = (euint32)family_index;
		euint32 sIndex = (euint32)style_index;
		return((fIndex << 16) | sIndex);
	}

	EFontEngine* Engine() const
	{
		EAutolock <ELocker> autolock(&etk_font_info_locker);
		return engine;
	}

	void SetSize(float fsize)
	{
		EAutolock <ELocker> autolock(&etk_font_info_locker);
		size = fsize;
	}

	float Size() const
	{
		EAutolock <ELocker> autolock(&etk_font_info_locker);
		return size;
	}

	void SetSpacing(float fspacing)
	{
		EAutolock <ELocker> autolock(&etk_font_info_locker);
		spacing = fspacing;
	}

	float Spacing() const
	{
		EAutolock <ELocker> autolock(&etk_font_info_locker);
		return spacing;
	}

	void SetShear(float fshear)
	{
		EAutolock <ELocker> autolock(&etk_font_info_locker);
		shear = fshear;
	}

	float Shear() const
	{
		EAutolock <ELocker> autolock(&etk_font_info_locker);
		return shear;
	}

	void SetBoldStyle(bool fbold)
	{
		EAutolock <ELocker> autolock(&etk_font_info_locker);
		bold = fbold;
	}

	bool IsBoldStyle() const
	{
		EAutolock <ELocker> autolock(&etk_font_info_locker);
		return bold;
	}

	float StringWidth(const char *string, eint32 length, float tabWidth) const
	{
		if(string == NULL || *string == 0 || length == 0) return 0;

		etk_font_info_locker.Lock();

		float width = 0;
		if(engine)
		{
			EString aStr(string, length);

			engine->Lock();

			if(tabWidth == 0)
			{
				width = engine->StringWidth(aStr.String(), size, spacing, shear, bold, aStr.Length());
			}
			else
			{
				float spacing_width = spacing * size;
				if(tabWidth < 0)
				{
					if(tabWidth < E_FONT_MIN_TAB_WIDTH) tabWidth = -E_FONT_MIN_TAB_WIDTH;
					else tabWidth = (float)(ceil((double)(-tabWidth)));
					tabWidth = (tabWidth * engine->StringWidth(" ", size, spacing, shear, bold, 1)) +
						   (tabWidth - 1.f) * spacing_width;
				}

				for(eint32 aOffset = 0; aOffset < aStr.Length(); aOffset++)
				{
					eint32 oldOffset = aOffset, len;
					aOffset = aStr.FindFirst("\t", aOffset);

					len = (aOffset < 0 ? aStr.Length() : aOffset) - oldOffset;
					if(len > 0)
						width += engine->StringWidth(aStr.String() + oldOffset,
									     size, spacing, shear, bold, len);

					if(aOffset < 0) break;
					width += (aOffset > 0 ? spacing_width : 0) + tabWidth;
				}
			}

			engine->Unlock();
		}

		etk_font_info_locker.Unlock();

		return width;
	}

	float* CharWidths(const char *string, eint32 length, eint32 *nChars, float tabWidth) const
	{
		if(string == NULL || *string == 0 || length == 0 || nChars == NULL) return NULL;

		eint32 strLen = (eint32)strlen(string);
		if(length < 0 || length > strLen) length = strLen;

		float *widths = new float[length];
		if(widths == NULL) return NULL;

		etk_font_info_locker.Lock();

		euint8 len = 0;
		const char *ch = e_utf8_at(string, 0, &len);
		eint32 count = 0;

		if(engine)
		{
			engine->Lock();
			if(tabWidth < 0)
			{
				float spacing_width = spacing * size;
				if(tabWidth < E_FONT_MIN_TAB_WIDTH) tabWidth = -E_FONT_MIN_TAB_WIDTH;
				else tabWidth = (float)(ceil((double)(-tabWidth)));
				tabWidth = (tabWidth * engine->StringWidth(" ", size, spacing, shear, bold, 1)) +
					   (tabWidth - 1.f) * spacing_width;
			}
		}

		while(!(ch == NULL || len <= 0 || (ch - string > length - len)))
		{
			if(engine == NULL)
			{
				widths[count] = 0;
			}
			else if(tabWidth == 0 || *ch != '\t')
			{
				widths[count] = engine->StringWidth(ch, size, spacing, shear, bold, (eint32)len);
			}
			else
			{
				widths[count] = tabWidth;
			}

			count++;
			ch = e_utf8_next(ch, &len);
		}
		if(engine) engine->Unlock();

		etk_font_info_locker.Unlock();

		*nChars = count;

		return widths;
	}

	void GetHeight(e_font_height *height) const
	{
		if(!height) return;

		etk_font_info_locker.Lock();

		if(engine)
		{
			engine->Lock();
			engine->GetHeight(height, size, shear, bold);
			engine->Unlock();
		}

		etk_font_info_locker.Unlock();
	}

	~etk_font_info()
	{
		etk_font_info_locker.Lock();
		EFontEngine *_engine_ = engine;
		if(_engine_)
		{
			_engine_->Lock();
			_engine_->Detach(detach_callback);
			_engine_->Unlock();
		}
		if(detach_callback) detach_callback->data = NULL;
		engine = NULL;
		detach_callback = NULL;
		etk_font_info_locker.Unlock();
	}
} etk_font_info;


EFont::EFont()
	: fInfo(NULL)
{
	etk_font_info *fontInfo = new etk_font_info;
	if(!fontInfo) return;

	fInfo = (void*)fontInfo;
}


EFont::EFont(const EFont &font)
	: fInfo(NULL)
{
	etk_font_info *fontInfo = new etk_font_info;
	if(!fontInfo) return;

	if(font.fInfo)
		*fontInfo = *((etk_font_info*)(font.fInfo));

	fInfo = (void*)fontInfo;
}


EFont::EFont(const EFont *font)
	: fInfo(NULL)
{
	etk_font_info *fontInfo = new etk_font_info;
	if(!fontInfo) return;

	if(font)
	{
		if(font->fInfo) *fontInfo = *((etk_font_info*)(font->fInfo));
	}

	fInfo = (void*)fontInfo;
}


EFont::EFont(const e_font_desc &fontDesc)
	: fInfo(NULL)
{
	etk_font_info *fontInfo = new etk_font_info;
	if(!fontInfo) return;

	fInfo = (void*)fontInfo;

	*this = fontDesc;
}


EFont::~EFont()
{
	if(fInfo) delete (etk_font_info*)fInfo;
}


EFont&
EFont::operator=(const EFont &font)
{
	if(font.fInfo == NULL)
	{
		if(fInfo) delete (etk_font_info*)fInfo;
		fInfo = NULL;
		return *this;
	}

	if(fInfo == NULL)
	{
		etk_font_info *fontInfo = new etk_font_info;
		if(!fontInfo) return *this;
		*fontInfo = *((etk_font_info*)(font.fInfo));
		fInfo = (void*)fontInfo;
	}
	else
	{
		etk_font_info *fontInfo = (etk_font_info*)fInfo;
		*fontInfo = *((etk_font_info*)(font.fInfo));
	}

	return *this;
}


EFont&
EFont::operator=(const e_font_desc &fontDesc)
{
	SetFamilyAndStyle(fontDesc.family, fontDesc.style);
	SetSize(fontDesc.size);
	SetSpacing(fontDesc.spacing);
	SetShear(fontDesc.shear);
	SetBoldStyle(fontDesc.bold);
	return *this;
}


bool
EFont::operator==(const EFont &font)
{
	if(fInfo == NULL && font.fInfo == NULL) return true;
	if(fInfo == NULL || font.fInfo == NULL) return false;
	etk_font_info* fInfoA = (etk_font_info*)fInfo;
	etk_font_info* fInfoB = (etk_font_info*)font.fInfo;
	return(*fInfoA == *fInfoB);
}


bool
EFont::operator!=(const EFont &font)
{
	if(fInfo == NULL && font.fInfo == NULL) return false;
	if(fInfo == NULL || font.fInfo == NULL) return true;
	etk_font_info* fInfoA = (etk_font_info*)fInfo;
	etk_font_info* fInfoB = (etk_font_info*)font.fInfo;
	return(*fInfoA != *fInfoB);
}


void
EFont::SetSize(float size)
{
	if(size <= 0) return;

	etk_font_info *fontInfo = (etk_font_info*)fInfo;
	if(fontInfo) fontInfo->SetSize(size);
}


float
EFont::Size() const
{
	etk_font_info *fontInfo = (etk_font_info*)fInfo;
	return(fontInfo ? fontInfo->Size() : -1.f);
}


void
EFont::SetSpacing(float spacing)
{
	etk_font_info *fontInfo = (etk_font_info*)fInfo;
	if(fontInfo) fontInfo->SetSpacing(spacing);
}


float
EFont::Spacing() const
{
	etk_font_info *fontInfo = (etk_font_info*)fInfo;
	return(fontInfo ? fontInfo->Spacing() : 0.f);
}


void
EFont::SetShear(float shear)
{
	if(shear < 45.f) shear = 45.f;
	else if(shear > 135.f) shear = 135.f;

	etk_font_info *fontInfo = (etk_font_info*)fInfo;
	if(fontInfo) fontInfo->SetShear(shear);
}


float
EFont::Shear() const
{
	etk_font_info *fontInfo = (etk_font_info*)fInfo;
	return(fontInfo ? fontInfo->Shear() : -1.f);
}


void
EFont::SetBoldStyle(bool bold)
{
	etk_font_info *fontInfo = (etk_font_info*)fInfo;
	if(fontInfo) fontInfo->SetBoldStyle(bold);
}


bool
EFont::IsBoldStyle() const
{
	etk_font_info *fontInfo = (etk_font_info*)fInfo;
	return(fontInfo ? fontInfo->IsBoldStyle() : false);
}


EFontEngine*
EFont::Engine() const
{
	etk_font_info *fontInfo = (etk_font_info*)fInfo;
	return(fontInfo ? fontInfo->Engine() : NULL);
}


bool
EFont::IsScalable() const
{
	EFontEngine *engine = Engine();
	return(engine ? engine->IsScalable() : false);
}


bool
EFont::HasFixedSize(eint32 *count) const
{
	EFontEngine *engine = Engine();
	return(engine ? engine->HasFixedSize(count) : false);
}


bool
EFont::GetFixedSize(float *size, eint32 index) const
{
	EFontEngine *engine = Engine();
	return(engine ? engine->GetFixedSize(size, index) : false);
}


e_status_t
EFont::SetFamilyAndStyle(const e_font_family family, const e_font_style style)
{
	etk_font_info *fontInfo = (etk_font_info*)fInfo;
	if(!fontInfo) return E_ERROR;

	EFontEngine *engine = etk_get_font_engine(family, style);
	if(!engine) return E_ERROR;

	return(fontInfo->SetEngine(engine) ? E_OK : E_ERROR);
}


e_status_t
EFont::SetFamilyAndStyle(euint32 code)
{
	if(code == E_MAXUINT32) return E_BAD_VALUE;

	etk_font_info *fontInfo = (etk_font_info*)fInfo;
	if(!fontInfo) return E_ERROR;

	euint32 familyIndex = code >> 16;
	euint32 styleIndex = code & 0xffff;

	EFontEngine *engine = etk_get_font_engine((eint32)familyIndex, (eint32)styleIndex);
	if(!engine) return E_ERROR;

	return(fontInfo->SetEngine(engine) ? E_OK : E_ERROR);
}


e_status_t
EFont::GetFamilyAndStyle(e_font_family *family, e_font_style *style) const
{
	if(!family || !style) return E_BAD_VALUE;

	etk_font_info *fontInfo = (etk_font_info*)fInfo;
	if(!fontInfo) return E_ERROR;

	euint32 code = fontInfo->FamilyAndStyle();
	if(code == E_MAXUINT32) return E_ERROR;

	euint32 familyIndex = code >> 16;
	euint32 styleIndex = code & 0xffff;

	const char *fFamily = NULL;
	const char *fStyle = NULL;
	etk_get_font_family((eint32)familyIndex, &fFamily);
	etk_get_font_style(fFamily, (eint32)styleIndex, &fStyle);

	if(!fFamily || !fStyle) return E_ERROR;

	strncpy(*family, fFamily, sizeof(e_font_family));
	strncpy(*style, fStyle, sizeof(e_font_style));

	return E_OK;
}


euint32
EFont::FamilyAndStyle() const
{
	etk_font_info *fontInfo = (etk_font_info*)fInfo;
	if(!fontInfo) return E_MAXUINT32;

	return fontInfo->FamilyAndStyle();
}


float
EFont::StringWidth(const char *string, eint32 length, float tabWidth) const
{
	if(string == NULL || *string == 0 || length == 0) return 0;

	etk_font_info *fontInfo = (etk_font_info*)fInfo;
	if(!fontInfo) return 0;

	return fontInfo->StringWidth(string, length, tabWidth);
}


float
EFont::StringWidth(const EString &str, eint32 length, float tabWidth) const
{
	return StringWidth(str.String(), length, tabWidth);
}


void
EFont::GetHeight(e_font_height *height) const
{
	if(!height) return;

	etk_font_info *fontInfo = (etk_font_info*)fInfo;
	if(!fontInfo) return;

	fontInfo->GetHeight(height);
}


float*
EFont::CharWidths(const char *string, eint32 *nChars, float tabWidth) const
{
	if(string == NULL || *string == 0 || nChars == NULL) return NULL;

	etk_font_info *fontInfo = (etk_font_info*)fInfo;
	if(!fontInfo) return NULL;

	return fontInfo->CharWidths(string, -1, nChars, tabWidth);
}


float*
EFont::CharWidths(const EString &str, eint32 *nChars, float tabWidth) const
{
	return CharWidths(str.String(), nChars, tabWidth);
}


float*
EFont::CharWidths(const char *string, eint32 length, eint32 *nChars, float tabWidth) const
{
	if(string == NULL || *string == 0 || length == 0 || nChars == NULL) return NULL;

	etk_font_info *fontInfo = (etk_font_info*)fInfo;
	if(!fontInfo) return NULL;

	return fontInfo->CharWidths(string, length, nChars, tabWidth);
}


float*
EFont::CharWidths(const EString &str, eint32 length, eint32 *nChars, float tabWidth) const
{
	return CharWidths(str.String(), length, nChars, tabWidth);
}


void
EFont::PrintToStream() const
{
	e_font_family family;
	e_font_style style;
	bzero(family, sizeof(e_font_family));
	bzero(style, sizeof(e_font_style));
	GetFamilyAndStyle(&family, &style);
	euint32 code = FamilyAndStyle();
	euint32 familyIndex = code >> 16;
	euint32 styleIndex = code & 0xffff;
	float size = Size();
	float spacing = Spacing();
	float shear = Shear();
	bool scalable = IsScalable();

	ETK_OUTPUT("\n");
	ETK_OUTPUT("Family: %s\t\tStyle: %s\t\t(%s)\n", family, style, scalable ? "Scalable" : "Not Scalable");
	ETK_OUTPUT("code = %I32u(%I32u,%I32u)\n", code, familyIndex, styleIndex);
	ETK_OUTPUT("size = %g\tspacing = %g\tshear = %g\n", size, spacing, shear);

	eint32 count;
	if(HasFixedSize(&count))
	{
		ETK_OUTPUT("fixed size [%I32i] --- ", count);
		for(eint32 i = 0; i < count; i++)
		{
			float fixedSize;
			if(GetFixedSize(&fixedSize, i)) ETK_OUTPUT("%g ", fixedSize);
		}
	}

	ETK_OUTPUT("\n");
}


