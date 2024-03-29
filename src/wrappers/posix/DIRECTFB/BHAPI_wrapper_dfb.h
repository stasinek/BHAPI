/* --------------------------------------------------------------------------
 *
 * DirectFB Graphics Add-on for BHAPI++
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
 * File: BHAPI_wrapper_dfb.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_DIRECTFB_H
#define BHAPI_DIRECTFB_H

#ifdef LINUX
#ifdef DIRECTFB

#include <directfb.h>
#include <directfb_version.h>

#if DIRECTFB_MAJOR_VERSION == 0 && DIRECTFB_MINOR_VERSION == 9 && DIRECTFB_MICRO_VERSION < 21
	#define DSFLIP_NONE		(DFBSurfaceFlipFlags)0
	#define DWCAPS_NODECORATION	0
#endif

#if DIRECTFB_MAJOR_VERSION == 0 && DIRECTFB_MINOR_VERSION == 9 && DIRECTFB_MICRO_VERSION >= 22
	#define DFB_HAVE_FILLRECTANGLES
#endif

#include <kits/add-ons/graphics/GraphicsEngine.h>
#include <kits/support/Locker.h>
#include <kits/support/List.h>
#include <kits/interface/Window.h>
#include <kits/app/MessageFilter.h>

#ifdef __cplusplus /* just for C++ */


class EDFBGraphicsEngine : public BGraphicsEngine {
public:
	EDFBGraphicsEngine();
	virtual ~EDFBGraphicsEngine();

	status_t			InitCheck();

	bool				Lock();
	void				Unlock();

	bool				SetDFBWindowData(IDirectFBWindow *dfbWin, void *data, void **old_data = NULL);
	void				*GetDFBWindowData(IDirectFBWindow *dfbWin);
	void				*GetDFBWindowData(DFBWindowID dfbWinID);

	bool				ConvertRegion(const BRegion *region, DFBRegion **dfbRegions, int *nRegions);

	virtual status_t		Initalize();
	virtual void			Cancel();

	virtual BGraphicsContext*	CreateContext();
	virtual BGraphicsDrawable*	CreatePixmap(uint32_t w,  uint32_t h);
	virtual BGraphicsWindow*	CreateWindow(int32_t x,  int32_t y,  uint32_t w,  uint32_t h);

	virtual status_t		InitalizeFonts();
	virtual void			DestroyFonts();
	virtual status_t		UpdateFonts(bool check_only);

	virtual status_t		GetDesktopBounds(uint32_t *w,  uint32_t *h);
	virtual status_t		GetCurrentWorkspace(uint32_t *workspace);
	virtual status_t		SetCursor(const void *cursor_data);
	virtual status_t		GetDefaultCursor(BCursor *cursor);

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
	BLocker fLocker;
	void *fDFBThread;
	BMessageFilter *fClipboardFilter;

	struct bhapi::dfb_data {
		IDirectFBWindow *win;
		void *data;
	};
	BList fDFBDataList;
};


class EDFBGraphicsDrawable : public BGraphicsDrawable {
public:
	EDFBGraphicsDrawable(EDFBGraphicsEngine *dfbEngine,  uint32_t w,  uint32_t h);
	virtual ~EDFBGraphicsDrawable();

	virtual status_t		SetBackgroundColor(bhapi::rgb_color bkColor);

	virtual status_t		ResizeTo(uint32_t w,  uint32_t h);
	virtual status_t		CopyTo(BGraphicsContext *dc,
					       BGraphicsDrawable *dstDrawable,
					        int32_t x,  int32_t y,  uint32_t w,  uint32_t h,
					        int32_t dstX,  int32_t dstY,  uint32_t dstW,  uint32_t dstH);
	virtual status_t		DrawPixmap(BGraphicsContext *dc, const BPixmap *pix,
						    int32_t x,  int32_t y,  uint32_t w,  uint32_t h,
						    int32_t dstX,  int32_t dstY,  uint32_t dstW,  uint32_t dstH);

