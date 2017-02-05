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
 * File: etk-x11.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_X11_H__
#define __ETK_X11_H__

#ifdef LINUX

#include "./../support/SupportDefs.h"

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

#include "./../../support/Locker.h"
#include "./../../support/StringArray.h"
#include "./../../add-ons/graphics/GraphicsEngine.h"
#include "./../../app/MessageFilter.h"

#ifdef __cplusplus

#define StandardWidgetXEventMask	(KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask | ButtonMotionMask | ExposureMask | VisibilityChangeMask | StructureNotifyMask | FocusChangeMask | PropertyChangeMask)

#if SIZEOF_VOID_P == 2
	typedef euint16 etk_x11_address_t;
	#define ETK_X11_ADDRESS_T_FORMAT 16
	#define ETK_X11_ADDRESS_T_NELEMENTS 1
#elif SIZEOF_VOID_P == 4
	typedef euint32 etk_x11_address_t;
	#define ETK_X11_ADDRESS_T_FORMAT 32
	#define ETK_X11_ADDRESS_T_NELEMENTS 1
#elif SIZEOF_VOID_P == 8
	typedef euint64 etk_x11_address_t;
	#define ETK_X11_ADDRESS_T_FORMAT 32
	#define ETK_X11_ADDRESS_T_NELEMENTS 2
#else
	#error "Size of (void*) isn't 16-bit or 32-bit, nor 64-bit"
#endif


class EXGraphicsEngine : public EGraphicsEngine {
public:
	EXGraphicsEngine();
	virtual ~EXGraphicsEngine();

	e_status_t			InitCheck();

	bool				Lock();
	void				Unlock();

	bool				GetContactor(Window w, EMessenger *msgr);
	bool				ConvertRegion(const ERegion *region, Region *xRegion);
	bool				ConvertRegion(const ERegion *region, XRectangle **xRects, int *nrects);

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
	EStringArray xFontEngines;

	unsigned long xPrevMouseDownSerial;
	Time xPrevMouseDownTime;
	eint32 xPrevMouseDownCount;

private:
	ELocker fLocker;
	eint64 xLocksCount;

	void *fX11Thread;
	EMessageFilter *fClipboardFilter;
};


class EXGraphicsContext : public EGraphicsContext {
public:
	EXGraphicsContext(EXGraphicsEngine *x11Engine);
	virtual ~EXGraphicsContext();

	e_status_t			GetXClipping(Region *xRegion) const;
	e_status_t			GetXHighColor(unsigned long *pixel) const;
	e_status_t			GetXLowColor(unsigned long *pixel) const;
	void				PrepareXColor();

	virtual e_status_t		SetDrawingMode(e_drawing_mode mode);
	virtual e_status_t		SetClipping(const ERegion &clipping);
	virtual e_status_t		SetHighColor(e_rgb_color highColor);
	virtual e_status_t		SetLowColor(e_rgb_color lowColor);
	virtual e_status_t		SetPattern(e_pattern pattern);
	virtual e_status_t		SetPenSize(euint32 penSize);

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

	static bool AllocXColor(EXGraphicsEngine *engine, e_rgb_color color, unsigned long *pixel);
	static void FreeXColor(EXGraphicsEngine *engine, unsigned long pixel);
};


class EXGraphicsDrawable : public EGraphicsDrawable {
public:
	EXGraphicsDrawable(EXGraphicsEngine *x11Engine, euint32 w, euint32 h);
	virtual ~EXGraphicsDrawable();

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


class EXGraphicsWindow : public EGraphicsWindow {
public:
	EXGraphicsWindow(EXGraphicsEngine *x11Engine, eint32 x, eint32 y, euint32 w, euint32 h);
	virtual ~EXGraphicsWindow();

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

	Window xWindow;

private:
	friend class EXGraphicsEngine;

	EXGraphicsEngine *fEngine;

	GC xGC;

	bool xBackgroundAlloced;
	unsigned long xBackground;

	EMessenger fMsgr;
	e_window_look fLook;
	e_window_feel fFeel;
	euint32 fFlags;
};

#endif /* __cplusplus */

#endif /* LINUX */

#endif /* __ETK_X11_H__ */

