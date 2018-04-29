﻿/* --------------------------------------------------------------------------
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
 * File: FreeType2.cpp
 *
 * --------------------------------------------------------------------------*/

#include <kits/add-ons/font/FontEngine.h>
#include <kits/storage/Directory.h>
#include <kits/support/Locker.h>
#include <kits/support/Autolock.h>
#include <kits/support/StringArray.h>
#include <kits/support/String.h>
#include <kits/support/UTF8.h>
#include <kits/support/SupportDefs.h>
#include <kits/debug/Errors.h>
#include <os/debug.h>

#include <kits/netfreetype/include/ft2build.h>
#include FT_FREETYPE_H
#include <kits/netfreetype/include/freetype/freetype.h>


#include <kits/netmath.h>

#ifdef BHAPI_OS_BEOS
#define FT_ENCODING_UNICODE		ft_encoding_unicode
#define FT_ENCODING_NONE		ft_encoding_none
#define FT_PIXEL_MODE_GRAY		ft_pixel_mode_grays
#define FT_PIXEL_MODE_MONO		ft_pixel_mode_mono
#endif

namespace bhapi {
static FT_Library ft2_library;
static bool ft2_initialized = false;
static BLocker ft2_font_locker;
}

BHAPI_EXPORT bool bhapi::font_freetype2_init(void)
{
    BAutolock <BLocker> autolock(&bhapi::ft2_font_locker);

    if(!bhapi::ft2_initialized)
    {
        FT_Error error = FT_Init_FreeType(&bhapi::ft2_library);
        if(error)
        {
            BHAPI_WARNING("[FONT]: %s --- CAN NOT initialize freetype engine %d\n", __PRETTY_FUNCTION__, error);
            return false;
        }
        bhapi::ft2_initialized = true;
    }

    return true;
}


BHAPI_EXPORT bool bhapi::font_freetype2_is_valid(void)
{
    BAutolock <BLocker> autolock(&bhapi::ft2_font_locker);

    return bhapi::ft2_initialized;
}


BHAPI_EXPORT void bhapi::font_freetype2_cancel(void)
{
    BAutolock <BLocker> autolock(&bhapi::ft2_font_locker);

    if(bhapi::ft2_initialized)
    {
        FT_Done_FreeType(bhapi::ft2_library);
        bhapi::ft2_initialized = false;
    }
}

class BFontFT2 : public BFontEngine {
public:
    BFontFT2(const BEntry *entry,  int32 faceIndex);
    virtual ~BFontFT2();

     int32 CountFaces() const;

    virtual bool IsValid() const;

    virtual bool IsScalable() const;
    virtual void ForceFontAliasing(bool enable);

    virtual float StringWidth(const char *string, float size, float spacing, float shear, bool bold,  int32 length) const;
    virtual void GetHeight(bhapi::font_height *height, float size, float shear, bool bold) const;
    virtual  uint8* RenderString(const char *string,  int32 *width,  int32 *height, bool *is_mono,
                     float size, float spacing, float shear, bool bold,  int32 length);

    virtual bhapi::font_detach_callback* Attach(void (*callback)(void*), void *data);
    virtual bool Detach(bhapi::font_detach_callback *callback);

private:
    char *fFilename;
     int32 fFaceIndex;
     int32 nFaces;
    FT_Face fFace;
    bool fScalable;
    bool fForceFontAliasing;

    bool IsFixedSize(float size) const;
};


