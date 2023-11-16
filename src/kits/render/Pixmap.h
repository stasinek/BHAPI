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
 * File: Pixmap.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_PIXMAP_H
#define BHAPI_PIXMAP_H
#include <Haiku.h>
#include <kits/interface/GraphicsDefs.h>
#include <kits/interface/Region.h>
#include <kits/render/Render.h>

#ifdef __cplusplus /* Just for C++ */

class BHAPI_IMPEXP BPixmap : public BRender {
public:
    BPixmap();
    BPixmap(uint32_t width,  uint32_t height, bhapi::color_space space);
    BPixmap(BRect bounds, bhapi::color_space space);
    virtual ~BPixmap();

	void*		Bits() const;
     uint32_t		BitsLength() const;
     uint32_t		BytesPerRow() const;
    bhapi::color_space	ColorSpace() const;
    BRect		Bounds() const;
	void		MakeEmpty();

    bool		ResizeTo(uint32_t width,  uint32_t height, bhapi::color_space space);
    bool		ResizeTo(BRect bounds, bhapi::color_space space);

    void		SetBits(const void *data,  int32_t length,  int32_t offset, bhapi::color_space space);

    void		SetPixel(int32_t x,  int32_t y, bhapi::rgb_color color);
    bhapi::rgb_color	GetPixel(int32_t x,  int32_t y) const;

	void		DrawXPM(const char **xpm_data,
                 int32_t destX,  int32_t destY,
                 int32_t srcX,  int32_t srcY,
                 int32_t srcW = -1,  int32_t srcH = -1,
                 uint8 alpha = 255);

private:
	void* fPtr;
    bhapi::color_space fColorSpace;
     uint32_t fRows;
     uint32_t fColumns;
     uint32_t fRowBytes;

	virtual void *AllocData(size_t size);
	virtual void FreeData(void *data);

	virtual status_t InitCheck() const;
    virtual void GetFrame(int32_t *originX,  int32_t *originY,  uint32_t *width,  uint32_t *height) const;
    virtual void GetPixel(int32_t x,  int32_t y, bhapi::rgb_color &color) const;
    virtual void PutPixel(int32_t x,  int32_t y, bhapi::rgb_color color);
    virtual void PutRect(int32_t x,  int32_t y,  uint32_t width,  uint32_t height, bhapi::rgb_color color);
};

#endif /* __cplusplus */

#endif /* BHAPI_PIXMAP_H */

