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
 * File: etk-beos-graphics.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_BEOS_GRAPHICS__H
#define BHAPI_BEOS_GRAPHICS__H

#ifdef BEOS

#include <be/app/Messenger.h>
#include <be/interface/View.h>
#include <be/interface/Window.h>
#include <be/interface/Bitmap.h>

#include "./../../support/Locker.h"
#include "./../../add-ons/graphics/GraphicsEngine.h"
#include "./../../app/MessageFilter.h"

#ifdef __cplusplus /* just for C++ */


class EBeGraphicsEngine : public BGraphicsEngine {
public:
	EBeGraphicsEngine();
	virtual ~EBeGraphicsEngine();

	b_status_t			InitCheck();

	bool				Lock();
	void				Unlock();

	virtual b_status_t		Initalize();
	virtual void			Cancel();

	virtual BGraphicsContext*	CreateContext();
	virtual BGraphicsDrawable*	CreatePixmap(b_uint32 w, b_uint32 h);
	virtual BGraphicsWindow*	CreateWindow(b_int32 x, b_int32 y, b_uint32 w, b_uint32 h);

	virtual b_status_t		InitalizeFonts();
	virtual void			DestroyFonts();
	virtual b_status_t		UpdateFonts(bool check_only);

	virtual b_status_t		GetDesktopBounds(b_uint32 *w, b_uint32 *h);
	virtual b_status_t		GetCurrentWorkspace(b_uint32 *workspace);
	virtual b_status_t		SetCursor(const void *cursor_data);
	virtual b_status_t		GetDefaultCursor(BCursor *cursor);

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
	EBeBitmapPriv(b_uint32 w, b_uint32 h);
	virtual ~EBeBitmapPriv();

	BView *fView;
};


class EBeGraphicsDrawable : public BGraphicsDrawable {
public:
	EBeGraphicsDrawable(EBeGraphicsEngine *beEngine, b_uint32 w, b_uint32 h);
	virtual ~EBeGraphicsDrawable();

	virtual b_status_t		SetBackgroundColor(b_rgb_color bkColor);

	virtual b_status_t		ResizeTo(b_uint32 w, b_uint32 h);
	virtual b_status_t		CopyTo(BGraphicsContext *dc,
					       BGraphicsDrawable *dstDrawable,
					       b_int32 x, b_int32 y, b_uint32 w, b_uint32 h,
					       b_int32 dstX, b_int32 dstY, b_uint32 dstW, b_uint32 dstH);
	virtual b_status_t		DrawPixmap(BGraphicsContext *dc, const BPixmap *pix,
						   b_int32 x, b_int32 y, b_uint32 w, b_uint32 h,
						   b_int32 dstX, b_int32 dstY, b_uint32 dstW, b_uint32 dstH);

	virtual b_status_t		StrokePoint(BGraphicsContext *dc,
						    b_int32 x, b_int32 y);
	virtual b_status_t		StrokePoints(BGraphicsContext *dc,
						     const b_int32 *pts, b_int32 count);
	virtual b_status_t		StrokePoints_Colors(BGraphicsContext *dc,
							    const BList *ptsArrayLists, b_int32 arrayCount,
							    const b_rgb_color *highColors);
	virtual b_status_t		StrokePoints_Alphas(BGraphicsContext *dc,
							    const b_int32 *pts, const b_uint8 *alpha, b_int32 count);
	virtual b_status_t		StrokeLine(BGraphicsContext *dc,
						   b_int32 x0, b_int32 y0, b_int32 x1, b_int32 y1);
	virtual b_status_t		StrokePolygon(BGraphicsContext *dc,
						      const b_int32 *pts, b_int32 count, bool closed);
	virtual b_status_t		FillPolygon(BGraphicsContext *dc,
						    const b_int32 *pts, b_int32 count);
	virtual b_status_t		StrokeRect(BGraphicsContext *dc,
						   b_int32 x, b_int32 y, b_uint32 w, b_uint32 h);
	virtual b_status_t		FillRect(BGraphicsContext *dc,
						 b_int32 x, b_int32 y, b_uint32 w, b_uint32 h);
	virtual b_status_t		StrokeRects(BGraphicsContext *dc,
						    const b_int32 *rects, b_int32 count);
	virtual b_status_t		FillRects(BGraphicsContext *dc,
						  const b_int32 *rects, b_int32 count);
	virtual b_status_t		FillRegion(BGraphicsContext *dc,
						   const BRegion &region);
	virtual b_status_t		StrokeRoundRect(BGraphicsContext *dc,
							b_int32 x, b_int32 y, b_uint32 w, b_uint32 h, b_uint32 xRadius, b_uint32 yRadius);
	virtual b_status_t		FillRoundRect(BGraphicsContext *dc,
						      b_int32 x, b_int32 y, b_uint32 w, b_uint32 h, b_uint32 xRadius, b_uint32 yRadius);
	virtual b_status_t		StrokeArc(BGraphicsContext *dc,
						  b_int32 x, b_int32 y, b_uint32 w, b_uint32 h, float startAngle, float endAngle);
	virtual b_status_t		FillArc(BGraphicsContext *dc,
						b_int32 x, b_int32 y, b_uint32 w, b_uint32 h, float startAngle, float endAngle);

