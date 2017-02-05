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
 * File: FreeType2.cpp
 *
 * --------------------------------------------------------------------------*/

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "./../../../../../freetype/include/ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "./../../support/SupportDefs.h"
#include "./../../add-ons/font/FontEngine.h"
#include "./../../support/StringArray.h"
#include "./../../support/Locker.h"
#include "./../../support/Autolock.h"
#include "./../../storage/Directory.h"

#ifdef ETK_OS_BEOS
#define FT_ENCODING_UNICODE		ft_encoding_unicode
#define FT_ENCODING_NONE		ft_encoding_none
#define FT_PIXEL_MODE_GRAY		ft_pixel_mode_grays
#define FT_PIXEL_MODE_MONO		ft_pixel_mode_mono
#endif

static FT_Library _etk_ft2_library_;
static bool _etk_ft2_initialized_ = false;
static ELocker etk_ft2_font_locker;

_IMPEXP_ETK bool etk_font_freetype2_init(void)
{
	EAutolock <ELocker> autolock(&etk_ft2_font_locker);

	if(!_etk_ft2_initialized_)
	{
		FT_Error error = FT_Init_FreeType(&_etk_ft2_library_);
		if(error)
		{
			ETK_WARNING("[FONT]: %s --- CAN NOT initialize freetype engine %d\n", __PRETTY_FUNCTION__, error);
			return false;
		}
		_etk_ft2_initialized_ = true;
	}

	return true;
}


_IMPEXP_ETK bool etk_font_freetype2_is_valid(void)
{
	EAutolock <ELocker> autolock(&etk_ft2_font_locker);

	return _etk_ft2_initialized_;
}


_IMPEXP_ETK void etk_font_freetype2_cancel(void)
{
	EAutolock <ELocker> autolock(&etk_ft2_font_locker);

	if(_etk_ft2_initialized_)
	{
		FT_Done_FreeType(_etk_ft2_library_);
		_etk_ft2_initialized_ = false;
	}
}


class EFontFT2 : public EFontEngine {
public:
	EFontFT2(const EEntry *entry, eint32 faceIndex);
	virtual ~EFontFT2();

	eint32 CountFaces() const;

	virtual bool IsValid() const;

	virtual bool IsScalable() const;
	virtual void ForceFontAliasing(bool enable);

	virtual float StringWidth(const char *string, float size, float spacing, float shear, bool bold, eint32 length) const;
	virtual void GetHeight(e_font_height *height, float size, float shear, bool bold) const;
	virtual euint8* RenderString(const char *string, eint32 *width, eint32 *height, bool *is_mono,
				     float size, float spacing, float shear, bool bold, eint32 length);

	virtual e_font_detach_callback* Attach(void (*callback)(void*), void *data);
	virtual bool Detach(e_font_detach_callback *callback);

private:
	char *fFilename;
	eint32 fFaceIndex;
	eint32 nFaces;
	FT_Face fFace;
	bool fScalable;
	bool fForceFontAliasing;

	bool IsFixedSize(float size) const;
};


