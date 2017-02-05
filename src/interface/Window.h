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
 * File: Window.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_WINDOW_H__
#define __ETK_WINDOW_H__

#include "./../support/List.h"
#include "./GraphicsDefs.h"
#include "./Region.h"
#include "./../app/Looper.h"
#include "./../app/MessageRunner.h"

typedef enum e_window_type {
	E_UNTYPED_WINDOW = 0,
	E_TITLED_WINDOW,
	E_MODAL_WINDOW,
	E_DOCUMENT_WINDOW,
	E_BORDERED_WINDOW,
	E_FLOATING_WINDOW
} e_window_type;


typedef enum e_window_look {
	E_BORDERED_WINDOW_LOOK = 1,
	E_NO_BORDER_WINDOW_LOOK,
	E_TITLED_WINDOW_LOOK,
	E_DOCUMENT_WINDOW_LOOK,
	E_MODAL_WINDOW_LOOK,
	E_FLOATING_WINDOW_LOOK
} e_window_look;


typedef enum e_window_feel {
	E_NORMAL_WINDOW_FEEL = 1,
	E_MODAL_SUBSET_WINDOW_FEEL,
	E_MODAL_APP_WINDOW_FEEL,
	E_MODAL_ALL_WINDOW_FEEL,
	E_FLOATING_SUBSET_WINDOW_FEEL,
	E_FLOATING_APP_WINDOW_FEEL,
	E_FLOATING_ALL_WINDOW_FEEL
} e_window_feel;


enum {
	E_NOT_MOVABLE				= 1,
	E_NOT_CLOSABLE				= 1 << 1,
	E_NOT_ZOOMABLE				= 1 << 2,
	E_NOT_MINIMIZABLE			= 1 << 3,
	E_NOT_RESIZABLE				= 1 << 4,
	E_NOT_H_RESIZABLE			= 1 << 5,
	E_NOT_V_RESIZABLE			= 1 << 6,
	E_AVOID_FRONT				= 1 << 7,
	E_AVOID_FOCUS				= 1 << 8,
	E_WILL_ACCEPT_FIRST_CLICK		= 1 << 9,
	E_OUTLINE_RESIZE			= 1 << 10,
	E_NO_WORKSPACE_ACTIVATION		= 1 << 11,
	E_NOT_ANCHORED_ON_ACTIVATE		= 1 << 12,
	E_QUIT_ON_WINDOW_CLOSE			= 1 << 13
};

#define E_CURRENT_WORKSPACE	0
#define E_ALL_WORKSPACES	0xffffffff

#ifdef __cplusplus /* Just for C++ */

class EApplication;
class EView;
class EGraphicsContext;
class EGraphicsDrawable;
class EGraphicsWindow;
class ELayoutContainer;

class _IMPEXP_ETK EWindow : public ELooper {
public:
	EWindow(ERect frame,
		const char *title,
		e_window_type type,
		euint32 flags,
		euint32 workspace = E_CURRENT_WORKSPACE);
	EWindow(ERect frame,
		const char *title,
		e_window_look look,
		e_window_feel feel,
		euint32 flags,
		euint32 workspace = E_CURRENT_WORKSPACE);
	virtual ~EWindow();

	virtual void	DispatchMessage(EMessage *msg, EHandler *target);

	virtual void	Quit();

	virtual void	Show();
	virtual void	Hide();
	virtual void	Minimize(bool minimize);
	bool		IsHidden() const;
	bool		IsMinimized() const;

	void		Activate(bool state = true);
	bool		IsActivate() const;

	e_status_t	SendBehind(const EWindow *window);

	ERect		Bounds() const;
	ERect		Frame() const;

	void		AddChild(EView *child, EView *childNextSibling = NULL);
	bool		RemoveChild(EView *child);
	eint32		CountChildren() const;
	EView		*ChildAt(eint32 index) const;

	void		ConvertToScreen(EPoint* pt) const;
	EPoint		ConvertToScreen(EPoint pt) const;
	void		ConvertFromScreen(EPoint* pt) const;
	EPoint		ConvertFromScreen(EPoint pt) const;

	void		ConvertToScreen(ERect *r) const;
	ERect		ConvertToScreen(ERect r) const;
	void		ConvertFromScreen(ERect *r) const;
	ERect		ConvertFromScreen(ERect r) const;

