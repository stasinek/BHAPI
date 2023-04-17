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
 * File: BHAPI_wrapper_win32gdi.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_WIN32GDI_H
#define BHAPI_WIN32GDI_H

#ifdef WIN32

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#ifndef WINVER
#define WINVER 0x0500
#endif

#include <kits/support/SupportDefs.h>

#include <kits/add-ons/graphics/GraphicsEngine.h>
#include <kits/app/MessageFilter.h>
#include <kits/app/Message.h>
#include <kits/app/Messenger.h>
#include <kits/support/Locker.h>

#define WMBHAPI_MESSAGE_APP			0xa0
#define WMBHAPI_MESSAGE_WINDOW			0xa1
#define WMBHAPI_MESSAGE_PIXMAP			0xa2
#define WMBHAPI_MESSAGE_DRAWING			0xa3
#define WMBHAPI_MESSAGB_FONT			0xa4

enum {
	WMBHAPI_MESSAGE_CHANGE_APP_CURSOR = 0,
};

enum {
	WMBHAPI_MESSAGE_CREATE_WINDOW = 0,
	WMBHAPI_MESSAGE_DESTROY_WINDOW,
	WMBHAPI_MESSAGE_SHOW_WINDOW,
	WMBHAPI_MESSAGE_HIDE_WINDOW,
	WMBHAPI_MESSAGE_ICONIFY_WINDOW,
	WMBHAPI_MESSAGE_MOVE_RESIZE_WINDOW,
	WMBHAPI_MESSAGE_RESIZE_WINDOW,
	WMBHAPI_MESSAGE_SET_WINDOW_USIZE,
	WMBHAPI_MESSAGE_GET_WINDOW_USIZE,
	WMBHAPI_MESSAGE_SET_WINDOW_BACKGROUND,
	WMBHAPI_MESSAGE_SET_WINDOW_LOOK,
	WMBHAPI_MESSAGE_SET_WINDOW_FEEL,
	WMBHAPI_MESSAGE_SET_WINDOW_FLAGS,
	WMBHAPI_MESSAGE_RAISE_WINDOW,
	WMBHAPI_MESSAGE_LOWER_WINDOW,
	WMBHAPI_MESSAGE_ACTIVATE_WINDOW,
	WMBHAPI_MESSAGE_GET_WINDOW_ACTIVATE_STATE,
	WMBHAPI_MESSAGE_GRAB_WINDOW,
};

enum {
	WMBHAPI_MESSAGE_CREATE_PIXMAP = 0,
	WMBHAPI_MESSAGE_DESTROY_PIXMAP,
	WMBHAPI_MESSAGE_RESIZE_PIXMAP,
	WMBHAPI_MESSAGE_DRAW_PIXMAP,
	WMBHAPI_MESSAGE_DRAW_EPIXMAP,
};

enum {
	WMBHAPI_MESSAGE_STROKE_POINT = 0,
	WMBHAPI_MESSAGE_STROKE_POINTS,
	WMBHAPI_MESSAGE_STROKE_POINTS_COLOR,
	WMBHAPI_MESSAGE_STROKE_POINTS_ALPHA,
	WMBHAPI_MESSAGE_STROKE_LINE,
	WMBHAPI_MESSAGE_STROKE_RECT,
	WMBHAPI_MESSAGE_STROKE_RECTS,
	WMBHAPI_MESSAGE_FILL_RECT,
	WMBHAPI_MESSAGE_FILL_RECTS,
	WMBHAPI_MESSAGE_FILL_REGION,
	WMBHAPI_MESSAGE_STROKE_ROUND_RECT,
	WMBHAPI_MESSAGE_FILL_ROUND_RECT,
	WMBHAPI_MESSAGE_STROKE_ARC,
	WMBHAPI_MESSAGE_FILL_ARC,
	WMBHAPI_MESSAGE_STROKE_POLYGON,
	WMBHAPI_MESSAGE_FILL_POLYGON,
};

enum {
	WMBHAPI_MESSAGE_CREATB_FONT = 0,
	WMBHAPI_MESSAGE_DESTROY_FONT,
	WMBHAPI_MESSAGB_FONT_STRING_WIDTH,
	WMBHAPI_MESSAGB_FONT_GET_HEIGHT,
	WMBHAPI_MESSAGB_FONT_RENDER_STRING,
	WMBHAPI_MESSAGE_CREATB_FONT_TMP_DC,
	WMBHAPI_MESSAGE_DESTROY_FONT_TMP_DC,
};

class EWin32GraphicsEngine;
class EWin32GraphicsDrawable;
class EWin32GraphicsWindow;

namespace bhapi {
extern bool win32_window_get_rect(HWND hWnd, RECT *r);
extern bool win32_window_convert_to_screen(HWND hWnd, int *x, int *y);
extern bool win32_window_convert_window_to_client(HWND hWnd, RECT *wr);
} /* namespace */

