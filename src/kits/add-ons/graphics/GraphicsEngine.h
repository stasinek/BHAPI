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
 * File: GraphicsEngine.h
 * Description: Graphics Engine Addon
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_GRAPHICS_ENGINE_H
#define BHAPI_GRAPHICS_ENGINE_H

#include "../../support/SupportDefs.h"

#ifdef __cplusplus /* Just for C++ */
#include "../../interface/GraphicsDefs.h"
class BCursor;
class BPixmap;
#include "../../interface/Window.h"
#include "../../interface/Region.h"
class IMPEXPBHAPI BGraphicsContext {
public:
	BGraphicsContext();
	virtual ~BGraphicsContext();

    virtual status_t		SetDrawingMode(bhapi::drawing_mode mode);
    virtual status_t		SetClipping(const BRegion &clipping);
    virtual status_t		SetHighColor(bhapi::rgb_color highColor);
    virtual status_t		SetLowColor(bhapi::rgb_color lowColor);
    virtual status_t		SetPattern(bhapi::pattern pattern);
    virtual status_t		SetPenSize(__be_uint32 penSize);
	virtual status_t		SetSquarePointStyle(bool state);

	bhapi::drawing_mode			DrawingMode() const;
    const BRegion			*Clipping() const;
	bhapi::pattern			Pattern() const;
     __be_uint32				PenSize() const;
	bool				IsSquarePointStyle() const;

    status_t			SetHighColor(__be_uint8 r,  __be_uint8 g,  __be_uint8 b,  __be_uint8 a);
    status_t			SetLowColor(__be_uint8 r,  __be_uint8 g,  __be_uint8 b,  __be_uint8 a);
	bhapi::rgb_color			HighColor() const;
	bhapi::rgb_color			LowColor() const;

private:
	bhapi::drawing_mode fDrawingMode;
    BRegion fClipping;
	bhapi::rgb_color fHighColor;
	bhapi::rgb_color fLowColor;
	bhapi::pattern fPattern;
     __be_uint32 fPenSize;
	bool fSquarePoint;
};


class IMPEXPBHAPI BGraphicsDrawable {
public:
	BGraphicsDrawable();
	virtual ~BGraphicsDrawable();

    virtual status_t		SetBackgroundColor(bhapi::rgb_color bkColor);

    virtual status_t		ResizeTo(__be_uint32 w,  __be_uint32 h) = 0;
	virtual status_t		CopyTo(BGraphicsContext *dc,
					       BGraphicsDrawable *dstDrawable,
                            __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h,
                            __be_int32 dstX,  __be_int32 dstY,  __be_uint32 dstW,  __be_uint32 dstH) = 0;
	virtual status_t		DrawPixmap(BGraphicsContext *dc, const BPixmap *pix,
                            __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h,
                            __be_int32 dstX,  __be_int32 dstY,  __be_uint32 dstW,  __be_uint32 dstH) = 0;

	virtual status_t		StrokePoint(BGraphicsContext *dc,
                             __be_int32 x,  __be_int32 y) = 0;
	virtual status_t		StrokePoints(BGraphicsContext *dc,
                             const  __be_int32 *pts,  __be_int32 count) = 0;
	virtual status_t		StrokePoints_Colors(BGraphicsContext *dc,
                                const BList *ptsArrayLists,  __be_int32 arrayCount,
							    const bhapi::rgb_color *highColors) = 0;
	virtual status_t		StrokePoints_Alphas(BGraphicsContext *dc,
                                const  __be_int32 *pts, const  __be_uint8 *alpha,  __be_int32 count) = 0;
	virtual status_t		StrokeLine(BGraphicsContext *dc,
                            __be_int32 x0,  __be_int32 y0,  __be_int32 x1,  __be_int32 y1) = 0;
	virtual status_t		StrokePolygon(BGraphicsContext *dc,
                              const  __be_int32 *pts,  __be_int32 count, bool closed) = 0;
	virtual status_t		FillPolygon(BGraphicsContext *dc,
                            const  __be_int32 *pts,  __be_int32 count) = 0;
	virtual status_t		StrokeRect(BGraphicsContext *dc,
                            __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h) = 0;
	virtual status_t		FillRect(BGraphicsContext *dc,
                          __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h) = 0;
	virtual status_t		StrokeRects(BGraphicsContext *dc,
                            const  __be_int32 *rects,  __be_int32 count) = 0;
	virtual status_t		FillRects(BGraphicsContext *dc,
                          const  __be_int32 *rects,  __be_int32 count) = 0;
	virtual status_t		FillRegion(BGraphicsContext *dc,
                           const BRegion &region) = 0;
	virtual status_t		StrokeRoundRect(BGraphicsContext *dc,
                             __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h,  __be_uint32 xRadius,  __be_uint32 yRadius) = 0;
	virtual status_t		FillRoundRect(BGraphicsContext *dc,
                               __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h,  __be_uint32 xRadius,  __be_uint32 yRadius) = 0;
	virtual status_t		StrokeArc(BGraphicsContext *dc,
                           __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h, float startAngle, float endAngle) = 0;
	virtual status_t		FillArc(BGraphicsContext *dc,
                         __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h, float startAngle, float endAngle) = 0;