BFontFT2::BFontFT2(const BEntry *entry,  int32 faceIndex)
    : BFontEngine(), fFilename(NULL), fFaceIndex(-1), nFaces(-1), fFace(NULL), fScalable(false), fForceFontAliasing(false)
{
    BPath aPath;
    if(entry == NULL || entry->Exists() == false || entry->GetPath(&aPath) != B_OK) return;
    BString filename = aPath.Path();
#ifdef _WIN32
    filename.ReplaceAll("/", "\\");
#endif

    SetRenderMode(B_FONT_RENDER_PIXMAP);

    BAutolock <BLocker> autolock(&bhapi::ft2_font_locker);
    if(!bhapi::ft2_initialized) return;

    FT_Error error = FT_New_Face(bhapi::ft2_library, filename.String(), faceIndex, &fFace);
    if(error || !fFace)
    {
        BHAPI_DEBUG("[FONT]: %s --- CAN NOT load face[%s:%d].", __PRETTY_FUNCTION__, aPath.Path(), faceIndex);
        return;
    }

    if(FT_Select_Charmap(fFace, FT_ENCODING_UNICODE))
    {
//		BHAPI_DEBUG("[FONT]: %s --- font[%s] don't support ENCODING_UNICODE.", __PRETTY_FUNCTION__, aPath.Path());

        if(FT_Select_Charmap(fFace, FT_ENCODING_NONE))
        {
//			BHAPI_WARNING("[FONT]: %s --- font[%s] don't support unicode at all.", __PRETTY_FUNCTION__, aPath.Path());
            FT_Done_Face(fFace);
            fFace = NULL;
            return;
        }
    }

    fFilename = bhapi::strdup(filename.String());
    fFaceIndex = faceIndex;
    nFaces = fFace->num_faces;

    BString family = fFace->family_name;
    if(family.Length() <= 0)
    {
        family = aPath.Leaf();
         int32 cFound;
        if((cFound = family.FindFirst('.')) >= 0) family.Remove(cFound, -1);
        if(family.Length() < 0) family = "Unknown";
    }
    SetFamily(family.String());

    BString style = fFace->style_name;
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
        SetFixedSize(sizes, (int32)fFace->num_fixed_sizes);
        delete[] sizes;
    }

    FT_Done_Face(fFace);
    fFace = NULL;
}


BFontFT2::~BFontFT2()
{
    if(fFilename) delete[] fFilename;

    if(fFace)
    {
        BAutolock <BLocker> autolock(&bhapi::ft2_font_locker);
        FT_Done_Face(fFace);
    }
}


bhapi::font_detach_callback*
BFontFT2::Attach(void (*callback)(void*), void *data)
{
    BAutolock <BLocker> autolock(&bhapi::ft2_font_locker);

    bhapi::font_detach_callback *eCallback = BFontEngine::Attach(callback, data);

    if(eCallback && !fFace)
    {
        if(FT_New_Face(bhapi::ft2_library, fFilename, fFaceIndex, &fFace) || !fFace)
        {
            BHAPI_DEBUG("[FONT]: %s --- CAN NOT load face[%s:%d].", __PRETTY_FUNCTION__, fFilename, fFaceIndex);
            BFontEngine::Detach(eCallback);
            return NULL;
        }

        if(FT_Select_Charmap(fFace, FT_ENCODING_UNICODE))
        {
            if(FT_Select_Charmap(fFace, FT_ENCODING_NONE))
            {
                BHAPI_WARNING("[FONT]: %s --- font[%s] don't support unicode at all.", __PRETTY_FUNCTION__, fFilename);
                FT_Done_Face(fFace);
                fFace = NULL;
                BFontEngine::Detach(eCallback);
                return NULL;
            }
        }
    }

    return eCallback;
}


bool 
BFontFT2::Detach(bhapi::font_detach_callback *callback)
{
    BAutolock <BLocker> autolock(&bhapi::ft2_font_locker);

    if(!BFontEngine::Detach(callback)) return false;

    if(!IsAttached() && fFace)
    {
        FT_Done_Face(fFace);
        fFace = NULL;
    }

    return true;
}


int32
BFontFT2::CountFaces() const
{
    return nFaces;
}


bool 
BFontFT2::IsValid() const
{
    return(fFilename != NULL && fFaceIndex >= 0 && nFaces > 0 && Family() != NULL && Style() != NULL);
}


bool 
BFontFT2::IsScalable() const
{
    return fScalable;
}


void 
BFontFT2::ForceFontAliasing(bool enable)
{
    if(fForceFontAliasing != enable)
    {
        fForceFontAliasing = enable;
    }
}


bool 
BFontFT2::IsFixedSize(float size) const
{
     int32 count = 0;
    if(size <= 0 || !HasFixedSize(&count) || count <= 0) return false;
    for(int32 i = 0; i < count; i++)
    {
        float nsize = 0;
        if(!GetFixedSize(&nsize, i)) continue;
        if(nsize == size) return true;
    }
    return false;
}