EFontFT2::EFontFT2(const EEntry *entry, eint32 faceIndex)
	: EFontEngine(), fFilename(NULL), fFaceIndex(-1), nFaces(-1), fFace(NULL), fScalable(false), fForceFontAliasing(false)
{
	EPath aPath;
	if(entry == NULL || entry->Exists() == false || entry->GetPath(&aPath) != E_OK) return;
	EString filename = aPath.Path();
#ifdef _WIN32
	filename.ReplaceAll("/", "\\");
#endif

	SetRenderMode(E_FONT_RENDER_PIXMAP);

	EAutolock <ELocker> autolock(&etk_ft2_font_locker);
	if(!_etk_ft2_initialized_) return;

	FT_Error error = FT_New_Face(_etk_ft2_library_, filename.String(), faceIndex, &fFace);
	if(error || !fFace)
	{
		ETK_DEBUG("[FONT]: %s --- CAN NOT load face[%s:%d].", __PRETTY_FUNCTION__, aPath.Path(), faceIndex);
		return;
	}

	if(FT_Select_Charmap(fFace, FT_ENCODING_UNICODE))
	{
//		ETK_DEBUG("[FONT]: %s --- font[%s] don't support ENCODING_UNICODE.", __PRETTY_FUNCTION__, aPath.Path());

		if(FT_Select_Charmap(fFace, FT_ENCODING_NONE))
		{
//			ETK_WARNING("[FONT]: %s --- font[%s] don't support unicode at all.", __PRETTY_FUNCTION__, aPath.Path());
			FT_Done_Face(fFace);
			fFace = NULL;
			return;
		}
	}

	fFilename = EStrdup(filename.String());
	fFaceIndex = faceIndex;
	nFaces = fFace->num_faces;

	EString family = fFace->family_name;
	if(family.Length() <= 0)
	{
		family = aPath.Leaf();
		eint32 cFound;
		if((cFound = family.FindFirst('.')) >= 0) family.Remove(cFound, -1);
		if(family.Length() < 0) family = "Unknown";
	}
	SetFamily(family.String());

	EString style = fFace->style_name;
	if(style.Length() <= 0)
	{
		if((fFace->style_flags & FT_STYLE_FLAG_BOLD) && (fFace->style_flags & FT_STYLE_FLAG_ITALIC))
			style = "Bold Italic";
		else if(fFace->style_flags & FT_STYLE_FLAG_BOLD)
			style = "Bold";
		else if(fFace->style_flags & FT_STYLE_FLAG_ITALIC)
			style = "Italic";
		else
			style = "Regular";
	}
	SetStyle(style.String());

	if(FT_IS_SCALABLE(fFace)) fScalable = true;

	if(fFace->num_fixed_sizes > 0)
	{
		float *sizes = new float[(int)fFace->num_fixed_sizes];
		for(int i = 0; i < fFace->num_fixed_sizes; i++) sizes[i] = (float)(fFace->available_sizes[i].height);
		SetFixedSize(sizes, (eint32)fFace->num_fixed_sizes);
		delete[] sizes;
	}

	FT_Done_Face(fFace);
	fFace = NULL;
}


EFontFT2::~EFontFT2()
{
	if(fFilename) delete[] fFilename;

	if(fFace)
	{
		EAutolock <ELocker> autolock(&etk_ft2_font_locker);
		FT_Done_Face(fFace);
	}
}


e_font_detach_callback*
EFontFT2::Attach(void (*callback)(void*), void *data)
{
	EAutolock <ELocker> autolock(&etk_ft2_font_locker);

	e_font_detach_callback *eCallback = EFontEngine::Attach(callback, data);

	if(eCallback && !fFace)
	{
		if(FT_New_Face(_etk_ft2_library_, fFilename, fFaceIndex, &fFace) || !fFace)
		{
			ETK_DEBUG("[FONT]: %s --- CAN NOT load face[%s:%d].", __PRETTY_FUNCTION__, fFilename, fFaceIndex);
			EFontEngine::Detach(eCallback);
			return NULL;
		}

		if(FT_Select_Charmap(fFace, FT_ENCODING_UNICODE))
		{
			if(FT_Select_Charmap(fFace, FT_ENCODING_NONE))
			{
				ETK_WARNING("[FONT]: %s --- font[%s] don't support unicode at all.", __PRETTY_FUNCTION__, fFilename);
				FT_Done_Face(fFace);
				fFace = NULL;
				EFontEngine::Detach(eCallback);
				return NULL;
			}
		}
	}

	return eCallback;
}


bool
EFontFT2::Detach(e_font_detach_callback *callback)
{
	EAutolock <ELocker> autolock(&etk_ft2_font_locker);

	if(!EFontEngine::Detach(callback)) return false;

	if(!IsAttached() && fFace)
	{
		FT_Done_Face(fFace);
		fFace = NULL;
	}

	return true;
}


eint32
EFontFT2::CountFaces() const
{
	return nFaces;
}


bool
EFontFT2::IsValid() const
{
	return(fFilename != NULL && fFaceIndex >= 0 && nFaces > 0 && Family() != NULL && Style() != NULL);
}


