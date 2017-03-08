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
#include "../support/SupportDefs.h"
#include "../interface/GraphicsDefs.h"
#include "../interface/Region.h"
#include "../render/Render.h"

#ifdef __cplusplus /* Just for C++ */

class IMPEXPBHAPI BPixmap : public BRender {
public:
    BPixmap();
    BPixmap(__be_uint32 width,  __be_uint32 height, bhapi::color_space space);
    BPixmap(BRect bounds, bhapi::color_space space);
    virtual ~BPixmap();

	void*		Bits() const;
     __be_uint32		BitsLength() const;
     __be_uint32		BytesPerRow() const;
    bhapi::color_space	ColorSpace() const;
    BRect		Bounds() const;
	void		MakeEmpty();

    bool		ResizeTo(__be_uint32 width,  __be_uint32 height, bhapi::color_space space);
    bool		ResizeTo(BRect bounds, bhapi::color_space space);

    void		SetBits(const void *data,  __be_int32 length,  __be_int32 offset, bhapi::color_space space);

    void		SetPixel(__be_int32 x,  __be_int32 y, bhapi::rgb_color color);
    bhapi::rgb_color	GetPixel(__be_int32 x,  __be_int32 y) const;

	void		DrawXPM(const char **xpm_data,
                 __be_int32 destX,  __be_int32 destY,
                 __be_int32 srcX,  __be_int32 srcY,
                 __be_int32 srcW = -1,  __be_int32 srcH = -1,
                 __be_uint8 alpha = 255);

private:
	void* fPtr;
    bhapi::color_space fColorSpace;
     __be_uint32 fRows;
     __be_uint32 fColumns;
     __be_uint32 fRowBytes;

	virtual void *AllocData(size_t size);
	virtual void FreeData(void *data);

	virtual status_t InitCheck() const;
    virtual void GetFrame(__be_int32 *originX,  __be_int32 *originY,  __be_uint32 *width,  __be_uint32 *height) const;
    virtual void GetPixel(__be_int32 x,  __be_int32 y, bhapi::rgb_color &color) const;
    virtual void PutPixel(__be_int32 x,  __be_int32 y, bhapi::rgb_color color);
    virtual void PutRect(__be_int32 x,  __be_int32 y,  __be_uint32 width,  __be_uint32 height, bhapi::rgb_color color);
};

#endif /* __cplusplus */

#endif /* BHAPI_PIXMAP_H */

