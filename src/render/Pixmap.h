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
 * File: Pixmap.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_PIXMAP__H
#define BHAPI_PIXMAP__H
#include "./../support/SupportDefs.h"
#include "./../interface/Region.h"
#include "./../render/Render.h"

#ifdef __cplusplus /* Just for C++ */

class IMPEXP_BHAPI BPixmap : public BRender {
public:
    BPixmap();
    BPixmap(b_uint32 width, b_uint32 height, b_color_space space);
    BPixmap(BRect bounds, b_color_space space);
    virtual ~BPixmap();

	void*		Bits() const;
    b_uint32		BitsLength() const;
    b_uint32		BytesPerRow() const;
	b_color_space	ColorSpace() const;
    BRect		Bounds() const;
	void		MakeEmpty();

    bool		ResizeTo(b_uint32 width, b_uint32 height, b_color_space space);
    bool		ResizeTo(BRect bounds, b_color_space space);

    void		SetBits(const void *data, b_int32 length, b_int32 offset, b_color_space space);

    void		SetPixel(b_int32 x, b_int32 y, b_rgb_color color);
    b_rgb_color	GetPixel(b_int32 x, b_int32 y) const;

	void		DrawXPM(const char **xpm_data,
                b_int32 destX, b_int32 destY,
                b_int32 srcX, b_int32 srcY,
                b_int32 srcW = -1, b_int32 srcH = -1,
                b_uint8 alpha = 255);

private:
	void* fPtr;
	b_color_space fColorSpace;
    b_uint32 fRows;
    b_uint32 fColumns;
    b_uint32 fRowBytes;

	virtual void *AllocData(size_t size);
	virtual void FreeData(void *data);

	virtual b_status_t InitCheck() const;
    virtual void GetFrame(b_int32 *originX, b_int32 *originY, b_uint32 *width, b_uint32 *height) const;
    virtual void GetPixel(b_int32 x, b_int32 y, b_rgb_color &color) const;
    virtual void PutPixel(b_int32 x, b_int32 y, b_rgb_color color);
    virtual void PutRect(b_int32 x, b_int32 y, b_uint32 width, b_uint32 height, b_rgb_color color);
};

#endif /* __cplusplus */

#endif /* BHAPI_PIXMAP__H */