extern "C" {
// free it by "free"
namespace bhapi {
extern char* win32_convert_utf8_to_active(const char *str,  __be_int32_t length);
extern char* win32_convert_active_to_utf8(const char *str,  __be_int32_t length);
} /* namespace */
} /* extern  C */

class EWin32GraphicsEngine : public BGraphicsEngine {
public:
	EWin32GraphicsEngine();
	virtual ~EWin32GraphicsEngine();

	status_t			InitCheck();

	bool				Lock();
	void				Unlock();

	bool				GetContactor(HWND hWnd, BMessenger *msgr);
	EWin32GraphicsWindow		*GetWin32Window(HWND hWnd);
	HRGN				ConvertRegion(const BRegion *region);
	bool				PrepareContext(EWin32GraphicsDrawable *pixmap, BGraphicsContext *dc,
						       bool hollowBrush, bool setPenSize);

	virtual status_t		Initalize();
	virtual void			Cancel();

	virtual BGraphicsContext*	CreateContext();
	virtual BGraphicsDrawable*	CreatePixmap(__be_uint32_t w,  __be_uint32_t h);
	virtual BGraphicsWindow*	CreateWindow(__be_int32_t x,  __be_int32_t y,  __be_uint32_t w,  __be_uint32_t h);

	virtual status_t		InitalizeFonts();
	virtual void			DestroyFonts();
	virtual status_t		UpdateFonts(bool check_only);

	virtual status_t		GetDesktopBounds(__be_uint32_t *w,  __be_uint32_t *h);
	virtual status_t		GetCurrentWorkspace(__be_uint32_t *workspace);
	virtual status_t		SetCursor(const void *cursor_data);
	virtual status_t		GetDefaultCursor(BCursor *cursor);

	HINSTANCE win32Hinstance;
	ATOM win32RegisterClass;
	HDC win32ScreenHDC;

	DWORD win32ThreadID;
	HWND win32RequestWin;
	HWND win32RequestAsyncWin;
	UINT WMBHAPI_MESSAGE;

	HWND win32NextClipboardViewer;
	HCURSOR win32Cursor;

	BMessenger win32PrevMouseMovedWin;
	int win32PrevMouseMovedX;
	int win32PrevMouseMovedY;
	bigtime_t win32PrevMouseDownTime;
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
	EWin32GraphicsDrawable(EWin32GraphicsEngine *win32Engine,  __be_uint32_t w,  __be_uint32_t h);
	virtual ~EWin32GraphicsDrawable();

	virtual status_t		SetBackgroundColor(bhapi::rgb_color bkColor);

	virtual status_t		ResizeTo(__be_uint32_t w,  __be_uint32_t h);
	virtual status_t		CopyTo(BGraphicsContext *dc,
					       BGraphicsDrawable *dstDrawable,
					        __be_int32_t x,  __be_int32_t y,  __be_uint32_t w,  __be_uint32_t h,
					        __be_int32_t dstX,  __be_int32_t dstY,  __be_uint32_t dstW,  __be_uint32_t dstH);
	virtual status_t		DrawPixmap(BGraphicsContext *dc, const BPixmap *pix,
						    __be_int32_t x,  __be_int32_t y,  __be_uint32_t w,  __be_uint32_t h,
						    __be_int32_t dstX,  __be_int32_t dstY,  __be_uint32_t dstW,  __be_uint32_t dstH);