bool
EFontFT2::IsScalable() const
{
	return fScalable;
}


void
EFontFT2::ForceFontAliasing(bool enable)
{
	if(fForceFontAliasing != enable)
	{
		fForceFontAliasing = enable;
	}
}


bool
EFontFT2::IsFixedSize(float size) const
{
	eint32 count = 0;
	if(size <= 0 || !HasFixedSize(&count) || count <= 0) return false;
	for(eint32 i = 0; i < count; i++)
	{
		float nsize = 0;
		if(!GetFixedSize(&nsize, i)) continue;
		if(nsize == size) return true;
	}
	return false;
}


float
EFontFT2::StringWidth(const char *string, float size, float spacing, float shear, bool bold, eint32 length) const
{
	EAutolock <ELocker> autolock(&etk_ft2_font_locker);

	if(!IsAttached()) return 0;

	bool isfixed = IsFixedSize(size);
	if(!fScalable && !isfixed) return 0;

	if(isfixed ? FT_Set_Pixel_Sizes(fFace, 0, (FT_UInt)size) :
		     FT_Set_Char_Size(fFace, 0, (FT_F26Dot6)(size * 64.f), 0, 0)) return 0;
//	if(FT_Set_Pixel_Sizes(fFace, 0, (FT_UInt)size)) return 0;

	eunichar *unicode = e_utf8_convert_to_unicode(string, length);
	if(!unicode) return 0;

	float width = 0;

	int minx = 0, maxx = 0;

	const eunichar *ch;
	int x = 0;
	int fontSpacing = (int)ceil((double)(spacing * size)) * 64;
	for(ch = unicode; !(ch == NULL || *ch == 0); ch = e_unicode_next(ch, NULL))
	{
		FT_UInt glyph_index = FT_Get_Char_Index(fFace, *ch);
		if(FT_Load_Glyph(fFace, glyph_index, FT_LOAD_DEFAULT))
		{
			ETK_DEBUG("[FONT]: %s --- FT_Load_Glyph failed.", __PRETTY_FUNCTION__);
			continue;
		}

		FT_Glyph_Metrics *metrics = &(fFace->glyph->metrics);

		minx = min_c(minx, x + metrics->horiBearingX);
		maxx = max_c(maxx, x + max_c(metrics->horiAdvance, metrics->horiBearingX + metrics->width));

		x += metrics->horiAdvance + fontSpacing;
	}
	if(x > fontSpacing) x -= fontSpacing;

	width = (float)(maxx - minx) / 64.f;

	free(unicode);

	return width;
}


void
EFontFT2::GetHeight(e_font_height *height, float size, float shear, bool bold) const
{
	if(!height) return;

	bzero(height, sizeof(e_font_height));

	EAutolock <ELocker> autolock(&etk_ft2_font_locker);

	if(!IsAttached()) return;

	bool isfixed = IsFixedSize(size);
	if(!fScalable && !isfixed) return;

	if(isfixed ? FT_Set_Pixel_Sizes(fFace, 0, (FT_UInt)size) :
		     FT_Set_Char_Size(fFace, 0, (FT_F26Dot6)(size * 64.f), 0, 0)) return;
//	if(FT_Set_Pixel_Sizes(fFace, 0, (FT_UInt)size)) return;

	if(!isfixed)
	{
		FT_Fixed scale = fFace->size->metrics.y_scale;
		height->ascent = (float)(FT_MulFix(fFace->bbox.yMax, scale)) / 64.f;
		height->descent = -1.f * (float)(FT_MulFix(fFace->bbox.yMin, scale)) / 64.f;
		height->leading = (float)(FT_MulFix(fFace->height, scale)) / 64.f - height->ascent - height->descent;
		if(height->leading < 0) height->leading *= -1.f;
		else height->ascent += height->leading;
	}
	else
	{
		// TODO
		height->ascent = size * 0.9f;
		height->descent = size * 0.1f;
		height->leading = 0;
	}
}


