/* --------------------------------------------------------------------------
 * 
 * DirectFB Graphics Add-on for ETK++
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
 * File: etk-dfb.h
 * 
 * --------------------------------------------------------------------------*/

#ifndef __ETK_DIRECTFB_H__
#define __ETK_DIRECTFB_H__

#include <directfb.h>
#include <directfb_version.h>

#if DIRECTFB_MAJOR_VERSION == 0 && DIRECTFB_MINOR_VERSION == 9 && DIRECTFB_MICRO_VERSION < 21
	#define DSFLIP_NONE		(DFBSurfaceFlipFlags)0
	#define DWCAPS_NODECORATION	0
#endif

#if DIRECTFB_MAJOR_VERSION == 0 && DIRECTFB_MINOR_VERSION == 9 && DIRECTFB_MICRO_VERSION >= 22
	#define DFB_HAVE_FILLRECTANGLES
#endif

#include "./../add-ons/graphics/GraphicsEngine.h>
#include "./../support/Locker.h>
#include "./../support/List.h>
#include "./../interface/Window.h>
#include "./../app/MessageFilter.h>

#ifdef __cplusplus /* just for C++ */


class EDFBGraphicsEngine : public EGraphicsEngine {
public:
	EDFBGraphicsEngine();
	virtual ~EDFBGraphicsEngine();

	e_status_t			InitCheck();

	bool				Lock();
	void				Unlock();

	bool				SetDFBWindowData(IDirectFBWindow *dfbWin, void *data, void **old_data = NULL);
	void				*GetDFBWindowData(IDirectFBWindow *dfbWin);
	void				*GetDFBWindowData(DFBWindowID dfbWinID);

	bool				ConvertRegion(const ERegion *region, DFBRegion **dfbRegions, int *nRegions);

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

	IDirectFB *dfbDisplay;
	IDirectFBDisplayLayer *dfbDisplayLayer;
	IDirectFBEventBuffer *dfbEventBuffer;

	int dfbDisplayWidth;
	int dfbDisplayHeight;

	DFBWindowID dfbCurFocusWin;
	DFBWindowID dfbCurPointerGrabbed;
	struct timeval dfbClipboardTimeStamp;
	IDirectFBSurface *dfbCursor;

	bool dfbDoQuit;

private:
	ELocker fLocker;
	void *fDFBThread;
	EMessageFilter *fClipboardFilter;

	struct etk_dfb_data {
		IDirectFBWindow *win;
		void *data;
	};
	EList fDFBDataList;
};


class EDFBGraphicsDrawable : public EGraphicsDrawable {
public:
	EDFBGraphicsDrawable(EDFBGraphicsEngine *dfbEngine, euint32 w, euint32 h);
	virtual ~EDFBGraphicsDrawable();

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

	IDirectFBSurface *dfbSurface;
	euint32 fWidth;
	euint32 fHeight;

private:
	EDFBGraphicsEngine *fEngine;
};


class EDFBGraphicsWindow : public EGraphicsWindow {
public:
	EDFBGraphicsWindow(EDFBGraphicsEngine *dfbEngine, eint32 x, eint32 y, euint32 w, euint32 h);
	virtual ~EDFBGraphicsWindow();

	e_status_t			GetContactor(EMessenger *msgr);
	void				AdjustFrameByDecoration();
	void				RenderDecoration();
	bool				HandleMouseEvent(DFBWindowEvent *event);

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

	IDirectFBWindow *dfbWindow;
	IDirectFBSurface *dfbSurface;
	ERect fMargins;
	DFBWindowID dfbWindowID;
	euint32 fFlags;
	eint32 fOriginX;
	eint32 fOriginY;
	euint32 fWidth;
	euint32 fHeight;
	bool fHidden;

private:
	friend class EDFBGraphicsEngine;

	EDFBGraphicsEngine *fEngine;

	EMessenger fMsgr;

	e_window_look fLook;
	e_window_feel fFeel;

	char *fTitle;

	bool fHandlingMove;
	bool fHandlingResize;

	int wmPointerOffsetX;
	int wmPointerOffsetY;

#if 0
	int minW;
	int minH;
	int maxW;
	int maxH;
#endif
};


#define DUET_EVENTPENDING	0
#define DUET_WINDOWREDRAWALL	E_MAXUINT


extern e_status_t etk_dfb_stroke_point(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
				       eint32 x, eint32 y, ERect *margins = NULL);
extern e_status_t etk_dfb_stroke_points(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
					const eint32 *pts, eint32 count, ERect *margins = NULL);
extern e_status_t etk_dfb_stroke_points_color(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
				              const EList *ptsArrayLists, eint32 arrayCount, const e_rgb_color *highColors,
					      ERect *margins = NULL);
extern e_status_t etk_dfb_stroke_line(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
				      eint32 x0, eint32 y0, eint32 x1, eint32 y1, ERect *margins = NULL);
extern e_status_t etk_dfb_stroke_rect(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
				      eint32 x, eint32 y, euint32 w, euint32 h, ERect *margins = NULL);
extern e_status_t etk_dfb_fill_rect(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
				    eint32 x, eint32 y, euint32 w, euint32 h, ERect *margins = NULL);
extern e_status_t etk_dfb_stroke_rects(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
				       const eint32 *rects, eint32 count, ERect *margins = NULL);
extern e_status_t etk_dfb_fill_rects(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
				     const eint32 *rects, eint32 count, ERect *margins = NULL);
extern e_status_t etk_dfb_fill_region(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
				      const ERegion &region, ERect *margins = NULL);
extern e_status_t etk_dfb_stroke_arc(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
				     eint32 x, eint32 y, euint32 w, euint32 h, float startAngle, float endAngle, ERect *margins = NULL);
extern e_status_t etk_dfb_fill_arc(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
				   eint32 x, eint32 y, euint32 w, euint32 h, float startAngle, float endAngle, ERect *margins = NULL);
extern e_status_t etk_dfb_draw_epixmap(IDirectFBSurface *dfbSurface, EGraphicsContext *dc, const EPixmap *pix,
				       eint32 x, eint32 y, euint32 w, euint32 h,
				       eint32 dstX, eint32 dstY, euint32 dstW, euint32 dstH, ERect *margins = NULL);

extern e_status_t etk_dfb_stroke_points_alphas(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
					       const eint32 *pts, const euint8 *alpha, eint32 count, ERect *margins = NULL);
extern e_status_t etk_dfb_stroke_polygon(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
					 const eint32 *pts, eint32 count, bool closed, ERect *margins = NULL);
extern e_status_t etk_dfb_fill_polygon(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
				       const eint32 *pts, eint32 count, ERect *margins = NULL);
extern e_status_t etk_dfb_stroke_round_rect(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
					    eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius,
					    ERect *margins = NULL);
extern e_status_t etk_dfb_fill_round_rect(IDirectFBSurface *dfbSurface, EGraphicsContext *dc,
					  eint32 x, eint32 y, euint32 w, euint32 h, euint32 xRadius, euint32 yRadius,
					  ERect *margins = NULL);


#endif /* __cplusplus */

#endif /* __ETK_DIRECTFB_H__ */

