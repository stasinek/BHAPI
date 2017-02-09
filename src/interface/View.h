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
 * File: View.h
 * Description: BView --- drawing/layout/control within BWindow
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_VIEW__H
#define BHAPI_VIEW__H

#include "./../support/SupportDefs.h"

enum {
    B_ENTERED_VIEW = 0,
    B_INSIDE_VIEW,
    B_EXITED_VIEW,
    B_OUTSIDE_VIEW
};

enum {
    B_POINTER_EVENTS = 1,
    B_KEYBOARD_EVENTS = 1 << 1
};

enum {
    B_LOCK_WINDOW_FOCUS	= 1,
    B_SUSPEND_VIEW_FOCUS	= 1 << 1,
    B_NO_POINTER_HISTORY	= 1 << 2
};

enum {
    B_WILL_DRAW			= 1,
    B_PULSE_NEEDED			= 1 << 1,
    B_NAVIGABLE_JUMP		= 1 << 2,
    B_NAVIGABLE			= 1 << 3,
    B_FRAME_EVENTS			= 1 << 4,
    B_UPDATE_WITH_REGION		= 1 << 5,
    B_DRAW_ON_CHILDREN		= 1 << 6,
    B_INPUT_METHOD_AWARE		= 1 << 7
};

enum {
    B_FONT_FAMILY_AND_STYLE		= 1,
    B_FONT_SIZE			= 1 << 1,
    B_FONT_SHEAR			= 1 << 2,
    B_FONT_SPACING     		= 1 << 3,
    B_FONT_ALL			= 0xff
};

#ifdef __cplusplus /* Just for C++ */

#include "./../interface/InterfaceDefs.h"
class BWindow;
class BGraphicsContext;
class BScrollView;
class BBitmap;
class BCursor;
class BLayoutItem;
class BMessage;
class BView;
#include "./../interface/Region.h"
#include "./../interface/Polygon.h"
#include "./../interface/Font.h"
#include "./../interface/Point.h"
#include "./../support/List.h"
#include "./../app/Handler.h"
class IMPEXP_BHAPI BView : public BHandler {
public:
	BView(BRect frame,
	      const char *name,
          b_uint32 resizingMode,
          b_uint32 flags);
	virtual ~BView();

	virtual void	MessageReceived(BMessage *msg);

	void		AddChild(BView *child, BView *childNextSibling = NULL);
	bool		RemoveChild(BView *child);
	bool		RemoveSelf();
    b_int32		CountChildren() const;
    BView		*ChildAt(b_int32 index) const;

	BView		*NextSibling() const;
	BView		*PreviousSibling() const;
	bool		IsSibling(const BView *sibling) const;

    BWindow		*Window() const;
	BView		*Parent() const;
	BView		*Ancestor() const;
	BView		*FindView(const char *name) const;

	BRect		Bounds() const;
	BRect		Frame() const;
	BPoint		LeftTop() const;

	bool		IsVisible() const;
	BRect		VisibleBounds() const;
	BRect		VisibleFrame() const;
	BRegion		VisibleBoundsRegion() const;
	BRegion		VisibleFrameRegion() const;

	// Empty functions BEGIN --- just for derivative class
	virtual void	AttachedToWindow();
	virtual void	AllAttached();
	virtual void	DetachedFromWindow();
	virtual void	AllDetached();
	virtual void	Draw(BRect updateRect);
	virtual void	DrawAfterChildren(BRect updateRect);
	virtual void	MouseDown(BPoint where);
	virtual void	MouseUp(BPoint where);
    virtual void	MouseMoved(BPoint where, b_uint32 code, const BMessage *a_message);
	virtual void	WindowActivated(bool state);
    virtual void	KeyDown(const char *bytes, b_int32 numBytes);
    virtual void	KeyUp(const char *bytes, b_int32 numBytes);
	virtual void	Pulse();
	virtual void	FrameMoved(BPoint new_position);
	virtual void	FrameResized(float new_width, float new_height);
	// Empty functions END

	virtual void	Show();
	virtual void	Hide();
	bool		IsHidden() const;

	virtual void	SetEnabled(bool state);
	bool		IsEnabled() const;

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

	void		ConvertToParent(BPoint *pt) const;
	BPoint		ConvertToParent(BPoint pt) const;
	void		ConvertFromParent(BPoint *pt) const;
	BPoint		ConvertFromParent(BPoint pt) const;

	void		ConvertToParent(BRect *r) const;
	BRect		ConvertToParent(BRect r) const;
	void		ConvertFromParent(BRect *r) const;
	BRect		ConvertFromParent(BRect r) const;

	void		ConvertToParent(BRegion *region) const;
	BRegion		ConvertToParent(const BRegion &region) const;
	void		ConvertFromParent(BRegion *region) const;
	BRegion		ConvertFromParent(const BRegion &region) const;