euint8*
EFontFT2::RenderString(const char *string, eint32 *width, eint32 *height, bool *is_mono,
		       float size, float spacing, float shear, bool bold, eint32 length)
{
	if(string == NULL || *string == 0 || length == 0 || width == NULL || height == NULL || is_mono == NULL) return NULL;

	EAutolock <ELocker> autolock(&etk_ft2_font_locker);

	if(!IsAttached()) return NULL;

	bool isfixed = IsFixedSize(size);
	if(!fScalable && !isfixed) return NULL;

	float stringWidth;
	e_font_height fontHeight;

	if((stringWidth = StringWidth(string, size, spacing, shear, bold, length)) <= 0) return NULL;
	GetHeight(&fontHeight, size, shear, bold);

	eint32 w, h;
	w = (eint32)ceil(stringWidth) + 1;
	h = (eint32)ceil(fontHeight.ascent + fontHeight.descent) + 1;

	euint8 *bitmap = new euint8[w * h];
	if(!bitmap)
	{
		ETK_WARNING("[FONT]: %s --- Unable to alloc memory for bitmap data.", __PRETTY_FUNCTION__);
		return NULL;
	}
	bzero(bitmap, sizeof(euint8) * (size_t)(w * h));

	eunichar *unicode = e_utf8_convert_to_unicode(string, length);
	if(!unicode)
	{
		delete[] bitmap;
		return NULL;
	}

	const eunichar *ch;
	euint32 x = 0;
	euint32 y = (euint32)ceil(fontHeight.ascent);
	bool do_mono = fForceFontAliasing;
	for(ch = unicode; !(ch == NULL || *ch == 0); ch = e_unicode_next(ch, NULL))
	{
		if(FT_Load_Char(fFace, *ch, (do_mono ? (FT_LOAD_RENDER | FT_LOAD_MONOCHROME) : FT_LOAD_RENDER)))
		{
			ETK_DEBUG("[FONT]: %s --- FT_Load_Char failed.", __PRETTY_FUNCTION__);
			continue;
		}

		FT_Bitmap *ftbitmap = &(fFace->glyph->bitmap);

		eint32 xx = x + (eint32)(fFace->glyph->bitmap_left);
		eint32 yy = y - (eint32)(fFace->glyph->bitmap_top);
		eint32 bitmapWidth = (eint32)(ftbitmap->width);
		eint32 bitmapHeight = (eint32)(ftbitmap->rows);
		eint32 lineBytes = (eint32)(ftbitmap->pitch > 0 ? ftbitmap->pitch : -(ftbitmap->pitch));
		eint32 maxxx = min_c(w, xx + bitmapWidth);
		eint32 maxyy = min_c(h, yy + bitmapHeight);

		for(eint32 i = yy, p = 0; i < maxyy; i++, p++)
		{
			euint8* dest = bitmap;
			dest += i * w + xx;
			unsigned char* src = ftbitmap->buffer;
			src += p * lineBytes;

			switch(ftbitmap->pixel_mode)
			{
				case FT_PIXEL_MODE_GRAY:
					for(eint32 j = xx; j < maxxx; j++) *dest++ = (euint8)(*src++);
					break;

				case FT_PIXEL_MODE_MONO:
					for(eint32 j = xx; j < maxxx; )
					{
						euint8 val = (euint8)(*src++);
						eint32 left = maxxx - j >= 8 ? 8 : maxxx - j;
						euint8 left_offset = 7;

						for(eint32 k = 0; k < left; k++, left_offset--, j++)
							*dest++ = (val & (1 << left_offset)) ? 255 : 0;
					}
					break;

				default:
					ETK_DEBUG("[FONT]: %s --- The mode of freetype bitmap not supported.", __PRETTY_FUNCTION__);
			}
		}

		x += (euint32)((float)(fFace->glyph->metrics.horiAdvance) / 64.f) + (euint32)ceil((double)(spacing * size)); // next x
	}

	free(unicode);

	*width = w;
	*height = h;
	*is_mono = do_mono;

	return bitmap;
}


