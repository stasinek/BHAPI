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
 * File: FontEngine.cpp
 *
 * --------------------------------------------------------------------------*/

#ifdef __BEOS__
#include "../../app/AppDefs.h"
#include "../../interface/Font.h"
#endif

#include "FontEngine.h"

#include "../../support/Locker.h"
#include "../../support/Autolock.h"
#include "../../support/Errors.h"
#include "../../support/StringClass.h"
#include "../../app/Application.h"
#include "../../add-ons/graphics/GraphicsEngine.h"
#include "../../kernel/Debug.h"

#ifdef HAVE_FT2
namespace bhapi {
extern bool font_freetype2_init(void);
extern bool font_freetype2_is_valid(void);
extern void font_freetype2_cancel(void);
extern bool update_freetype2_font_families(bool check_only);
}
#endif // HAVE_FT2

static BFont* temp_plain_font = NULL;
static BFont* temp_bold_font = NULL;
static BFont* temp_fixed_font = NULL;

namespace bhapi {
BLocker font_locker;
bool font_initialized = false;
bool font_canceling = false;
BStringArray font_families;
}

BHAPI_EXPORT bool bhapi::font_add(const char *family, const char *style, BFontEngine *engine)
{
    if(family == NULL || *family == 0 || style == NULL || *style == 0 || engine == NULL) return false;

    BAutolock <BLocker> autolock(&bhapi::font_locker);
    if(!bhapi::font_initialized || engine->fServing != NULL) return false;

    BStringArray *styles = NULL;
    bhapi::font_families.ItemAt(bhapi::font_families.FindString(family), (void**)&styles);
    if(styles ? styles->FindString(style) >= 0 : false)
    {
//		BHAPI_DEBUG("[FONT]: %s --- style[%s] of family[%s] already exists.", __PRETTY_FUNCTION__, style, family);
        return false;
    }

    if(!styles)
    {
        styles = new BStringArray;
        if(!styles || !styles->AddItem(style, (void*)engine) ||
           !bhapi::font_families.AddItem(family, (void*)styles))
        {
            BHAPI_DEBUG("[FONT]: %s --- Add style[%s] of family[%s] failed.", __PRETTY_FUNCTION__, style, family);
            if(styles) delete styles;
            return false;
        }
    }
    else
    {
        if(!styles->AddItem(style, (void*)engine))
        {
            BHAPI_DEBUG("[FONT]: %s --- Add style[%s] of family[%s] failed.", __PRETTY_FUNCTION__, style, family);
            return false;
        }
    }

    engine->fServing = styles;

    return true;
}


BHAPI_EXPORT BFontEngine::BFontEngine()
    : fFamily(NULL), fStyle(NULL), fFixedSize(NULL), nFixedSize(0), fRenderMode(B_FONT_RENDER_UNKNOWN), fServing(NULL)
{
}


BHAPI_EXPORT BFontEngine::BFontEngine(const char *family, const char *style)
    : fFamily(NULL), fStyle(NULL), fFixedSize(NULL), nFixedSize(0), fRenderMode(B_FONT_RENDER_UNKNOWN), fServing(NULL)
{
    SetFamily(family);
    SetStyle(style);
}


bool BFontEngine::InServing() const
{
    BAutolock <BLocker> autolock(&bhapi::font_locker);
    return(fServing != NULL);
}


void BFontEngine::OutOfServing()
{
    BAutolock <BLocker> autolock(&bhapi::font_locker);
    if(bhapi::font_canceling || fServing == NULL) return;

    BFontEngine *engine = NULL;
    for(int32 i = 0; i < fServing->CountItems(); i++)
    {
        fServing->ItemAt(i, (void**)&engine);
        if(engine != this) continue;
        if(fServing->RemoveItem(i))
        {
            if(fServing->CountItems() <= 0)
            {
                BStringArray *styles = NULL;
                for(int32 j = 0; j < bhapi::font_families.CountItems(); j++)
                {
                    bhapi::font_families.ItemAt(j, (void**)&styles);
                    if(styles != fServing) continue;
                    if(bhapi::font_families.RemoveItem(j)) delete fServing;
                    break;
                }
            }
            fServing = NULL;
        }
        break;
    }
}