	virtual status_t		StrokePoint(BGraphicsContext *dc,
						     __be_int32_t x,  __be_int32_t y);
	virtual status_t		StrokePoints(BGraphicsContext *dc,
						     const  __be_int32_t *pts,  __be_int32_t count);
	virtual status_t		StrokePoints_Colors(BGraphicsContext *dc,
							    const BList *ptsArrayLists,  __be_int32_t arrayCount,
							    const bhapi::rgb_color *highColors);
	virtual status_t		StrokePoints_Alphas(BGraphicsContext *dc,
							    const  __be_int32_t *pts, const  __be_uint8 *alpha,  __be_int32_t count);
	virtual status_t		StrokeLine(BGraphicsContext *dc,
						    __be_int32_t x0,  __be_int32_t y0,  __be_int32_t x1,  __be_int32_t y1);
	virtual status_t		StrokePolygon(BGraphicsContext *dc,
						      const  __be_int32_t *pts,  __be_int32_t count, bool closed);
	virtual status_t		FillPolygon(BGraphicsContext *dc,
						    const  __be_int32_t *pts,  __be_int32_t count);
	virtual status_t		StrokeRect(BGraphicsContext *dc,
						    __be_int32_t x,  __be_int32_t y,  __be_uint32_t w,  __be_uint32_t h);
	virtual status_t		FillRect(BGraphicsContext *dc,
						  __be_int32_t x,  __be_int32_t y,  __be_uint32_t w,  __be_uint32_t h);
	virtual status_t		StrokeRects(BGraphicsContext *dc,
						    const  __be_int32_t *rects,  __be_int32_t count);
	virtual status_t		FillRects(BGraphicsContext *dc,
						  const  __be_int32_t *rects,  __be_int32_t count);
	virtual status_t		FillRegion(BGraphicsContext *dc,
						   const BRegion &region);
	virtual status_t		StrokeRoundRect(BGraphicsContext *dc,
							__be_int32_t x,  __be_int32_t y,  __be_uint32_t w,  __be_uint32_t h,  __be_uint32_t xRadius,  __be_uint32_t yRadius);
	virtual status_t		FillRoundRect(BGraphicsContext *dc,
						       __be_int32_t x,  __be_int32_t y,  __be_uint32_t w,  __be_uint32_t h,  __be_uint32_t xRadius,  __be_uint32_t yRadius);
	virtual status_t		StrokeArc(BGraphicsContext *dc,
						   __be_int32_t x,  __be_int32_t y,  __be_uint32_t w,  __be_uint32_t h, float startAngle, float endAngle);
	virtual status_t		FillArc(BGraphicsContext *dc,
						__be_int32_t x,  __be_int32_t y,  __be_uint32_t w,  __be_uint32_t h, float startAngle, float endAngle);

	HBITMAP win32Pixmap;
	HDC win32HDC;
	HPEN win32Pen;
	HBRUSH win32Brush;

private:
	HWND fRequestAsyncWin;
	UINT WMBHAPI_MESSAGE;
};


class EWin32GraphicsWindow : public BGraphicsWindow {
public:
	EWin32GraphicsWindow(EWin32GraphicsEngine *win32Engine,  __be_int32_t x,  __be_int32_t y,  __be_uint32_t w,  __be_uint32_t h);
	virtual ~EWin32GraphicsWindow();

	virtual status_t		ContactTo(const BMessenger *msgr);
	virtual status_t		SetBackgroundColor(bhapi::rgb_color bkColor);
	virtual status_t		SetFlags(__be_uint32_t flags);
	virtual status_t		SetLook(bhapi::window_look look);
	virtual status_t		SetFeel(bhapi::window_feel feel);
	virtual status_t		SetTitle(const char *title);
	virtual status_t		SetWorkspaces(__be_uint32_t workspaces);
	virtual status_t		GetWorkspaces(__be_uint32_t *workspaces);
	virtual status_t		Iconify();
	virtual status_t		Show();
	virtual status_t		Hide();
	virtual status_t		Raise();
	virtual status_t		Lower(BGraphicsWindow *frontWin);
	virtual status_t		Activate(bool state);
	virtual status_t		GetActivatedState(bool *state) const;
	virtual status_t		MoveTo(__be_int32_t x,  __be_int32_t y);
	virtual status_t		ResizeTo(__be_uint32_t w,  __be_uint32_t h);
	virtual status_t		MoveAndResizeTo(__be_int32_t x,  __be_int32_t y,  __be_uint32_t w,  __be_uint32_t h);
	virtual status_t		SetSizeLimits(__be_uint32_t min_w,  __be_uint32_t max_w,  __be_uint32_t min_h,  __be_uint32_t max_h);
	virtual status_t		GetSizeLimits(__be_uint32_t *min_w,  __be_uint32_t *max_w,  __be_uint32_t *min_h,  __be_uint32_t *max_h);
	virtual status_t		GrabMouse();
	virtual status_t		UngrabMouse();
	virtual status_t		GrabKeyboard();
	virtual status_t		UngrabKeyboard();
	virtual status_t		QueryMouse(__be_int32_t *x,  __be_int32_t *y,  __be_int32_t *buttons);

	virtual status_t		CopyTo(BGraphicsContext *dc,
					       BGraphicsDrawable *dstDrawable,
					        __be_int32_t x,  __be_int32_t y,  __be_uint32_t w,  __be_uint32_t h,
					        __be_int32_t dstX,  __be_int32_t dstY,  __be_uint32_t dstW,  __be_uint32_t dstH);
	virtual status_t		DrawPixmap(BGraphicsContext *dc, const BPixmap *pix,
						    __be_int32_t x,  __be_int32_t y,  __be_uint32_t w,  __be_uint32_t h,
						    __be_int32_t dstX,  __be_int32_t dstY,  __be_uint32_t dstW,  __be_uint32_t dstH);

