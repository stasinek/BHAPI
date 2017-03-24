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
 * File: FontEngine.h
 * Description: Font Engine Addon
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_FONT_ENGINE_H
#define BHAPI_FONT_ENGINE_H

#include "../../interface/Font.h"
#include "../../interface/Point.h"
#include "../../app/Handler.h"
#include "../../support/Locker.h"
#include "../../support/StringArray.h"
#include "../../support/SupportDefs.h"

#ifdef __cplusplus /* Just for C++ */

class BFontEngine;

namespace bhapi {

typedef enum font_render_mode {
    B_FONT_RENDER_UNKNOWN = 0,
    B_FONT_RENDER_DIRECTLY = 1,
    B_FONT_RENDER_PIXMAP = 1 << 1,
} font_render_mode;

typedef struct font_detach_callback {
    void (*callback)(void*);
    void *data;
} font_detach_callback;

BHAPI_IMPEXP BFontEngine*	get_font_engine(const char *family, const char *style);
BHAPI_IMPEXP BFontEngine*	get_font_engine(int32 familyIndex,  int32 styleIndex);
BHAPI_LOCAL bool font_init(void);
BHAPI_IMPEXP bool font_add(const char *family, const char *style, BFontEngine *engine);
BHAPI_IMPEXP bool update_font_families(bool);
BHAPI_LOCAL bool font_lock(void);
BHAPI_LOCAL void font_unlock(void);
BHAPI_LOCAL void font_cancel(void);
BHAPI_LOCAL bool font_other_init();
BHAPI_LOCAL bool update_other_font_families(bool check_only);
BHAPI_LOCAL void font_other_cancel();
BHAPI_IMPEXP bool font_freetype2_init(void);
BHAPI_IMPEXP bool font_freetype2_is_valid(void);
BHAPI_IMPEXP void font_freetype2_cancel(void);
}

using namespace bhapi;
class BHAPI_IMPEXP BFontEngine {
public:
    BFontEngine();
    BFontEngine(const char *family, const char *style);
    virtual ~BFontEngine();

    const char* Family() const;
    const char* Style() const;

    virtual bool IsValid() const;

    virtual bool IsScalable() const;
    bool HasFixedSize(int32 *count = NULL) const;
    bool GetFixedSize(float *size,  int32 index = 0) const;

    virtual float StringWidth(const char *string, float size, float spacing = 0,
                  float shear = 90, bool bold = false,  int32 length = -1) const;
    float StringWidth(const BString &str, float size, float spacing = 0,
              float shear = 90, bool bold = false,  int32 length = -1) const;

    virtual void GetHeight(bhapi::font_height *height, float size, float shear = 90, bool bold = false) const;

    bhapi::font_render_mode RenderMode() const;

    // ForceFontAliasing: just affected before calling "RenderString"
    virtual void ForceFontAliasing(bool enable);

    // B_FONT_RENDER_DIRECTLY callback: return value is update rect with view coordinate
    virtual BRect RenderString(BHandler *view, const char *string,
                   float size, float spacing = 0,
                   float shear = 90, bool bold = false,  int32 length = -1);
    // B_FONT_RENDER_PIXMAP callback: return value of "RenderString" must free by "delete[]"
    virtual  uint8* RenderString(const char *string,  int32 *width,  int32 *height, bool *is_mono,
                     float size, float spacing = 0,
                     float shear = 90, bool bold = false,  int32 length = -1);

    BRect RenderString(BHandler *view, const BString &str,
               float size, float spacing = 0,
               float shear = 90, bool bold = false,  int32 length = -1);
     uint8* RenderString(const BString &str,  int32 *width,  int32 *height, bool *is_mono,
                 float size, float spacing = 0,
                 float shear = 90, bool bold = false,  int32 length = -1);

    virtual bhapi::font_detach_callback* Attach(void (*callback)(void*), void *data);
    bool IsAttached() const;
    virtual bool Detach(bhapi::font_detach_callback *callback);

    bool Lock();
    void Unlock();

protected:
    void SetFamily(const char *family);
    void SetStyle(const char *style);
    void SetFixedSize(float *sizes,  int32 count);
    void SetRenderMode(bhapi::font_render_mode rmode);

    bool InServing() const;
    void OutOfServing();

private:
    BList fAttached;
    BLocker fLocker;

    char *fFamily;
    char *fStyle;

    float *fFixedSize;
     int32 nFixedSize;

    bhapi::font_render_mode fRenderMode;

    friend BHAPI_IMPEXP bool bhapi::font_add(const char *family, const char *style, BFontEngine *engine);
    BStringArray *fServing;
};
#endif /* __cplusplus */

#endif /* BHAPI_FONT_ENGINE_H */

