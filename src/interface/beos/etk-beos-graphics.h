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
 * File: etk-beos-graphics.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_BEOS_GRAPHICS_H__
#define __ETK_BEOS_GRAPHICS_H__

#ifdef BEOS

#include <be/app/Messenger.h>
#include <be/interface/View.h>
#include <be/interface/Window.h>
#include <be/interface/Bitmap.h>

#include "./../../support/Locker.h"
#include "./../../add-ons/graphics/GraphicsEngine.h"
#include "./../../app/MessageFilter.h"

#ifdef __cplusplus /* just for C++ */


class EBeGraphicsEngine : public EGraphicsEngine {
public:
	EBeGraphicsEngine();
	virtual ~EBeGraphicsEngine();

	e_status_t			InitCheck();

	bool				Lock();
	void				Unlock();

	virtual e_status_t		Initalize();
	virtual void			Cancel();

	virtual EGraphicsContext*	CreateContext();
	virtual EGraphicsDrawable*	CreatePixmap(euint32 w, euint32 h);
	virtual EGraphicsWindow*	CreateWindow(eint32 x, eint32 y, euint32 w, euint32 h);

	virtual e_status_t		InitalizeFonts();
	virtual void			DestroyFonts();
	virtual e_status_t		UpdateFonts(bool check_only);

	virtual e_status_t		GetDesktopBounds(euint32 *w, euint32 *h);
	virtual e_status_t		GetCurrentWorkspace(euint32 *workspace);
	virtual e_status_t		SetCursor(const void *cursor_data);
	virtual e_status_t		GetDefaultCursor(ECursor *cursor);

	void *fRequestSem;
	bool beDoQuit;

private:
	ELocker fLocker;
	void *fBeThread;
	EMessageFilter *fClipboardFilter;
};


class EBeBitmapPriv : public BBitmap
{
public:
	EBeBitmapPriv(euint32 w, euint32 h);
	virtual ~EBeBitmapPriv();

	BView *fView;
};


class EBeGraphicsDrawable : public EGraphicsDrawable {
public:
	EBeGraphicsDrawable(EBeGraphicsEngine *beEngine, euint32 w, euint32 h);
	virtual ~EBeGraphicsDrawable();

	virtual e_status_t		SetBackgroundColor(e_rgb_color bkColor);

	virtual e_status_t		ResizeTo(euint32 w, euint32 h);
	virtual e_status_t		CopyTo(EGraphicsContext *dc,
					       EGraphicsDrawable *dstDrawable,
					       eint32 x, eint32 y, euint32 w, euint32 h,
					       eint32 dstX, eint32 dstY, euint32 dstW, euint32 dstH);
	virtual e_status_t		DrawPixmap(EGraphicsContext *dc, const EPixmap *pix,
						   eint32 x, eint32 y, euint32 w, euint32 h,
						   eint32 dstX, eint32 dstY, euint32 dstW, euint32 dstH);

	virtual e_status_t		StrokePoint(EGraphicsContext *dc,
						    eint32 x, eint32 y);
	virtual e_status_t		StrokePoints(EGraphicsContext *dc,
						     const eint32 *pts, eint32 count);
	virtual e_status_t		StrokePoints_Colors(EGraphicsContext *dc,
							    const EList *ptsArrayLists, eint32 arrayCount,
							    const e_rgb_color *highColors);
	virtual e_status_t		StrokePoints_Alphas(EGraphicsContext *dc,
							    const eint32 *pts, const euint8 *alpha, eint32 count);
	virtual e_status_t		StrokeLine(EGraphicsContext *dc,
						   eint32 x0, eint32 y0, eint32 x1, eint32 y1);
	virtual e_status_t		StrokePolygon(EGraphicsContext *dc,
						      const eint32 *pts, eint32 count, bool closed);
	virtual e_status_t		FillPolygon(EGraphicsContext *dc,
						    const eint32 *pts, eint32 count);
	virtual e_status_t		StrokeRect(EGraphicsContext *dc,
						   eint32 x, eint32 y, euint32 w, euint32 h);
	virtual e_status_t		FillRect(EGraphicsContext *dc,
						 eint32 x, eint32 y, euint32 w, euint32 h);
	virtual e_status_t		StrokeRects(EGraphicsContext *dc,
						    const eint32 *rects, eint32 count);
	virtual e_status_t		FillRects(EGraphicsContext *dc,
						  const eint32 *rects, eint32 count);
	virtual e_status_t		FillRegion(EGraphicsContext *dc,
						   const ERegion &region);
	virtual e_status_t		StrokeRoundRect(EGraphicsContext *dc,
							eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius);
	virtual e_status_t		FillRoundRect(EGraphicsContext *dc,
						      eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius);
	virtual e_status_t		StrokeArc(EGraphicsContext *dc,
						  eint32 x, eint32 y, euint32 w, euint32 h, float startAngle, float endAngle);
	virtual e_status_t		FillArc(EGraphicsContext *dc,
						eint32 x, eint32 y, euint32 w, euint32 h, float startAngle, float endAngle);

	EBeBitmapPriv *beBitmap;

private:
	EBeGraphicsEngine *fEngine;
};


class EBeGraphicsWindow : public EGraphicsWindow {
public:
	EBeGraphicsWindow(EBeGraphicsEngine *beEngine, eint32 x, eint32 y, euint32 w, euint32 h);
	virtual ~EBeGraphicsWindow();