	virtual status_t		StrokePoint(BGraphicsContext *dc,
						     __be_int32_t x,  __be_int32_t y);
	virtual status_t		StrokePoints(BGraphicsContext *dc,
						     const  __be_int32_t *pts,  __be_int32_t count);
	virtual status_t		StrokePoints_Colors(BGraphicsContext *dc,
							    const BList *ptsArrayLists,  __be_int32_t arrayCount,
							    const bhapi::rgb_color *highColors);
	virtual status_t		StrokePoints_Alphas(BGraphicsContext *dc,
							    const  __be_int32_t *pts, const  __be_uint8 *alpha,  __be_int32_t count);
	virtual status_t		StrokeLine(BGraphicsContext *dc,
						    __be_int32_t x0,  __be_int32_t y0,  __be_int32_t x1,  __be_int32_t y1);
	virtual status_t		StrokePolygon(BGraphicsContext *dc,
						      const  __be_int32_t *pts,  __be_int32_t count, bool closed);
	virtual status_t		FillPolygon(BGraphicsContext *dc,
						    const  __be_int32_t *pts,  __be_int32_t count);
	virtual status_t		StrokeRect(BGraphicsContext *dc,
						    __be_int32_t x,  __be_int32_t y,  __be_uint32_t w,  __be_uint32_t h);
	virtual status_t		FillRect(BGraphicsContext *dc,
						  __be_int32_t x,  __be_int32_t y,  __be_uint32_t w,  __be_uint32_t h);
	virtual status_t		StrokeRects(BGraphicsContext *dc,
						    const  __be_int32_t *rects,  __be_int32_t count);
	virtual status_t		FillRects(BGraphicsContext *dc,
						  const  __be_int32_t *rects,  __be_int32_t count);
	virtual status_t		FillRegion(BGraphicsContext *dc,
						   const BRegion &region);
	virtual status_t		StrokeRoundRect(BGraphicsContext *dc,
							__be_int32_t x,  __be_int32_t y,  __be_uint32_t w,  __be_uint32_t h,  __be_uint32_t xRadius,  __be_uint32_t yRadius);
	virtual status_t		FillRoundRect(BGraphicsContext *dc,
						       __be_int32_t x,  __be_int32_t y,  __be_uint32_t w,  __be_uint32_t h,  __be_uint32_t xRadius,  __be_uint32_t yRadius);

	virtual status_t		StrokeArc(BGraphicsContext *dc,
						   __be_int32_t x,  __be_int32_t y,  __be_uint32_t w,  __be_uint32_t h, float startAngle, float endAngle);
	virtual status_t		FillArc(BGraphicsContext *dc,
						__be_int32_t x,  __be_int32_t y,  __be_uint32_t w,  __be_uint32_t h, float startAngle, float endAngle);

	HWND win32Window;
	bhapi::window_look fLook;
	bhapi::window_feel fFeel;
	bool fActivateWhenShown;
	HBRUSH hbrBackground;

private:
	friend class EWin32GraphicsEngine;

	EWin32GraphicsEngine *fEngine;

	BMessenger fMsgr;

	HWND fRequestWin;
	HWND fRequestAsyncWin;
	UINT WMBHAPI_MESSAGE;
};


typedef struct bhapi::win32_gdi_callback_t {
	int command;

	EWin32GraphicsWindow *win;
	__be_int32_t x;
	__be_int32_t y;
	__be_uint32_t w;
	__be_uint32_t h;
	bhapi::window_look look;
	bhapi::window_feel feel;
	__be_uint32_t flags;
	bhapi::rgb_color bkColor;
	EWin32GraphicsWindow *frontWin;
	bool activate_state;
	bool grab_state;
	bool grab_mouse;
	__be_uint32_t min_w;
	__be_uint32_t min_h;
	__be_uint32_t max_w;
	__be_uint32_t max_h;

	EWin32GraphicsDrawable *pixmap;
	BGraphicsDrawable *dstDrawable;
	__be_int32_t wx;
	__be_int32_t wy;
	__be_uint32_t ww;
	__be_uint32_t wh;

	BGraphicsContext *dc;

	const  __be_int32_t *pts;
	const  __be_uint8 *ptsAlpha;
	__be_int32_t ptsCount;
	bool polyClosed;

	float startAngle;
	float endAngle;

	const BList *ptsArrayLists;
	__be_int32_t ptsArrayCount;
	const bhapi::rgb_color *ptsColors;

	const BRegion *region;

	const char* fontFamily;
	const char* fontString;
	bool fontAliasing;
	__be_uint32_t fontSpacing;
	HFONT *font;
	HDC *fontTmpDC;

	const void *data;
} bhapi::win32_gdi_callback_t;


#ifndef GET_WHEEL_DELTA_WPARAM
#define GET_WHEEL_DELTA_WPARAM(wparam)	((short)HIWORD(wparam))
#endif

#endif /* WIN32 */
#endif /* BHAPI_WIN32GDI_H */

