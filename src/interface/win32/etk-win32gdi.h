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
 * File: etk-win32gdi.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_WIN32GDI_H__
#define __ETK_WIN32GDI_H__

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#ifndef WINVER
#define WINVER 0x0500
#endif

#include <windows.h>

#include "./../support/Locker.h>
#include "./../add-ons/graphics/GraphicsEngine.h>
#include "./../app/MessageFilter.h>

#define WM_ETK_MESSAGE_APP			0xa0
#define WM_ETK_MESSAGE_WINDOW			0xa1
#define WM_ETK_MESSAGE_PIXMAP			0xa2
#define WM_ETK_MESSAGE_DRAWING			0xa3
#define WM_ETK_MESSAGE_FONT			0xa4

enum {
	WM_ETK_MESSAGE_CHANGE_APP_CURSOR = 0,
};

enum {
	WM_ETK_MESSAGE_CREATE_WINDOW = 0,
	WM_ETK_MESSAGE_DESTROY_WINDOW,
	WM_ETK_MESSAGE_SHOW_WINDOW,
	WM_ETK_MESSAGE_HIDE_WINDOW,
	WM_ETK_MESSAGE_ICONIFY_WINDOW,
	WM_ETK_MESSAGE_MOVE_RESIZE_WINDOW,
	WM_ETK_MESSAGE_RESIZE_WINDOW,
	WM_ETK_MESSAGE_SET_WINDOW_USIZE,
	WM_ETK_MESSAGE_GET_WINDOW_USIZE,
	WM_ETK_MESSAGE_SET_WINDOW_BACKGROUND,
	WM_ETK_MESSAGE_SET_WINDOW_LOOK,
	WM_ETK_MESSAGE_SET_WINDOW_FEEL,
	WM_ETK_MESSAGE_SET_WINDOW_FLAGS,
	WM_ETK_MESSAGE_RAISE_WINDOW,
	WM_ETK_MESSAGE_LOWER_WINDOW,
	WM_ETK_MESSAGE_ACTIVATE_WINDOW,
	WM_ETK_MESSAGE_GET_WINDOW_ACTIVATE_STATE,
	WM_ETK_MESSAGE_GRAB_WINDOW,
};

enum {
	WM_ETK_MESSAGE_CREATE_PIXMAP = 0,
	WM_ETK_MESSAGE_DESTROY_PIXMAP,
	WM_ETK_MESSAGE_RESIZE_PIXMAP,
	WM_ETK_MESSAGE_DRAW_PIXMAP,
	WM_ETK_MESSAGE_DRAW_EPIXMAP,
};

enum {
	WM_ETK_MESSAGE_STROKE_POINT = 0,
	WM_ETK_MESSAGE_STROKE_POINTS,
	WM_ETK_MESSAGE_STROKE_POINTS_COLOR,
	WM_ETK_MESSAGE_STROKE_POINTS_ALPHA,
	WM_ETK_MESSAGE_STROKE_LINE,
	WM_ETK_MESSAGE_STROKE_RECT,
	WM_ETK_MESSAGE_STROKE_RECTS,
	WM_ETK_MESSAGE_FILL_RECT,
	WM_ETK_MESSAGE_FILL_RECTS,
	WM_ETK_MESSAGE_FILL_REGION,
	WM_ETK_MESSAGE_STROKE_ROUND_RECT,
	WM_ETK_MESSAGE_FILL_ROUND_RECT,
	WM_ETK_MESSAGE_STROKE_ARC,
	WM_ETK_MESSAGE_FILL_ARC,
	WM_ETK_MESSAGE_STROKE_POLYGON,
	WM_ETK_MESSAGE_FILL_POLYGON,
};

enum {
	WM_ETK_MESSAGE_CREATE_FONT = 0,
	WM_ETK_MESSAGE_DESTROY_FONT,
	WM_ETK_MESSAGE_FONT_STRING_WIDTH,
	WM_ETK_MESSAGE_FONT_GET_HEIGHT,
	WM_ETK_MESSAGE_FONT_RENDER_STRING,
	WM_ETK_MESSAGE_CREATE_FONT_TMP_DC,
	WM_ETK_MESSAGE_DESTROY_FONT_TMP_DC,
};

class EWin32GraphicsEngine;
class EWin32GraphicsDrawable;
class EWin32GraphicsWindow;

extern bool etk_win32_window_get_rect(HWND hWnd, RECT *r);
extern bool etk_win32_window_convert_to_screen(HWND hWnd, int *x, int *y);
extern bool etk_win32_window_convert_window_to_client(HWND hWnd, RECT *wr);