_IMPEXP_ETK bool etk_update_freetype2_font_families(bool check_only)
{
	EString fonts_dirs;

#ifdef _WIN32
	const char dir_env_sep = ';';
#else
	const char dir_env_sep = ':';
#endif

	const char *dirs = getenv("FREETYPE_FONTS_DIR");
	if(dirs) fonts_dirs += dirs;

	if(fonts_dirs.Length() <= 0)
	{
#ifdef _WIN32
		fonts_dirs = "C:\\Progra~1\\freetype";
#elif defined(ETK_OS_BEOS)
		fonts_dirs = "/boot/beos/etc/fonts/ttfonts";
		ETK_WARNING("[FONT]: you can set the environment \"FREETYPE_FONTS_DIR\" to match the correct dirs.");
#else
		fonts_dirs = "/usr/share/fonts/freetype";
#endif
	}

	EAutolock <ELocker> autolock(&etk_ft2_font_locker);

	if(check_only)
	{
		ETK_WARNING("[FONT]: %s --- check_only not implement yet.", __PRETTY_FUNCTION__);
		return false;
	}

	if(!_etk_ft2_initialized_)
	{
		ETK_WARNING("[FONT]: Freetype engine not initialize! REFUSE TO LOAD FONTS!!!");
		return false;
	}

	EStringArray *fonts_dirs_array = fonts_dirs.Split(dir_env_sep);
	if(!fonts_dirs_array)
	{
		ETK_WARNING("[FONT]: %s --- Couldn't find any font directory.", __PRETTY_FUNCTION__);
		return false;
	}

	ETK_DEBUG("[FONT]: Updating FreeType2 fonts ...");
//	ETK_DEBUG("[FONT]: Fonts directory number: %d", fonts_dirs_array->CountItems());

	const EString *_fonts_dir;
	for(eint32 m = 0; (_fonts_dir = fonts_dirs_array->ItemAt(m)) != NULL; m++)
	{
		EDirectory directory(_fonts_dir->String());
		if(directory.InitCheck() != E_OK)
		{
			ETK_WARNING("[FONT]: CAN NOT open fonts directory - \"%s\"!", _fonts_dir->String());
			continue;
		}
//		ETK_DEBUG("[FONT]: Opening font directory \"%s\"...", _fonts_dir->String());

		EEntry aEntry;
		while(directory.GetNextEntry(&aEntry, true) == E_OK)
		{
			EPath aPath;
			if(aEntry.GetPath(&aPath) != E_OK) continue;
			EString filename = aPath.Leaf();

			// Ignore not "*.ttf" etc...
			if(filename.Length() < 5) continue;
			const char *fontPattern[] = {".ttf", ".ttc", ".pcf", ".fon", ".pfa", ".pfb"};
			bool isPatternMatched = false;
			for(euint8 i = 0; i < 6; i++)
			{
				if(filename.IFindLast(fontPattern[i]) == filename.Length() - (eint32)strlen(fontPattern[i]))
				{
					isPatternMatched = true;
					break;
				}
			}
			if(!isPatternMatched) continue;

//			ETK_DEBUG("[FONT]: Reading font file \"%s\" ...", aPath.Path());

			eint32 faceIndex = 0, nFaces = 0;
			do{
				EFontFT2 *engine = new EFontFT2(&aEntry, faceIndex);
				if(!engine || !engine->IsValid())
				{
					if(engine) delete engine;

					if(faceIndex == 0)
						break;
					else
					{
						faceIndex++;
						continue;
					}
				}

				if(faceIndex == 0)
				{
					nFaces = engine->CountFaces();
//					ETK_DEBUG("\tFaces Number: %d", nFaces);
				}

//				ETK_DEBUG("\tFamily[%d]: %s", faceIndex, engine->Family());
//				ETK_DEBUG("\t\tStyle: %s", engine->Style());

				if(!etk_font_add(engine->Family(), engine->Style(), engine)) delete engine;

				faceIndex++;
			}while(faceIndex < nFaces);
		}
	}

	if(fonts_dirs_array) delete fonts_dirs_array;

	ETK_DEBUG("[FONT]: FreeType2 fonts updated.");

	return true;
}

