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
 * File: Window.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __BHAPI_WINDOW_H__
#define __BHAPI_WINDOW_H__

#include "./../support/List.h"
#include "./GraphicsDefs.h"
#include "./Region.h"
#include "./../app/Looper.h"
#include "./../app/MessageRunner.h"

typedef enum b_window_type {
    B_UNTYPED_WINDOW = 0,
    B_TITLED_WINDOW,
    B_MODAL_WINDOW,
    B_DOCUMENT_WINDOW,
    B_BORDERED_WINDOW,
    B_FLOATING_WINDOW
} b_window_type;


typedef enum b_window_look {
    B_BORDERED_WINDOW_LOOK = 1,
    B_NO_BORDER_WINDOW_LOOK,
    B_TITLED_WINDOW_LOOK,
    B_DOCUMENT_WINDOW_LOOK,
    B_MODAL_WINDOW_LOOK,
    B_FLOATING_WINDOW_LOOK
} b_window_look;


typedef enum b_window_feel {
    B_NORMAL_WINDOW_FEEL = 1,
    B_MODAL_SUBSET_WINDOW_FEEL,
    B_MODAL_APP_WINDOW_FEEL,
    B_MODAL_ALL_WINDOW_FEEL,
    B_FLOATING_SUBSET_WINDOW_FEEL,
    B_FLOATING_APP_WINDOW_FEEL,
    B_FLOATING_ALL_WINDOW_FEEL
} b_window_feel;


enum {
    B_NOT_MOVABLE				= 1,
    B_NOT_CLOSABLE				= 1 << 1,
    B_NOT_ZOOMABLE				= 1 << 2,
    B_NOT_MINIMIZABLE			= 1 << 3,
    B_NOT_RESIZABLE				= 1 << 4,
    B_NOT_H_RESIZABLE			= 1 << 5,
    B_NOT_V_RESIZABLE			= 1 << 6,
    B_AVOID_FRONT				= 1 << 7,
    B_AVOID_FOCUS				= 1 << 8,
    B_WILL_ACCEPT_FIRST_CLICK		= 1 << 9,
    B_OUTLINE_RESIZE			= 1 << 10,
    B_NO_WORKSPACE_ACTIVATION		= 1 << 11,
    B_NOT_ANCHORED_ON_ACTIVATE		= 1 << 12,
    B_QUIT_ON_WINDOW_CLOSE			= 1 << 13
};

#define B_CURRENT_WORKSPACE	0
#define B_ALL_WORKSPACES	0xffffffff

#ifdef __cplusplus /* Just for C++ */

class BApplication;
class BView;
class BGraphicsContext;
class BGraphicsDrawable;
class BGraphicsWindow;
class BLayoutContainer;

class _IMPEXP_BHAPI BWindow : public BLooper {
public:
    BWindow(BRect frame,
		const char *title,
		b_window_type type,
        b_uint32 flags,
        b_uint32 workspace = B_CURRENT_WORKSPACE);
    BWindow(BRect frame,
		const char *title,
		b_window_look look,
		b_window_feel feel,
        b_uint32 flags,
        b_uint32 workspace = B_CURRENT_WORKSPACE);
    virtual ~BWindow();

	virtual void	DispatchMessage(BMessage *msg, BHandler *target);

	virtual void	Quit();

	virtual void	Show();
	virtual void	Hide();
	virtual void	Minimize(bool minimize);
	bool		IsHidden() const;
	bool		IsMinimized() const;

	void		Activate(bool state = true);
	bool		IsActivate() const;

    b_status_t	SendBehind(const BWindow *window);

	BRect		Bounds() const;
	BRect		Frame() const;

	void		AddChild(BView *child, BView *childNextSibling = NULL);
	bool		RemoveChild(BView *child);
    b_int32		CountChildren() const;
    BView		*ChildAt(b_int32 index) const;

	void		ConvertToScreen(BPoint* pt) const;
	BPoint		ConvertToScreen(BPoint pt) const;
	void		ConvertFromScreen(BPoint* pt) const;
	BPoint		ConvertFromScreen(BPoint pt) const;

	void		ConvertToScreen(BRect *r) const;
	BRect		ConvertToScreen(BRect r) const;
	void		ConvertFromScreen(BRect *r) const;
	BRect		ConvertFromScreen(BRect r) const;