extern "C" {
// free it by "free"
extern char* etk_win32_convert_utf8_to_active(const char *str, eint32 length);
extern char* etk_win32_convert_active_to_utf8(const char *str, eint32 length);
}

class EWin32GraphicsEngine : public EGraphicsEngine {
public:
	EWin32GraphicsEngine();
	virtual ~EWin32GraphicsEngine();

	e_status_t			InitCheck();

	bool				Lock();
	void				Unlock();

	bool				GetContactor(HWND hWnd, EMessenger *msgr);
	EWin32GraphicsWindow		*GetWin32Window(HWND hWnd);
	HRGN				ConvertRegion(const ERegion *region);
	bool				PrepareContext(EWin32GraphicsDrawable *pixmap, EGraphicsContext *dc,
						       bool hollowBrush, bool setPenSize);

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

	HINSTANCE win32Hinstance;
	ATOM win32RegisterClass;
	HDC win32ScreenHDC;

	DWORD win32ThreadID;
	HWND win32RequestWin;
	HWND win32RequestAsyncWin;
	UINT WM_ETK_MESSAGE;

	HWND win32NextClipboardViewer;
	HCURSOR win32Cursor;

	EMessenger win32PrevMouseMovedWin;
	int win32PrevMouseMovedX;
	int win32PrevMouseMovedY;
	e_bigtime_t win32PrevMouseDownTime;
	int win32PrevMouseDownCount;

	bool win32DoQuit;

	void *fRequestSem;
	void *fRequestAsyncSem;

private:
	ELocker fLocker;
	void *fRequestThread;
	void *fRequestAsyncThread;
	EMessageFilter *fClipboardFilter;
};


class EWin32GraphicsDrawable : public EGraphicsDrawable {
public:
	EWin32GraphicsDrawable(EWin32GraphicsEngine *win32Engine, euint32 w, euint32 h);
	virtual ~EWin32GraphicsDrawable();

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

	HBITMAP win32Pixmap;
	HDC win32HDC;
	HPEN win32Pen;
	HBRUSH win32Brush;

private:
	HWND fRequestAsyncWin;
	UINT WM_ETK_MESSAGE;
};


class EWin32GraphicsWindow : public EGraphicsWindow {
public:
	EWin32GraphicsWindow(EWin32GraphicsEngine *win32Engine, eint32 x, eint32 y, euint32 w, euint32 h);
	virtual ~EWin32GraphicsWindow();

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

	HWND win32Window;
	e_window_look fLook;
	e_window_feel fFeel;
	bool fActivateWhenShown;
	HBRUSH hbrBackground;

private:
	friend class EWin32GraphicsEngine;

	EWin32GraphicsEngine *fEngine;

	EMessenger fMsgr;

	HWND fRequestWin;
	HWND fRequestAsyncWin;
	UINT WM_ETK_MESSAGE;
};


typedef struct etk_win32_gdi_callback_t {
	int command;

	EWin32GraphicsWindow *win;
	eint32 x;
	eint32 y;
	euint32 w;
	euint32 h;
	e_window_look look;
	e_window_feel feel;
	euint32 flags;
	e_rgb_color bkColor;
	EWin32GraphicsWindow *frontWin;
	bool activate_state;
	bool grab_state;
	bool grab_mouse;
	euint32 min_w;
	euint32 min_h;
	euint32 max_w;
	euint32 max_h;

	EWin32GraphicsDrawable *pixmap;
	EGraphicsDrawable *dstDrawable;
	eint32 wx;
	eint32 wy;
	euint32 ww;
	euint32 wh;

	EGraphicsContext *dc;

	const eint32 *pts;
	const euint8 *ptsAlpha;
	eint32 ptsCount;
	bool polyClosed;

	float startAngle;
	float endAngle;

	const EList *ptsArrayLists;
	eint32 ptsArrayCount;
	const e_rgb_color *ptsColors;

	const ERegion *region;

	const char* fontFamily;
	const char* fontString;
	bool fontAliasing;
	euint32 fontSpacing;
	HFONT *font;
	HDC *fontTmpDC;

	const void *data;
} etk_win32_gdi_callback_t;


#ifndef GET_WHEEL_DELTA_WPARAM
#define GET_WHEEL_DELTA_WPARAM(wparam)	((short)HIWORD(wparam))
#endif

#endif /* __ETK_WIN32GDI_H__ */

