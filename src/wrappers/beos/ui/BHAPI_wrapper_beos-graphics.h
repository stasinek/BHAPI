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
 * File: BHAPI_wrapper_beos-graphics.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_BEOS_GRAPHICS_H
#define BHAPI_BEOS_GRAPHICS_H

#ifdef BEOS

#include <be/app/Messenger.h>
#include <be/interface/View.h>
#include <be/interface/Window.h>
#include <be/interface/Bitmap.h>

#include "../../support/Locker.h"
#include "../../add-ons/graphics/GraphicsEngine.h"
#include "../../app/MessageFilter.h"

#ifdef __cplusplus /* just for C++ */


class EBeGraphicsEngine : public BGraphicsEngine {
public:
	EBeGraphicsEngine();
	virtual ~EBeGraphicsEngine();

	status_t			InitCheck();

	bool				Lock();
	void				Unlock();

	virtual status_t		Initalize();
	virtual void			Cancel();

	virtual BGraphicsContext*	CreateContext();
	virtual BGraphicsDrawable*	CreatePixmap(__be_uint32 w,  __be_uint32 h);
	virtual BGraphicsWindow*	CreateWindow(__be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h);

	virtual status_t		InitalizeFonts();
	virtual void			DestroyFonts();
	virtual status_t		UpdateFonts(bool check_only);

	virtual status_t		GetDesktopBounds(__be_uint32 *w,  __be_uint32 *h);
	virtual status_t		GetCurrentWorkspace(__be_uint32 *workspace);
	virtual status_t		SetCursor(const void *cursor_data);
	virtual status_t		GetDefaultCursor(BCursor *cursor);

	void *fRequestSem;
	bool beDoQuit;

private:
	BLocker fLocker;
	void *fBeThread;
	BMessageFilter *fClipboardFilter;
};


class EBeBitmapPriv : public BBitmap
{
public:
	EBeBitmapPriv(__be_uint32 w,  __be_uint32 h);
	virtual ~EBeBitmapPriv();

	BView *fView;
};


class EBeGraphicsDrawable : public BGraphicsDrawable {
public:
	EBeGraphicsDrawable(EBeGraphicsEngine *beEngine,  __be_uint32 w,  __be_uint32 h);
	virtual ~EBeGraphicsDrawable();

	virtual status_t		SetBackgroundColor(bhapi::rgb_color bkColor);

	virtual status_t		ResizeTo(__be_uint32 w,  __be_uint32 h);
	virtual status_t		CopyTo(BGraphicsContext *dc,
					       BGraphicsDrawable *dstDrawable,
					        __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h,
					        __be_int32 dstX,  __be_int32 dstY,  __be_uint32 dstW,  __be_uint32 dstH);
	virtual status_t		DrawPixmap(BGraphicsContext *dc, const BPixmap *pix,
						    __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h,
						    __be_int32 dstX,  __be_int32 dstY,  __be_uint32 dstW,  __be_uint32 dstH);

	virtual status_t		StrokePoint(BGraphicsContext *dc,
						     __be_int32 x,  __be_int32 y);
	virtual status_t		StrokePoints(BGraphicsContext *dc,
						     const  __be_int32 *pts,  __be_int32 count);
	virtual status_t		StrokePoints_Colors(BGraphicsContext *dc,
							    const BList *ptsArrayLists,  __be_int32 arrayCount,
							    const bhapi::rgb_color *highColors);
	virtual status_t		StrokePoints_Alphas(BGraphicsContext *dc,
							    const  __be_int32 *pts, const  __be_uint8 *alpha,  __be_int32 count);
	virtual status_t		StrokeLine(BGraphicsContext *dc,
						    __be_int32 x0,  __be_int32 y0,  __be_int32 x1,  __be_int32 y1);
	virtual status_t		StrokePolygon(BGraphicsContext *dc,
						      const  __be_int32 *pts,  __be_int32 count, bool closed);
	virtual status_t		FillPolygon(BGraphicsContext *dc,
						    const  __be_int32 *pts,  __be_int32 count);
	virtual status_t		StrokeRect(BGraphicsContext *dc,
						    __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h);
	virtual status_t		FillRect(BGraphicsContext *dc,
						  __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h);
	virtual status_t		StrokeRects(BGraphicsContext *dc,
						    const  __be_int32 *rects,  __be_int32 count);
	virtual status_t		FillRects(BGraphicsContext *dc,
						  const  __be_int32 *rects,  __be_int32 count);
	virtual status_t		FillRegion(BGraphicsContext *dc,
						   const BRegion &region);
	virtual status_t		StrokeRoundRect(BGraphicsContext *dc,
							__be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h,  __be_uint32 xRadius,  __be_uint32 yRadius);
	virtual status_t		FillRoundRect(BGraphicsContext *dc,
						       __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h,  __be_uint32 xRadius,  __be_uint32 yRadius);
	virtual status_t		StrokeArc(BGraphicsContext *dc,
						   __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h, float startAngle, float endAngle);
	virtual status_t		FillArc(BGraphicsContext *dc,
						__be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h, float startAngle, float endAngle);

	EBeBitmapPriv *beBitmap;

private:
	EBeGraphicsEngine *fEngine;
};


class EBeGraphicsWindow : public BGraphicsWindow {
public:
	EBeGraphicsWindow(EBeGraphicsEngine *beEngine,  __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h);
	virtual ~EBeGraphicsWindow();

