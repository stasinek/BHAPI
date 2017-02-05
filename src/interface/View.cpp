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
 * File: View.cpp
 * Description: EView --- drawing/layout/control within EWindow
 *
 * --------------------------------------------------------------------------*/

#include <math.h>
#include "./../support/SupportDefs.h"

#include "./../support/ClassInfo.h"
#include "./../add-ons/font/FontEngine.h"
#include "./../support/List.h"
#include "./../add-ons/graphics/GraphicsEngine.h"
#include "./../app/Application.h"

#include "GraphicsDefs.h"
#include "Window.h"
#include "ScrollView.h"
#include "Box.h"
#include "Bitmap.h"
#include "ViewPrivate.h"

class _LOCAL EViewState {
public:
	e_drawing_mode		DrawingMode;
	EPoint			PenLocation;
	float			PenSize;
	e_rgb_color		HighColor;
	e_rgb_color		LowColor;
	EFont			Font;
	ERegion			*Clipping;
	bool			SquarePointStyle;
	EViewState		*PreviousState;

	inline EViewState(EViewState *prev)
	{
		Clipping = NULL;
		PreviousState = prev;

		if(prev != NULL)
		{
			DrawingMode = prev->DrawingMode;
			PenLocation = prev->PenLocation;
			PenSize = prev->PenSize;
			HighColor = prev->HighColor;
			LowColor = prev->LowColor;
			Font = prev->Font;
			if(prev->Clipping != NULL) Clipping = new ERegion(*(prev->Clipping));
			SquarePointStyle = prev->SquarePointStyle;
		}
	};

	inline ~EViewState()
	{
		if(Clipping != NULL) delete Clipping;
	}
};


void
EView::InitSelf(ERect frame, euint32 resizingMode, euint32 flags)
{
	if(etk_app == NULL || etk_app->fGraphicsEngine == NULL)
		ETK_ERROR("[INTERFACE]: %s --- View must created within a application which has graphics-engine!", __PRETTY_FUNCTION__);

	if((fDC = etk_app->fGraphicsEngine->CreateContext()) == NULL)
		ETK_ERROR("[INTERFACE]: %s --- Unable to create draw context!", __PRETTY_FUNCTION__);

	EViewState *viewState = new EViewState(NULL);

	viewState->DrawingMode = E_OP_COPY;
	viewState->PenLocation.Set(0, 0);
	viewState->PenSize = 0;
	viewState->HighColor.set_to(0, 0, 0);
	viewState->LowColor.set_to(255, 255, 255);
	viewState->Font = *etk_plain_font;
	viewState->Clipping = NULL;
	viewState->SquarePointStyle = false;

	fStates = (void*)viewState;

	fViewFlags = flags;
	fViewColor = e_ui_color(E_PANEL_BACKGROUND_COLOR);
	fForceFontAliasing = false;
	fMouseInside = false;
	fScrollTimeStamp = 0;

	fMouseGrabbed = false;
	fKeyboardGrabbed = false;
	fEventStored = false;
	fEventMaskStored = 0;
	fEventOptionsStored = 0;
	fEventMask = 0;
	fEventOptions = 0;

	fLayout = new EViewLayout(this, frame, resizingMode);
}


EView::EView(ERect frame, const char *name, euint32 resizingMode, euint32 flags)
	: EHandler(name)
{
	InitSelf(frame, resizingMode, flags);
}


EView::~EView()
{
	if(Parent() != NULL || Window() != NULL)
	{
		ETK_WARNING("[INTERFACE]: %s --- It's recommended that remove self from parent before delete.", __PRETTY_FUNCTION__);
		RemoveSelf();
	}

	for(EView *child = ChildAt(0); child != NULL; child = ChildAt(0))
	{
		RemoveChild(child);
		delete child;
	}

	delete fDC;
	delete fLayout;

	EViewState *states = ((EViewState*)fStates);
	while(states != NULL)
	{
		EViewState *viewState = states;
		states = viewState->PreviousState;
		delete viewState;
	}
}


void
EView::MessageReceived(EMessage *msg)
{
	switch(msg->what)
	{
		case E_PULSE:
			Pulse();
			break;

		case E_VIEW_MOVED:
			{
				EPoint where;
				if(msg->FindPoint("where", &where) == false) break;
				FrameMoved(where);
			}
			break;

		case E_VIEW_RESIZED:
			{
				float width, height;
				if(msg->FindFloat("width", &width) == false) break;
				if(msg->FindFloat("height", &height) == false) break;
				FrameResized(width, height);
			}
			break;

		case E_MOUSE_DOWN:
		case E_MOUSE_UP:
		case E_MOUSE_MOVED:
			{
				EWindow *win = Window();
				if(win == NULL) break;

				EPoint where;
				if(msg->FindPoint("where", &where) == false) break;

				for(EView *child = ChildAt(0); child != NULL; child = child->NextSibling())
				{
					EPoint pt = child->fLayout->ConvertFromContainer(where);

					if(child->fLayout->VisibleRegion()->Contains(pt) == false) continue;
					if(!(child->EventMask() & E_POINTER_EVENTS))
					{
						EMessage aMsg(*msg);
						aMsg.ReplacePoint("where", pt);
						win->PostMessage(&aMsg, child);
					}

					return; // just one child can receive the message
				}

				if(msg->what == E_MOUSE_DOWN)
				{
					MouseDown(where);
				}
				else if(msg->what == E_MOUSE_UP)
				{
					if(fEventStored)
					{
						if(fMouseGrabbed)
						{
							win->_UngrabMouse();
							fMouseGrabbed = false;
						}

						euint32 eventMask = fEventMask & ~E_POINTER_EVENTS;

						if(eventMask == 0)
						{
							_SetEventMask(fEventMaskStored, fEventOptionsStored);
							fEventStored = false;
						}
						else
						{
							_SetEventMask(eventMask, fEventOptions);
							if(!fKeyboardGrabbed) fEventOptions &= ~E_LOCK_WINDOW_FOCUS;
						}
					}

					MouseUp(where);
				}
				else // E_MOUSE_MOVED
				{
					bool insided = fLayout->VisibleRegion()->Contains(where);
					euint32 transit = (insided ? E_INSIDE_VIEW : E_OUTSIDE_VIEW);
					if(fMouseInside != insided)
					{
						if(!insided)
						{
							fMouseInside = false;
							transit = E_EXITED_VIEW;
							win->fMouseInsideViews.RemoveItem(this);
						}
						else
						{
							fMouseInside = true;
							transit = E_ENTERED_VIEW;
							win->fMouseInsideViews.AddItem(this);
						}
					}
					// TODO: E_NO_POINTER_HISTORY, drag info
					MouseMoved(where, transit, NULL);
				}
			}
			break;

		case E_UNMAPPED_KEY_DOWN:
		case E_UNMAPPED_KEY_UP:
		case E_KEY_DOWN:
		case E_KEY_UP:
			{
				EWindow *win = Window();
				if(win == NULL) break;

				if(msg->what == E_UNMAPPED_KEY_UP || msg->what == E_KEY_UP)
				{
					if(fEventStored)
					{
						if(fKeyboardGrabbed)
						{
							win->_UngrabKeyboard();
							fKeyboardGrabbed = false;
						}

						euint32 eventMask = fEventMask & ~E_KEYBOARD_EVENTS;

						if(eventMask == 0)
						{
							_SetEventMask(fEventMaskStored, fEventOptionsStored);
							fEventStored = false;
						}
						else
						{
							_SetEventMask(eventMask, fEventOptions);
							if(!fMouseGrabbed) fEventOptions &= ~E_LOCK_WINDOW_FOCUS;
						}
					}
				}

				char _bytes[4];
				bzero(_bytes, 4);
				const char *bytes = NULL;

				if(msg->FindString("bytes", &bytes) != true || bytes == NULL)
				{
					bytes = _bytes;
					if((msg->what == E_KEY_DOWN || msg->what == E_KEY_UP))
					{
						for(eint32 i = 0; i < 3; i++)
						{
							eint8 val;
							if(!msg->FindInt8("byte", i, &val)) break;
							_bytes[i] = val;
						}
					}
				}

				if(msg->what == E_KEY_DOWN || msg->what == E_UNMAPPED_KEY_DOWN)
					KeyDown(bytes, strlen(bytes));
				else
					KeyUp(bytes, strlen(bytes));
			}
			break;

		case E_WINDOW_ACTIVATED:
			if(Window() == NULL) break;
			WindowActivated(Window()->IsActivate());
			for(EView *child = ChildAt(0); child != NULL; child = child->NextSibling())
			{
				Looper()->PostMessage(msg, child);
			}
			break;

		default:
			EHandler::MessageReceived(msg);
	}
}


void
EView::Show()
{
	if(fLayout->IsHidden(false) == false) return;

	fLayout->Show();

	if(!e_is_kind_of(this, EScrollBar) ||
	   !e_is_kind_of(Parent(), EScrollView) ||
	   e_cast_as(Parent(), EScrollView)->fTarget == NULL) return;

	if(e_cast_as(Parent(), EScrollView)->fHSB == this ||
	   e_cast_as(Parent(), EScrollView)->fVSB == this)
		e_cast_as(Parent(), EScrollView)->fTarget->fLayout->UpdateVisibleRegion();
}


