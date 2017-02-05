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
 * File: Bitmap.h
 * Description: EBitmap --- a rectangular image for drawing
 * Warning: Unfinished.
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_BITMAP_H__
#define __ETK_BITMAP_H__

#include "./../interface/View.h>

#ifdef __cplusplus /* Just for C++ */

class EGraphicsDrawable;
class EPixmap;

class _IMPEXP_ETK EBitmap : public EArchivable {
public:
	EBitmap(ERect bounds, bool acceptsViews = false);
	EBitmap(const EBitmap *bitmap, bool acceptsViews = false);
	EBitmap(const EPixmap *pixmap, bool acceptsViews = false);
	virtual ~EBitmap();

	e_status_t	InitCheck() const;
	bool		IsValid() const;

	ERect		Bounds() const;

	virtual	void	AddChild(EView *view);
	virtual	bool	RemoveChild(EView *view);
	eint32		CountChildren() const;
	EView		*ChildAt(eint32 index) const;
	EView		*FindView(const char *name) const;
	EView		*FindView(EPoint where) const;
	bool		Lock();
	void		Unlock();

private:
	friend class EView;

	euint32 fRows;
	euint32 fColumns;

	EGraphicsDrawable *fPixmap;
	EWindow *fWindow;

	void InitSelf(ERect, bool);
};

#endif /* __cplusplus */

#endif /* __ETK_BITMAP_H__ */

