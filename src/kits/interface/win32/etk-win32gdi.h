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
 * File: etk-win32gdi.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_WIN32GDI__H
#define BHAPI_WIN32GDI__H

#ifdef WIN32

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#ifndef WINVER
#define WINVER 0x0500
#endif

#include "../../support/SupportDefs.h"

#include "../../add-ons/graphics/GraphicsEngine.h"
#include "../../app/MessageFilter.h"
#include "../../app/Message.h"
#include "../../app/Messenger.h"
#include "../../support/Locker.h"

#define WM_BHAPI_MESSAGE_APP			0xa0
#define WM_BHAPI_MESSAGE_WINDOW			0xa1
#define WM_BHAPI_MESSAGE_PIXMAP			0xa2
#define WM_BHAPI_MESSAGE_DRAWING			0xa3
#define WM_BHAPI_MESSAGB_FONT			0xa4

enum {
	WM_BHAPI_MESSAGE_CHANGE_APP_CURSOR = 0,
};

enum {
	WM_BHAPI_MESSAGE_CREATE_WINDOW = 0,
	WM_BHAPI_MESSAGE_DESTROY_WINDOW,
	WM_BHAPI_MESSAGE_SHOW_WINDOW,
	WM_BHAPI_MESSAGE_HIDE_WINDOW,
	WM_BHAPI_MESSAGE_ICONIFY_WINDOW,
	WM_BHAPI_MESSAGE_MOVE_RESIZE_WINDOW,
	WM_BHAPI_MESSAGE_RESIZE_WINDOW,
	WM_BHAPI_MESSAGE_SET_WINDOW_USIZE,
	WM_BHAPI_MESSAGE_GET_WINDOW_USIZE,
	WM_BHAPI_MESSAGE_SET_WINDOW_BACKGROUND,
	WM_BHAPI_MESSAGE_SET_WINDOW_LOOK,
	WM_BHAPI_MESSAGE_SET_WINDOW_FEEL,
	WM_BHAPI_MESSAGE_SET_WINDOW_FLAGS,
	WM_BHAPI_MESSAGE_RAISE_WINDOW,
	WM_BHAPI_MESSAGE_LOWER_WINDOW,
	WM_BHAPI_MESSAGE_ACTIVATE_WINDOW,
	WM_BHAPI_MESSAGE_GET_WINDOW_ACTIVATE_STATE,
	WM_BHAPI_MESSAGE_GRAB_WINDOW,
};

enum {
	WM_BHAPI_MESSAGE_CREATE_PIXMAP = 0,
	WM_BHAPI_MESSAGE_DESTROY_PIXMAP,
	WM_BHAPI_MESSAGE_RESIZE_PIXMAP,
	WM_BHAPI_MESSAGE_DRAW_PIXMAP,
	WM_BHAPI_MESSAGE_DRAW_EPIXMAP,
};

enum {
	WM_BHAPI_MESSAGE_STROKE_POINT = 0,
	WM_BHAPI_MESSAGE_STROKE_POINTS,
	WM_BHAPI_MESSAGE_STROKE_POINTS_COLOR,
	WM_BHAPI_MESSAGE_STROKE_POINTS_ALPHA,
	WM_BHAPI_MESSAGE_STROKE_LINE,
	WM_BHAPI_MESSAGE_STROKE_RECT,
	WM_BHAPI_MESSAGE_STROKE_RECTS,
	WM_BHAPI_MESSAGE_FILL_RECT,
	WM_BHAPI_MESSAGE_FILL_RECTS,
	WM_BHAPI_MESSAGE_FILL_REGION,
	WM_BHAPI_MESSAGE_STROKE_ROUND_RECT,
	WM_BHAPI_MESSAGE_FILL_ROUND_RECT,
	WM_BHAPI_MESSAGE_STROKE_ARC,
	WM_BHAPI_MESSAGE_FILL_ARC,
	WM_BHAPI_MESSAGE_STROKE_POLYGON,
	WM_BHAPI_MESSAGE_FILL_POLYGON,
};

enum {
	WM_BHAPI_MESSAGE_CREATB_FONT = 0,
	WM_BHAPI_MESSAGE_DESTROY_FONT,
	WM_BHAPI_MESSAGB_FONT_STRING_WIDTH,
	WM_BHAPI_MESSAGB_FONT_GET_HEIGHT,
	WM_BHAPI_MESSAGB_FONT_RENDER_STRING,
	WM_BHAPI_MESSAGE_CREATB_FONT_TMP_DC,
	WM_BHAPI_MESSAGE_DESTROY_FONT_TMP_DC,
};

class EWin32GraphicsEngine;
class EWin32GraphicsDrawable;
class EWin32GraphicsWindow;