void
EView::Hide()
{
	if(fLayout->IsHidden(false)) return;

	EView::MakeFocus(false);

	fLayout->Hide();

	if(!e_is_kind_of(this, EScrollBar) ||
	   !e_is_kind_of(Parent(), EScrollView) ||
	   e_cast_as(Parent(), EScrollView)->fTarget == NULL) return;

	if(e_cast_as(Parent(), EScrollView)->fHSB == this ||
	   e_cast_as(Parent(), EScrollView)->fVSB == this)
		e_cast_as(Parent(), EScrollView)->fTarget->fLayout->UpdateVisibleRegion();
}


bool
EView::IsHidden() const
{
	if(Window() == NULL) return true;
	return fLayout->IsHidden();
}


void
EView::AttachedToWindow()
{
}


void
EView::AllAttached()
{
}


void
EView::DetachedFromWindow()
{
}


void
EView::AllDetached()
{
}


void
EView::ChildRemoving(EView *child)
{
}


void
EView::AddChild(EView *child, EView *nextSibling)
{
	if(child == NULL || child->Looper() != NULL || child->Parent() != NULL ||
	   (nextSibling == NULL ? false : nextSibling->Parent() != this))
	{
		ETK_WARNING("[INTERFACE]: %s --- Unable to add child.", __PRETTY_FUNCTION__);
		return;
	}

	EWindow *win = Window();
	if(win != NULL) win->AddHandler(child);

	if(fLayout->AddItem(child->fLayout, nextSibling == NULL ? -1 : fLayout->IndexOf(nextSibling->fLayout)) == false)
	{
		if(win != NULL) win->RemoveHandler(child);
		ETK_WARNING("[INTERFACE]: %s --- Unable to add child.", __PRETTY_FUNCTION__);
		return;
	}

	if(win != NULL)
	{
		if(child->Window() == win)
		{
			child->AttachToWindow();
			child->AttachedToWindow();

			win->AddViewChildrenToHandlersList(win, child);
			child->AllAttached();
		}
		else
		{
			ETK_OUTPUT("Warning: [INTERFACE]: %s --- Unable to attch child to window, but child added.\n", __PRETTY_FUNCTION__);
		}
	}
}


bool
EView::IsSibling(const EView *sibling) const
{
	if(sibling == NULL || sibling == this) return false;
	if(fLayout->Container() != sibling->fLayout->Container()) return false;
	return(fLayout->Container() != NULL);
}


bool
EView::RemoveChild(EView *child)
{
	if(child == NULL || child->Parent() != this) return false;

	if(child->fScrollBar.IsEmpty() == false)
	{
		for(eint32 i = 0; i < child->fScrollBar.CountItems(); i++)
		{
			EScrollBar *scrollbar = (EScrollBar*)child->fScrollBar.ItemAt(i);
			scrollbar->fTarget = NULL;
		}
		child->fScrollBar.MakeEmpty();
	}

	if(e_is_kind_of(child, EScrollBar))
	{
		EScrollBar *scrollbar = e_cast_as(child, EScrollBar);
		if(scrollbar->fTarget != NULL)
		{
			scrollbar->fTarget->fScrollBar.RemoveItem(scrollbar);
			scrollbar->fTarget = NULL;
		}
	}

	ChildRemoving(child);

	EWindow *win = Window();
	if(win != NULL)
	{
		win->RemoveViewChildrenFromHandlersList(win, child);
		child->AllDetached();

		child->DetachedFromWindow();

		child->DetachFromWindow();
		win->RemoveHandler(child);
	}

	fLayout->RemoveItem(child->fLayout);

	return true;
}


bool
EView::RemoveSelf()
{
	if(Parent() != NULL) return Parent()->RemoveChild(this);
	if(Window() != NULL) return Window()->RemoveChild(this);
	return false;
}


eint32
EView::CountChildren() const
{
	return fLayout->CountItems();
}


EView*
EView::ChildAt(eint32 index) const
{
	return(fLayout->ItemAt(index) != NULL ? (EView*)fLayout->ItemAt(index)->PrivateData() : NULL);
}


EView*
EView::NextSibling() const
{
	return(fLayout->NextSibling() != NULL ? (EView*)fLayout->NextSibling()->PrivateData() : NULL);
}


EView*
EView::PreviousSibling() const
{
	return(fLayout->PreviousSibling() != NULL ? (EView*)fLayout->PreviousSibling()->PrivateData() : NULL);
}


EWindow*
EView::Window() const
{
	return e_cast_as(Looper(), EWindow);
}


EView*
EView::Parent() const
{
	return(e_is_kind_of(fLayout->Container(), EViewLayout) ? (EView*)fLayout->Container()->PrivateData() : NULL);
}


EView*
EView::Ancestor() const
{
	if(Parent() == NULL) return (EView*)this;

	EView *ancestor = Parent();
	while(ancestor->Parent() != NULL) ancestor = ancestor->Parent();

	return ancestor;
}


ERect
EView::Bounds() const
{
	return fLayout->Bounds();
}


EPoint
EView::LeftTop() const
{
	return fLayout->LeftTop();
}


ERect
EView::Frame() const
{
	return fLayout->Frame();
}


ERect
EView::VisibleBounds() const
{
	return fLayout->VisibleRegion()->Frame();
}


ERect
EView::VisibleFrame() const
{
	return ConvertToParent(fLayout->VisibleRegion()->Frame());
}


ERegion
EView::VisibleBoundsRegion() const
{
	ERegion region(*(fLayout->VisibleRegion()));
	return region;
}


ERegion
EView::VisibleFrameRegion() const
{
	ERegion region(*(fLayout->VisibleRegion()));
	ConvertToParent(&region);
	return region;
}


bool
EView::IsVisible() const
{
	return(fLayout->VisibleRegion()->CountRects() > 0);
}


void
EView::MouseDown(EPoint where)
{
}


void
EView::MouseUp(EPoint where)
{
}


void
EView::MouseMoved(EPoint where, euint32 code, const EMessage *a_message)
{
}


void
EView::WindowActivated(bool state)
{
}


EView*
EView::FindView(const char *name) const
{
	EString srcStr(name);

	for(EView *child = ChildAt(0); child != NULL; child = child->NextSibling())
	{
		EString destStr(child->Name());

		if(srcStr == destStr) return child;

		EView *view = child->FindView(name);
		if(view != NULL) return view;
	}

	return NULL;
}


void
EView::KeyDown(const char *bytes, eint32 numBytes)
{
}


void
EView::KeyUp(const char *bytes, eint32 numBytes)
{
}


void
EView::Pulse()
{
}


void
EView::FrameMoved(EPoint new_position)
{
}


void
EView::FrameResized(float new_width, float new_height)
{
}


void
EView::TargetedByScrollView(EScrollView *scroll_view)
{
}


void
EView::Draw(ERect updateRect)
{
}


void
EView::DrawAfterChildren(ERect updateRect)
{
}


void
EView::ConvertToScreen(EPoint* pt) const
{
	if(!pt) return;

	EWindow *win = Window();
	if(win == NULL) return;

	ConvertToWindow(pt);
	win->ConvertToScreen(pt);
}


EPoint
EView::ConvertToScreen(EPoint pt) const
{
	EPoint pt1 = pt;
	ConvertToScreen(&pt1);
	return pt1;
}


void
EView::ConvertFromScreen(EPoint* pt) const
{
	if(pt == NULL || Window() == NULL) return;
	*pt -= ConvertToScreen(E_ORIGIN);
}


EPoint
EView::ConvertFromScreen(EPoint pt) const
{
	EPoint pt1 = pt;
	ConvertFromScreen(&pt1);
	return pt1;
}


void
EView::ConvertToScreen(ERect *r) const
{
	if(r == NULL) return;
	r->OffsetTo(ConvertToScreen(r->LeftTop()));
}


ERect
EView::ConvertToScreen(ERect r) const
{
	ERect rect = r;
	ConvertToScreen(&rect);
	return rect;
}


void
EView::ConvertFromScreen(ERect *r) const
{
	if(r == NULL) return;
	r->OffsetBy(ConvertFromScreen(E_ORIGIN));
}


ERect
EView::ConvertFromScreen(ERect r) const
{
	ERect rect = r;
	ConvertFromScreen(&rect);
	return rect;
}


void
EView::ConvertToScreen(ERegion *region) const
{
	if(region == NULL || region->CountRects() <= 0) return;
	EPoint pt = ConvertToScreen(region->Frame().LeftTop());
	region->OffsetBy(pt - region->Frame().LeftTop());
}


ERegion
EView::ConvertToScreen(const ERegion &region) const
{
	ERegion aRegion(region);
	ConvertToScreen(&aRegion);
	return aRegion;
}


void
EView::ConvertFromScreen(ERegion *region) const
{
	if(region == NULL || region->CountRects() <= 0) return;
	region->OffsetBy(ConvertFromScreen(E_ORIGIN));
}


ERegion
EView::ConvertFromScreen(const ERegion &region) const
{
	ERegion aRegion(region);
	ConvertFromScreen(&aRegion);
	return aRegion;
}


