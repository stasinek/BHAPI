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
 * File: View.h
 * Description: EView --- drawing/layout/control within EWindow
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_VIEW_H__
#define __ETK_VIEW_H__

#include "./../support/List.h"
#include "./../interface/InterfaceDefs.h"
#include "./../interface/Region.h"
#include "./../interface/Polygon.h"
#include "./../interface/Font.h"
#include "./../app/Handler.h"

enum {
	E_ENTERED_VIEW = 0,
	E_INSIDE_VIEW,
	E_EXITED_VIEW,
	E_OUTSIDE_VIEW
};

enum {
	E_POINTER_EVENTS = 1,
	E_KEYBOARD_EVENTS = 1 << 1
};

enum {
	E_LOCK_WINDOW_FOCUS	= 1,
	E_SUSPEND_VIEW_FOCUS	= 1 << 1,
	E_NO_POINTER_HISTORY	= 1 << 2
};

enum {
	E_WILL_DRAW			= 1,
	E_PULSE_NEEDED			= 1 << 1,
	E_NAVIGABLE_JUMP		= 1 << 2,
	E_NAVIGABLE			= 1 << 3,
	E_FRAME_EVENTS			= 1 << 4,
	E_UPDATE_WITH_REGION		= 1 << 5,
	E_DRAW_ON_CHILDREN		= 1 << 6,
	E_INPUT_METHOD_AWARE		= 1 << 7
};

enum {
	E_FONT_FAMILY_AND_STYLE		= 1,
	E_FONT_SIZE			= 1 << 1,
	E_FONT_SHEAR			= 1 << 2,
	E_FONT_SPACING     		= 1 << 3,
	E_FONT_ALL			= 0xff
};

#ifdef __cplusplus /* Just for C++ */

class EWindow;
class EGraphicsContext;
class EScrollView;
class EBitmap;
class ECursor;
class ELayoutItem;


class _IMPEXP_ETK EView : public EHandler {
public:
	EView(ERect frame,
	      const char *name,
	      euint32 resizingMode,
	      euint32 flags);
	virtual ~EView();

	virtual void	MessageReceived(EMessage *msg);

	void		AddChild(EView *child, EView *childNextSibling = NULL);
	bool		RemoveChild(EView *child);
	bool		RemoveSelf();
	eint32		CountChildren() const;
	EView		*ChildAt(eint32 index) const;

	EView		*NextSibling() const;
	EView		*PreviousSibling() const;
	bool		IsSibling(const EView *sibling) const;

	EWindow		*Window() const;
	EView		*Parent() const;
	EView		*Ancestor() const;
	EView		*FindView(const char *name) const;

	ERect		Bounds() const;
	ERect		Frame() const;
	EPoint		LeftTop() const;

	bool		IsVisible() const;
	ERect		VisibleBounds() const;
	ERect		VisibleFrame() const;
	ERegion		VisibleBoundsRegion() const;
	ERegion		VisibleFrameRegion() const;

	// Empty functions BEGIN --- just for derivative class
	virtual void	AttachedToWindow();
	virtual void	AllAttached();
	virtual void	DetachedFromWindow();
	virtual void	AllDetached();
	virtual void	Draw(ERect updateRect);
	virtual void	DrawAfterChildren(ERect updateRect);
	virtual void	MouseDown(EPoint where);
	virtual void	MouseUp(EPoint where);
	virtual void	MouseMoved(EPoint where, euint32 code, const EMessage *a_message);
	virtual void	WindowActivated(bool state);
	virtual void	KeyDown(const char *bytes, eint32 numBytes);
	virtual void	KeyUp(const char *bytes, eint32 numBytes);
	virtual void	Pulse();
	virtual void	FrameMoved(EPoint new_position);
	virtual void	FrameResized(float new_width, float new_height);
	// Empty functions END

	virtual void	Show();
	virtual void	Hide();
	bool		IsHidden() const;

	virtual void	SetEnabled(bool state);
	bool		IsEnabled() const;

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

	void		ConvertToParent(EPoint *pt) const;
	EPoint		ConvertToParent(EPoint pt) const;
	void		ConvertFromParent(EPoint *pt) const;
	EPoint		ConvertFromParent(EPoint pt) const;

	void		ConvertToParent(ERect *r) const;
	ERect		ConvertToParent(ERect r) const;
	void		ConvertFromParent(ERect *r) const;
	ERect		ConvertFromParent(ERect r) const;

	void		ConvertToParent(ERegion *region) const;
	ERegion		ConvertToParent(const ERegion &region) const;
	void		ConvertFromParent(ERegion *region) const;
	ERegion		ConvertFromParent(const ERegion &region) const;