	void		ConvertToScreen(ERegion *region) const;
	ERegion		ConvertToScreen(const ERegion &region) const;
	void		ConvertFromScreen(ERegion *region) const;
	ERegion		ConvertFromScreen(const ERegion &region) const;

	void		MoveBy(float dx, float dy);
	void		MoveTo(EPoint leftTop);
	void		MoveToCenter();
	void		ResizeBy(float dx, float dy);
	void		ResizeTo(float width, float height);

	// Empty functions BEGIN --- just for derivative class
	virtual void	WindowActivated(bool state);
	virtual void	FrameMoved(EPoint new_position);
	virtual void	FrameResized(float new_width, float new_height);
	virtual void	WorkspacesChanged(euint32 old_ws, euint32 new_ws);
	virtual void	WorkspaceActivated(eint32 ws, bool state);
	// Empty functions END

	void		Invalidate(ERect invalRect, bool redraw = true);
	void		DisableUpdates();
	void		EnableUpdates();

	bool		NeedsUpdate() const;
	void		UpdateIfNeeded();
	EView		*FindView(const char *name) const;
	EView		*FindView(EPoint where) const;
	EView		*CurrentFocus() const;

	virtual void	SetBackgroundColor(e_rgb_color c);
	void		SetBackgroundColor(euint8 r, euint8 g, euint8 b, euint8 a = 255);
	e_rgb_color	BackgroundColor() const;

	void		SetTitle(const char *title);
	const char*	Title() const;

	e_status_t	SetType(e_window_type type);
	e_window_type	Type() const;

	e_status_t	SetLook(e_window_look look);
	e_window_look	Look() const;

	e_status_t	SetFeel(e_window_feel feel);
	e_window_feel	Feel() const;

	e_status_t	SetFlags(euint32 flags);
	euint32		Flags() const;

	void		SetWorkspaces(euint32 workspace);
	euint32		Workspaces() const;

	void		SetSizeLimits(float min_h, float max_h, float min_v, float max_v);
	void		GetSizeLimits(float *min_h, float *max_h, float *min_v, float *max_v) const;

	void		SetPulseRate(e_bigtime_t rate);
	e_bigtime_t	PulseRate() const;

protected:
	bool GrabMouse();
	bool IsMouseGrabbed() const;
	void UngrabMouse();
	bool GrabKeyboard();
	bool IsKeyboardGrabbed() const;
	void UngrabKeyboard();

private:
	friend class EApplication;
	friend class EView;
	friend class EGraphicsEngine;
	friend class EBitmap;

	EGraphicsWindow *fWindow;
	EGraphicsDrawable *fPixmap;
	EGraphicsContext *fDC;
	ELayoutContainer *fLayout;

	char *fWindowTitle;
	e_window_look fWindowLook;
	e_window_feel fWindowFeel;
	euint32 fWindowFlags;
	euint32 fWindowWorkspaces;

	EView *fFocus;
	EList fMouseInterestedViews;
	EList fKeyboardInterestedViews;
	EList fMouseInsideViews;

	static void AddViewChildrenToHandlersList(EWindow *win, EView *child);
	static void RemoveViewChildrenFromHandlersList(EWindow *win, EView *child);

	eint64 fUpdateHolderThreadId;
	eint64 fUpdateHolderCount;
	ERect fUpdateRect;
	ERect fExposeRect;
	bool fInUpdate;

	void _Update(ERect rect, bool force_update);
	void _Expose(ERect rect, e_bigtime_t when);
	void _UpdateIfNeeded(e_bigtime_t when);
	bool InUpdate() const;

	bool _HasResizeMessage(bool setBrokeOnExpose);

	bool fMinimized;
	bool fActivated;

	e_bigtime_t fActivatedTimeStamp;
	e_bigtime_t fPositionChangedTimeStamp;
	e_bigtime_t fSizeChangedTimeStamp;

	euint32 fMouseGrabCount;
	euint32 fKeyboardGrabCount;
	bool _GrabMouse();
	bool _GrabKeyboard();
	void _UngrabMouse();
	void _UngrabKeyboard();

	bool fBrokeOnExpose;

	e_bigtime_t fPulseRate;
	EMessageRunner *fPulseRunner;
	EList fNeededToPulseViews;

	void InitSelf(ERect, const char*, e_window_look, e_window_feel, euint32, euint32);
};

#endif /* __cplusplus */

#endif /* __ETK_WINDOW_H__ */