void
EView::ConvertToParent(EPoint *pt) const
{
	fLayout->ConvertToContainer(pt);
}


EPoint
EView::ConvertToParent(EPoint pt) const
{
	EPoint pt1 = pt;
	ConvertToParent(&pt1);
	return pt1;
}


void
EView::ConvertFromParent(EPoint *pt) const
{
	fLayout->ConvertFromContainer(pt);
}


EPoint
EView::ConvertFromParent(EPoint pt) const
{
	EPoint pt1 = pt;
	ConvertFromParent(&pt1);
	return pt1;
}


void
EView::ConvertToParent(ERect *r) const
{
	if(r == NULL) return;
	r->OffsetTo(ConvertToParent(r->LeftTop()));
}


ERect
EView::ConvertToParent(ERect r) const
{
	ERect rect = r;
	ConvertToParent(&rect);
	return rect;
}


void
EView::ConvertFromParent(ERect *r) const
{
	if(r == NULL) return;
	r->OffsetBy(ConvertFromParent(E_ORIGIN));
}


ERect
EView::ConvertFromParent(ERect r) const
{
	ERect rect = r;
	ConvertFromParent(&rect);
	return rect;
}


void
EView::ConvertToParent(ERegion *region) const
{
	if(region == NULL || region->CountRects() <= 0) return;
	EPoint pt = ConvertToParent(region->Frame().LeftTop());
	region->OffsetBy(pt - region->Frame().LeftTop());
}


ERegion
EView::ConvertToParent(const ERegion &region) const
{
	ERegion aRegion(region);
	ConvertToParent(&aRegion);
	return aRegion;
}


void
EView::ConvertFromParent(ERegion *region) const
{
	if(region == NULL || region->CountRects() <= 0) return;
	region->OffsetBy(ConvertFromParent(E_ORIGIN));
}


ERegion
EView::ConvertFromParent(const ERegion &region) const
{
	ERegion aRegion(region);
	ConvertFromParent(&aRegion);
	return aRegion;
}


void
EView::ConvertToWindow(EPoint* pt) const
{
	if(pt == NULL) return;

	EWindow *win = Window();
	if(win == NULL) return;

	for(ELayoutItem *container = fLayout;
	    container->Container() != win->fLayout;
	    container = e_cast_as(container->Container(), ELayoutItem))
	{
		container->ConvertToContainer(pt);
	}
}


EPoint
EView::ConvertToWindow(EPoint pt) const
{
	EPoint pt1 = pt;
	ConvertToWindow(&pt1);
	return pt1;
}


void
EView::ConvertFromWindow(EPoint* pt) const
{
	if(pt == NULL || Window() == NULL) return;
	*pt -= ConvertToWindow(E_ORIGIN);
}


EPoint
EView::ConvertFromWindow(EPoint pt) const
{
	EPoint pt1 = pt;
	ConvertFromWindow(&pt1);
	return pt1;
}


void
EView::ConvertToWindow(ERect *r) const
{
	if(r == NULL) return;
	r->OffsetTo(ConvertToWindow(r->LeftTop()));
}


ERect
EView::ConvertToWindow(ERect r) const
{
	ERect rect = r;
	ConvertToWindow(&rect);
	return rect;
}


void
EView::ConvertFromWindow(ERect *r) const
{
	if(r == NULL) return;
	r->OffsetBy(ConvertFromWindow(E_ORIGIN));
}


ERect
EView::ConvertFromWindow(ERect r) const
{
	ERect rect = r;
	ConvertFromWindow(&rect);
	return rect;
}


void
EView::ConvertToWindow(ERegion *region) const
{
	if(region == NULL || region->CountRects() <= 0) return;
	EPoint pt = ConvertToWindow(region->Frame().LeftTop());
	region->OffsetBy(pt - region->Frame().LeftTop());
}


ERegion
EView::ConvertToWindow(const ERegion &region) const
{
	ERegion aRegion(region);
	ConvertToWindow(&aRegion);
	return aRegion;
}


void
EView::ConvertFromWindow(ERegion *region) const
{
	if(region == NULL || region->CountRects() <= 0) return;
	region->OffsetBy(ConvertFromWindow(E_ORIGIN));
}


ERegion
EView::ConvertFromWindow(const ERegion &region) const
{
	ERegion aRegion(region);
	ConvertFromWindow(&aRegion);
	return aRegion;
}


void
EView::SetFlags(euint32 flags)
{
	EWindow *win = Window();
	if(win != NULL)
	{
		euint32 oldPulseNeeded = fViewFlags & E_PULSE_NEEDED;
		euint32 newPulseNeeded = flags & E_PULSE_NEEDED;

		if(oldPulseNeeded != newPulseNeeded)
		{
			if(newPulseNeeded == E_PULSE_NEEDED)
				win->fNeededToPulseViews.AddItem(this);
			else
				win->fNeededToPulseViews.RemoveItem(this);

			if(win->fPulseRunner)
				win->fPulseRunner->SetCount((win->fPulseRate > E_INT64_CONSTANT(0) &&
							     win->fNeededToPulseViews.CountItems() > 0 &&
							     win->IsHidden() == false) ? -1 : 0);
		}
	}

	fViewFlags = flags;
}


euint32
EView::Flags() const
{
	return fViewFlags;
}


void
EView::SetResizingMode(euint32 mode)
{
	fLayout->SetResizingMode(mode);
}


euint32
EView::ResizingMode() const
{
	return fLayout->ResizingMode();
}


void
EView::_FrameChanged(ERect oldFrame, ERect newFrame)
{
	if(oldFrame == newFrame) return;

	EWindow *win = Window();

	if(fViewFlags & E_FRAME_EVENTS)
	{
		EMessage aMsg(E_VIEW_MOVED);
		aMsg.AddInt64("when", e_real_time_clock_usecs());
		aMsg.AddFloat("width", newFrame.Width());
		aMsg.AddFloat("height", newFrame.Height());
		aMsg.AddPoint("where", newFrame.LeftTop());

		if(oldFrame.LeftTop() != newFrame.LeftTop())
		{
			if(win)
				win->PostMessage(&aMsg, this);
			else
				MessageReceived(&aMsg);
		}

		if(oldFrame.Width() != newFrame.Width() || oldFrame.Height() != newFrame.Height())
		{
			aMsg.what = E_VIEW_RESIZED;
			if(win)
				win->PostMessage(&aMsg, this);
			else
				MessageReceived(&aMsg);
		}
	}

	if(!(e_is_kind_of(Parent(), EScrollView) == false || e_cast_as(Parent(), EScrollView)->fTarget != this))
	{
		ERect rect = Parent()->Frame();
		e_cast_as(Parent(), EScrollView)->FrameResized(rect.Width(), rect.Height());
	}
}


void
EView::MoveBy(float dh, float dv)
{
	MoveTo(fLayout->Frame().LeftTop() + EPoint(dh, dv));
}


void
EView::MoveTo(EPoint where)
{
	fLayout->MoveTo(where);
}


void
EView::MoveTo(float x, float y)
{
	MoveTo(EPoint(x, y));
}


void
EView::ResizeBy(float dh, float dv)
{
	ResizeTo(fLayout->Width() + dh, fLayout->Height() + dv);
}


void
EView::ResizeTo(float width, float height)
{
	fLayout->ResizeTo(width, height);
}


void
EView::AttachToWindow()
{
	EWindow *win = Window();

	if(fEventMask & E_POINTER_EVENTS)
		win->fMouseInterestedViews.AddItem(this);

	if(fEventMask & E_KEYBOARD_EVENTS)
		win->fKeyboardInterestedViews.AddItem(this);

	Invalidate();

	if(Flags() & E_PULSE_NEEDED)
	{
		win->fNeededToPulseViews.AddItem(this);
		if(win->fPulseRunner)
			win->fPulseRunner->SetCount((win->fPulseRate > E_INT64_CONSTANT(0) &&
						     win->fNeededToPulseViews.CountItems() > 0 &&
						     win->IsHidden() == false) ? -1 : 0);
	}
}


void
EView::DetachFromWindow()
{
	EWindow *win = Window();

	if(fEventStored)
	{
		if(fMouseGrabbed)
		{
			win->_UngrabMouse();
			fMouseGrabbed = false;
		}

		if(fKeyboardGrabbed)
		{
			win->_UngrabKeyboard();
			fKeyboardGrabbed = false;
		}

		_SetEventMask(fEventMaskStored, fEventOptionsStored);

		fEventStored = false;
	}

	if(fMouseInside)
		win->fMouseInsideViews.RemoveItem(this);

	if(fEventMask & E_POINTER_EVENTS)
		win->fMouseInterestedViews.RemoveItem(this);

	if(fEventMask & E_KEYBOARD_EVENTS)
		win->fKeyboardInterestedViews.RemoveItem(this);

	EView::MakeFocus(false);

	Invalidate();

	if(Flags() & E_PULSE_NEEDED)
	{
		win->fNeededToPulseViews.RemoveItem(this);
		if(win->fPulseRunner)
			win->fPulseRunner->SetCount((win->fPulseRate > E_INT64_CONSTANT(0) &&
						     win->fNeededToPulseViews.CountItems() > 0 &&
						     win->IsHidden() == false) ? -1 : 0);
	}
}