float
BFontFT2::StringWidth(const char *string, float size, float spacing, float shear, bool bold,  int32 length) const
{
    BAutolock <BLocker> autolock(&bhapi::ft2_font_locker);

    if(!IsAttached()) return 0;

    bool isfixed = IsFixedSize(size);
    if(!fScalable && !isfixed) return 0;

    if(isfixed ? FT_Set_Pixel_Sizes(fFace, 0, (FT_UInt)size) :
             FT_Set_Char_Size(fFace, 0, (FT_F26Dot6)(size * 64.f), 0, 0)) return 0;
//	if(FT_Set_Pixel_Sizes(fFace, 0, (FT_UInt)size)) return 0;

    unichar16*unicode = bhapi::utf8_convert_to_unicode(string, length);
    if(!unicode) return 0;

    float width = 0;

    int minx = 0, maxx = 0;

    const unichar16*ch;
    int x = 0;
    int fontSpacing = (int)ceil((double)(spacing * size)) * 64;
    for(ch = unicode; !(ch == NULL || *ch == 0); ch = bhapi::unicode_next(ch, NULL))
    {
        FT_UInt glyph_index = FT_Get_Char_Index(fFace, *ch);
        if(FT_Load_Glyph(fFace, glyph_index, FT_LOAD_DEFAULT))
        {
            BHAPI_DEBUG("[FONT]: %s --- FT_Load_Glyph failed.", __PRETTY_FUNCTION__);
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
BFontFT2::GetHeight(bhapi::font_height *height, float size, float shear, bool bold) const
{
    if(!height) return;

    bzero(height, sizeof(bhapi::font_height));

    BAutolock <BLocker> autolock(&bhapi::ft2_font_locker);

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


uint8*
BFontFT2::RenderString(const char *string,  int32 *width,  int32 *height, bool *is_mono,
               float size, float spacing, float shear, bool bold,  int32 length)
{
    if(string == NULL || *string == 0 || length == 0 || width == NULL || height == NULL || is_mono == NULL) return NULL;

    BAutolock <BLocker> autolock(&bhapi::ft2_font_locker);

    if(!IsAttached()) return NULL;

    bool isfixed = IsFixedSize(size);
    if(!fScalable && !isfixed) return NULL;

    float stringWidth;
    bhapi::font_height fontHeight;

    if((stringWidth = StringWidth(string, size, spacing, shear, bold, length)) <= 0) return NULL;
    GetHeight(&fontHeight, size, shear, bold);

     int32 w, h;
    w = (int32)ceil(stringWidth) + 1;
    h = (int32)ceil(fontHeight.ascent + fontHeight.descent) + 1;

     uint8 *bitmap = new  uint8[w * h];
    if(!bitmap)
    {
        BHAPI_WARNING("[FONT]: %s --- Unable to alloc memory for bitmap data.", __PRETTY_FUNCTION__);
        return NULL;
    }
    bzero(bitmap, sizeof(uint8) * (size_t)(w * h));

    unichar16*unicode = bhapi::utf8_convert_to_unicode(string, length);
    if(!unicode)
    {
        delete[] bitmap;
        return NULL;
    }

    const unichar16*ch;
     uint32 x = 0;
     uint32 y = (uint32)ceil(fontHeight.ascent);
    bool do_mono = fForceFontAliasing;
    for(ch = unicode; !(ch == NULL || *ch == 0); ch = bhapi::unicode_next(ch, NULL))
    {
        if(FT_Load_Char(fFace, *ch, (do_mono ? (FT_LOAD_RENDER | FT_LOAD_MONOCHROME) : FT_LOAD_RENDER)))
        {
            BHAPI_DEBUG("[FONT]: %s --- FT_Load_Char failed.", __PRETTY_FUNCTION__);
            continue;
        }

        FT_Bitmap *ftbitmap = &(fFace->glyph->bitmap);

         int32 xx = x + (int32)(fFace->glyph->bitmap_left);
         int32 yy = y - (int32)(fFace->glyph->bitmap_top);
         int32 bitmapWidth = (int32)(ftbitmap->width);
         int32 bitmapHeight = (int32)(ftbitmap->rows);
         int32 lineBytes = (int32)(ftbitmap->pitch > 0 ? ftbitmap->pitch : -(ftbitmap->pitch));
         int32 maxxx = min_c(w, xx + bitmapWidth);
         int32 maxyy = min_c(h, yy + bitmapHeight);

        for(int32 i = yy, p = 0; i < maxyy; i++, p++)
        {
             uint8* dest = bitmap;
            dest += i * w + xx;
            unsigned char* src = ftbitmap->buffer;
            src += p * lineBytes;

            switch(ftbitmap->pixel_mode)
            {
                case FT_PIXEL_MODE_GRAY:
                    for(int32 j = xx; j < maxxx; j++) *dest++ = (uint8)(*src++);
                    break;

                case FT_PIXEL_MODE_MONO:
                    for(int32 j = xx; j < maxxx; )
                    {
                         uint8 val = (uint8)(*src++);
                         int32 left = maxxx - j >= 8 ? 8 : maxxx - j;
                         uint8 left_offset = 7;

                        for(int32 k = 0; k < left; k++, left_offset--, j++)
                            *dest++ = (val & (1 << left_offset)) ? 255 : 0;
                    }
                    break;

                default:
                    BHAPI_DEBUG("[FONT]: %s --- The mode of freetype bitmap not supported.", __PRETTY_FUNCTION__);
            }
        }

        x += (uint32)((float)(fFace->glyph->metrics.horiAdvance) / 64.f) + (uint32)ceil((double)(spacing * size)); // next x
    }

    free(unicode);

    *width = w;
    *height = h;
    *is_mono = do_mono;

    return bitmap;
}


BHAPI_EXPORT bool b_update_freetype2_font_families(bool check_only)
{
    BString fonts_dirs;

#ifdef _WIN32
    const char dir_env_sep = ';';
#else
    const char dir_env_sep = ':';
#endif

    const char *dirs = getenv("FREETYPB_FONTS_DIR");
    if(dirs) fonts_dirs += dirs;

    if(fonts_dirs.Length() <= 0)
    {
#ifdef _WIN32
        fonts_dirs = "C:\\Progra~1\\freetype";
#elif defined(BHAPI_OS_BEOS)
        fonts_dirs = "/boot/os/etc/fonts/ttfonts";
        BHAPI_WARNING("[FONT]: you can set the environment \"FREETYPB_FONTS_DIR\" to match the correct dirs.");
#else
        fonts_dirs = "/usr/share/fonts/freetype";
#endif
    }

    BAutolock <BLocker> autolock(&bhapi::ft2_font_locker);

    if(check_only)
    {
        BHAPI_WARNING("[FONT]: %s --- check_only not implement yet.", __PRETTY_FUNCTION__);
        return false;
    }

    if(!bhapi::ft2_initialized)
    {
        BHAPI_WARNING("[FONT]: Freetype engine not initialize! REFUSE TO LOAD FONTS!!!");
        return false;
    }

    BStringArray *fonts_dirs_array = fonts_dirs.Split(dir_env_sep);
    if(!fonts_dirs_array)
    {
        BHAPI_WARNING("[FONT]: %s --- Couldn't find any font directory.", __PRETTY_FUNCTION__);
        return false;
    }

    BHAPI_DEBUG("[FONT]: Updating FreeType2 fonts ...");
//	BHAPI_DEBUG("[FONT]: Fonts directory number: %d", fonts_dirs_array->CountItems());

    const BString *_fonts_dir;
    for(int32 m = 0; (_fonts_dir = fonts_dirs_array->ItemAt(m)) != NULL; m++)
    {
        BDirectory directory(_fonts_dir->String());
        if(directory.InitCheck() != B_OK)
        {
            BHAPI_WARNING("[FONT]: CAN NOT open fonts directory - \"%s\"!", _fonts_dir->String());
            continue;
        }
//		BHAPI_DEBUG("[FONT]: Opening font directory \"%s\"...", _fonts_dir->String());

        BEntry aEntry;
        while(directory.GetNextEntry(&aEntry, true) == B_OK)
        {
            BPath aPath;
            if(aEntry.GetPath(&aPath) != B_OK) continue;
            BString filename = aPath.Leaf();

            // Ignore not "*.ttf" etc...
            if(filename.Length() < 5) continue;
            const char *fontPattern[] = {".ttf", ".ttc", ".pcf", ".fon", ".pfa", ".pfb"};
            bool isPatternMatched = false;
            for(uint8 i = 0; i < 6; i++)
            {
                if(filename.IFindLast(fontPattern[i]) == filename.Length() - (int32)strlen(fontPattern[i]))
                {
                    isPatternMatched = true;
                    break;
                }
            }
            if(!isPatternMatched) continue;

//			BHAPI_DEBUG("[FONT]: Reading font file \"%s\" ...", aPath.Path());

             int32 faceIndex = 0, nFaces = 0;
            do{
                BFontFT2 *engine = new BFontFT2(&aEntry, faceIndex);
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
//					BHAPI_DEBUG("\tFaces Number: %d", nFaces);
                }

//				BHAPI_DEBUG("\tFamily[%d]: %s", faceIndex, engine->Family());
//				BHAPI_DEBUG("\t\tStyle: %s", engine->Style());

                if(!bhapi::font_add(engine->Family(), engine->Style(), engine)) delete engine;

                faceIndex++;
            }while(faceIndex < nFaces);
        }
    }

    if(fonts_dirs_array) delete fonts_dirs_array;

    BHAPI_DEBUG("[FONT]: FreeType2 fonts updated.");

    return true;
}