	virtual status_t		ContactTo(const BMessenger *msgr);
	virtual status_t		SetBackgroundColor(bhapi::rgb_color bkColor);
	virtual status_t		SetFlags(__be_uint32 flags);
	virtual status_t		SetLook(bhapi::window_look look);
	virtual status_t		SetFeel(bhapi::window_feel feel);
	virtual status_t		SetTitle(const char *title);
	virtual status_t		SetWorkspaces(__be_uint32 workspaces);
	virtual status_t		GetWorkspaces(__be_uint32 *workspaces);
	virtual status_t		Iconify();
	virtual status_t		Show();
	virtual status_t		Hide();
	virtual status_t		Raise();
	virtual status_t		Lower(BGraphicsWindow *frontWin);
	virtual status_t		Activate(bool state);
	virtual status_t		GetActivatedState(bool *state) const;
	virtual status_t		MoveTo(__be_int32 x,  __be_int32 y);
	virtual status_t		ResizeTo(__be_uint32 w,  __be_uint32 h);
	virtual status_t		MoveAndResizeTo(__be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h);
	virtual status_t		SetSizeLimits(__be_uint32 min_w,  __be_uint32 max_w,  __be_uint32 min_h,  __be_uint32 max_h);
	virtual status_t		GetSizeLimits(__be_uint32 *min_w,  __be_uint32 *max_w,  __be_uint32 *min_h,  __be_uint32 *max_h);
	virtual status_t		GrabMouse();
	virtual status_t		UngrabMouse();
	virtual status_t		GrabKeyboard();
	virtual status_t		UngrabKeyboard();
	virtual status_t		QueryMouse(__be_int32 *x,  __be_int32 *y,  __be_int32 *buttons);

	virtual status_t		CopyTo(BGraphicsContext *dc,
					       BGraphicsDrawable *dstDrawable,
					        __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h,
					        __be_int32 dstX,  __be_int32 dstY,  __be_uint32 dstW,  __be_uint32 dstH);
	virtual status_t		DrawPixmap(BGraphicsContext *dc, const BPixmap *pix,
						    __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h,
						    __be_int32 dstX,  __be_int32 dstY,  __be_uint32 dstW,  __be_uint32 dstH);

	virtual status_t		StrokePoint(BGraphicsContext *dc,
						     __be_int32 x,  __be_int32 y);
	virtual status_t		StrokePoints(BGraphicsContext *dc,
						     const  __be_int32 *pts,  __be_int32 count);
	virtual status_t		StrokePoints_Colors(BGraphicsContext *dc,
							    const BList *ptsArrayLists,  __be_int32 arrayCount,
							    const bhapi::rgb_color *highColors);
	virtual status_t		StrokePoints_Alphas(BGraphicsContext *dc,
							    const  __be_int32 *pts, const  __be_uint8 *alpha,  __be_int32 count);
	virtual status_t		StrokeLine(BGraphicsContext *dc,
						    __be_int32 x0,  __be_int32 y0,  __be_int32 x1,  __be_int32 y1);
	virtual status_t		StrokePolygon(BGraphicsContext *dc,
						      const  __be_int32 *pts,  __be_int32 count, bool closed);
	virtual status_t		FillPolygon(BGraphicsContext *dc,
						    const  __be_int32 *pts,  __be_int32 count);
	virtual status_t		StrokeRect(BGraphicsContext *dc,
						    __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h);
	virtual status_t		FillRect(BGraphicsContext *dc,
						  __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h);
	virtual status_t		StrokeRects(BGraphicsContext *dc,
						    const  __be_int32 *rects,  __be_int32 count);
	virtual status_t		FillRects(BGraphicsContext *dc,
						  const  __be_int32 *rects,  __be_int32 count);
	virtual status_t		FillRegion(BGraphicsContext *dc,
						   const BRegion &region);
	virtual status_t		StrokeRoundRect(BGraphicsContext *dc,
							__be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h,  __be_uint32 xRadius,  __be_uint32 yRadius);
	virtual status_t		FillRoundRect(BGraphicsContext *dc,
						       __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h,  __be_uint32 xRadius,  __be_uint32 yRadius);

	virtual status_t		StrokeArc(BGraphicsContext *dc,
						   __be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h, float startAngle, float endAngle);
	virtual status_t		FillArc(BGraphicsContext *dc,
						__be_int32 x,  __be_int32 y,  __be_uint32 w,  __be_uint32 h, float startAngle, float endAngle);

	BMessenger beWinMsgr;

private:
	friend class EBeGraphicsEngine;

	EBeGraphicsEngine *fEngine;
};


extern bool __bhapi_prepare_beview(BView *view, BGraphicsContext *dc);

enum {
	BHAPI_BEOS_QUIT = 1,
	BHAPI_BEOS_CONTACT_TO,
	BHAPI_BEOS_SET_BACKGROUND,
	BHAPI_BEOS_SET_LOOK,
	BHAPI_BEOS_SET_TITLE,
	BHAPI_BEOS_SET_WORKSPACES,
	BHAPI_BEOS_GET_WORKSPACES,
	BHAPI_BEOS_ICONIFY,
	BHAPI_BEOS_SHOW,
	BHAPI_BEOS_HIDE,
	BHAPI_BEOS_RAISE,
	BHAPI_BEOS_LOWER,
	BHAPI_BEOS_ACTIVATE,
	BHAPI_BEOS_GET_ACTIVATED_STATE,
	BHAPI_BEOS_MOVE_RESIZE,
	BHAPI_BEOS_SET_SIZE_LIMITS,
	BHAPI_BEOS_GET_SIZE_LIMITS,
	BHAPI_BEOS_GRAB_MOUSE,
	BHAPI_BEOS_UNGRAB_MOUSE,
	BHAPI_BEOS_GRAB_KEYBOARD,
	BHAPI_BEOS_UNGRAB_KEYBOARD,
	BHAPI_BEOS_QUERY_MOUSE,
	BHAPI_BEOS_DRAW_BITMAP,
};


#endif /* __cplusplus */
#endif /* BEOS */

#endif // BHAPI_BEOS_GRAPHICS_H