	void		ConvertToWindow(BPoint *pt) const;
	BPoint		ConvertToWindow(BPoint pt) const;
	void		ConvertFromWindow(BPoint *pt) const;
	BPoint		ConvertFromWindow(BPoint pt) const;

	void		ConvertToWindow(BRect *r) const;
	BRect		ConvertToWindow(BRect r) const;
	void		ConvertFromWindow(BRect *r) const;
	BRect		ConvertFromWindow(BRect r) const;

	void		ConvertToWindow(BRegion *region) const;
	BRegion		ConvertToWindow(const BRegion &region) const;
	void		ConvertFromWindow(BRegion *region) const;
	BRegion		ConvertFromWindow(const BRegion &region) const;

    b_status_t	SetEventMask(b_uint32 mask, b_uint32 options = 0);
    b_uint32		EventMask() const;
    b_status_t	GetMouse(BPoint *location, b_int32 *buttons, bool checkMessageQueue = true);
    bool		QueryCurrentMouse(bool pushed, b_int32 buttons, bool btnsAlone = true, b_int32 *clicks = NULL) const;

    // Next KeyUp(B_KEYBOARD_EVENTS) or MouseUp(B_POINTER_EVENTS) will restore the previous general event_mask.
    // If the current message isn't B_KEY_DOWN(B_UNMAPPED_KEY_DOWN) or B_MOUSE_DOWN, B_ERROR is return.
	// That's means: you should use this funtion within "KeyDown" or "MouseDown" etc...
	// The argument "mask" should not be "0" or any union.
	// That's means: you should not pass "mask" with "E_KEYBOARD_EVENTS | B_POINTER_EVENTS".
    b_status_t	SetPrivateEventMask(b_uint32 mask, b_uint32 options = 0);

    virtual void	SetFlags(b_uint32 flags);
    b_uint32		Flags() const;
    virtual void	SetResizingMode(b_uint32 mode);
    b_uint32		ResizingMode() const;
	void		MoveBy(float dh, float dv);
	void		MoveTo(BPoint where);
	void		MoveTo(float x, float y);
	void		ResizeBy(float dh, float dv);
	void		ResizeTo(float width, float height);
	void		ScrollBy(float dh, float dv);
	void		ScrollTo(float x, float y);
	virtual void	ScrollTo(BPoint where);
	virtual void	MakeFocus(bool focusState = true);
	bool		IsFocus() const;

    virtual void	SetDrawingMode(b_drawing_mode mode);
	b_drawing_mode	DrawingMode() const;

	void		MovePenTo(BPoint pt);
	void		MovePenTo(float x, float y);
	void		MovePenBy(float dx, float dy);
	BPoint		PenLocation() const;

	virtual void	SetPenSize(float size);
	float		PenSize() const;

    virtual void	SetViewColor(b_rgb_color c);
    void		SetViewColor(b_uint8 r, b_uint8 g, b_uint8 b, b_uint8 a = 255);
	b_rgb_color	ViewColor() const;

    virtual void	SetHighColor(b_rgb_color c);
    void		SetHighColor(b_uint8 r, b_uint8 g, b_uint8 b, b_uint8 a = 255);
	b_rgb_color	HighColor() const;

    virtual void	SetLowColor(b_rgb_color c);
    void		SetLowColor(b_uint8 r, b_uint8 g, b_uint8 b, b_uint8 a = 255);
	b_rgb_color	LowColor() const;

	void		PushState();
	void		PopState();

	void		Invalidate(BRect invalRect, bool redraw = true);
	void		Invalidate(bool redraw = true);

	// Note: The "Fill*()" functions isn't affected by the "PenSize()", it won't draw out of the edge.
	void		SetSquarePointStyle(bool state);
	bool		IsSquarePointStyle() const;
	void		StrokePoint(BPoint pt, b_pattern p = B_SOLID_HIGH);
    void		StrokePoints(const BPoint *pts, b_int32 count, const b_uint8 *alpha = NULL, b_pattern p = B_SOLID_HIGH);

	void		StrokeLine(BPoint pt, b_pattern p = B_SOLID_HIGH);
	void		StrokeLine(BPoint pt0, BPoint pt1, b_pattern p = B_SOLID_HIGH);

	void		StrokePolygon(const BPolygon *aPolygon, bool closed = true, b_pattern p = B_SOLID_HIGH);
    void		StrokePolygon(const BPoint *ptArray, b_int32 numPts, bool closed = true, b_pattern p = B_SOLID_HIGH);
	void		FillPolygon(const BPolygon *aPolygon, b_pattern p = B_SOLID_HIGH);
    void		FillPolygon(const BPoint *ptArray, b_int32 numPts, b_pattern p = B_SOLID_HIGH);

	void		StrokeTriangle(BPoint pt1, BPoint pt2, BPoint pt3, b_pattern p = B_SOLID_HIGH);
	void		FillTriangle(BPoint pt1, BPoint pt2, BPoint pt3, b_pattern p = B_SOLID_HIGH);

