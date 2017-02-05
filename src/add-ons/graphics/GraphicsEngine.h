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
 * File: GraphicsEngine.h
 * Description: Graphics Engine Addon
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_GRAPHICS_ENGINE_H__
#define __ETK_GRAPHICS_ENGINE_H__

#include "./../../interface/Window.h"
#include "./../../interface/View.h"

#ifdef __cplusplus /* Just for C++ */


class ECursor;
class EPixmap;


class _IMPEXP_ETK EGraphicsContext {
public:
	EGraphicsContext();
	virtual ~EGraphicsContext();

	virtual e_status_t		SetDrawingMode(e_drawing_mode mode);
	virtual e_status_t		SetClipping(const ERegion &clipping);
	virtual e_status_t		SetHighColor(e_rgb_color highColor);
	virtual e_status_t		SetLowColor(e_rgb_color lowColor);
	virtual e_status_t		SetPattern(e_pattern pattern);
	virtual e_status_t		SetPenSize(euint32 penSize);
	virtual e_status_t		SetSquarePointStyle(bool state);

	e_drawing_mode			DrawingMode() const;
	const ERegion			*Clipping() const;
	e_pattern			Pattern() const;
	euint32				PenSize() const;
	bool				IsSquarePointStyle() const;

	e_status_t			SetHighColor(euint8 r, euint8 g, euint8 b, euint8 a);
	e_status_t			SetLowColor(euint8 r, euint8 g, euint8 b, euint8 a);
	e_rgb_color			HighColor() const;
	e_rgb_color			LowColor() const;

private:
	e_drawing_mode fDrawingMode;
	ERegion fClipping;
	e_rgb_color fHighColor;
	e_rgb_color fLowColor;
	e_pattern fPattern;
	euint32 fPenSize;
	bool fSquarePoint;
};


class _IMPEXP_ETK EGraphicsDrawable {
public:
	EGraphicsDrawable();
	virtual ~EGraphicsDrawable();

	virtual e_status_t		SetBackgroundColor(e_rgb_color bkColor);

	virtual e_status_t		ResizeTo(euint32 w, euint32 h) = 0;
	virtual e_status_t		CopyTo(EGraphicsContext *dc,
					       EGraphicsDrawable *dstDrawable,
					       eint32 x, eint32 y, euint32 w, euint32 h,
					       eint32 dstX, eint32 dstY, euint32 dstW, euint32 dstH) = 0;
	virtual e_status_t		DrawPixmap(EGraphicsContext *dc, const EPixmap *pix,
						   eint32 x, eint32 y, euint32 w, euint32 h,
						   eint32 dstX, eint32 dstY, euint32 dstW, euint32 dstH) = 0;

	virtual e_status_t		StrokePoint(EGraphicsContext *dc,
						    eint32 x, eint32 y) = 0;
	virtual e_status_t		StrokePoints(EGraphicsContext *dc,
						     const eint32 *pts, eint32 count) = 0;
	virtual e_status_t		StrokePoints_Colors(EGraphicsContext *dc,
							    const EList *ptsArrayLists, eint32 arrayCount,
							    const e_rgb_color *highColors) = 0;
	virtual e_status_t		StrokePoints_Alphas(EGraphicsContext *dc,
							    const eint32 *pts, const euint8 *alpha, eint32 count) = 0;
	virtual e_status_t		StrokeLine(EGraphicsContext *dc,
						   eint32 x0, eint32 y0, eint32 x1, eint32 y1) = 0;
	virtual e_status_t		StrokePolygon(EGraphicsContext *dc,
						      const eint32 *pts, eint32 count, bool closed) = 0;
	virtual e_status_t		FillPolygon(EGraphicsContext *dc,
						    const eint32 *pts, eint32 count) = 0;
	virtual e_status_t		StrokeRect(EGraphicsContext *dc,
						   eint32 x, eint32 y, euint32 w, euint32 h) = 0;
	virtual e_status_t		FillRect(EGraphicsContext *dc,
						 eint32 x, eint32 y, euint32 w, euint32 h) = 0;
	virtual e_status_t		StrokeRects(EGraphicsContext *dc,
						    const eint32 *rects, eint32 count) = 0;
	virtual e_status_t		FillRects(EGraphicsContext *dc,
						  const eint32 *rects, eint32 count) = 0;
	virtual e_status_t		FillRegion(EGraphicsContext *dc,
						   const ERegion &region) = 0;
	virtual e_status_t		StrokeRoundRect(EGraphicsContext *dc,
							eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius) = 0;
	virtual e_status_t		FillRoundRect(EGraphicsContext *dc,
						      eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius) = 0;
	virtual e_status_t		StrokeArc(EGraphicsContext *dc,
						  eint32 x, eint32 y, euint32 w, euint32 h, float startAngle, float endAngle) = 0;
	virtual e_status_t		FillArc(EGraphicsContext *dc,
						eint32 x, eint32 y, euint32 w, euint32 h, float startAngle, float endAngle) = 0;