	EBeBitmapPriv *beBitmap;

private:
	EBeGraphicsEngine *fEngine;
};


class EBeGraphicsWindow : public BGraphicsWindow {
public:
	EBeGraphicsWindow(EBeGraphicsEngine *beEngine, b_int32 x, b_int32 y, b_uint32 w, b_uint32 h);
	virtual ~EBeGraphicsWindow();

	virtual b_status_t		ContactTo(const BMessenger *msgr);
	virtual b_status_t		SetBackgroundColor(b_rgb_color bkColor);
	virtual b_status_t		SetFlags(b_uint32 flags);
	virtual b_status_t		SetLook(b_window_look look);
	virtual b_status_t		SetFeel(b_window_feel feel);
	virtual b_status_t		SetTitle(const char *title);
	virtual b_status_t		SetWorkspaces(b_uint32 workspaces);
	virtual b_status_t		GetWorkspaces(b_uint32 *workspaces);
	virtual b_status_t		Iconify();
	virtual b_status_t		Show();
	virtual b_status_t		Hide();
	virtual b_status_t		Raise();
	virtual b_status_t		Lower(BGraphicsWindow *frontWin);
	virtual b_status_t		Activate(bool state);
	virtual b_status_t		GetActivatedState(bool *state) const;
	virtual b_status_t		MoveTo(b_int32 x, b_int32 y);
	virtual b_status_t		ResizeTo(b_uint32 w, b_uint32 h);
	virtual b_status_t		MoveAndResizeTo(b_int32 x, b_int32 y, b_uint32 w, b_uint32 h);
	virtual b_status_t		SetSizeLimits(b_uint32 min_w, b_uint32 max_w, b_uint32 min_h, b_uint32 max_h);
	virtual b_status_t		GetSizeLimits(b_uint32 *min_w, b_uint32 *max_w, b_uint32 *min_h, b_uint32 *max_h);
	virtual b_status_t		GrabMouse();
	virtual b_status_t		UngrabMouse();
	virtual b_status_t		GrabKeyboard();
	virtual b_status_t		UngrabKeyboard();
	virtual b_status_t		QueryMouse(b_int32 *x, b_int32 *y, b_int32 *buttons);

	virtual b_status_t		CopyTo(BGraphicsContext *dc,
					       BGraphicsDrawable *dstDrawable,
					       b_int32 x, b_int32 y, b_uint32 w, b_uint32 h,
					       b_int32 dstX, b_int32 dstY, b_uint32 dstW, b_uint32 dstH);
	virtual b_status_t		DrawPixmap(BGraphicsContext *dc, const BPixmap *pix,
						   b_int32 x, b_int32 y, b_uint32 w, b_uint32 h,
						   b_int32 dstX, b_int32 dstY, b_uint32 dstW, b_uint32 dstH);

	virtual b_status_t		StrokePoint(BGraphicsContext *dc,
						    b_int32 x, b_int32 y);
	virtual b_status_t		StrokePoints(BGraphicsContext *dc,
						     const b_int32 *pts, b_int32 count);
	virtual b_status_t		StrokePoints_Colors(BGraphicsContext *dc,
							    const BList *ptsArrayLists, b_int32 arrayCount,
							    const b_rgb_color *highColors);
	virtual b_status_t		StrokePoints_Alphas(BGraphicsContext *dc,
							    const b_int32 *pts, const b_uint8 *alpha, b_int32 count);
	virtual b_status_t		StrokeLine(BGraphicsContext *dc,
						   b_int32 x0, b_int32 y0, b_int32 x1, b_int32 y1);
	virtual b_status_t		StrokePolygon(BGraphicsContext *dc,
						      const b_int32 *pts, b_int32 count, bool closed);
	virtual b_status_t		FillPolygon(BGraphicsContext *dc,
						    const b_int32 *pts, b_int32 count);
	virtual b_status_t		StrokeRect(BGraphicsContext *dc,
						   b_int32 x, b_int32 y, b_uint32 w, b_uint32 h);
	virtual b_status_t		FillRect(BGraphicsContext *dc,
						 b_int32 x, b_int32 y, b_uint32 w, b_uint32 h);
	virtual b_status_t		StrokeRects(BGraphicsContext *dc,
						    const b_int32 *rects, b_int32 count);
	virtual b_status_t		FillRects(BGraphicsContext *dc,
						  const b_int32 *rects, b_int32 count);
	virtual b_status_t		FillRegion(BGraphicsContext *dc,
						   const BRegion &region);
	virtual b_status_t		StrokeRoundRect(BGraphicsContext *dc,
							b_int32 x, b_int32 y, b_uint32 w, b_uint32 h, b_uint32 xRadius, b_uint32 yRadius);
	virtual b_status_t		FillRoundRect(BGraphicsContext *dc,
						      b_int32 x, b_int32 y, b_uint32 w, b_uint32 h, b_uint32 xRadius, b_uint32 yRadius);

	virtual b_status_t		StrokeArc(BGraphicsContext *dc,
						  b_int32 x, b_int32 y, b_uint32 w, b_uint32 h, float startAngle, float endAngle);
	virtual b_status_t		FillArc(BGraphicsContext *dc,
						b_int32 x, b_int32 y, b_uint32 w, b_uint32 h, float startAngle, float endAngle);

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

#endif // BHAPI_BEOS_GRAPHICS__H

