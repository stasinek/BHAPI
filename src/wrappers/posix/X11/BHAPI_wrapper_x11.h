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
 * File: BHAPI_wrapper_x11.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_X11_H
#define BHAPI_X11_H

#ifdef LINUX

#include <Haiku.h>

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
	typedef  uint16 b_x11_address_t;
	#define BHAPI_X11_ADDRESS_T_FORMAT 16
	#define BHAPI_X11_ADDRESS_T_NELEMENTS 1
#elif SIZEOF_VOID_P == 4
	typedef  uint32 b_x11_address_t;
	#define BHAPI_X11_ADDRESS_T_FORMAT 32
	#define BHAPI_X11_ADDRESS_T_NELEMENTS 1
#elif SIZEOF_VOID_P == 8
	typedef  uint64 b_x11_address_t;
	#define BHAPI_X11_ADDRESS_T_FORMAT 32
	#define BHAPI_X11_ADDRESS_T_NELEMENTS 2
#else
	#error "Size of (void*) isn't 16-bit or 32-bit, nor 64-bit"
#endif


class EXGraphicsEngine : public BGraphicsEngine {
public:
	EXGraphicsEngine();
	virtual ~EXGraphicsEngine();

	status_t			InitCheck();

	bool				Lock();
	void				Unlock();

	bool				GetContactor(Window w, BMessenger *msgr);
	bool				ConvertRegion(const BRegion *region, Region *xRegion);
	bool				ConvertRegion(const BRegion *region, XRectangle **xRects, int *nrects);

	virtual status_t		Initalize();
	virtual void			Cancel();

	virtual BGraphicsContext*	CreateContext();
	virtual BGraphicsDrawable*	CreatePixmap(uint32 w,  uint32 h);
	virtual BGraphicsWindow*	CreateWindow(int32 x,  int32 y,  uint32 w,  uint32 h);

	virtual status_t		InitalizeFonts();
	virtual void			DestroyFonts();
	virtual status_t		UpdateFonts(bool check_only);

	virtual status_t		GetDesktopBounds(uint32 *w,  uint32 *h);
	virtual status_t		GetCurrentWorkspace(uint32 *workspace);
	virtual status_t		SetCursor(const void *cursor_data);
	virtual status_t		GetDefaultCursor(BCursor *cursor);

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
	int32 xPrevMouseDownCount;

private:
	BLocker fLocker;
	int64 xLocksCount;

	void *fX11Thread;
	BMessageFilter *fClipboardFilter;
};


class EXGraphicsContext : public BGraphicsContext {
public:
	EXGraphicsContext(EXGraphicsEngine *x11Engine);
	virtual ~EXGraphicsContext();

	status_t			GetXClipping(Region *xRegion) const;
	status_t			GetXHighColor(unsigned long *pixel) const;
	status_t			GetXLowColor(unsigned long *pixel) const;
	void				PrepareXColor();

	virtual status_t		SetDrawingMode(bhapi::drawing_mode mode);
	virtual status_t		SetClipping(const BRegion &clipping);
	virtual status_t		SetHighColor(bhapi::rgb_color highColor);
	virtual status_t		SetLowColor(bhapi::rgb_color lowColor);
	virtual status_t		SetPattern(bhapi::pattern pattern);
	virtual status_t		SetPenSize(uint32 penSize);

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

	static bool AllocXColor(EXGraphicsEngine *engine, bhapi::rgb_color color, unsigned long *pixel);
	static void FreeXColor(EXGraphicsEngine *engine, unsigned long pixel);
};


class EXGraphicsDrawable : public BGraphicsDrawable {
public:
	EXGraphicsDrawable(EXGraphicsEngine *x11Engine,  uint32 w,  uint32 h);
	virtual ~EXGraphicsDrawable();

	virtual status_t		SetBackgroundColor(bhapi::rgb_color bkColor);