extern bool bhapi::win32_window_get_rect(HWND hWnd, RECT *r);
extern bool bhapi::win32_window_convert_to_screen(HWND hWnd, int *x, int *y);
extern bool bhapi::win32_window_convert_window_to_client(HWND hWnd, RECT *wr);

extern "C" {
// free it by "free"
extern char* bhapi::win32_convert_utf8_to_active(const char *str, b_int32 length);
extern char* bhapi::win32_convert_active_to_utf8(const char *str, b_int32 length);
}

class EWin32GraphicsEngine : public BGraphicsEngine {
public:
	EWin32GraphicsEngine();
	virtual ~EWin32GraphicsEngine();

	b_status_t			InitCheck();

	bool				Lock();
	void				Unlock();

	bool				GetContactor(HWND hWnd, BMessenger *msgr);
	EWin32GraphicsWindow		*GetWin32Window(HWND hWnd);
	HRGN				ConvertRegion(const BRegion *region);
	bool				PrepareContext(EWin32GraphicsDrawable *pixmap, BGraphicsContext *dc,
						       bool hollowBrush, bool setPenSize);

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

	HINSTANCE win32Hinstance;
	ATOM win32RegisterClass;
	HDC win32ScreenHDC;

	DWORD win32ThreadID;
	HWND win32RequestWin;
	HWND win32RequestAsyncWin;
	UINT WM_BHAPI_MESSAGE;

	HWND win32NextClipboardViewer;
	HCURSOR win32Cursor;

	BMessenger win32PrevMouseMovedWin;
	int win32PrevMouseMovedX;
	int win32PrevMouseMovedY;
	b_bigtime_t win32PrevMouseDownTime;
	int win32PrevMouseDownCount;

	bool win32DoQuit;

	void *fRequestSem;
	void *fRequestAsyncSem;

private:
	BLocker fLocker;
	void *fRequestThread;
	void *fRequestAsyncThread;
	BMessageFilter *fClipboardFilter;
};


class EWin32GraphicsDrawable : public BGraphicsDrawable {
public:
	EWin32GraphicsDrawable(EWin32GraphicsEngine *win32Engine, b_uint32 w, b_uint32 h);
	virtual ~EWin32GraphicsDrawable();

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

	HBITMAP win32Pixmap;
	HDC win32HDC;
	HPEN win32Pen;
	HBRUSH win32Brush;

private:
	HWND fRequestAsyncWin;
	UINT WM_BHAPI_MESSAGE;
};


class EWin32GraphicsWindow : public BGraphicsWindow {
public:
	EWin32GraphicsWindow(EWin32GraphicsEngine *win32Engine, b_int32 x, b_int32 y, b_uint32 w, b_uint32 h);
	virtual ~EWin32GraphicsWindow();

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

	HWND win32Window;
	b_window_look fLook;
	b_window_feel fFeel;
	bool fActivateWhenShown;
	HBRUSH hbrBackground;

private:
	friend class EWin32GraphicsEngine;

	EWin32GraphicsEngine *fEngine;

	BMessenger fMsgr;

	HWND fRequestWin;
	HWND fRequestAsyncWin;
	UINT WM_BHAPI_MESSAGE;
};


typedef struct bhapi::win32_gdi_callback_t {
	int command;

	EWin32GraphicsWindow *win;
	b_int32 x;
	b_int32 y;
	b_uint32 w;
	b_uint32 h;
	b_window_look look;
	b_window_feel feel;
	b_uint32 flags;
	b_rgb_color bkColor;
	EWin32GraphicsWindow *frontWin;
	bool activate_state;
	bool grab_state;
	bool grab_mouse;
	b_uint32 min_w;
	b_uint32 min_h;
	b_uint32 max_w;
	b_uint32 max_h;

	EWin32GraphicsDrawable *pixmap;
	BGraphicsDrawable *dstDrawable;
	b_int32 wx;
	b_int32 wy;
	b_uint32 ww;
	b_uint32 wh;

	BGraphicsContext *dc;

	const b_int32 *pts;
	const b_uint8 *ptsAlpha;
	b_int32 ptsCount;
	bool polyClosed;

	float startAngle;
	float endAngle;

	const BList *ptsArrayLists;
	b_int32 ptsArrayCount;
	const b_rgb_color *ptsColors;

	const BRegion *region;

	const char* fontFamily;
	const char* fontString;
	bool fontAliasing;
	b_uint32 fontSpacing;
	HFONT *font;
	HDC *fontTmpDC;

	const void *data;
} bhapi::win32_gdi_callback_t;


#ifndef GET_WHEEL_DELTA_WPARAM
#define GET_WHEEL_DELTA_WPARAM(wparam)	((short)HIWORD(wparam))
#endif

#endif /* WIN32 */
#endif /* BHAPI_WIN32GDI__H */