void
EView::MovePenTo(EPoint pt)
{
	((EViewState*)fStates)->PenLocation = pt;
}


void
EView::MovePenTo(float x, float y)
{
	((EViewState*)fStates)->PenLocation.x = x;
	((EViewState*)fStates)->PenLocation.y = y;
}


void
EView::MovePenBy(float dx, float dy)
{
	((EViewState*)fStates)->PenLocation.x += dx;
	((EViewState*)fStates)->PenLocation.y += dy;
}


EPoint
EView::PenLocation() const
{
	return ((EViewState*)fStates)->PenLocation;
}


void
EView::SetPenSize(float size)
{
	if(size < 0) return;
	if(((EViewState*)fStates)->PenSize != size)
	{
		((EViewState*)fStates)->PenSize = size;
		fDC->SetPenSize((euint32)ceil((double)size));
	}
}


float
EView::PenSize() const
{
	return ((EViewState*)fStates)->PenSize;
}


void
EView::SetViewColor(e_rgb_color c)
{
	if(fViewColor != c)
	{
		fViewColor = c;
		Invalidate();
	}
}


void
EView::SetViewColor(euint8 r, euint8 g, euint8 b, euint8 a)
{
	e_rgb_color c;
	c.set_to(r, g, b, a);
	SetViewColor(c);
}


e_rgb_color
EView::ViewColor() const
{
	return fViewColor;
}


void
EView::SetHighColor(e_rgb_color c)
{
	((EViewState*)fStates)->HighColor = c;
}


void
EView::SetHighColor(euint8 r, euint8 g, euint8 b, euint8 a)
{
	e_rgb_color c;
	c.set_to(r, g, b, a);
	SetHighColor(c);
}


e_rgb_color
EView::HighColor() const
{
	return ((EViewState*)fStates)->HighColor;
}


void
EView::SetLowColor(e_rgb_color c)
{
	((EViewState*)fStates)->LowColor = c;
}


void
EView::SetLowColor(euint8 r, euint8 g, euint8 b, euint8 a)
{
	e_rgb_color c;
	c.set_to(r, g, b, a);
	SetLowColor(c);
}


e_rgb_color
EView::LowColor() const
{
	return ((EViewState*)fStates)->LowColor;
}


void
EView::StrokePoint(EPoint pt, e_pattern p)
{
	MovePenTo(pt);

	if(IsVisible() == false) return;

	ConvertToWindow(&pt);

	EPoint _pt_ = EPoint(PenSize() / 2.f, PenSize() / 2.f);
	ERect updateRect(pt - _pt_, pt + _pt_);

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	pt.Floor();
	if(Window()->fPixmap->StrokePoint(fDC, (eint32)pt.x, (eint32)pt.y) == E_OK) Window()->_Update(updateRect, false);
}


void
EView::StrokePoints(const EPoint *_pts, eint32 count, const euint8 *alpha, e_pattern p)
{
	if(_pts == NULL || count <= 0) return;

	MovePenTo(_pts[count - 1]);

	if(IsVisible() == false) return;

	eint32 *pts = new eint32[2 * count];
	eint32 *tmp = pts;

	EPoint pmin, pmax;

	for(eint32 i = 0; i < count; i++)
	{
		EPoint pt = ConvertToWindow(_pts[i]);

		if(i == 0)
			pmin = pmax = pt;
		else
		{
			pmin.x = min_c(pmin.x, pt.x);
			pmin.y = min_c(pmin.y, pt.y);
			pmax.x = max_c(pmax.x, pt.x);
			pmax.y = max_c(pmax.y, pt.y);
		}

		pt.Floor();

		*tmp++ = (eint32)pt.x;
		*tmp++ = (eint32)pt.y;
	}

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	e_status_t status;
	if(alpha) status = Window()->fPixmap->StrokePoints_Alphas(fDC, pts, alpha, count);
	else status = Window()->fPixmap->StrokePoints(fDC, pts, count);

	delete[] pts;

	if(status == E_OK)
	{
		ERect updateRect(ERect(pmin, pmax).InsetByCopy(PenSize() / -2.f, PenSize() / -2.f));
		Window()->_Update(updateRect, false);
	}
}


void
EView::StrokeLine(EPoint pt, e_pattern p)
{
	StrokeLine(PenLocation(), pt, p);
}


void
EView::StrokeLine(EPoint pt0, EPoint pt1, e_pattern p)
{
	MovePenTo(pt1);

	if(IsVisible() == false) return;

	ConvertToWindow(&pt0);
	ConvertToWindow(&pt1);

	ERect updateRect;
	updateRect.left = min_c(pt0.x, pt1.x) - PenSize() / 2.f;
	updateRect.top = min_c(pt0.y, pt1.y) - PenSize() / 2.f;
	updateRect.right = max_c(pt0.x, pt1.x) + PenSize() / 2.f;
	updateRect.bottom = max_c(pt0.y, pt1.y) + PenSize() / 2.f;

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	pt0.Floor();
	pt1.Floor();
	if(Window()->fPixmap->StrokeLine(fDC,
					 (eint32)pt0.x, (eint32)pt0.y,
					 (eint32)pt1.x, (eint32)pt1.y) == E_OK) Window()->_Update(updateRect, false);
}


void
EView::StrokeRect(ERect r, e_pattern p)
{
	if(r.IsValid() == false || IsVisible() == false) return;

	ConvertToWindow(&r);
	ERect updateRect(r.InsetByCopy(PenSize() / -2.f, PenSize() / -2.f));

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	r.Floor();
	if(Window()->fPixmap->StrokeRect(fDC,
					 (eint32)r.left, (eint32)r.top,
					 (euint32)r.Width(), (euint32)r.Height()) == E_OK) Window()->_Update(updateRect, false);
}


void
EView::StrokePolygon(const EPolygon *aPolygon, bool closed, e_pattern p)
{
	if(aPolygon == NULL || aPolygon->CountPoints() <= 0 || IsVisible() == false) return;

	eint32 *pts = new eint32[2 * aPolygon->CountPoints()];
	eint32 *tmp = pts;
	const EPoint *polyPts = aPolygon->Points();

	for(eint32 i = 0; i < aPolygon->CountPoints(); i++)
	{
		EPoint pt = ConvertToWindow(*polyPts++).FloorSelf();
		*tmp++ = (eint32)pt.x;
		*tmp++ = (eint32)pt.y;
	}

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	if(Window()->fPixmap->StrokePolygon(fDC, pts, aPolygon->CountPoints(), closed) == E_OK)
	{
		ERect r = ConvertToWindow(aPolygon->Frame());
		ERect updateRect(r.InsetByCopy(PenSize() / -2.f, PenSize() / -2.f));
		Window()->_Update(updateRect, false);
	}

	delete[] pts;
}


void
EView::StrokePolygon(const EPoint *ptArray, eint32 numPts, bool closed, e_pattern p)
{
	EPolygon aPolygon(ptArray, numPts);
	if(aPolygon.CountPoints() <= 0) return;
	StrokePolygon(&aPolygon, closed, p);
}


void
EView::FillPolygon(const EPolygon *aPolygon, e_pattern p)
{
	if(aPolygon == NULL || aPolygon->CountPoints() <= 0 || IsVisible() == false) return;

	eint32 *pts = new eint32[2 * aPolygon->CountPoints()];
	eint32 *tmp = pts;
	const EPoint *polyPts = aPolygon->Points();

	for(eint32 i = 0; i < aPolygon->CountPoints(); i++)
	{
		EPoint pt = ConvertToWindow(*polyPts++).FloorSelf();
		*tmp++ = (eint32)pt.x;
		*tmp++ = (eint32)pt.y;
	}

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	if(Window()->fPixmap->FillPolygon(fDC, pts, aPolygon->CountPoints()) == E_OK)
	{
		ERect r = ConvertToWindow(aPolygon->Frame());
		ERect updateRect(r.InsetByCopy(PenSize() / -2.f, PenSize() / -2.f));
		Window()->_Update(updateRect, false);
	}

	delete[] pts;
}


void
EView::FillPolygon(const EPoint *ptArray, eint32 numPts, e_pattern p)
{
	EPolygon aPolygon(ptArray, numPts);
	if(aPolygon.CountPoints() <= 0) return;
	FillPolygon(&aPolygon, p);
}


void
EView::StrokeTriangle(EPoint pt1, EPoint pt2, EPoint pt3, e_pattern p)
{
	EPoint pts[3] = {pt1, pt2, pt3};
	StrokePolygon(pts, 3, true, p);
}


void
EView::FillTriangle(EPoint pt1, EPoint pt2, EPoint pt3, e_pattern p)
{
	EPoint pts[3] = {pt1, pt2, pt3};
	FillPolygon(pts, 3, p);
}