	virtual e_status_t		ContactTo(const EMessenger *msgr);
	virtual e_status_t		SetBackgroundColor(e_rgb_color bkColor);
	virtual e_status_t		SetFlags(euint32 flags);
	virtual e_status_t		SetLook(e_window_look look);
	virtual e_status_t		SetFeel(e_window_feel feel);
	virtual e_status_t		SetTitle(const char *title);
	virtual e_status_t		SetWorkspaces(euint32 workspaces);
	virtual e_status_t		GetWorkspaces(euint32 *workspaces);
	virtual e_status_t		Iconify();
	virtual e_status_t		Show();
	virtual e_status_t		Hide();
	virtual e_status_t		Raise();
	virtual e_status_t		Lower(EGraphicsWindow *frontWin);
	virtual e_status_t		Activate(bool state);
	virtual e_status_t		GetActivatedState(bool *state) const;
	virtual e_status_t		MoveTo(eint32 x, eint32 y);
	virtual e_status_t		ResizeTo(euint32 w, euint32 h);
	virtual e_status_t		MoveAndResizeTo(eint32 x, eint32 y, euint32 w, euint32 h);
	virtual e_status_t		SetSizeLimits(euint32 min_w, euint32 max_w, euint32 min_h, euint32 max_h);
	virtual e_status_t		GetSizeLimits(euint32 *min_w, euint32 *max_w, euint32 *min_h, euint32 *max_h);
	virtual e_status_t		GrabMouse();
	virtual e_status_t		UngrabMouse();
	virtual e_status_t		GrabKeyboard();
	virtual e_status_t		UngrabKeyboard();
	virtual e_status_t		QueryMouse(eint32 *x, eint32 *y, eint32 *buttons);

	virtual e_status_t		CopyTo(EGraphicsContext *dc,
					       EGraphicsDrawable *dstDrawable,
					       eint32 x, eint32 y, euint32 w, euint32 h,
					       eint32 dstX, eint32 dstY, euint32 dstW, euint32 dstH);
	virtual e_status_t		DrawPixmap(EGraphicsContext *dc, const EPixmap *pix,
						   eint32 x, eint32 y, euint32 w, euint32 h,
						   eint32 dstX, eint32 dstY, euint32 dstW, euint32 dstH);

	virtual e_status_t		StrokePoint(EGraphicsContext *dc,
						    eint32 x, eint32 y);
	virtual e_status_t		StrokePoints(EGraphicsContext *dc,
						     const eint32 *pts, eint32 count);
	virtual e_status_t		StrokePoints_Colors(EGraphicsContext *dc,
							    const EList *ptsArrayLists, eint32 arrayCount,
							    const e_rgb_color *highColors);
	virtual e_status_t		StrokePoints_Alphas(EGraphicsContext *dc,
							    const eint32 *pts, const euint8 *alpha, eint32 count);
	virtual e_status_t		StrokeLine(EGraphicsContext *dc,
						   eint32 x0, eint32 y0, eint32 x1, eint32 y1);
	virtual e_status_t		StrokePolygon(EGraphicsContext *dc,
						      const eint32 *pts, eint32 count, bool closed);
	virtual e_status_t		FillPolygon(EGraphicsContext *dc,
						    const eint32 *pts, eint32 count);
	virtual e_status_t		StrokeRect(EGraphicsContext *dc,
						   eint32 x, eint32 y, euint32 w, euint32 h);
	virtual e_status_t		FillRect(EGraphicsContext *dc,
						 eint32 x, eint32 y, euint32 w, euint32 h);
	virtual e_status_t		StrokeRects(EGraphicsContext *dc,
						    const eint32 *rects, eint32 count);
	virtual e_status_t		FillRects(EGraphicsContext *dc,
						  const eint32 *rects, eint32 count);
	virtual e_status_t		FillRegion(EGraphicsContext *dc,
						   const ERegion &region);
	virtual e_status_t		StrokeRoundRect(EGraphicsContext *dc,
							eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius);
	virtual e_status_t		FillRoundRect(EGraphicsContext *dc,
						      eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius);

	virtual e_status_t		StrokeArc(EGraphicsContext *dc,
						  eint32 x, eint32 y, euint32 w, euint32 h, float startAngle, float endAngle);
	virtual e_status_t		FillArc(EGraphicsContext *dc,
						eint32 x, eint32 y, euint32 w, euint32 h, float startAngle, float endAngle);

	BMessenger beWinMsgr;

private:
	friend class EBeGraphicsEngine;

	EBeGraphicsEngine *fEngine;
};


extern bool __etk_prepare_beview(BView *view, EGraphicsContext *dc);

enum {
	ETK_BEOS_QUIT = 1,
	ETK_BEOS_CONTACT_TO,
	ETK_BEOS_SET_BACKGROUND,
	ETK_BEOS_SET_LOOK,
	ETK_BEOS_SET_TITLE,
	ETK_BEOS_SET_WORKSPACES,
	ETK_BEOS_GET_WORKSPACES,
	ETK_BEOS_ICONIFY,
	ETK_BEOS_SHOW,
	ETK_BEOS_HIDE,
	ETK_BEOS_RAISE,
	ETK_BEOS_LOWER,
	ETK_BEOS_ACTIVATE,
	ETK_BEOS_GET_ACTIVATED_STATE,
	ETK_BEOS_MOVE_RESIZE,
	ETK_BEOS_SET_SIZE_LIMITS,
	ETK_BEOS_GET_SIZE_LIMITS,
	ETK_BEOS_GRAB_MOUSE,
	ETK_BEOS_UNGRAB_MOUSE,
	ETK_BEOS_GRAB_KEYBOARD,
	ETK_BEOS_UNGRAB_KEYBOARD,
	ETK_BEOS_QUERY_MOUSE,
	ETK_BEOS_DRAW_BITMAP,
};


#endif /* __cplusplus */
#endif /* BEOS */

#endif // __ETK_BEOS_GRAPHICS_H__