	virtual status_t		StrokePoint(BGraphicsContext *dc,
						     int32_t x,  int32_t y);
	virtual status_t		StrokePoints(BGraphicsContext *dc,
						     const  int32_t *pts,  int32_t count);
	virtual status_t		StrokePoints_Colors(BGraphicsContext *dc,
							    const BList *ptsArrayLists,  int32_t arrayCount,
							    const bhapi::rgb_color *highColors);
	virtual status_t		StrokePoints_Alphas(BGraphicsContext *dc,
							    const  int32_t *pts, const  uint8 *alpha,  int32_t count);
	virtual status_t		StrokeLine(BGraphicsContext *dc,
						    int32_t x0,  int32_t y0,  int32_t x1,  int32_t y1);
	virtual status_t		StrokePolygon(BGraphicsContext *dc,
						      const  int32_t *pts,  int32_t count, bool closed);
	virtual status_t		FillPolygon(BGraphicsContext *dc,
						    const  int32_t *pts,  int32_t count);
	virtual status_t		StrokeRect(BGraphicsContext *dc,
						    int32_t x,  int32_t y,  uint32_t w,  uint32_t h);
	virtual status_t		FillRect(BGraphicsContext *dc,
						  int32_t x,  int32_t y,  uint32_t w,  uint32_t h);
	virtual status_t		StrokeRects(BGraphicsContext *dc,
						    const  int32_t *rects,  int32_t count);
	virtual status_t		FillRects(BGraphicsContext *dc,
						  const  int32_t *rects,  int32_t count);
	virtual status_t		FillRegion(BGraphicsContext *dc,
						   const BRegion &region);
	virtual status_t		StrokeRoundRect(BGraphicsContext *dc,
							int32_t x,  int32_t y,  uint32_t w,  uint32_t h,  uint32_t xRadius,  uint32_t yRadius);
	virtual status_t		FillRoundRect(BGraphicsContext *dc,
						       int32_t x,  int32_t y,  uint32_t w,  uint32_t h,  uint32_t xRadius,  uint32_t yRadius);
	virtual status_t		StrokeArc(BGraphicsContext *dc,
						   int32_t x,  int32_t y,  uint32_t w,  uint32_t h, float startAngle, float endAngle);
	virtual status_t		FillArc(BGraphicsContext *dc,
						int32_t x,  int32_t y,  uint32_t w,  uint32_t h, float startAngle, float endAngle);

	IDirectFBSurface *dfbSurface;
	uint32_t fWidth;
	uint32_t fHeight;

private:
	EDFBGraphicsEngine *fEngine;
};


class EDFBGraphicsWindow : public BGraphicsWindow {
public:
	EDFBGraphicsWindow(EDFBGraphicsEngine *dfbEngine,  int32_t x,  int32_t y,  uint32_t w,  uint32_t h);
	virtual ~EDFBGraphicsWindow();

	status_t			GetContactor(BMessenger *msgr);
	void				AdjustFrameByDecoration();
	void				RenderDecoration();
	bool				HandleMouseEvent(DFBWindowEvent *event);