void
EView::StrokeRects(const ERect *rs, eint32 count, e_pattern p)
{
	if(rs == NULL || count <= 0 || IsVisible() == false) return;

	eint32 *rects = new eint32[4 * count];

	ERect updateRect;
	eint32 _count_ = 0;

	eint32 *tRects = rects;
	for(eint32 i = 0; i < count; i++)
	{
		ERect r = *rs++;
		ConvertToWindow(&r);
		if(r.IsValid() == false) continue;
		updateRect |= r;
		r.Floor();
		*tRects++ = (eint32)r.left; *tRects++ = (eint32)r.top; *tRects++ = (eint32)r.Width(); *tRects++ = (eint32)r.Height();
		_count_++;
	}

	if(_count_ > 0)
	{
		fDC->SetHighColor(HighColor());
		fDC->SetLowColor(LowColor());
		fDC->SetPattern(p);

		if(Window()->fPixmap->StrokeRects(fDC, rects, _count_) == E_OK)
		{
			updateRect.InsetBy(PenSize() / -2.f, PenSize() / -2.f);
			Window()->_Update(updateRect, false);
		}
	}

	delete[] rects;
}


void
EView::FillRect(ERect r, e_pattern p)
{
	if(r.IsValid() == false || IsVisible() == false) return;

	ConvertToWindow(&r);
	ERect updateRect(r.InsetByCopy(PenSize() / -2.f, PenSize() / -2.f));

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	r.Floor();
	if(Window()->fPixmap->FillRect(fDC,
				       (eint32)r.left, (eint32)r.top,
				       (euint32)r.Width(), (euint32)r.Height()) == E_OK) Window()->_Update(updateRect, false);
}


void
EView::FillRects(const ERect *rs, eint32 count, e_pattern p)
{
	if(rs == NULL || count <= 0 || IsVisible() == false) return;

	eint32 *rects = new eint32[4 * count];

	ERect updateRect;
	eint32 _count_ = 0;

	eint32 *tRects = rects;
	for(eint32 i = 0; i < count; i++)
	{
		ERect r = *rs++;
		ConvertToWindow(&r);
		if(r.IsValid() == false) continue;
		updateRect |= r;
		r.Floor();
		*tRects++ = (eint32)r.left; *tRects++ = (eint32)r.top; *tRects++ = (eint32)r.Width(); *tRects++ = (eint32)r.Height();
		_count_++;
	}

	if(_count_ > 0)
	{
		fDC->SetHighColor(HighColor());
		fDC->SetLowColor(LowColor());
		fDC->SetPattern(p);

		if(Window()->fPixmap->FillRects(fDC, rects, _count_) == E_OK)
		{
			updateRect.InsetBy(PenSize() / -2.f, PenSize() / -2.f);
			Window()->_Update(updateRect, false);
		}
	}

	delete[] rects;
}


void
EView::FillRegion(const ERegion *region, e_pattern p)
{
	if(region == NULL || IsVisible() == false) return;

	ERect updateRect = region->Frame().InsetByCopy(PenSize() / -2.f, PenSize() / -2.f);

	ERegion aRegion(*region);
	ConvertToWindow(&aRegion);
	if(aRegion.CountRects() <= 0) return;

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	if(Window()->fPixmap->FillRegion(fDC, aRegion) == E_OK) Window()->_Update(updateRect, false);
}


void
EView::StrokeRoundRect(ERect r, float xRadius, float yRadius, e_pattern p)
{
	if(r.IsValid() == false || xRadius < 0 || yRadius < 0 || IsVisible() == false) return;
	if(r.Width() == 0 || r.Height() == 0 || (xRadius == 0 && yRadius == 0))
	{
		StrokeRect(r, p);
		return;
	}

	ConvertToWindow(&r);
	ERect updateRect(r.InsetByCopy(PenSize() / -2.f, PenSize() / -2.f));

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	r.Floor();
	if(Window()->fPixmap->StrokeRoundRect(fDC,
					      (eint32)r.left, (eint32)r.top,
					      (euint32)r.Width(), (euint32)r.Height(),
					      (euint32)ceil((double)xRadius),
					      (euint32)ceil((double)yRadius)) == E_OK) Window()->_Update(updateRect, false);
}


void
EView::FillRoundRect(ERect r, float xRadius, float yRadius, e_pattern p)
{
	if(r.IsValid() == false || xRadius < 0 || yRadius < 0 || IsVisible() == false) return;
	if(r.Width() == 0 || r.Height() == 0 || (xRadius == 0 && yRadius == 0))
	{
		FillRect(r, p);
		return;
	}

	ConvertToWindow(&r);
	ERect updateRect(r.InsetByCopy(PenSize() / -2.f, PenSize() / -2.f));

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	r.Floor();
	if(Window()->fPixmap->FillRoundRect(fDC,
					    (eint32)r.left, (eint32)r.top,
					    (euint32)r.Width(), (euint32)r.Height(),
					    (euint32)ceil((double)xRadius),
					    (euint32)ceil((double)yRadius)) == E_OK) Window()->_Update(updateRect, false);
}


void
EView::StrokeArc(EPoint center, float xRadius, float yRadius, float start_angle, float arc_angle, e_pattern p)
{
	if(xRadius <= 0 || yRadius <= 0) return;

	ERect r;
	r.left = center.x - xRadius;
	r.top = center.y - yRadius;
	r.right = center.x + xRadius;
	r.bottom = center.y + yRadius;

	StrokeArc(r, start_angle, arc_angle, p);
}


void
EView::StrokeArc(ERect r, float start_angle, float arc_angle, e_pattern p)
{
	if(r.IsValid() == false || IsVisible() == false) return;

	ConvertToWindow(&r);
	ERect updateRect(r.InsetByCopy(PenSize() / -2.f, PenSize() / -2.f));

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	r.Floor();
	if(Window()->fPixmap->StrokeArc(fDC,
					(eint32)r.left, (eint32)r.top,
					(euint32)r.Width(), (euint32)r.Height(),
					start_angle, start_angle + arc_angle) == E_OK) Window()->_Update(updateRect, false);
}


void
EView::FillArc(EPoint center, float xRadius, float yRadius, float start_angle, float arc_angle, e_pattern p)
{
	if(xRadius <= 0 || yRadius <= 0) return;

	ERect r;
	r.left = center.x - xRadius;
	r.top = center.y - yRadius;
	r.right = center.x + xRadius;
	r.bottom = center.y + yRadius;

	FillArc(r, start_angle, arc_angle, p);
}


void
EView::FillArc(ERect r, float start_angle, float arc_angle, e_pattern p)
{
	if(r.IsValid() == false || IsVisible() == false) return;

	ConvertToWindow(&r);
	ERect updateRect(r.InsetByCopy(PenSize() / -2.f, PenSize() / -2.f));

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(p);

	r.Floor();
	if(Window()->fPixmap->FillArc(fDC,
				      (eint32)r.left, (eint32)r.top,
				      (euint32)r.Width(), (euint32)r.Height(),
				      start_angle, start_angle + arc_angle) == E_OK) Window()->_Update(updateRect, false);
}


void
EView::StrokeEllipse(EPoint center, float xRadius, float yRadius, e_pattern p)
{
	StrokeArc(center, xRadius, yRadius, 0, 360, p);
}


void
EView::StrokeEllipse(ERect rect, e_pattern p)
{
	StrokeArc(rect, 0, 360, p);
}


void
EView::FillEllipse(EPoint center, float xRadius, float yRadius, e_pattern p)
{
	FillArc(center, xRadius, yRadius, 0, 360, p);
}


void
EView::FillEllipse(ERect rect, e_pattern p)
{
	FillArc(rect, 0, 360, p);
}


void
EView::PushState()
{
	if(Window() == NULL)
		ETK_ERROR("[INTERFACE]: %s --- View isn't attached to a window!", __PRETTY_FUNCTION__);
	fStates = (void*)(new EViewState((EViewState*)fStates));
}


void
EView::PopState()
{
	if(Window() == NULL)
		ETK_ERROR("[INTERFACE]: %s --- View isn't attached to a window!", __PRETTY_FUNCTION__);

	EViewState *viewStates = (EViewState*)fStates;
	if(viewStates->PreviousState == NULL)
	{
		ETK_WARNING("[INTERFACE]: %s --- Maybe you don't call \"PushState\" and \"PopState\" at same times.", __PRETTY_FUNCTION__);
		return;
	}
	fStates = (void*)viewStates->PreviousState;
	delete viewStates;

	fDC->SetDrawingMode(((EViewState*)fStates)->DrawingMode);
	fDC->SetSquarePointStyle(((EViewState*)fStates)->SquarePointStyle);
	fDC->SetPenSize((euint32)ceil((double)((EViewState*)fStates)->PenSize));

	ERegion clipping(*(fLayout->VisibleRegion()));
	if(((EViewState*)fStates)->Clipping != NULL) clipping &= *(((EViewState*)fStates)->Clipping);
	if(fClippingTemp.CountRects() > 0) clipping &= fClippingTemp;
	ConvertToWindow(&clipping);
	fDC->SetClipping(clipping);
}


void
EView::Invalidate(ERect invalRect, bool redraw)
{
	if(invalRect.IsValid() == false || IsVisible() == false) return;
	if(fLayout->VisibleRegion()->Intersects(invalRect) == false) return;
	if(Window() != NULL) Window()->Invalidate(ConvertToWindow(invalRect), redraw);
}


void
EView::Invalidate(bool redraw)
{
	Invalidate(Bounds(), redraw);
}