	void		ConvertToWindow(EPoint *pt) const;
	EPoint		ConvertToWindow(EPoint pt) const;
	void		ConvertFromWindow(EPoint *pt) const;
	EPoint		ConvertFromWindow(EPoint pt) const;

	void		ConvertToWindow(ERect *r) const;
	ERect		ConvertToWindow(ERect r) const;
	void		ConvertFromWindow(ERect *r) const;
	ERect		ConvertFromWindow(ERect r) const;

	void		ConvertToWindow(ERegion *region) const;
	ERegion		ConvertToWindow(const ERegion &region) const;
	void		ConvertFromWindow(ERegion *region) const;
	ERegion		ConvertFromWindow(const ERegion &region) const;

	e_status_t	SetEventMask(euint32 mask, euint32 options = 0);
	euint32		EventMask() const;
	e_status_t	GetMouse(EPoint *location, eint32 *buttons, bool checkMessageQueue = true);
	bool		QueryCurrentMouse(bool pushed, eint32 buttons, bool btnsAlone = true, eint32 *clicks = NULL) const;

	// Next KeyUp(E_KEYBOARD_EVENTS) or MouseUp(E_POINTER_EVENTS) will restore the previous general event_mask.
	// If the current message isn't E_KEY_DOWN(E_UNMAPPED_KEY_DOWN) or E_MOUSE_DOWN, E_ERROR is return.
	// That's means: you should use this funtion within "KeyDown" or "MouseDown" etc...
	// The argument "mask" should not be "0" or any union.
	// That's means: you should not pass "mask" with "E_KEYBOARD_EVENTS | E_POINTER_EVENTS".
	e_status_t	SetPrivateEventMask(euint32 mask, euint32 options = 0);

	virtual void	SetFlags(euint32 flags);
	euint32		Flags() const;
	virtual void	SetResizingMode(euint32 mode);
	euint32		ResizingMode() const;
	void		MoveBy(float dh, float dv);
	void		MoveTo(EPoint where);
	void		MoveTo(float x, float y);
	void		ResizeBy(float dh, float dv);
	void		ResizeTo(float width, float height);
	void		ScrollBy(float dh, float dv);
	void		ScrollTo(float x, float y);
	virtual void	ScrollTo(EPoint where);
	virtual void	MakeFocus(bool focusState = true);
	bool		IsFocus() const;

	virtual void	SetDrawingMode(e_drawing_mode mode);
	e_drawing_mode	DrawingMode() const;

	void		MovePenTo(EPoint pt);
	void		MovePenTo(float x, float y);
	void		MovePenBy(float dx, float dy);
	EPoint		PenLocation() const;

	virtual void	SetPenSize(float size);
	float		PenSize() const;

	virtual void	SetViewColor(e_rgb_color c);
	void		SetViewColor(euint8 r, euint8 g, euint8 b, euint8 a = 255);
	e_rgb_color	ViewColor() const;

	virtual void	SetHighColor(e_rgb_color c);
	void		SetHighColor(euint8 r, euint8 g, euint8 b, euint8 a = 255);
	e_rgb_color	HighColor() const;

	virtual void	SetLowColor(e_rgb_color c);
	void		SetLowColor(euint8 r, euint8 g, euint8 b, euint8 a = 255);
	e_rgb_color	LowColor() const;

	void		PushState();
	void		PopState();

	void		Invalidate(ERect invalRect, bool redraw = true);
	void		Invalidate(bool redraw = true);

	// Note: The "Fill*()" functions isn't affected by the "PenSize()", it won't draw out of the edge.
	void		SetSquarePointStyle(bool state);
	bool		IsSquarePointStyle() const;
	void		StrokePoint(EPoint pt, e_pattern p = E_SOLID_HIGH);
	void		StrokePoints(const EPoint *pts, eint32 count, const euint8 *alpha = NULL, e_pattern p = E_SOLID_HIGH);

	void		StrokeLine(EPoint pt, e_pattern p = E_SOLID_HIGH);
	void		StrokeLine(EPoint pt0, EPoint pt1, e_pattern p = E_SOLID_HIGH);

	void		StrokePolygon(const EPolygon *aPolygon, bool closed = true, e_pattern p = E_SOLID_HIGH);
	void		StrokePolygon(const EPoint *ptArray, eint32 numPts, bool closed = true, e_pattern p = E_SOLID_HIGH);
	void		FillPolygon(const EPolygon *aPolygon, e_pattern p = E_SOLID_HIGH);
	void		FillPolygon(const EPoint *ptArray, eint32 numPts, e_pattern p = E_SOLID_HIGH);

	void		StrokeTriangle(EPoint pt1, EPoint pt2, EPoint pt3, e_pattern p = E_SOLID_HIGH);
	void		FillTriangle(EPoint pt1, EPoint pt2, EPoint pt3, e_pattern p = E_SOLID_HIGH);