	e_status_t			SetBackgroundColor(euint8 r, euint8 g, euint8 b, euint8 a);
	e_rgb_color			BackgroundColor() const;

private:
	e_rgb_color fBkColor;
};


class _IMPEXP_ETK EGraphicsWindow : public EGraphicsDrawable {
public:
	EGraphicsWindow();
	virtual ~EGraphicsWindow();

	virtual e_status_t		ContactTo(const EMessenger *msgr) = 0;
	virtual e_status_t		SetFlags(euint32 flags) = 0;
	virtual e_status_t		SetLook(e_window_look look) = 0;
	virtual e_status_t		SetFeel(e_window_feel feel) = 0;
	virtual e_status_t		SetTitle(const char *title) = 0;
	virtual e_status_t		SetWorkspaces(euint32 workspaces) = 0;
	virtual e_status_t		GetWorkspaces(euint32 *workspaces) = 0;
	virtual e_status_t		Iconify() = 0;
	virtual e_status_t		Show() = 0;
	virtual e_status_t		Hide() = 0;
	virtual e_status_t		Raise() = 0;
	virtual e_status_t		Lower(EGraphicsWindow* frontWin) = 0;
	virtual e_status_t		Activate(bool state) = 0;
	virtual e_status_t		GetActivatedState(bool *state) const = 0;
	virtual e_status_t		MoveTo(eint32 x, eint32 y) = 0;
	virtual e_status_t		MoveAndResizeTo(eint32 x, eint32 y, euint32 w, euint32 h) = 0;
	virtual e_status_t		SetSizeLimits(euint32 min_w, euint32 max_w, euint32 min_h, euint32 max_h) = 0;
	virtual e_status_t		GetSizeLimits(euint32 *min_w, euint32 *max_w, euint32 *min_h, euint32 *max_h) = 0;
	virtual e_status_t		GrabMouse() = 0;
	virtual e_status_t		UngrabMouse() = 0;
	virtual e_status_t		GrabKeyboard() = 0;
	virtual e_status_t		UngrabKeyboard() = 0;
	virtual e_status_t		QueryMouse(eint32 *x, eint32 *y, eint32 *buttons) = 0;
};


// NOTE:
// 1. Addon must have C function like below and all the null virtual functions of class must be implemented.
// 		extern "C" _EXPORT EGraphicsEngine* instantiate_graphics_engine();
// 2. Usually, addons were put into the directory located at "$E_ADDONS_DIRECTORY/etkxx/graphics" or
//    "$E_USER_ADDONS_DIRECTORY/etkxx/graphics", such as "/usr/lib/add-ons/etkxx/graphics".
// 3. When ETK++ find no graphics-engine addons, it try built-in graphics-engine when possible.
class _IMPEXP_ETK EGraphicsEngine {
public:
	EGraphicsEngine();
	virtual ~EGraphicsEngine();

	virtual e_status_t		Initalize() = 0;
	virtual void			Cancel() = 0;

	virtual EGraphicsContext*	CreateContext() = 0;
	virtual EGraphicsDrawable*	CreatePixmap(euint32 w, euint32 h) = 0;
	virtual EGraphicsWindow*	CreateWindow(eint32 x, eint32 y, euint32 w, euint32 h) = 0;

	virtual e_status_t		InitalizeFonts() = 0;
	virtual void			DestroyFonts() = 0;
	virtual e_status_t		UpdateFonts(bool check_only) = 0;

	virtual e_status_t		GetDesktopBounds(euint32 *w, euint32 *h) = 0;
	virtual e_status_t		GetCurrentWorkspace(euint32 *workspace) = 0;
	virtual e_status_t		SetCursor(const void *cursor_data) = 0;
	virtual e_status_t		GetDefaultCursor(ECursor *cursor) = 0;

	static EGraphicsWindow		*GetWindow(EWindow *win);
	static EGraphicsDrawable	*GetPixmap(EWindow *win);
	static EGraphicsContext		*GetContext(EView *view);
};


#endif /* __cplusplus */

#endif /* __ETK_GRAPHICS_ENGINE_H__ */