	virtual status_t		ResizeTo(uint32 w,  uint32 h);
	virtual status_t		CopyTo(BGraphicsContext *dc,
					       BGraphicsDrawable *dstDrawable,
					        int32 x,  int32 y,  uint32 w,  uint32 h,
					        int32 dstX,  int32 dstY,  uint32 dstW,  uint32 dstH);
	virtual status_t		DrawPixmap(BGraphicsContext *dc, const BPixmap *pix,
						    int32 x,  int32 y,  uint32 w,  uint32 h,
						    int32 dstX,  int32 dstY,  uint32 dstW,  uint32 dstH);

	virtual status_t		StrokePoint(BGraphicsContext *dc,
						     int32 x,  int32 y);
	virtual status_t		StrokePoints(BGraphicsContext *dc,
						     const  int32 *pts,  int32 count);
	virtual status_t		StrokePoints_Colors(BGraphicsContext *dc,
							    const BList *ptsArrayLists,  int32 arrayCount,
							    const bhapi::rgb_color *highColors);
	virtual status_t		StrokePoints_Alphas(BGraphicsContext *dc,
							    const  int32 *pts, const  uint8 *alpha,  int32 count);
	virtual status_t		StrokeLine(BGraphicsContext *dc,
						    int32 x0,  int32 y0,  int32 x1,  int32 y1);
	virtual status_t		StrokePolygon(BGraphicsContext *dc,
						      const  int32 *pts,  int32 count, bool closed);
	virtual status_t		FillPolygon(BGraphicsContext *dc,
						    const  int32 *pts,  int32 count);
	virtual status_t		StrokeRect(BGraphicsContext *dc,
						    int32 x,  int32 y,  uint32 w,  uint32 h);
	virtual status_t		FillRect(BGraphicsContext *dc,
						  int32 x,  int32 y,  uint32 w,  uint32 h);
	virtual status_t		StrokeRects(BGraphicsContext *dc,
						    const  int32 *rects,  int32 count);
	virtual status_t		FillRects(BGraphicsContext *dc,
						  const  int32 *rects,  int32 count);
	virtual status_t		FillRegion(BGraphicsContext *dc,
						   const BRegion &region);
	virtual status_t		StrokeRoundRect(BGraphicsContext *dc,
							int32 x,  int32 y,  uint32 w,  uint32 h,  uint32 xRadius,  uint32 yRadius);
	virtual status_t		FillRoundRect(BGraphicsContext *dc,
						       int32 x,  int32 y,  uint32 w,  uint32 h,  uint32 xRadius,  uint32 yRadius);
	virtual status_t		StrokeArc(BGraphicsContext *dc,
						   int32 x,  int32 y,  uint32 w,  uint32 h, float startAngle, float endAngle);
	virtual status_t		FillArc(BGraphicsContext *dc,
						int32 x,  int32 y,  uint32 w,  uint32 h, float startAngle, float endAngle);

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
	EXGraphicsWindow(EXGraphicsEngine *x11Engine,  int32 x,  int32 y,  uint32 w,  uint32 h);
	virtual ~EXGraphicsWindow();

	virtual status_t		ContactTo(const BMessenger *msgr);
	virtual status_t		SetBackgroundColor(bhapi::rgb_color bkColor);
	virtual status_t		SetFlags(uint32 flags);
	virtual status_t		SetLook(bhapi::window_look look);
	virtual status_t		SetFeel(bhapi::window_feel feel);
	virtual status_t		SetTitle(const char *title);
	virtual status_t		SetWorkspaces(uint32 workspaces);
	virtual status_t		GetWorkspaces(uint32 *workspaces);
	virtual status_t		Iconify();
	virtual status_t		Show();
	virtual status_t		Hide();
	virtual status_t		Raise();
	virtual status_t		Lower(BGraphicsWindow *frontWin);
	virtual status_t		Activate(bool state);
	virtual status_t		GetActivatedState(bool *state) const;
	virtual status_t		MoveTo(int32 x,  int32 y);
	virtual status_t		ResizeTo(uint32 w,  uint32 h);
	virtual status_t		MoveAndResizeTo(int32 x,  int32 y,  uint32 w,  uint32 h);
	virtual status_t		SetSizeLimits(uint32 min_w,  uint32 max_w,  uint32 min_h,  uint32 max_h);
	virtual status_t		GetSizeLimits(uint32 *min_w,  uint32 *max_w,  uint32 *min_h,  uint32 *max_h);
	virtual status_t		GrabMouse();
	virtual status_t		UngrabMouse();
	virtual status_t		GrabKeyboard();
	virtual status_t		UngrabKeyboard();
	virtual status_t		QueryMouse(int32 *x,  int32 *y,  int32 *buttons);

