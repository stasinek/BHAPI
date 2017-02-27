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
 * File: etk-x11.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_X11__H
#define BHAPI_X11__H

#ifdef LINUX

#include "../support/SupportDefs.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#if defined(HAVE_X11_XPOLL_H) || defined(WIN32) || defined(__BEOS__)
#include <X11/Xpoll.h>
#endif // HAVE_X11_XPOLL_H
#include <X11/keysym.h>
#include <X11/Xatom.h>

#ifdef HAVE_XFT
#include <X11/Xft/Xft.h>
#endif

#endif

#include "../../support/Locker.h"
#include "../../support/StringArray.h"
#include "../../add-ons/graphics/GraphicsEngine.h"
#include "../../app/MessageFilter.h"

#ifdef __cplusplus

#define StandardWidgetXEventMask	(KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask | ButtonMotionMask | ExposureMask | VisibilityChangeMask | StructureNotifyMask | FocusChangeMask | PropertyChangeMask)

#if SIZEOF_VOID_P == 2
	typedef b_uint16 b_x11_address_t;
	#define BHAPI_X11_ADDRESS_T_FORMAT 16
	#define BHAPI_X11_ADDRESS_T_NELEMENTS 1
#elif SIZEOF_VOID_P == 4
	typedef b_uint32 b_x11_address_t;
	#define BHAPI_X11_ADDRESS_T_FORMAT 32
	#define BHAPI_X11_ADDRESS_T_NELEMENTS 1
#elif SIZEOF_VOID_P == 8
	typedef b_uint64 b_x11_address_t;
	#define BHAPI_X11_ADDRESS_T_FORMAT 32
	#define BHAPI_X11_ADDRESS_T_NELEMENTS 2
#else
	#error "Size of (void*) isn't 16-bit or 32-bit, nor 64-bit"
#endif


class EXGraphicsEngine : public BGraphicsEngine {
public:
	EXGraphicsEngine();
	virtual ~EXGraphicsEngine();

	b_status_t			InitCheck();

	bool				Lock();
	void				Unlock();

	bool				GetContactor(Window w, BMessenger *msgr);
	bool				ConvertRegion(const BRegion *region, Region *xRegion);
	bool				ConvertRegion(const BRegion *region, XRectangle **xRects, int *nrects);

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

	Display *xDisplay;
	bool xSupportThreads;
	int xSocket;
	int xScreen;
	Window xRootWindow;
	Window xProtocolsWindow;
	Visual *xVisual;
	Colormap xColormap;
	int xDepth;
	unsigned int xDisplayWidth;
	unsigned int xDisplayHeight;
	Cursor xCursor;
	Atom atomProtocols;
	Atom atomDeleteWindow;
	Atom atomWMDeleteWindow;
	Atom atomWMFocus;
	Atom atomWMDesktop;
	Atom atomCurrentDesktop;
	Atom atomEventPending;
	Atom atomClipboard;
	Atom atomCompoundText;
	unsigned long xBlackPixel;
	unsigned long xWhitePixel;
	long xInputMethodEventMask;
	XIM xInputMethod;
	XIC xInputContext;
	bool xDoQuit;
	BStringArray xFontEngines;

	unsigned long xPrevMouseDownSerial;
	Time xPrevMouseDownTime;
	b_int32 xPrevMouseDownCount;

private:
	BLocker fLocker;
	b_int64 xLocksCount;

	void *fX11Thread;
	BMessageFilter *fClipboardFilter;
};


class EXGraphicsContext : public BGraphicsContext {
public:
	EXGraphicsContext(EXGraphicsEngine *x11Engine);
	virtual ~EXGraphicsContext();

	b_status_t			GetXClipping(Region *xRegion) const;
	b_status_t			GetXHighColor(unsigned long *pixel) const;
	b_status_t			GetXLowColor(unsigned long *pixel) const;
	void				PrepareXColor();

	virtual b_status_t		SetDrawingMode(b_drawing_mode mode);
	virtual b_status_t		SetClipping(const BRegion &clipping);
	virtual b_status_t		SetHighColor(b_rgb_color highColor);
	virtual b_status_t		SetLowColor(b_rgb_color lowColor);
	virtual b_status_t		SetPattern(b_pattern pattern);
	virtual b_status_t		SetPenSize(b_uint32 penSize);

	GC xGC;

private:
	friend class EXGraphicsDrawable;
	friend class EXGraphicsWindow;

	EXGraphicsEngine *fEngine;

	unsigned long xHighColor;
	unsigned long xLowColor;
	bool xHighColorAlloced;
	bool xLowColorAlloced;
	Region xClipping;

	static bool AllocXColor(EXGraphicsEngine *engine, b_rgb_color color, unsigned long *pixel);
	static void FreeXColor(EXGraphicsEngine *engine, unsigned long pixel);
};


class EXGraphicsDrawable : public BGraphicsDrawable {
public:
	EXGraphicsDrawable(EXGraphicsEngine *x11Engine, b_uint32 w, b_uint32 h);
	virtual ~EXGraphicsDrawable();

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

	Pixmap xPixmap;
#ifdef HAVE_XFT
	XftDraw *xDraw;
#endif

private:
	EXGraphicsEngine *fEngine;

	GC xGC;
	bool xBackgroundAlloced;
	unsigned long xBackground;
};


class EXGraphicsWindow : public BGraphicsWindow {
public:
	EXGraphicsWindow(EXGraphicsEngine *x11Engine, b_int32 x, b_int32 y, b_uint32 w, b_uint32 h);
	virtual ~EXGraphicsWindow();

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

	Window xWindow;

private:
	friend class EXGraphicsEngine;

	EXGraphicsEngine *fEngine;

	GC xGC;

	bool xBackgroundAlloced;
	unsigned long xBackground;

	BMessenger fMsgr;
	b_window_look fLook;
	b_window_feel fFeel;
	b_uint32 fFlags;
};

#endif /* __cplusplus */

#endif /* LINUX */

#endif /* BHAPI_X11__H */

