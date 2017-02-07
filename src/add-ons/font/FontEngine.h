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
 * File: FontEngine.h
 * Description: Font Engine Addon
 *
 * --------------------------------------------------------------------------*/

#ifndef __BHAPI_FONT_ENGINE_H__
#define __BHAPI_FONT_ENGINE_H__

#include "./../../support/StringArray.h"
#include "./../../interface/Point.h"
#include "./../../interface/Font.h"
#include "./../../support/Locker.h"
#include "./../../app/Handler.h"

#ifdef __cplusplus /* Just for C++ */

typedef enum b_font_render_mode {
    B_FONT_RENDER_UNKNOWN = 0,
    B_FONT_RENDER_DIRECTLY = 1,
    B_FONT_RENDER_PIXMAP = 1 << 1,
} b_font_render_mode;


typedef struct b_font_detach_callback {
	void (*callback)(void*);
	void *data;
} b_font_detach_callback;


class _IMPEXP_BHAPI BFontEngine {
public:
    BFontEngine();
    BFontEngine(const char *family, const char *style);
    virtual ~BFontEngine();

	const char* Family() const;
	const char* Style() const;

	virtual bool IsValid() const;

	virtual bool IsScalable() const;
    bool HasFixedSize(b_int32 *count = NULL) const;
    bool GetFixedSize(float *size, b_int32 index = 0) const;

	virtual float StringWidth(const char *string, float size, float spacing = 0,
                  float shear = 90, bool bold = false, b_int32 length = -1) const;
    float StringWidth(const BString &str, float size, float spacing = 0,
              float shear = 90, bool bold = false, b_int32 length = -1) const;

    virtual void GetHeight(b_font_height *height, float size, float shear = 90, bool bold = false) const;

	b_font_render_mode RenderMode() const;

	// ForceFontAliasing: just affected before calling "RenderString"
	virtual void ForceFontAliasing(bool enable);

	// B_FONT_RENDER_DIRECTLY callback: return value is update rect with view coordinate
	virtual BRect RenderString(BHandler *view, const char *string,
				   float size, float spacing = 0,
                   float shear = 90, bool bold = false, b_int32 length = -1);
	// B_FONT_RENDER_PIXMAP callback: return value of "RenderString" must free by "delete[]"
    virtual b_uint8* RenderString(const char *string, b_int32 *width, b_int32 *height, bool *is_mono,
				     float size, float spacing = 0,
                     float shear = 90, bool bold = false, b_int32 length = -1);

    BRect RenderString(BHandler *view, const BString &str,
			   float size, float spacing = 0,
               float shear = 90, bool bold = false, b_int32 length = -1);
    b_uint8* RenderString(const BString &str, b_int32 *width, b_int32 *height, bool *is_mono,
			     float size, float spacing = 0,
                 float shear = 90, bool bold = false, b_int32 length = -1);

	virtual b_font_detach_callback* Attach(void (*callback)(void*), void *data);
	bool IsAttached() const;
    virtual bool Detach(b_font_detach_callback *callback);

	bool Lock();
	void Unlock();

protected:
	void SetFamily(const char *family);
	void SetStyle(const char *style);
    void SetFixedSize(float *sizes, b_int32 count);
    void SetRenderMode(b_font_render_mode rmode);

	bool InServing() const;
	void OutOfServing();

private:
	BList fAttached;
	BLocker fLocker;

	char *fFamily;
	char *fStyle;

	float *fFixedSize;
    b_int32 nFixedSize;

	b_font_render_mode fRenderMode;

    friend _IMPEXP_BHAPI bool bhapi_font_add(const char *family, const char *style, BFontEngine *engine);
    BStringArray *fServing;
};

_IMPEXP_BHAPI bool		bhapi_font_add(const char *family, const char *style, BFontEngine *engine);
_IMPEXP_BHAPI BFontEngine*	bhapi_get_font_engine(const char *family, const char *style);
_IMPEXP_BHAPI BFontEngine*	bhapi_get_font_engine(b_int32 familyIndex, b_int32 styleIndex);

#endif /* __cplusplus */

#endif /* __BHAPI_FONT_ENGINE_H__ */

