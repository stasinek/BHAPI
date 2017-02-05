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
 * File: Pixmap.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_PIXMAP_H__
#define __ETK_PIXMAP_H__

#include "./../interface/Region.h"
#include "./../render/Render.h"

#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_ETK EPixmap : public ERender {
public:
	EPixmap();
	EPixmap(euint32 width, euint32 height, e_color_space space);
	EPixmap(ERect bounds, e_color_space space);
	virtual ~EPixmap();

	void*		Bits() const;
	euint32		BitsLength() const;
	euint32		BytesPerRow() const;
	e_color_space	ColorSpace() const;
	ERect		Bounds() const;
	void		MakeEmpty();

	bool		ResizeTo(euint32 width, euint32 height, e_color_space space);
	bool		ResizeTo(ERect bounds, e_color_space space);

	void		SetBits(const void *data, eint32 length, eint32 offset, e_color_space space);

	void		SetPixel(eint32 x, eint32 y, e_rgb_color color);
	e_rgb_color	GetPixel(eint32 x, eint32 y) const;

	void		DrawXPM(const char **xpm_data,
				eint32 destX, eint32 destY,
				eint32 srcX, eint32 srcY,
				eint32 srcW = -1, eint32 srcH = -1,
				euint8 alpha = 255);

private:
	void* fPtr;
	e_color_space fColorSpace;
	euint32 fRows;
	euint32 fColumns;
	euint32 fRowBytes;

	virtual void *AllocData(size_t size);
	virtual void FreeData(void *data);

	virtual e_status_t InitCheck() const;
	virtual void GetFrame(eint32 *originX, eint32 *originY, euint32 *width, euint32 *height) const;
	virtual void GetPixel(eint32 x, eint32 y, e_rgb_color &color) const;
	virtual void PutPixel(eint32 x, eint32 y, e_rgb_color color);
	virtual void PutRect(eint32 x, eint32 y, euint32 width, euint32 height, e_rgb_color color);
};

#endif /* __cplusplus */

#endif /* __ETK_PIXMAP_H__ */