	void		StrokeRect(BRect r, b_pattern p = B_SOLID_HIGH);
	void		FillRect(BRect r, b_pattern p = B_SOLID_HIGH);

    void		StrokeRects(const BRect *rects, b_int32 count, b_pattern p = B_SOLID_HIGH);
    void		FillRects(const BRect *rects, b_int32 count, b_pattern p = B_SOLID_HIGH);
	void		FillRegion(const BRegion *region, b_pattern p = B_SOLID_HIGH);

	void		StrokeRoundRect(BRect r, float xRadius, float yRadius, b_pattern p = B_SOLID_HIGH);
	void		FillRoundRect(BRect r, float xRadius, float yRadius, b_pattern p = B_SOLID_HIGH);

	void		StrokeArc(BPoint ctPt, float xRadius, float yRadius, float startAngle, float arcAngle, b_pattern p = B_SOLID_HIGH);
	void		StrokeArc(BRect r, float startAngle, float arcAngle, b_pattern p = B_SOLID_HIGH);
	void		FillArc(BPoint ctPt, float xRadius, float yRadius, float startAngle, float arcAngle, b_pattern p = B_SOLID_HIGH);
	void		FillArc(BRect r, float start_angle, float arc_angle, b_pattern p = B_SOLID_HIGH);

	void		StrokeEllipse(BPoint ctPt, float xRadius, float yRadius, b_pattern p = B_SOLID_HIGH);
	void		StrokeEllipse(BRect r, b_pattern p = B_SOLID_HIGH);
	void		FillEllipse(BPoint ctPt, float xRadius, float yRadius, b_pattern p = B_SOLID_HIGH);
	void		FillEllipse(BRect r, b_pattern p = B_SOLID_HIGH);

    void		DrawString(const char *aString, b_int32 length = -1, float tabWidth = 0);
    void		DrawString(const char *aString, BPoint location, b_int32 length = -1, float tabWidth = 0);
    void		DrawString(const char *aString, b_int32 length, BPoint location, float tabWidth = 0);

    virtual void	SetFont(const BFont *font, b_uint8 mask = B_FONT_ALL);
    void		SetFont(const b_font_desc *fontDesc, b_uint8 mask = B_FONT_ALL);
	void		GetFont(BFont *font) const;
	void		SetFontSize(float size);
    void		GetFontHeight(b_font_height *height) const;
	void		ForceFontAliasing(bool enable);

	virtual void	GetPreferredSize(float *width, float *height);
	virtual void	ResizeToPreferred();

	void		GetClippingRegion(BRegion *clipping) const;
	void		ConstrainClippingRegion(const BRegion *clipping);
	void		ConstrainClippingRegion(BRect clipping);

	bool		IsPrinting() const;
	float		UnitsPerPixel() const;

	void		DrawBitmap(const BBitmap *bitmap);
	void		DrawBitmap(const BBitmap *bitmap, BPoint where);
	void		DrawBitmap(const BBitmap *bitmap, BRect destRect);
	void		DrawBitmap(const BBitmap *bitmap, BRect srcRect, BRect destRect);
	void		CopyBits(BRect srcRect, BRect destRect);

	void		Flush() const;
	void		Sync() const;

protected:
	// Empty functions BEGIN --- just for derivative class
	virtual void	ChildRemoving(BView *child);
	virtual void	TargetedByScrollView(BScrollView *scroll_view);
	// Empty functions END

private:
    friend class BWindow;
	friend class BScrollBar;
	friend class BScrollView;
	friend class BGraphicsEngine;
	friend class BViewLayout;

	BGraphicsContext *fDC;
    BLayoutItem *fLayout;

	void *fStates;

    b_uint32 fViewFlags;
	b_rgb_color fViewColor;
	bool fForceFontAliasing;
	BRegion fClippingTemp;
	bool fMouseInside;

	BList fScrollBar;
	b_bigtime_t fScrollTimeStamp;

	bool fMouseGrabbed;
	bool fKeyboardGrabbed;
	bool fEventStored;
    b_uint32 fEventMaskStored;
    b_uint32 fEventOptionsStored;
    b_uint32 fEventMask;
    b_uint32 fEventOptions;

	void AttachToWindow();
	void DetachFromWindow();
    void DrawStringInDirectlyMode(const char *aString, BPoint location, b_int32 length);
    void DrawStringInPixmapMode(const char *aString, BPoint location, b_int32 length);

    b_status_t _SetEventMask(b_uint32 mask, b_uint32 options);
	void _Expose(BRegion region, b_bigtime_t when);

	void _UpdateVisibleRegion();
	void _FrameChanged(BRect oldFrame, BRect newFrame);

    void InitSelf(BRect, b_uint32, b_uint32);
};

#endif /* __cplusplus */

#endif /* BHAPI_VIEW__H */