	virtual status_t		ContactTo(const BMessenger *msgr);
	virtual status_t		SetBackgroundColor(bhapi::rgb_color bkColor);
	virtual status_t		SetFlags(uint32_t flags);
	virtual status_t		SetLook(bhapi::window_look look);
	virtual status_t		SetFeel(bhapi::window_feel feel);
	virtual status_t		SetTitle(const char *title);
	virtual status_t		SetWorkspaces(uint32_t workspaces);
	virtual status_t		GetWorkspaces(uint32_t *workspaces);
	virtual status_t		Iconify();
	virtual status_t		Show();
	virtual status_t		Hide();
	virtual status_t		Raise();
	virtual status_t		Lower(BGraphicsWindow *frontWin);
	virtual status_t		Activate(bool state);
	virtual status_t		GetActivatedState(bool *state) const;
	virtual status_t		MoveTo(int32_t x,  int32_t y);
	virtual status_t		ResizeTo(uint32_t w,  uint32_t h);
	virtual status_t		MoveAndResizeTo(int32_t x,  int32_t y,  uint32_t w,  uint32_t h);
	virtual status_t		SetSizeLimits(uint32_t min_w,  uint32_t max_w,  uint32_t min_h,  uint32_t max_h);
	virtual status_t		GetSizeLimits(uint32_t *min_w,  uint32_t *max_w,  uint32_t *min_h,  uint32_t *max_h);
	virtual status_t		GrabMouse();
	virtual status_t		UngrabMouse();
	virtual status_t		GrabKeyboard();
	virtual status_t		UngrabKeyboard();
	virtual status_t		QueryMouse(int32_t *x,  int32_t *y,  int32_t *buttons);

	virtual status_t		CopyTo(BGraphicsContext *dc,
					       BGraphicsDrawable *dstDrawable,
					        int32_t x,  int32_t y,  uint32_t w,  uint32_t h,
					        int32_t dstX,  int32_t dstY,  uint32_t dstW,  uint32_t dstH);
	virtual status_t		DrawPixmap(BGraphicsContext *dc, const BPixmap *pix,
						    int32_t x,  int32_t y,  uint32_t w,  uint32_t h,
						    int32_t dstX,  int32_t dstY,  uint32_t dstW,  uint32_t dstH);

	virtual status_t		StrokePoint(BGraphicsContext *dc,
						     int32_t x,  int32_t y);
	virtual status_t		StrokePoints(BGraphicsContext *dc,
						     const  int32_t *pts,  int32_t count);
	virtual status_t		StrokePoints_Colors(BGraphicsContext *dc,
							    const BList *ptsArrayLists,  int32_t arrayCount,
							    const bhapi::rgb_color *highColors);
	virtual status_t		StrokePoints_Alphas(BGraphicsContext *dc,
							    const  int32_t *pts, const  uint8 *alpha,  int32_t count);
	virtual status_t		StrokeLine(BGraphicsContext *dc,
						    int32_t x0,  int32_t y0,  int32_t x1,  int32_t y1);
	virtual status_t		StrokePolygon(BGraphicsContext *dc,
						      const  int32_t *pts,  int32_t count, bool closed);
	virtual status_t		FillPolygon(BGraphicsContext *dc,
						    const  int32_t *pts,  int32_t count);
	virtual status_t		StrokeRect(BGraphicsContext *dc,
						    int32_t x,  int32_t y,  uint32_t w,  uint32_t h);
	virtual status_t		FillRect(BGraphicsContext *dc,
						  int32_t x,  int32_t y,  uint32_t w,  uint32_t h);
	virtual status_t		StrokeRects(BGraphicsContext *dc,
						    const  int32_t *rects,  int32_t count);
	virtual status_t		FillRects(BGraphicsContext *dc,
						  const  int32_t *rects,  int32_t count);
	virtual status_t		FillRegion(BGraphicsContext *dc,
						   const BRegion &region);
	virtual status_t		StrokeRoundRect(BGraphicsContext *dc,
							int32_t x,  int32_t y,  uint32_t w,  uint32_t h,  uint32_t xRadius,  uint32_t yRadius);
	virtual status_t		FillRoundRect(BGraphicsContext *dc,
						       int32_t x,  int32_t y,  uint32_t w,  uint32_t h,  uint32_t xRadius,  uint32_t yRadius);

	virtual status_t		StrokeArc(BGraphicsContext *dc,
						   int32_t x,  int32_t y,  uint32_t w,  uint32_t h, float startAngle, float endAngle);
	virtual status_t		FillArc(BGraphicsContext *dc,
						int32_t x,  int32_t y,  uint32_t w,  uint32_t h, float startAngle, float endAngle);