	void		ConvertToScreen(BRegion *region) const;
	BRegion		ConvertToScreen(const BRegion &region) const;
	void		ConvertFromScreen(BRegion *region) const;
	BRegion		ConvertFromScreen(const BRegion &region) const;

	void		MoveBy(float dx, float dy);
	void		MoveTo(BPoint leftTop);
	void		MoveToCenter();
	void		ResizeBy(float dx, float dy);
	void		ResizeTo(float width, float height);

	// Empty functions BEGIN --- just for derivative class
	virtual void	WindowActivated(bool state);
	virtual void	FrameMoved(BPoint new_position);
	virtual void	FrameResized(float new_width, float new_height);
    virtual void	WorkspacesChanged(b_uint32 old_ws, b_uint32 new_ws);
    virtual void	WorkspaceActivated(b_int32 ws, bool state);
	// Empty functions END

	void		Invalidate(BRect invalRect, bool redraw = true);
	void		DisableUpdates();
	void		EnableUpdates();

	bool		NeedsUpdate() const;
	void		UpdateIfNeeded();
	BView		*FindView(const char *name) const;
	BView		*FindView(BPoint where) const;
	BView		*CurrentFocus() const;

    virtual void	SetBackgroundColor(b_rgb_color c);
    void		SetBackgroundColor(b_uint8 r, b_uint8 g, b_uint8 b, b_uint8 a = 255);
	b_rgb_color	BackgroundColor() const;

	void		SetTitle(const char *title);
	const char*	Title() const;

    b_status_t	SetType(b_window_type type);
	b_window_type	Type() const;

    b_status_t	SetLook(b_window_look look);
	b_window_look	Look() const;

    b_status_t	SetFeel(b_window_feel feel);
	b_window_feel	Feel() const;

    b_status_t	SetFlags(b_uint32 flags);
    b_uint32		Flags() const;

    void		SetWorkspaces(b_uint32 workspace);
    b_uint32		Workspaces() const;

	void		SetSizeLimits(float min_h, float max_h, float min_v, float max_v);
	void		GetSizeLimits(float *min_h, float *max_h, float *min_v, float *max_v) const;

    void		SetPulseRate(b_bigtime_t rate);
	b_bigtime_t	PulseRate() const;

protected:
	bool GrabMouse();
	bool IsMouseGrabbed() const;
	void UngrabMouse();
	bool GrabKeyboard();
	bool IsKeyboardGrabbed() const;
	void UngrabKeyboard();

private:
	friend class BApplication;
	friend class BView;
	friend class BGraphicsEngine;
	friend class BBitmap;

	BGraphicsWindow *fWindow;
	BGraphicsDrawable *fPixmap;
	BGraphicsContext *fDC;
    BLayoutContainer *fLayout;

	char *fWindowTitle;
	b_window_look fWindowLook;
	b_window_feel fWindowFeel;
    b_uint32 fWindowFlags;
    b_uint32 fWindowWorkspaces;

	BView *fFocus;
	BList fMouseInterestedViews;
	BList fKeyboardInterestedViews;
	BList fMouseInsideViews;

    static void AddViewChildrenToHandlersList(BWindow *win, BView *child);
    static void RemoveViewChildrenFromHandlersList(BWindow *win, BView *child);

    b_int64 fUpdateHolderThreadId;
    b_int64 fUpdateHolderCount;
	BRect fUpdateRect;
	BRect fExposeRect;
	bool fInUpdate;

	void _Update(BRect rect, bool force_update);
	void _Expose(BRect rect, b_bigtime_t when);
    void _UpdateIfNeeded(b_bigtime_t when);
	bool InUpdate() const;

	bool _HasResizeMessage(bool setBrokeOnExpose);

	bool fMinimized;
	bool fActivated;

	b_bigtime_t fActivatedTimeStamp;
	b_bigtime_t fPositionChangedTimeStamp;
	b_bigtime_t fSizeChangedTimeStamp;

    b_uint32 fMouseGrabCount;
    b_uint32 fKeyboardGrabCount;
	bool _GrabMouse();
	bool _GrabKeyboard();
	void _UngrabMouse();
	void _UngrabKeyboard();

	bool fBrokeOnExpose;

	b_bigtime_t fPulseRate;
	BMessageRunner *fPulseRunner;
	BList fNeededToPulseViews;

    void InitSelf(BRect, const char*, b_window_look, b_window_feel, b_uint32, b_uint32);
};

#endif /* __cplusplus */

#endif /* __BHAPI_WINDOW_H__ */