void
EView::_Expose(ERegion region, e_bigtime_t when)
{
	if(IsVisible() == false) return;

	EWindow *win = Window();

	if(when < fScrollTimeStamp)
	{
		win->fBrokeOnExpose = true;
		return;
	}

	if(!(fViewFlags & E_UPDATE_WITH_REGION)) region.Set(region.Frame());

	ERegion clipping(*(fLayout->VisibleRegion()));

	region &= clipping;
	if(region.CountRects() <= 0) return;

	if(fViewColor != E_TRANSPARENT_COLOR)
	{
		fDC->SetHighColor(fViewColor);
		fDC->SetLowColor(fViewColor);
		fDC->SetPattern(E_SOLID_HIGH);

		fDC->SetPenSize(0);
		fDC->SetDrawingMode(E_OP_COPY);

		ERegion tmpClipping(region);
		ConvertToWindow(&tmpClipping);
		fDC->SetClipping(tmpClipping);

		ERect rect = ConvertToWindow(region.Frame());
		rect.Floor();
		win->fPixmap->FillRect(fDC, (eint32)rect.left, (eint32)rect.top, (euint32)rect.Width(), (euint32)rect.Height());
	}

	if(((EViewState*)fStates)->Clipping != NULL) clipping &= *(((EViewState*)fStates)->Clipping);
	fClippingTemp = region;
	clipping &= fClippingTemp;
	ConvertToWindow(&clipping);
	fDC->SetClipping(clipping);

	fDC->SetDrawingMode(((EViewState*)fStates)->DrawingMode);
	fDC->SetPenSize((euint32)ceil((double)((EViewState*)fStates)->PenSize));

	if(fViewFlags & E_WILL_DRAW)
	{
		if(fViewFlags & E_UPDATE_WITH_REGION)
			for(eint32 i = 0; i < region.CountRects(); i++) Draw(region.RectAt(i));
		else
			Draw(region.Frame());
	}

	bool doQuit = false;
	for(EView *child = ChildAt(0); child != NULL; child = child->NextSibling())
	{
		if(win->fBrokeOnExpose || win->_HasResizeMessage(true))
		{
			doQuit = true;
			break;
		}
		if(child->VisibleFrameRegion().Intersects(&region) == false) continue;
		child->_Expose(child->ConvertFromParent(region), when);
	}

	if(!doQuit && (fViewFlags & E_DRAW_ON_CHILDREN))
	{
		if(fViewFlags & E_UPDATE_WITH_REGION)
			for(eint32 i = 0; i < region.CountRects(); i++) DrawAfterChildren(region.RectAt(i));
		else
			DrawAfterChildren(region.Frame());
	}

	fClippingTemp.MakeEmpty();

	clipping = *(fLayout->VisibleRegion());
	if(((EViewState*)fStates)->Clipping != NULL) clipping &= *(((EViewState*)fStates)->Clipping);
	ConvertToWindow(&clipping);
	fDC->SetClipping(clipping);
}


void
EView::SetDrawingMode(e_drawing_mode mode)
{
	if(((EViewState*)fStates)->DrawingMode != mode)
	{
		((EViewState*)fStates)->DrawingMode = mode;
		fDC->SetDrawingMode(mode);
	}
}


e_drawing_mode
EView::DrawingMode() const
{
	return ((EViewState*)fStates)->DrawingMode;
}


void
EView::SetFont(const EFont *font, euint8 mask)
{
	if(font == NULL) return;

	if(mask == E_FONT_ALL)
	{
		((EViewState*)fStates)->Font = *font;
	}
	else
	{
		if(mask & E_FONT_FAMILY_AND_STYLE)
			((EViewState*)fStates)->Font.SetFamilyAndStyle(font->FamilyAndStyle());

		if(mask & E_FONT_SIZE)
			((EViewState*)fStates)->Font.SetSize(font->Size());

		if(mask & E_FONT_SHEAR)
			((EViewState*)fStates)->Font.SetShear(font->Shear());

		if(mask & E_FONT_SPACING)
			((EViewState*)fStates)->Font.SetSpacing(font->Spacing());
	}
}


void
EView::SetFont(const e_font_desc *fontDesc, euint8 mask)
{
	if(fontDesc == NULL) return;
	EFont font(*fontDesc);
	SetFont(&font, mask);
}


void
EView::GetFont(EFont *font) const
{
	if(font != NULL) *font = ((EViewState*)fStates)->Font;
}


void
EView::SetFontSize(float size)
{
	if(size <= 0) return;

	EFont font(((EViewState*)fStates)->Font);
	font.SetSize(size);

	SetFont(&font, E_FONT_SIZE);
}


void
EView::GetFontHeight(e_font_height *height) const
{
	((EViewState*)fStates)->Font.GetHeight(height);
}


void
EView::ForceFontAliasing(bool enable)
{
	fForceFontAliasing = enable;
}


void
EView::DrawString(const char *aString, eint32 length, float tabWidth)
{
	DrawString(aString, PenLocation(), length, tabWidth);
}


void
EView::DrawString(const char *aString, EPoint location, eint32 length, float tabWidth)
{
	if(aString == NULL || *aString == 0 || length == 0 || IsVisible() == false) return;

	EFontEngine *engine = ((EViewState*)fStates)->Font.Engine();
	if(engine == NULL) return;

	float size = ((EViewState*)fStates)->Font.Size();
	float spacing = ((EViewState*)fStates)->Font.Spacing();
	float shear = ((EViewState*)fStates)->Font.Shear();
	bool bold = ((EViewState*)fStates)->Font.IsBoldStyle();

	engine->Lock();
	e_font_render_mode renderMode = engine->RenderMode();
	float spaceWidth = engine->StringWidth(" ", size, spacing, shear, bold, 1);
	engine->Unlock();

	if(!(renderMode & (E_FONT_RENDER_DIRECTLY | E_FONT_RENDER_PIXMAP))) return;

	if(tabWidth == 0)
	{
		if(renderMode & E_FONT_RENDER_DIRECTLY)
			DrawStringInDirectlyMode(aString, location, length);
		else // E_FONT_RENDER_PIXMAP
			DrawStringInPixmapMode(aString, location, length);
	}
	else
	{
		EString aStr(aString, length);
		float spacing_width = spacing * size;

		if(tabWidth < 0)
		{
			if(tabWidth < E_FONT_MIN_TAB_WIDTH) tabWidth = -E_FONT_MIN_TAB_WIDTH;
			else tabWidth = (float)(ceil((double)(-tabWidth)));
			tabWidth = (tabWidth * spaceWidth) + (tabWidth - 1.f) * spacing_width;
		}

		MovePenTo(location);

		for(eint32 aOffset = 0; aOffset < aStr.Length(); aOffset++)
		{
			eint32 oldOffset = aOffset, len;
			aOffset = aStr.FindFirst("\t", aOffset);

			len = (aOffset < 0 ? aStr.Length() : aOffset) - oldOffset;
			if(len > 0)
			{
				if(renderMode & E_FONT_RENDER_DIRECTLY)
					DrawStringInDirectlyMode(aStr.String() + oldOffset, PenLocation(), len);
				else // E_FONT_RENDER_PIXMAP
					DrawStringInPixmapMode(aStr.String() + oldOffset, PenLocation(), len);
			}

			if(aOffset < 0) break;

			MovePenBy((aOffset > 0 ? spacing_width : 0) + tabWidth + UnitsPerPixel(), 0);
		}
	}
}


void
EView::DrawStringInDirectlyMode(const char *aString, EPoint location, eint32 length)
{
	EFontEngine *engine = ((EViewState*)fStates)->Font.Engine();

	fDC->SetHighColor(HighColor());
	fDC->SetLowColor(LowColor());
	fDC->SetPattern(E_SOLID_HIGH);

	float size = ((EViewState*)fStates)->Font.Size();
	float spacing = ((EViewState*)fStates)->Font.Spacing();
	float shear = ((EViewState*)fStates)->Font.Shear();
	bool bold = ((EViewState*)fStates)->Font.IsBoldStyle();
	bool force_mono = fForceFontAliasing;

	MovePenTo(location);

	engine->Lock();
	engine->ForceFontAliasing(force_mono);
	float width = engine->StringWidth(aString, size, spacing, shear, bold, length);
	ERect updateRect = engine->RenderString(this, aString, size, spacing, shear, bold, length);
	engine->Unlock();

	if(updateRect.IsValid())
	{
		ConvertToWindow(&updateRect);
		Window()->_Update(updateRect, false);
	}

	location.x += width + UnitsPerPixel();
	MovePenTo(location);
}