	IDirectFBWindow *dfbWindow;
	IDirectFBSurface *dfbSurface;
	BRect fMargins;
	DFBWindowID dfbWindowID;
	uint32_t fFlags;
	int32_t fOriginX;
	int32_t fOriginY;
	uint32_t fWidth;
	uint32_t fHeight;
	bool fHidden;

private:
	friend class EDFBGraphicsEngine;

	EDFBGraphicsEngine *fEngine;

	BMessenger fMsgr;

	bhapi::window_look fLook;
	bhapi::window_feel fFeel;

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
#define DUET_WINDOWREDRAWALL    B_MAXUINT


extern status_t bhapi::dfbhapi::stroke_point(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
				        int32_t x,  int32_t y, BRect *margins = NULL);
extern status_t bhapi::dfbhapi::stroke_points(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
					const  int32_t *pts,  int32_t count, BRect *margins = NULL);
extern status_t bhapi::dfbhapi::stroke_points_color(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
				              const BList *ptsArrayLists,  int32_t arrayCount, const bhapi::rgb_color *highColors,
					      BRect *margins = NULL);
extern status_t bhapi::dfbhapi::stroke_line(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
				       int32_t x0,  int32_t y0,  int32_t x1,  int32_t y1, BRect *margins = NULL);
extern status_t bhapi::dfbhapi::stroke_rect(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
				       int32_t x,  int32_t y,  uint32_t w,  uint32_t h, BRect *margins = NULL);
extern status_t bhapi::dfb_fill_rect(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
				     int32_t x,  int32_t y,  uint32_t w,  uint32_t h, BRect *margins = NULL);
extern status_t bhapi::dfbhapi::stroke_rects(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
				       const  int32_t *rects,  int32_t count, BRect *margins = NULL);
extern status_t bhapi::dfb_fill_rects(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
				     const  int32_t *rects,  int32_t count, BRect *margins = NULL);
extern status_t bhapi::dfb_fill_region(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
				      const BRegion &region, BRect *margins = NULL);
extern status_t bhapi::dfbhapi::stroke_arc(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
				      int32_t x,  int32_t y,  uint32_t w,  uint32_t h, float startAngle, float endAngle, BRect *margins = NULL);
extern status_t bhapi::dfb_fill_arc(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
				    int32_t x,  int32_t y,  uint32_t w,  uint32_t h, float startAngle, float endAngle, BRect *margins = NULL);
extern status_t bhapi::dfb_draw_epixmap(IDirectFBSurface *dfbSurface, BGraphicsContext *dc, const BPixmap *pix,
				        int32_t x,  int32_t y,  uint32_t w,  uint32_t h,
				        int32_t dstX,  int32_t dstY,  uint32_t dstW,  uint32_t dstH, BRect *margins = NULL);

extern status_t bhapi::dfbhapi::stroke_points_alphas(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
					       const  int32_t *pts, const  uint8 *alpha,  int32_t count, BRect *margins = NULL);
extern status_t bhapi::dfbhapi::stroke_polygon(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
					 const  int32_t *pts,  int32_t count, bool closed, BRect *margins = NULL);
extern status_t bhapi::dfb_fill_polygon(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
				       const  int32_t *pts,  int32_t count, BRect *margins = NULL);
extern status_t bhapi::dfbhapi::stroke_round_rect(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
					     int32_t x,  int32_t y,  uint32_t w,  uint32_t h,  uint32_t xRadius,  uint32_t yRadius,
					    BRect *margins = NULL);
extern status_t bhapi::dfb_fill_round_rect(IDirectFBSurface *dfbSurface, BGraphicsContext *dc,
					   int32_t x,  int32_t y,  uint32_t w,  uint32_t h,  uint32_t xRadius,  uint32_t yRadius,
					  BRect *margins = NULL);


#endif /* __cplusplus */

#endif /* DIRECTFB */
#endif /* LINUX */

#endif /* BHAPI_DIRECTFB_H */