    status_t			SetBackgroundColor(__be_uint8 r,  __be_uint8 g,  __be_uint8 b,  __be_uint8 a);
	bhapi::rgb_color			BackgroundColor() const;

private:
	bhapi::rgb_color fBkColor;
};


class IMPEXPBHAPI BGraphicsWindow : public BGraphicsDrawable {
public:
	BGraphicsWindow();
	virtual ~BGraphicsWindow();

	virtual status_t		ContactTo(const BMessenger *msgr) = 0;
    virtual status_t		SetFlags(__be_uint32 flags) = 0;
    virtual status_t		SetLook(bhapi::window_look look) = 0;
    virtual status_t		SetFeel(bhapi::window_feel feel) = 0;
	virtual status_t		SetTitle(const char *title) = 0;
    virtual status_t		SetWorkspaces(__be_uint32 workspaces) = 0;
    virtual status_t		GetWorkspaces(__be_uint32 *workspaces) = 0;
	virtual status_t		Iconify() = 0;
	virtual status_t		Show() = 0;
	virtual status_t		Hide() = 0;
	virtual status_t		Raise() = 0;
	virtual status_t		Lower(BGraphicsWindow* frontWin) = 0;
	virtual status_t		Activate(bool state) = 0;
	virtual status_t		GetActivatedState(bool *state) const = 0;
    virtual status_t		MoveTo(__be_int32 x,  __be_int32 y) = 0;
    virtual status_t		MoveAndResizeTo(__be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h) = 0;
    virtual status_t		SetSizeLimits(__be_uint32 min_w,  __be_uint32 max_w,  __be_uint32 min_h,  __be_uint32 max_h) = 0;
    virtual status_t		GetSizeLimits(__be_uint32 *min_w,  __be_uint32 *max_w,  __be_uint32 *min_h,  __be_uint32 *max_h) = 0;
	virtual status_t		GrabMouse() = 0;
	virtual status_t		UngrabMouse() = 0;
	virtual status_t		GrabKeyboard() = 0;
	virtual status_t		UngrabKeyboard() = 0;
    virtual status_t		QueryMouse(__be_int32 *x,  __be_int32 *y,  __be_int32 *buttons) = 0;
};


// NOTE:
// 1. Addon must have C function like below and all the null virtual functions of class must be implemented.
// 		extern "C" EXPORTBHAPI BGraphicsEngine* instantiate_graphics_engine();
// 2. Usually, addons were put into the directory located at "$E_ADDONS_DIRECTORY/etkxx/graphics" or
//    "$E_USER_ADDONS_DIRECTORY/etkxx/graphics", such as "/usr/lib/add-ons/etkxx/graphics".
// 3. When BHAPI++ find no graphics-engine addons, it try built-in graphics-engine when possible.
class IMPEXPBHAPI BGraphicsEngine {
public:
	BGraphicsEngine();
	virtual ~BGraphicsEngine();

	virtual status_t		Initalize() = 0;
	virtual void			Cancel() = 0;

	virtual BGraphicsContext*	CreateContext() = 0;
    virtual BGraphicsDrawable*	CreatePixmap(__be_uint32 w,  __be_uint32 h) = 0;
#undef CreateWindow
    virtual BGraphicsWindow*	CreateWindow(__be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h) = 0;

	virtual status_t		InitalizeFonts() = 0;
	virtual void			DestroyFonts() = 0;
	virtual status_t		UpdateFonts(bool check_only) = 0;

    virtual status_t		GetDesktopBounds(__be_uint32 *w,  __be_uint32 *h) = 0;
    virtual status_t		GetCurrentWorkspace(__be_uint32 *workspace) = 0;
	virtual status_t		SetCursor(const void *cursor_data) = 0;
	virtual status_t		GetDefaultCursor(BCursor *cursor) = 0;

    static BGraphicsWindow		*GetWindow(BWindow *win);
    static BGraphicsDrawable	*GetPixmap(BWindow *win);
	static BGraphicsContext		*GetContext(BView *view);
};


#endif /* __cplusplus */

#endif /* BHAPI_GRAPHICS_ENGINE_H */

