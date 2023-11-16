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
 * File: Bitmap.h
 * Description: BBitmap --- a rectangular image for drawing
 * Warning: Unfinished.
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_BITMAP_H
#define BHAPI_BITMAP_H

#include <kits/interface/View.h>

#ifdef __cplusplus /* Just for C++ */
class BView;
class BWindow;
namespace BPrivate {
    class BPrivateScreen;
}

enum {
    B_BITMAP_CLEAR_TO_WHITE				= 0x00000001,
    B_BITMAP_ACCEPTS_VIEWS				= 0x00000002,
    B_BITMAP_IS_AREA					= 0x00000004,
    B_BITMAP_IS_LOCKED					= 0x00000008 | B_BITMAP_IS_AREA,
    B_BITMAP_IS_CONTIGUOUS				= 0x00000010 | B_BITMAP_IS_LOCKED,
    B_BITMAP_IS_OFFSCREEN				= 0x00000020,
        // Offscreen but non-overlay bitmaps are not supported on Haiku,
        // but appearantly never were on BeOS either! The accelerant API
        // would need to be extended to so that the app_server can ask
        // the graphics driver to reserve memory for a bitmap and for this
        // to make any sense, an accelerated blit from this memory into
        // the framebuffer needs to be added to the API as well.
    B_BITMAP_WILL_OVERLAY				= 0x00000040 | B_BITMAP_IS_OFFSCREEN,
    B_BITMAP_RESERVE_OVERLAY_CHANNEL	= 0x00000080,

    // Haiku extensions:
    B_BITMAP_NO_SERVER_LINK				= 0x00000100,
        // Cheap to create, object will manage memory itself,
        // no BApplication needs to run, but one can't draw such
        // a BBitmap.
};

#define B_ANY_BYTES_PER_ROW	-1
class BGraphicsDrawable;
class BPixmap;
class BHAPI_IMPEXP BBitmap : public BArchivable {
public:
    BBitmap(BRect bounds, bool acceptsViews = false);
    BBitmap(const BBitmap *bitmap, bool acceptsViews = false);
    BBitmap(const BPixmap *pixmap, bool acceptsViews = false);
    virtual ~BBitmap();

    status_t	InitCheck() const;
    bool		IsValid() const;

    BRect		Bounds() const;

    virtual	void	AddChild(BView *view);
    virtual	bool	RemoveChild(BView *view);
     int32_t		CountChildren() const;
    BView		*ChildAt(int32_t index) const;
    BView		*FindView(const char *name) const;
    BView		*FindView(BPoint where) const;
    bool		Lock();
    void		Unlock();
    bool				IsLocked() const;
    class Private;

private:
    friend class BView;

     uint32_t fRows;
     uint32_t fColumns;

    BGraphicsDrawable *fPixmap;
    BWindow *fWindow;

    void InitSelf(BRect, bool);
};

#endif /* __cplusplus */

#endif /* BHAPI_BITMAP_H */