void
EView::DrawStringInPixmapMode(const char *aString, EPoint location, eint32 length)
{
	ERect rect = VisibleBounds();
	EFontEngine *engine = ((EViewState*)fStates)->Font.Engine();

	float size = ((EViewState*)fStates)->Font.Size();
	float spacing = ((EViewState*)fStates)->Font.Spacing();
	float shear = ((EViewState*)fStates)->Font.Shear();
	bool bold = ((EViewState*)fStates)->Font.IsBoldStyle();
	bool is_mono = fForceFontAliasing;

	eint32 w = 0, h = 0;

	engine->Lock();

	e_font_height fheight;
	engine->GetHeight(&fheight, size);

	EPoint startPoint = location;
	startPoint.y -= fheight.ascent + 1;

	engine->ForceFontAliasing(is_mono);

	float width = engine->StringWidth(aString, size, spacing, shear, bold, length);
	euint8 *bitmap = engine->RenderString(aString, &w, &h, &is_mono, size, spacing, shear, bold, length);

	engine->Unlock();

	rect &= ERect(startPoint, startPoint + EPoint((float)w, (float)h));
	if(rect.IsValid() == false || startPoint.x > rect.right || startPoint.y > rect.bottom ||
	   startPoint.x + (float)w < rect.left || startPoint.y + (float)h < rect.top)
	{
		if(bitmap) delete[] bitmap;
		return;
	}

	eint32 _w_ = w;
	eint32 _h_ = h;
	w = min_c(w, (eint32)(ceil(rect.Width())));
	h = min_c(h, (eint32)(ceil(rect.Height())));

	if(!bitmap || w <= 0 || h <= 0)
	{
		if(bitmap) delete[] bitmap;
		return;
	}

	eint32 *pts = new eint32[2 * w * h];
	euint8 *alpha = is_mono ? NULL : new euint8[w * h];
	eint32 pointCount = 0;

	eint32 i_start = max_c((eint32)(ceil(rect.top) - ceil(startPoint.y)), 0);
	eint32 j_start = max_c((eint32)(ceil(rect.left) - ceil(startPoint.x)), 0);

	ConvertToWindow(&startPoint);
	EPoint pmin, pmax;

	eint32 *tmp = pts;
	for(eint32 i = i_start; i < min_c(i_start + h, _h_); i++)
	{
		for(eint32 j = j_start; j < min_c(j_start + w, _w_); j++)
		{
			euint8 c = bitmap[i * _w_ + j];
			if(c != 0)
			{
				EPoint pt = startPoint + EPoint((float)j, (float)i);
				if(alpha) alpha[pointCount] = c;

				if(pointCount == 0)
					pmin = pmax = pt;
				else
				{
					pmin.x = min_c(pmin.x, pt.x);
					pmin.y = min_c(pmin.y, pt.y);
					pmax.x = max_c(pmax.x, pt.x);
					pmax.y = max_c(pmax.y, pt.y);
				}

				pt.Floor();

				*tmp++ = (eint32)pt.x;
				*tmp++ = (eint32)pt.y;

				pointCount++;
			}
		}
	}

	e_status_t status = E_ERROR;

	if(pointCount > 0)
	{
		fDC->SetHighColor(HighColor());
		fDC->SetLowColor(LowColor());
		fDC->SetPattern(E_SOLID_HIGH);

		if(alpha && DrawingMode() == E_OP_ALPHA)
		{
			status = Window()->fPixmap->StrokePoints_Alphas(fDC, pts, alpha, pointCount);
		}
		else
		{
			if(alpha)
			{
				// here we just generate 16 gray
				const euint8 *_alpha_ = alpha;
				EList ptsLists[16];
				for(eint32 i = 0; i < pointCount; i++)
				{
					eint32 a = (eint32)(*_alpha_++);
					ptsLists[a / 16].AddItem((void*)&pts[i * 2]);
				}

				e_rgb_color ptColors[16];
				e_rgb_color lcolor = LowColor();
				for(eint32 i = 0; i < 16; i++)
				{
					ptColors[i].set_to(HighColor());
					lcolor.alpha = 255 - (euint8)i * 17;
					ptColors[i].mix(lcolor);
				}

				status = Window()->fPixmap->StrokePoints_Colors(fDC, ptsLists, 16, ptColors);
			}
			else
			{
				status = Window()->fPixmap->StrokePoints(fDC, pts, pointCount);
			}
		}
	}

	delete[] pts;
	if(alpha) delete[] alpha;

	delete[] bitmap;

	if(status == E_OK)
	{
		EPoint _pt_ = EPoint(PenSize() / 2.f, PenSize() / 2.f);
		ERect updateRect(pmin - _pt_, pmax + _pt_);
		Window()->_Update(updateRect, false);
	}

	location.x += width + UnitsPerPixel();
	MovePenTo(location);
}


void
EView::DrawString(const char *aString, eint32 length, EPoint location, float tabWidth)
{
	DrawString(aString, location, length, tabWidth);
}


void
EView::MakeFocus(bool focusState)
{
	EWindow *win = Window();
	if(win == NULL) return;

	if(focusState)
	{
		if(win->fFocus != this)
		{
			if(win->fFocus) win->fFocus->MakeFocus(false);
			win->fFocus = this;
			win->SetPreferredHandler(this);
		}
	}
	else
	{
		if(win->fFocus == this)
		{
			win->fFocus = NULL;
			if(win->PreferredHandler() == this) win->SetPreferredHandler(NULL);
		}
	}
}


bool
EView::IsFocus() const
{
	EWindow *win = Window();
	if(win == NULL) return false;
	return(win->fFocus == this);
}


e_status_t
EView::SetEventMask(euint32 mask, euint32 options)
{
	if(fEventStored == false) return _SetEventMask(mask, options);

	fEventMaskStored = mask;
	fEventOptionsStored = options;

	return E_OK;
}


e_status_t
EView::_SetEventMask(euint32 mask, euint32 options)
{
	EWindow *win = Window();

	if(fEventMask != mask)
	{
		if(win == NULL)
		{
			fEventMask = mask;
		}
		else if(fEventMask != 0)
		{
			euint32 mask1 = fEventMask & ~mask;
			euint32 mask2 = mask & ~fEventMask;

			if(mask2 & E_POINTER_EVENTS)
			{
				if(win->fMouseInterestedViews.AddItem(this) == false) return E_ERROR;
			}
			else if(mask2 & E_KEYBOARD_EVENTS)
			{
				if(win->fKeyboardInterestedViews.AddItem(this) == false) return E_ERROR;
			}

			if(mask1 & E_POINTER_EVENTS)
				win->fMouseInterestedViews.RemoveItem(this);
			else if(mask1 & E_KEYBOARD_EVENTS)
				win->fKeyboardInterestedViews.RemoveItem(this);

			fEventMask = mask;
		}
		else
		{
			if(mask & E_POINTER_EVENTS)
			{
				if(win->fMouseInterestedViews.AddItem(this) == false) return E_ERROR;
			}

			if(mask & E_KEYBOARD_EVENTS)
			{
				if(win->fKeyboardInterestedViews.AddItem(this) == false)
				{
					if(mask & E_POINTER_EVENTS)
						win->fMouseInterestedViews.RemoveItem(this);
					return E_ERROR;
				}
			}

			fEventMask = mask;
		}
	}

	// TODO: E_NO_POINTER_HISTORY and action implement
	fEventOptions = (options & E_NO_POINTER_HISTORY ? E_NO_POINTER_HISTORY : 0);

	return E_OK;
}


euint32
EView::EventMask() const
{
	return fEventMask;
}


e_status_t
EView::SetPrivateEventMask(euint32 mask, euint32 options)
{
	// TODO: suspend etc...
	if(mask == 0) return E_ERROR;

	EWindow *win = Window();
	if(win == NULL) return E_ERROR;

	EMessage *msg = win->CurrentMessage();
	if(msg == NULL) return E_ERROR;

	if(mask == E_KEYBOARD_EVENTS)
	{
		if(!(msg->what != E_KEY_DOWN || msg->what != E_UNMAPPED_KEY_DOWN)) return E_ERROR;
	}
	else if(mask == E_POINTER_EVENTS)
	{
		if(msg->what != E_MOUSE_DOWN) return E_ERROR;
	}
	else
	{
		return E_ERROR;
	}

	euint32 eventMask, eventNewMask;
	euint32 eventOptions, eventNewOptions;

	eventMask = eventNewMask = fEventMask;
	eventOptions = eventNewOptions = fEventOptions;

	if(fEventStored == false)
	{
		fEventStored = true;

		fEventMaskStored = fEventMask;
		fEventOptionsStored = fEventOptions;

		eventNewMask = mask;
		eventNewOptions = options;
	}
	else
	{
		euint32 _mask_ = mask & ~eventNewMask;
		euint32 _options_ = options & ~eventNewOptions;

		if(_mask_ != 0)
		{
			if(_mask_ & E_POINTER_EVENTS) eventNewMask &= E_POINTER_EVENTS;
			if(_mask_ & E_KEYBOARD_EVENTS) eventNewMask &= E_KEYBOARD_EVENTS;
		}

		if(_options_ != 0)
		{
			if(_options_ & E_LOCK_WINDOW_FOCUS) eventNewOptions &= E_LOCK_WINDOW_FOCUS;
			if(_options_ & E_SUSPEND_VIEW_FOCUS) eventNewOptions &= E_SUSPEND_VIEW_FOCUS;
			if(_options_ & E_NO_POINTER_HISTORY) eventNewOptions &= E_NO_POINTER_HISTORY;
		}
	}

	if(eventMask != eventNewMask || eventOptions != eventNewOptions)
	{
		e_status_t status = _SetEventMask(eventNewMask, eventNewOptions);
		if(status != E_OK) return status;
		fEventOptions = eventNewOptions;
	}

	if(options & E_LOCK_WINDOW_FOCUS)
	{
		if(mask == E_POINTER_EVENTS)
		{
			if(fMouseGrabbed == false)
			{
				if(win->_GrabMouse() == false)
				{
					_SetEventMask(eventMask, eventOptions);
					fEventOptions = eventOptions;
					return E_ERROR;
				}
				fMouseGrabbed = true;
			}
		}
		else if(mask == E_KEYBOARD_EVENTS)
		{
			if(fKeyboardGrabbed == false)
			{
				if(win->_GrabKeyboard() == false)
				{
					_SetEventMask(eventMask, eventOptions);
					fEventOptions = eventOptions;
					return E_ERROR;
				}
				fKeyboardGrabbed = true;
			}
		}
	}

	return E_OK;
}