	void		StrokeRect(ERect r, e_pattern p = E_SOLID_HIGH);
	void		FillRect(ERect r, e_pattern p = E_SOLID_HIGH);

	void		StrokeRects(const ERect *rects, eint32 count, e_pattern p = E_SOLID_HIGH);
	void		FillRects(const ERect *rects, eint32 count, e_pattern p = E_SOLID_HIGH);
	void		FillRegion(const ERegion *region, e_pattern p = E_SOLID_HIGH);

	void		StrokeRoundRect(ERect r, float xRadius, float yRadius, e_pattern p = E_SOLID_HIGH);
	void		FillRoundRect(ERect r, float xRadius, float yRadius, e_pattern p = E_SOLID_HIGH);

	void		StrokeArc(EPoint ctPt, float xRadius, float yRadius, float startAngle, float arcAngle, e_pattern p = E_SOLID_HIGH);
	void		StrokeArc(ERect r, float startAngle, float arcAngle, e_pattern p = E_SOLID_HIGH);
	void		FillArc(EPoint ctPt, float xRadius, float yRadius, float startAngle, float arcAngle, e_pattern p = E_SOLID_HIGH);
	void		FillArc(ERect r, float start_angle, float arc_angle, e_pattern p = E_SOLID_HIGH);

	void		StrokeEllipse(EPoint ctPt, float xRadius, float yRadius, e_pattern p = E_SOLID_HIGH);
	void		StrokeEllipse(ERect r, e_pattern p = E_SOLID_HIGH);
	void		FillEllipse(EPoint ctPt, float xRadius, float yRadius, e_pattern p = E_SOLID_HIGH);
	void		FillEllipse(ERect r, e_pattern p = E_SOLID_HIGH);

	void		DrawString(const char *aString, eint32 length = -1, float tabWidth = 0);
	void		DrawString(const char *aString, EPoint location, eint32 length = -1, float tabWidth = 0);
	void		DrawString(const char *aString, eint32 length, EPoint location, float tabWidth = 0);

	virtual void	SetFont(const EFont *font, euint8 mask = E_FONT_ALL);
	void		SetFont(const e_font_desc *fontDesc, euint8 mask = E_FONT_ALL);
	void		GetFont(EFont *font) const;
	void		SetFontSize(float size);
	void		GetFontHeight(e_font_height *height) const;
	void		ForceFontAliasing(bool enable);

	virtual void	GetPreferredSize(float *width, float *height);
	virtual void	ResizeToPreferred();

	void		GetClippingRegion(ERegion *clipping) const;
	void		ConstrainClippingRegion(const ERegion *clipping);
	void		ConstrainClippingRegion(ERect clipping);

	bool		IsPrinting() const;
	float		UnitsPerPixel() const;

	void		DrawBitmap(const EBitmap *bitmap);
	void		DrawBitmap(const EBitmap *bitmap, EPoint where);
	void		DrawBitmap(const EBitmap *bitmap, ERect destRect);
	void		DrawBitmap(const EBitmap *bitmap, ERect srcRect, ERect destRect);
	void		CopyBits(ERect srcRect, ERect destRect);

	void		Flush() const;
	void		Sync() const;

protected:
	// Empty functions BEGIN --- just for derivative class
	virtual void	ChildRemoving(EView *child);
	virtual void	TargetedByScrollView(EScrollView *scroll_view);
	// Empty functions END

private:
	friend class EWindow;
	friend class EScrollBar;
	friend class EScrollView;
	friend class EGraphicsEngine;
	friend class EViewLayout;

	EGraphicsContext *fDC;
	ELayoutItem *fLayout;

	void *fStates;

	euint32 fViewFlags;
	e_rgb_color fViewColor;
	bool fForceFontAliasing;
	ERegion fClippingTemp;
	bool fMouseInside;

	EList fScrollBar;
	e_bigtime_t fScrollTimeStamp;

	bool fMouseGrabbed;
	bool fKeyboardGrabbed;
	bool fEventStored;
	euint32 fEventMaskStored;
	euint32 fEventOptionsStored;
	euint32 fEventMask;
	euint32 fEventOptions;

	void AttachToWindow();
	void DetachFromWindow();
	void DrawStringInDirectlyMode(const char *aString, EPoint location, eint32 length);
	void DrawStringInPixmapMode(const char *aString, EPoint location, eint32 length);

	e_status_t _SetEventMask(euint32 mask, euint32 options);
	void _Expose(ERegion region, e_bigtime_t when);

	void _UpdateVisibleRegion();
	void _FrameChanged(ERect oldFrame, ERect newFrame);

	void InitSelf(ERect, euint32, euint32);
};

#endif /* __cplusplus */

#endif /* __ETK_VIEW_H__ */