BFontEngine::~BFontEngine()
{
    if(InServing()) OutOfServing();

    for(int32 i = 0; i < fAttached.CountItems(); i++)
    {
        bhapi::font_detach_callback *eCallback = (bhapi::font_detach_callback*)fAttached.ItemAt(i);
        if(!eCallback) continue;
        if(eCallback->callback != NULL) eCallback->callback(eCallback->data);
        delete eCallback;
    }
    fAttached.MakeEmpty();

    if(fFamily) delete[] fFamily;
    if(fStyle) delete[] fStyle;
    if(fFixedSize) delete[] fFixedSize;
}


bhapi::font_detach_callback* BFontEngine::Attach(void (*callback)(void*), void *data)
{
    bhapi::font_detach_callback *eCallback = new bhapi::font_detach_callback;
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
BFontEngine::IsAttached() const
{
    return(fAttached.CountItems() > 0);
}


bool
BFontEngine::Detach(bhapi::font_detach_callback *callback)
{
    if(!callback) return false;
    for(int32 i = fAttached.CountItems() - 1; i >= 0; i--)
    {
        bhapi::font_detach_callback *eCallback = (bhapi::font_detach_callback*)fAttached.ItemAt(i);
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
BFontEngine::Lock()
{
    return fLocker.Lock();
}


BHAPI_EXPORT void BFontEngine::Unlock()
{
    fLocker.Unlock();
}


BHAPI_EXPORT const char* BFontEngine::Family() const
{
    return fFamily;
}


BHAPI_EXPORT const char* BFontEngine::Style() const
{
    return fStyle;
}


BHAPI_EXPORT bhapi::font_render_mode BFontEngine::RenderMode() const
{
    return fRenderMode;
}


BHAPI_EXPORT void BFontEngine::SetRenderMode(bhapi::font_render_mode rmode)
{
    fRenderMode = rmode;
}


BHAPI_EXPORT bool BFontEngine::HasFixedSize(int32 *count) const
{
    if(nFixedSize > 0 && fFixedSize != NULL)
    {
        if(count) *count = nFixedSize;
        return true;
    }

    return false;
}


BHAPI_EXPORT bool BFontEngine::GetFixedSize(float *size,  int32 index) const
{
    if(size == NULL || index < 0) return false;

    if(nFixedSize > index && fFixedSize != NULL)
    {
        *size = fFixedSize[index];
        return true;
    }

    return false;
}

BHAPI_EXPORT void BFontEngine::SetFamily(const char *family)
{
    if(fFamily) delete[] fFamily;
    fFamily = NULL;
    if(family) fFamily = bhapi::strdup_dirty(family);
}


BHAPI_EXPORT void BFontEngine::SetStyle(const char *style)
{
    if(fStyle) delete[] fStyle;
    fStyle = NULL;
    if(style) fStyle = bhapi::strdup_dirty(style);
}


BHAPI_EXPORT void BFontEngine::SetFixedSize(float *sizes,  int32 count)
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
BFontEngine::IsValid() const
{
    return false;
}


bool
BFontEngine::IsScalable() const
{
    return false;
}


void
BFontEngine::ForceFontAliasing(bool enable)
{
}


float
BFontEngine::StringWidth(const char *string, float size, float spacing, float shear, bool bold,  int32 length) const
{
    return 0;
}


void
BFontEngine::GetHeight(bhapi::font_height *height, float size, float shear, bool bold) const
{
    if(height) bzero(height, sizeof(bhapi::font_height));
}


float
BFontEngine::StringWidth(const BString &str, float size, float spacing, float shear, bool bold,  int32 length) const
{
    return StringWidth(str.String(), size, spacing, shear, bold, length);
}


BRect
BFontEngine::RenderString(BHandler *view, const char *string,
              float size, float spacing,
              float shear, bool bold,  int32 length)
{
    return BRect();
}


BRect
BFontEngine::RenderString(BHandler *view, const BString &str,
              float size, float spacing,
              float shear, bool bold,  int32 length)
{
    return RenderString(view, str.String(), size, spacing, shear, bold, length);
}


uint8*
BFontEngine::RenderString(const char *string,  int32 *width,  int32 *height, bool *is_mono,
              float size, float spacing,
              float shear, bool bold,  int32 length)
{
    return NULL;
}


uint8*
BFontEngine::RenderString(const BString &str,  int32 *width,  int32 *height, bool *is_mono,
              float size, float spacing,
              float shear, bool bold,  int32 length)
{
    return RenderString(str.String(), width, height, is_mono, size, spacing, shear, bold, length);
}


BHAPI_EXPORT  int32 bhapi::count_font_families(void)
{
    BAutolock <BLocker> autolock(&bhapi::font_locker);

    return bhapi::font_families.CountItems();
}


BHAPI_EXPORT status_t bhapi::get_font_family(int32 index, const char **name)
{
    if(!name) return B_BAD_VALUE;

    BAutolock <BLocker> autolock(&bhapi::font_locker);

    const BString *str = bhapi::font_families.ItemAt(index);
    if(!str) return B_ERROR;

    *name = str->String();
    return B_OK;
}


BHAPI_EXPORT  int32 bhapi::get_font_family_index(const char *name)
{
    if(!name) return -1;

    BAutolock <BLocker> autolock(&bhapi::font_locker);

     int32 fIndex = bhapi::font_families.FindString(name);
    return fIndex;
}


BHAPI_EXPORT  int32 bhapi::get_font_style_index(const char *family, const char *name)
{
    if(!family || !name) return -1;

    BAutolock <BLocker> autolock(&bhapi::font_locker);

     int32 index = bhapi::font_families.FindString(family);
    if(index < 0) return -1;

    BStringArray *styles = NULL;
    bhapi::font_families.ItemAt(index, (void**)&styles);
    if(!styles) return -1;

    index = styles->FindString(name);
    return index;
}


BHAPI_EXPORT  int32 bhapi::count_font_styles(const char *name)
{
    BAutolock <BLocker> autolock(&bhapi::font_locker);

    return bhapi::count_font_styles(bhapi::font_families.FindString(name));
}


BHAPI_EXPORT  int32 bhapi::count_font_styles(int32 index)
{
    if(index < 0) return -1;

    BAutolock <BLocker> autolock(&bhapi::font_locker);

    BStringArray *styles = NULL;
    bhapi::font_families.ItemAt(index, (void**)&styles);

    return(styles ? styles->CountItems() : -1);
}


BHAPI_EXPORT status_t bhapi::get_font_style(const char *family,  int32 index, const char **name)
{
    if(!family || !name) return B_BAD_VALUE;

    BAutolock <BLocker> autolock(&bhapi::font_locker);

     int32 fIndex = bhapi::font_families.FindString(family);
    if(fIndex < 0) return B_ERROR;

    BStringArray *styles = NULL;
    bhapi::font_families.ItemAt(fIndex, (void**)&styles);
    if(!styles) return B_ERROR;

    const BString *str = styles->ItemAt(index);
    if(!str) return B_ERROR;

    *name = str->String();
    return B_OK;
}


BHAPI_EXPORT BFontEngine* bhapi::get_font_engine(const char *family, const char *style)
{
    if(!family || !style) return NULL;

    BAutolock <BLocker> autolock(&bhapi::font_locker);

    BStringArray *styles = NULL;
    bhapi::font_families.ItemAt(bhapi::font_families.FindString(family), (void**)&styles);
    if(!styles) return NULL;

    BFontEngine *engine = NULL;
    styles->ItemAt(styles->FindString(style), (void**)&engine);

    return engine;
}


BHAPI_EXPORT BFontEngine* bhapi::get_font_engine(int32 familyIndex,  int32 styleIndex)
{
    BAutolock <BLocker> autolock(&bhapi::font_locker);

    BStringArray *styles = NULL;
    bhapi::font_families.ItemAt(familyIndex, (void**)&styles);
    if(!styles) return NULL;

    BFontEngine *engine = NULL;
    styles->ItemAt(styleIndex, (void**)&engine);

    return engine;
}

BHAPI_EXPORT bool bhapi::font_other_init()
{
    // TODO
    return true;
}


BHAPI_EXPORT void bhapi::font_other_cancel()
{
    // TODO
}


BHAPI_EXPORT bool bhapi::update_other_font_families(bool check_only)
{
    // TODO
    return(check_only ? false : true);
}


BHAPI_EXPORT bool bhapi::update_font_families(bool check_only)
{
    BAutolock <BLocker> autolock(&bhapi::font_locker);

    if(!bhapi::font_initialized) return false;

    if(!check_only)
    {
        BStringArray *styles;
        for(int32 i = 0; i < bhapi::font_families.CountItems(); i++)
        {
            styles = NULL;
            bhapi::font_families.ItemAt(i, (void**)&styles);
            if(styles)
            {
                BFontEngine *engine;
                for(int32 j = 0; j < styles->CountItems(); j++)
                {
                    engine = NULL;
                    styles->ItemAt(j, (void**)&engine);
                    if(engine) delete engine;
                }
                styles->MakeEmpty();
                delete styles;
            }
        }
        bhapi::font_families.MakeEmpty();
    }

    bool updateFailed = false;

    if(!(bhapi::be_app == NULL || bhapi::be_app->fGraphicsEngine == NULL)) bhapi::be_app->fGraphicsEngine->UpdateFonts(check_only);

    // TODO: fix the return value style
#ifdef HAVE_FT2
    if(bhapi::font_freetype2_is_valid())
    {
        if(bhapi::update_freetype2_font_families(check_only))
        {
            if(check_only) return true;
        }
        else if(!updateFailed && !check_only)
        {
            updateFailed = true;
        }
    }
#endif

    if(bhapi::update_other_font_families(check_only))
    {
        if(check_only) return true;
    }
    else if(!updateFailed && !check_only)
    {
        updateFailed = true;
    }

    return(check_only ? false : !updateFailed);
}


BHAPI_LOCAL bool bhapi::font_init(void)
{
    BAutolock <BLocker> autolock(&bhapi::font_locker);

    if(!bhapi::font_initialized)
    {
        BHAPI_DEBUG("[FONT]: Initalizing fonts ...");

#ifdef HAVE_FT2
        bhapi::font_freetype2_init();
#endif
        bhapi::font_other_init();
        bhapi::font_initialized = true;
        bhapi::update_font_families(false);

        // TODO
        temp_plain_font = new BFont();
        if(temp_plain_font)
        {
#ifdef BHAPI_GRAPHICS_BEOS_BUILT_IN
            font_family bFamily;
            font_style bStyle;
             __be_plain_font->GetFamilyAndStyle(&bFamily, &bStyle);
            if(bhapi::plain_font->SetFamilyAndStyle(bFamily, bStyle) == B_OK)
            {
                bhapi::plain_font->SetSize(__be_plain_font->Size());
            }
            else
            {
#endif // BHAPI_GRAPHICS_BEOS_BUILT_IN
            const char *family = getenv("BHAPI_PLAIN_FONT_FAMILY");
            const char *style = getenv("BHAPI_PLAIN_FONT_STYLE");
            if(((family == NULL || *family == 0 || style == NULL || *style == 0) ?
                (BHAPI_WARNING("[FONT]: $BHAPI_PLAIN_FONT_FAMILY = 0 || $BHAPI_PLAIN_FONT_STYLE = 0"), B_ERROR) :
                temp_plain_font->SetFamilyAndStyle(family, style)) != B_OK)
#ifdef BHAPI_GRAPHICS_WIN32_BUILT_IN
            if(temp_plain_font->SetFamilyAndStyle("SimSun", "Regular") != B_OK)
            if(temp_plain_font->SetFamilyAndStyle("宋体", "Regular") != B_OK)
            if(temp_plain_font->SetFamilyAndStyle("SimHei", "Regular") != B_OK)
            if(temp_plain_font->SetFamilyAndStyle("黑体", "Regular") != B_OK)
            if(temp_plain_font->SetFamilyAndStyle("Tahoma", "Regular") != B_OK)
#endif // BHAPI_GRAPHICS_WIN32_BUILT_IN
            if(temp_plain_font->SetFamilyAndStyle("helvetica", "medium") != B_OK) temp_plain_font->SetFamilyAndStyle(0);

            float fsize = 12;
            const char *fontSize = getenv("BHAPI_PLAIN_FONT_SIZE");
            if(!(fontSize == NULL || *fontSize == 0)) fsize = (float)atoi(fontSize);
            else BHAPI_WARNING("[FONT]: $BHAPI_PLAIN_FONT_SIZE = 0");

            if(temp_plain_font->IsScalable() == false)
            {
                temp_plain_font->GetFixedSize(&fsize);
                temp_plain_font->SetSize(fsize);
            }
            else
            {
                temp_plain_font->SetSize(fsize);
            }
#ifdef BHAPI_GRAPHICS_BEOS_BUILT_IN
            }
#endif // BHAPI_GRAPHICS_BEOS_BUILT_IN
            temp_plain_font->SetSpacing(0.05f);
            temp_plain_font->SetShear(90);
        }
        temp_bold_font = new BFont();
        if(temp_bold_font)
        {
#ifdef BHAPI_GRAPHICS_BEOS_BUILT_IN
            font_family bFamily;
            font_style bStyle;
            __be_bold_font->GetFamilyAndStyle(&bFamily, &bStyle);
            if(temp_bold_font->SetFamilyAndStyle(bFamily, bStyle) == B_OK)
            {
                temp_bold_font->SetSize(__be_bold_font->Size());
            }
            else
            {
#endif // BHAPI_GRAPHICS_BEOS_BUILT_IN
            const char *family = getenv("BHAPI_BOLD_FONT_FAMILY");
            const char *style = getenv("BHAPI_BOLD_FONT_STYLE");
            if(((family == NULL || *family == 0 || style == NULL || *style == 0) ?
                (BHAPI_WARNING("[FONT]: $BHAPI_BOLD_FONT_FAMILY = 0 || $BHAPI_BOLD_FONT_STYLE = 0"), B_ERROR) :
                temp_bold_font->SetFamilyAndStyle(family, style)) != B_OK)
#ifdef BHAPI_GRAPHICS_WIN32_BUILT_IN
            if(temp_bold_font->SetFamilyAndStyle("SimHei", "Regular") != B_OK)
            if(temp_bold_font->SetFamilyAndStyle("黑体", "Regular") != B_OK)
            if(temp_bold_font->SetFamilyAndStyle("Tahoma", "Regular") != B_OK)
#endif // BHAPI_GRAPHICS_WIN32_BUILT_IN
            if(temp_bold_font->SetFamilyAndStyle("helvetica", "bold") != B_OK) temp_bold_font->SetFamilyAndStyle(0);

            float fsize = 12;
            const char *fontSize = getenv("BHAPI_BOLD_FONT_SIZE");
            if(!(fontSize == NULL || *fontSize == 0)) fsize = (float)atoi(fontSize);
            else BHAPI_WARNING("[FONT]: $BHAPI_BOLD_FONT_SIZE = 0");

            if(temp_bold_font->IsScalable() == false)
            {
                temp_bold_font->GetFixedSize(&fsize);
                temp_bold_font->SetSize(fsize);
            }
            else
            {
                temp_bold_font->SetSize(fsize);
            }
#ifdef BHAPI_GRAPHICS_BEOS_BUILT_IN
            }
#endif // BHAPI_GRAPHICS_BEOS_BUILT_IN
            temp_bold_font->SetSpacing(0.05f);
            temp_bold_font->SetShear(90);
        }
        temp_fixed_font = new BFont();
        if(temp_fixed_font)
        {
#ifdef BHAPI_GRAPHICS_BEOS_BUILT_IN
            font_family bFamily;
            font_style bStyle;
            __be_fixed_font->GetFamilyAndStyle(&bFamily, &bStyle);
            if(temp_fixed_font->SetFamilyAndStyle(bFamily, bStyle) == B_OK)
            {
                temp_fixed_font->SetSize(__be_fixed_font->Size());
            }
            else
            {
#endif // BHAPI_GRAPHICS_BEOS_BUILT_IN
            const char *family = getenv("BHAPI_FIXED_FONT_FAMILY");
            const char *style = getenv("BHAPI_FIXED_FONT_STYLE");
            if(((family == NULL || *family == 0 || style == NULL || *style == 0) ?
                (BHAPI_WARNING("[FONT]: $BHAPI_FIXED_FONT_FAMILY = 0 || $BHAPI_FIXED_FONT_STYLE = 0"), B_ERROR) :
                temp_fixed_font->SetFamilyAndStyle(family, style)) != B_OK)
#ifdef BHAPI_GRAPHICS_WIN32_BUILT_IN
            if(temp_fixed_font->SetFamilyAndStyle("SimHei", "Regular") != B_OK)
            if(temp_fixed_font->SetFamilyAndStyle("黑体", "Regular") != B_OK)
            if(temp_fixed_font->SetFamilyAndStyle("Tahoma", "Regular") != B_OK)
#endif // BHAPI_GRAPHICS_WIN32_BUILT_IN
            if(temp_fixed_font->SetFamilyAndStyle("times", "medium") != B_OK) temp_fixed_font->SetFamilyAndStyle(0);

            float fsize = 10;
            const char *fontSize = getenv("BHAPI_FIXED_FONT_SIZE");
            if(!(fontSize == NULL || *fontSize == 0)) fsize = (float)atoi(fontSize);
            else BHAPI_WARNING("[FONT]: $BHAPI_FIXED_FONT_SIZE = 0");

            if(temp_fixed_font->IsScalable() == false)
            {
                temp_fixed_font->GetFixedSize(&fsize);
                temp_fixed_font->SetSize(fsize);
            }
            else
            {
                temp_fixed_font->SetSize(fsize);
            }
#ifdef BHAPI_GRAPHICS_BEOS_BUILT_IN
            }
#endif // BHAPI_GRAPHICS_BEOS_BUILT_IN
            temp_fixed_font->SetSpacing(0.05f);
            temp_fixed_font->SetShear(70);
        }

        temp_plain_font = new BFont(bhapi::plain_font);
        temp_bold_font = new BFont(bhapi::bold_font);
        temp_fixed_font = new BFont(bhapi::fixed_font);

        BHAPI_DEBUG("[FONT]: Fonts initalized.");
    }

    return(bhapi::font_families.CountItems() > 0);
}


BHAPI_LOCAL void bhapi::font_cancel(void)
{
    BAutolock <BLocker> autolock(&bhapi::font_locker);

    if(bhapi::font_initialized)
    {
        bhapi::font_canceling = true;

        if(temp_plain_font) delete temp_plain_font;
        temp_plain_font = NULL;
        if(temp_bold_font) delete temp_bold_font;
        temp_bold_font = NULL;
        if(temp_fixed_font) delete temp_fixed_font;
        temp_fixed_font = NULL;

#ifdef HAVE_FT2
        bhapi::font_freetype2_cancel();
#endif
        bhapi::font_other_cancel();

        BStringArray *styles;
        for(int32 i = 0; i < bhapi::font_families.CountItems(); i++)
        {
            styles = NULL;
            bhapi::font_families.ItemAt(i, (void**)&styles);
            if(styles)
            {
                BFontEngine *engine;
                for(int32 j = 0; j < styles->CountItems(); j++)
                {
                    engine = NULL;
                    styles->ItemAt(j, (void**)&engine);
                    if(engine) delete engine;
                }
                styles->MakeEmpty();
                delete styles;
            }
        }
        bhapi::font_families.MakeEmpty();

        bhapi::font_canceling = false;
        bhapi::font_initialized = false;
    }
}


BHAPI_LOCAL bool bhapi::font_lock(void)
{
    return bhapi::font_locker.Lock();
}


BHAPI_LOCAL void bhapi::font_unlock(void)
{
    bhapi::font_locker.Unlock();
}