void
EView::GetPreferredSize(float *width, float *height)
{
	fLayout->ELayoutItem::GetPreferredSize(width, height);
}


void
EView::ResizeToPreferred()
{
	fLayout->ELayoutItem::ResizeToPreferred();
}


void
EView::GetClippingRegion(ERegion *clipping) const
{
	if(clipping == NULL) return;

	*clipping = *(fLayout->VisibleRegion());
	if(((EViewState*)fStates)->Clipping != NULL) *clipping &= *(((EViewState*)fStates)->Clipping);
}


void
EView::ConstrainClippingRegion(const ERegion *_clipping)
{
	if(_clipping == NULL)
	{
		if(((EViewState*)fStates)->Clipping == NULL) return;

		delete ((EViewState*)fStates)->Clipping;
		((EViewState*)fStates)->Clipping = NULL;

		ERegion clipping(*(fLayout->VisibleRegion()));
		if(fClippingTemp.CountRects() > 0) clipping &= fClippingTemp;
		ConvertToWindow(&clipping);
		fDC->SetClipping(clipping);
	}
	else
	{
		if(((EViewState*)fStates)->Clipping == NULL)
			((EViewState*)fStates)->Clipping = new ERegion(*_clipping);
		else
			*(((EViewState*)fStates)->Clipping) = *_clipping;

		ERegion clipping(*(fLayout->VisibleRegion()));
		clipping &= *_clipping;
		if(fClippingTemp.CountRects() > 0) clipping &= fClippingTemp;
		ConvertToWindow(&clipping);
		fDC->SetClipping(clipping);
	}
}


void
EView::ConstrainClippingRegion(ERect clipping)
{
	ERegion aRegion(clipping);
	ConstrainClippingRegion(&aRegion);
}


e_status_t
EView::GetMouse(EPoint *location, eint32 *buttons, bool checkMessageQueue)
{
	EWindow *win = Window();
	if(win == NULL || location == NULL || buttons == NULL) return E_ERROR;

	if(checkMessageQueue)
	{
		// TODO
		return E_ERROR;
	}

	if(win->fWindow == NULL)
	{
		// TODO
		return E_ERROR;
	}

	eint32 mx, my;
	if(win->fWindow->QueryMouse(&mx, &my, buttons) != E_OK) return E_ERROR;
	location->x = (float)mx;
	location->y = (float)my;
	ConvertFromWindow(location);

	return E_OK;
}


bool
EView::QueryCurrentMouse(bool pushed, eint32 buttons, bool btnsAlone, eint32 *clicks) const
{
	EWindow *win = Window();
	if(win == NULL) return false;

	EMessage *msg = win->CurrentMessage();
	if(msg == NULL) return false;
	if(pushed && msg->what != E_MOUSE_DOWN) return false;
	if(pushed == false && msg->what != E_MOUSE_UP) return false;

	eint32 btns;
	if(msg->FindInt32("buttons", &btns) == false) return false;

	if(clicks)
	{
		if(msg->FindInt32("clicks", clicks) == false) *clicks = 1;
	}

	return(btnsAlone ? (buttons == btns) : (btns >= buttons));
}


void
EView::_UpdateVisibleRegion()
{
	if(!(e_is_kind_of(Parent(), EScrollView) == false || e_cast_as(Parent(), EScrollView)->fTarget != this))
	{
		ERegion *region;
		e_cast_as(fLayout, EViewLayout)->_GetVisibleRegion(&region);
		*region &= ConvertFromParent(e_cast_as(Parent(), EScrollView)->TargetValidFrame());
	}

	ERegion clipping(*(fLayout->VisibleRegion()));
	if(((EViewState*)fStates)->Clipping != NULL) clipping &= *(((EViewState*)fStates)->Clipping);
	if(fClippingTemp.CountRects() > 0) clipping &= fClippingTemp;
	ConvertToWindow(&clipping);
	fDC->SetClipping(clipping);
}


bool
EView::IsPrinting() const
{
	// TODO
	return false;
}


float
EView::UnitsPerPixel() const
{
	return(fLayout->Container() != NULL ? fLayout->Container()->UnitsPerPixel() : 1);
}


void
EView::SetEnabled(bool state)
{
	if(e_cast_as(fLayout, EViewLayout)->IsEnabled() != state)
	{
		e_cast_as(fLayout, EViewLayout)->SetEnabled(state);
		if(Flags() & E_WILL_DRAW) Invalidate();
	}
}


bool
EView::IsEnabled() const
{
	return e_cast_as(fLayout, EViewLayout)->IsEnabled();
}


void
EView::ScrollBy(float dh, float dv)
{
	ScrollTo(EPoint(dh, dv) + fLayout->LeftTop());
}


void
EView::ScrollTo(float x, float y)
{
	ScrollTo(EPoint(x, y));
}


void
EView::ScrollTo(EPoint where)
{
	if(LeftTop() != where)
	{
		fLayout->ScrollTo(where);

		fScrollTimeStamp = e_real_time_clock_usecs();
		Invalidate(Bounds(), true);

		for(eint32 i = 0; i < fScrollBar.CountItems(); i++)
		{
			EScrollBar *scrollbar = (EScrollBar*)fScrollBar.ItemAt(i);
			scrollbar->_SetValue(scrollbar->Orientation() == E_HORIZONTAL ? LeftTop().x : LeftTop().y, false);
		}
	}
}


void
EView::SetSquarePointStyle(bool state)
{
	if(((EViewState*)fStates)->SquarePointStyle != state)
	{
		if(fDC->SetSquarePointStyle(state) == E_OK) ((EViewState*)fStates)->SquarePointStyle = state;
	}
}


bool
EView::IsSquarePointStyle() const
{
	return ((EViewState*)fStates)->SquarePointStyle;
}


void
EView::DrawBitmap(const EBitmap *bitmap)
{
	DrawBitmap(bitmap, PenLocation());
}


void
EView::DrawBitmap(const EBitmap *bitmap, EPoint where)
{
	if(bitmap == NULL) return;

	ERect r = bitmap->Bounds();
	DrawBitmap(bitmap, r, r.OffsetToCopy(where));
}


void
EView::DrawBitmap(const EBitmap *bitmap, ERect destRect)
{
	if(bitmap == NULL) return;

	ERect r = bitmap->Bounds();
	DrawBitmap(bitmap, r, destRect);
}


void
EView::DrawBitmap(const EBitmap *bitmap, ERect srcRect, ERect destRect)
{
	if(bitmap == NULL || bitmap->fPixmap == NULL ||
	   srcRect.IsValid() == false || bitmap->Bounds().Intersects(srcRect) == false || destRect.IsValid() == false ||
	   fLayout->VisibleRegion()->Intersects(destRect) == false) return;

	ConvertToWindow(&destRect);
	ERect updateRect(destRect.InsetByCopy(PenSize() / -2.f, PenSize() / -2.f));

	srcRect.Floor();
	destRect.Floor();
	if(bitmap->fPixmap->CopyTo(fDC, Window()->fPixmap,
				   (eint32)srcRect.left, (eint32)srcRect.top,
				   (euint32)srcRect.Width(), (euint32)srcRect.Height(),
				   (eint32)destRect.left, (eint32)destRect.top,
				   (euint32)destRect.Width(), (euint32)destRect.Height()) == E_OK) Window()->_Update(updateRect, false);
}


void
EView::CopyBits(ERect srcRect, ERect destRect)
{
	if(srcRect.IsValid() == false || destRect.IsValid() == false ||
	   fLayout->VisibleRegion()->Intersects(destRect) == false) return;

	ConvertToWindow(&srcRect);
	ConvertToWindow(&destRect);
	ERect updateRect(destRect.InsetByCopy(PenSize() / -2.f, PenSize() / -2.f));

	srcRect.Floor();
	destRect.Floor();
	if(Window()->fPixmap->CopyTo(fDC, Window()->fPixmap,
				     (eint32)srcRect.left, (eint32)srcRect.top,
				     (euint32)srcRect.Width(), (euint32)srcRect.Height(),
				     (eint32)destRect.left, (eint32)destRect.top,
				     (euint32)destRect.Width(), (euint32)destRect.Height()) == E_OK) Window()->_Update(updateRect, false);
}


void
EView::Flush() const
{
	// nothing yet
}


void
EView::Sync() const
{
	// nothing yet
}