	virtual status_t		CopyTo(BGraphicsContext *dc,
					       BGraphicsDrawable *dstDrawable,
					        int32 x,  int32 y,  uint32 w,  uint32 h,
					        int32 dstX,  int32 dstY,  uint32 dstW,  uint32 dstH);
	virtual status_t		DrawPixmap(BGraphicsContext *dc, const BPixmap *pix,
						    int32 x,  int32 y,  uint32 w,  uint32 h,
						    int32 dstX,  int32 dstY,  uint32 dstW,  uint32 dstH);

	virtual status_t		StrokePoint(BGraphicsContext *dc,
						     int32 x,  int32 y);
	virtual status_t		StrokePoints(BGraphicsContext *dc,
						     const  int32 *pts,  int32 count);
	virtual status_t		StrokePoints_Colors(BGraphicsContext *dc,
							    const BList *ptsArrayLists,  int32 arrayCount,
							    const bhapi::rgb_color *highColors);
	virtual status_t		StrokePoints_Alphas(BGraphicsContext *dc,
							    const  int32 *pts, const  uint8 *alpha,  int32 count);
	virtual status_t		StrokeLine(BGraphicsContext *dc,
						    int32 x0,  int32 y0,  int32 x1,  int32 y1);
	virtual status_t		StrokePolygon(BGraphicsContext *dc,
						      const  int32 *pts,  int32 count, bool closed);
	virtual status_t		FillPolygon(BGraphicsContext *dc,
						    const  int32 *pts,  int32 count);
	virtual status_t		StrokeRect(BGraphicsContext *dc,
						    int32 x,  int32 y,  uint32 w,  uint32 h);
	virtual status_t		FillRect(BGraphicsContext *dc,
						  int32 x,  int32 y,  uint32 w,  uint32 h);
	virtual status_t		StrokeRects(BGraphicsContext *dc,
						    const  int32 *rects,  int32 count);
	virtual status_t		FillRects(BGraphicsContext *dc,
						  const  int32 *rects,  int32 count);
	virtual status_t		FillRegion(BGraphicsContext *dc,
						   const BRegion &region);
	virtual status_t		StrokeRoundRect(BGraphicsContext *dc,
							int32 x,  int32 y,  uint32 w,  uint32 h,  uint32 xRadius,  uint32 yRadius);
	virtual status_t		FillRoundRect(BGraphicsContext *dc,
						       int32 x,  int32 y,  uint32 w,  uint32 h,  uint32 xRadius,  uint32 yRadius);

	virtual status_t		StrokeArc(BGraphicsContext *dc,
						   int32 x,  int32 y,  uint32 w,  uint32 h, float startAngle, float endAngle);
	virtual status_t		FillArc(BGraphicsContext *dc,
						int32 x,  int32 y,  uint32 w,  uint32 h, float startAngle, float endAngle);

	Window xWindow;

private:
	friend class EXGraphicsEngine;

	EXGraphicsEngine *fEngine;

	GC xGC;

	bool xBackgroundAlloced;
	unsigned long xBackground;

	BMessenger fMsgr;
	bhapi::window_look fLook;
	bhapi::window_feel fFeel;
	uint32 fFlags;
};

#endif /* __cplusplus */

#endif /* LINUX */

#endif /* BHAPI_X11_H */

