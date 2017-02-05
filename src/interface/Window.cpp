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
 * File: Window.cpp
 *
 * --------------------------------------------------------------------------*/

#include <math.h>

#include "./../support/SupportDefs.h"
#include "./../support/ClassInfo.h"
#include "./../support/String.h"
#include "./../kernel/Kernel.h"
#include "./../app/Application.h"
#include "./../support/Autolock.h"
#include "./../add-ons/graphics/GraphicsEngine.h"

#include "InterfaceDefs.h"
#include "View.h"
#include "ScrollBar.h"
#include "Window.h"
#include "Screen.h"
#include "layout/Layout.h"


class _LOCAL EWindowLayoutItem : public ELayoutItem {
public:
	EWindowLayoutItem(ERect frame);
	virtual ~EWindowLayoutItem();

	virtual void	Invalidate(ERect rect);
};


class _LOCAL EWindowLayoutContainer : public ELayoutContainer {
public:
	EWindowLayoutContainer(EWindow *win, ERect frame);
	virtual ~EWindowLayoutContainer();

	void		MoveTo(EPoint where);
	void		ResizeTo(float width, float height);

	EPoint		Origin() const;
	ELayoutItem	*TopItem() const;

	virtual void	Invalidate(ERect rect);

private:
	EWindow *fWindow;
	EPoint fOrigin;
	ELayoutItem *fTopItem;
};


EWindowLayoutItem::EWindowLayoutItem(ERect frame)
	: ELayoutItem(frame, E_FOLLOW_NONE)
{
}


EWindowLayoutItem::~EWindowLayoutItem()
{
}


void
EWindowLayoutItem::Invalidate(ERect rect)
{
	if(Container() == NULL) return;
	rect.OffsetTo(ConvertToContainer(rect.LeftTop()));
	Container()->Invalidate(rect);
}


EWindowLayoutContainer::EWindowLayoutContainer(EWindow *win, ERect frame)
	: ELayoutContainer(), fWindow(NULL)
{
	fOrigin = frame.LeftTop();
	fTopItem = new EWindowLayoutItem(frame.OffsetToSelf(E_ORIGIN));
	fTopItem->Hide();
	AddItem(fTopItem);

	fWindow = win;
}


EWindowLayoutContainer::~EWindowLayoutContainer()
{
}


void
EWindowLayoutContainer::MoveTo(EPoint where)
{
	fOrigin = where;
}


void
EWindowLayoutContainer::ResizeTo(float width, float height)
{
	fTopItem->ResizeTo(width, height);
}


EPoint
EWindowLayoutContainer::Origin() const
{
	return fOrigin;
}


ELayoutItem*
EWindowLayoutContainer::TopItem() const
{
	return fTopItem;
}


void
EWindowLayoutContainer::Invalidate(ERect rect)
{
	if(fWindow == NULL) return;
	rect.OffsetTo(E_ORIGIN);
	fWindow->Invalidate(rect, true);
}


void
EWindow::InitSelf(ERect frame, const char *title, e_window_look look, e_window_feel feel, euint32 flags, euint32 workspace)
{
	if(etk_app == NULL || etk_app->fGraphicsEngine == NULL)
		ETK_ERROR("[INTERFACE]: Window must created within a application which has graphics-engine!");

#ifdef ETK_ENABLE_DEBUG
	EString winLooperName;
	winLooperName << "Window " << etk_get_handler_token(this);
	SetName(winLooperName.String());
#endif // ETK_ENABLE_DEBUG

	fLayout = new EWindowLayoutContainer(this, frame);

	frame.Floor();
	if((fWindow = etk_app->fGraphicsEngine->CreateWindow((eint32)frame.left, (eint32)frame.top,
							     (euint32)max_c(frame.Width(), 0),
							     (euint32)max_c(frame.Height(), 0))) == NULL)
		ETK_ERROR("[INTERFACE]: %s --- Unable to create window!", __PRETTY_FUNCTION__);
	else if((fPixmap = etk_app->fGraphicsEngine->CreatePixmap((euint32)max_c(frame.Width(), 0),
								  (euint32)max_c(frame.Height(), 0))) == NULL)
		ETK_ERROR("[INTERFACE]: %s --- Unable to create pixmap!", __PRETTY_FUNCTION__);
	else if((fDC = etk_app->fGraphicsEngine->CreateContext()) == NULL)
		ETK_ERROR("[INTERFACE]: %s --- Unable to create graphics context!", __PRETTY_FUNCTION__);

	fDC->SetClipping(ERegion(frame.OffsetToCopy(E_ORIGIN)));
	fDC->SetDrawingMode(E_OP_COPY);
	fDC->SetPattern(E_SOLID_HIGH);
	fDC->SetHighColor(e_ui_color(E_PANEL_BACKGROUND_COLOR));
	fDC->SetPenSize(0);

	fWindowFlags = flags;
	fWindowLook = look;
	fWindowFeel = feel;
	fWindowTitle = title ? EStrdup(title) : NULL;
	fWindow->SetFlags(fWindowFlags);
	fWindow->SetLook(fWindowLook);
	fWindow->SetFeel(fWindowFeel);
	fWindow->SetBackgroundColor(fDC->HighColor());
	fWindow->SetTitle(fWindowTitle);

	EMessenger msgrSelf(this);
	EMessage pulseMsg(E_PULSE);
	fPulseRate = 500000;
	fPulseRunner = new EMessageRunner(msgrSelf, &pulseMsg, fPulseRate, 0);

	fFocus = NULL;
	fUpdateHolderThreadId = 0;
	fUpdateHolderCount = E_INT64_CONSTANT(-1);
	fInUpdate = false;
	fMinimized = false;
	fActivated = false;
	fActivatedTimeStamp = 0;
	fPositionChangedTimeStamp = 0;
	fSizeChangedTimeStamp = 0;
	fMouseGrabCount = 0;
	fKeyboardGrabCount = 0;
	fBrokeOnExpose = false;
	fWindowWorkspaces = 0;

	fWindow->ContactTo(&msgrSelf);

	SetWorkspaces(workspace);
}


EWindow::EWindow(ERect frame, const char *title,
		 e_window_type type,
		 euint32 flags, euint32 workspace)
	: ELooper(NULL, E_DISPLAY_PRIORITY)
{
	e_window_look look;
	e_window_feel feel;

	switch(type)
	{
		case E_TITLED_WINDOW:
			look = E_TITLED_WINDOW_LOOK;
			feel = E_NORMAL_WINDOW_FEEL;
			break;

		case E_MODAL_WINDOW:
			look = E_MODAL_WINDOW_LOOK;
			feel = E_MODAL_APP_WINDOW_FEEL;
			break;

		case E_DOCUMENT_WINDOW:
			look = E_DOCUMENT_WINDOW_LOOK;
			feel = E_NORMAL_WINDOW_FEEL;
			break;

		case E_BORDERED_WINDOW:
			look = E_BORDERED_WINDOW_LOOK;
			feel = E_NORMAL_WINDOW_FEEL;
			break;

		case E_FLOATING_WINDOW:
			look = E_FLOATING_WINDOW_LOOK;
			feel = E_FLOATING_APP_WINDOW_FEEL;
			break;

		default:
			look = E_TITLED_WINDOW_LOOK;
			feel = E_NORMAL_WINDOW_FEEL;
	}

	InitSelf(frame, title, look, feel, flags, workspace);
}


EWindow::EWindow(ERect frame, const char *title,
		 e_window_look look, e_window_feel feel,
		 euint32 flags, euint32 workspace)
	: ELooper(NULL, E_DISPLAY_PRIORITY)
{
	InitSelf(frame, title, look, feel, flags, workspace);
}


EWindow::~EWindow()
{
	Hide();

	EView *child = NULL;
	while((child = ChildAt(CountChildren() - 1)) != NULL)
	{
		RemoveChild(child);
		delete child;
	}

	if(fWindow) delete fWindow;

	delete fPixmap;
	delete fDC;
	delete fLayout;
	delete fPulseRunner;
	if(fWindowTitle) delete[] fWindowTitle;
}


void
EWindow::DispatchMessage(EMessage *msg, EHandler *target)
{
	if(target == NULL) target = PreferredHandler();
	if(target == NULL || target->Looper() != this) return;

	if(target != this)
	{
		ELooper::DispatchMessage(msg, target);
		return;
	}

	bool sendNotices = true;

	msg->RemoveBool("etk:msg_from_gui");
	switch(msg->what)
	{
		case E_PULSE:
			if(IsHidden()) break;
			for(eint32 i = 0; i < fNeededToPulseViews.CountItems(); i++)
			{
				EView *view = (EView*)fNeededToPulseViews.ItemAt(i);
				if(view->IsHidden() == false) PostMessage(msg, view);
			}
			break;

		case E_MOUSE_DOWN:
		case E_MOUSE_UP:
		case E_MOUSE_MOVED:
		case E_MOUSE_WHEEL_CHANGED:
			{
				EPoint where;
				if(msg->FindPoint("where", &where) == false && msg->what != E_MOUSE_WHEEL_CHANGED)
				{
					if(msg->FindPoint("screen_where", &where) == false)
					{
						ETK_DEBUG("[INTERFACE]: %s --- Invalid message.", __PRETTY_FUNCTION__);
						break;
					}
					ConvertFromScreen(&where);
					msg->AddPoint("where", where);
				}

				EMessage aMsg(*msg);

				if(msg->what != E_MOUSE_WHEEL_CHANGED)
				{
					euint32 saveWhat = aMsg.what;
					aMsg.what = E_MOUSE_MOVED;
					for(eint32 i = 0; i < fMouseInsideViews.CountItems(); i++)
					{
						EView *view = (EView*)fMouseInsideViews.ItemAt(i);

						EPoint pt = view->ConvertFromWindow(where);
						if(view->fLayout->VisibleRegion()->Contains(pt)) continue;
						if(view->EventMask() & E_POINTER_EVENTS) continue;

						aMsg.ReplacePoint("where", pt);
						PostMessage(&aMsg, view);
					}
					aMsg.what = saveWhat;
				}

				for(EView *view = ChildAt(0); view != NULL; view = view->NextSibling())
				{
					EPoint pt = view->fLayout->ConvertFromContainer(where);
					if(view->fLayout->VisibleRegion()->Contains(pt) == false) continue;

					if(!(view->EventMask() & E_POINTER_EVENTS))
					{
						aMsg.ReplacePoint("where", pt);
						PostMessage(&aMsg, view);
					}

					break; // just one child can receive the message
				}

				for(eint32 i = 0; i < fMouseInterestedViews.CountItems(); i++)
				{
					EView *view = (EView*)fMouseInterestedViews.ItemAt(i);

					aMsg.ReplacePoint("where", view->ConvertFromWindow(where));
					PostMessage(&aMsg, view);
				}
			}
			break;

		case E_UNMAPPED_KEY_DOWN:
		case E_UNMAPPED_KEY_UP:
		case E_KEY_DOWN:
		case E_KEY_UP:
		case E_MODIFIERS_CHANGED:
			{
				// TODO: shortcuts
				for(eint32 i = -1; i < fKeyboardInterestedViews.CountItems(); i++)
				{
					EView *view = i < 0 ? CurrentFocus() : (EView*)fKeyboardInterestedViews.ItemAt(i);
					if((i < 0 && view == NULL) || (i >= 0 && view == CurrentFocus())) continue;
					PostMessage(msg, view);
				}
			}
			break;

		case E_WORKSPACES_CHANGED:
			{
				euint32 curWorkspace;

				if(msg->FindInt32("new", (eint32*)&curWorkspace) == false) break;
				if(curWorkspace != 0 && fWindowWorkspaces != curWorkspace)
				{
					euint32 oldWorkspace = fWindowWorkspaces;
					fWindowWorkspaces = curWorkspace;
					if(oldWorkspace != 0) WorkspacesChanged(oldWorkspace, curWorkspace);
				}
			}
			break;

		case E_WINDOW_ACTIVATED:
			{
				e_bigtime_t when;
				bool active = fActivated;

				if(msg->FindInt64("when", (eint64*)&when) == false) break;
				if(!(fWindow == NULL || fWindow->GetActivatedState(&active) == E_OK)) break;

				if(fActivated != active && fActivatedTimeStamp <= when)
				{
					fActivated = active;
					fActivatedTimeStamp = when;
					if((active && !(fWindowFlags & E_AVOID_FRONT)) && fWindow) fWindow->Raise();
					WindowActivated(active);
					for(EView *view = ChildAt(0); view != NULL; view = view->NextSibling())
					{
						PostMessage(msg, view);
					}
				}
			}
			break;

		case E_WINDOW_MOVED:
		case E_WINDOW_RESIZED:
			{
				e_bigtime_t when;
				if(msg->FindInt64("when", &when) == false) break;
				if(msg->what == E_WINDOW_MOVED && when < fPositionChangedTimeStamp) break;
				if(msg->what == E_WINDOW_RESIZED && when < fSizeChangedTimeStamp) break;

				ERect frame = Frame();
				EPoint where = frame.LeftTop();
				float w = frame.Width();
				float h = frame.Height();

				if(msg->what == E_WINDOW_RESIZED)
				{
					if(msg->FindFloat("width", &w) == false || msg->FindFloat("height", &h) == false) break;
					msg->FindPoint("where", &where);
				}
				else // E_WINDOW_MOVED
				{
					if(msg->FindPoint("where", &where) == false) break;
				}

				bool doMoved = frame.LeftTop() != where;
				bool doResized = (frame.Width() != w || frame.Height() != h);

				if(CurrentMessage() == msg)
				{
					MessageQueue()->Lock();
					while(MessageQueue()->IsEmpty() == false)
					{
						EMessage *aMsg = MessageQueue()->FindMessage((eint32)0);
						if(aMsg == NULL) break;

						if(!(aMsg->what == E_WINDOW_RESIZED || aMsg->what == E_WINDOW_MOVED))
						{
							if(aMsg->what == _UPDATE_ || aMsg->what == _UPDATE_IF_NEEDED_)
							{
								if(!doResized) break;
								MessageQueue()->RemoveMessage(aMsg);
								continue;
							}
							break;
						}

						if(aMsg->what == E_WINDOW_RESIZED)
						{
							float w1, h1;
							e_bigtime_t nextWhen;
							if(aMsg->FindFloat("width", &w1) == false ||
							   aMsg->FindFloat("height", &h1) == false ||
							   aMsg->FindInt64("when", &nextWhen) == false ||
							   nextWhen < when)
							{
								MessageQueue()->RemoveMessage(aMsg);
								continue;
							}
							w = w1; h = h1; when = nextWhen;
							aMsg->FindPoint("where", &where);
						}
						else // E_WINDOW_MOVED
						{
							e_bigtime_t nextWhen;
							if(aMsg->FindInt64("when", &nextWhen) == false ||
							   nextWhen < when ||
							   aMsg->FindPoint("where", &where) == false)
							{
								MessageQueue()->RemoveMessage(aMsg);
								continue;
							}
							when = nextWhen;
						}

						if(frame.LeftTop() != where) doMoved = true;
						if(frame.Width() != w || frame.Height() != h) doResized = true;

						MessageQueue()->RemoveMessage(aMsg);
					}
					MessageQueue()->Unlock();
				}

				if(doMoved)
				{
					fPositionChangedTimeStamp = when;
					e_cast_as(fLayout, EWindowLayoutContainer)->MoveTo(where);
				}

				if(doResized)
				{
					fSizeChangedTimeStamp = when;

					ERect rFrame = frame;
					rFrame.right = rFrame.left + w;
					rFrame.bottom = rFrame.top + h;
					rFrame.Floor();
					fPixmap->ResizeTo((euint32)max_c(rFrame.Width(), 0), (euint32)max_c(rFrame.Height(), 0));
					fDC->SetClipping(ERegion(rFrame.OffsetToCopy(E_ORIGIN)));

					fExposeRect = Bounds();
					fBrokeOnExpose = false;
					if(fInUpdate == false) PostMessage(_UPDATE_IF_NEEDED_, this);

					// for disable update
					bool saveInUpdate = fInUpdate;
					fInUpdate = true;
					e_cast_as(fLayout, EWindowLayoutContainer)->ResizeTo(w, h);
					fInUpdate = saveInUpdate;
				}
				else if(fBrokeOnExpose)
				{
					fBrokeOnExpose = false;
					if(fInUpdate == false) PostMessage(_UPDATE_IF_NEEDED_, this);
				}

				sendNotices = false;

				frame = Frame();
				if(doMoved)
				{
					FrameMoved(frame.LeftTop());
					if(IsWatched(E_WINDOW_MOVED))
					{
						EMessage aMsg(E_WINDOW_MOVED);
						aMsg.AddInt64("when", when);
						aMsg.AddPoint("where", frame.LeftTop());
						SendNotices(E_WINDOW_MOVED, &aMsg);
					}
				}
				if(doResized)
				{
					FrameResized(frame.Width(), frame.Height());
					if(IsWatched(E_WINDOW_RESIZED))
					{
						EMessage aMsg(E_WINDOW_RESIZED);
						aMsg.AddInt64("when", when);
						aMsg.AddFloat("width", frame.Width());
						aMsg.AddFloat("height", frame.Height());
						SendNotices(E_WINDOW_RESIZED, &aMsg);
					}
				}
			}
			break;

		case E_MINIMIZE:
		case E_MINIMIZED:
			{
				bool minimize;
				if(msg->FindBool("minimize", &minimize) == false) break;
				Minimize(minimize);
			}
			break;

		case _UPDATE_IF_NEEDED_:
			{
				sendNotices = false;

				e_bigtime_t when = e_real_time_clock_usecs();
				msg->FindInt64("when", (eint64*)&when);

				if(CurrentMessage() == msg)
				{
					bool noNeededToUpdate = false;
					EMessage *aMsg = NULL;

					MessageQueue()->Lock();
					if((aMsg = MessageQueue()->FindMessage((eint32)0)) != NULL)
					{
						if(aMsg->what == _UPDATE_IF_NEEDED_)
						{
							// Here we don't need to update until the next event
							noNeededToUpdate = true;
						}
						else if(aMsg->what == _UPDATE_)
						{
							// Here we don't need to update because of
							// that it's a expose event next to handle, and
							// probably within the short time for switching
							// another or more expose events will need to be handle.
							noNeededToUpdate = true;
						}
					}
					MessageQueue()->Unlock();
					if(noNeededToUpdate) break;
				}

				_UpdateIfNeeded(when);
			}
			break;

		case _UPDATE_: // TODO: speed up
			{
				sendNotices = false;

				ERect rect;
				if(msg->FindRect("etk:frame", &rect))
				{
					bool expose = false;
					msg->FindBool("etk:expose", &expose);

					rect &= Bounds();
					if(rect.IsValid())
					{
						if(expose) fExposeRect |= rect;
						else fUpdateRect |= rect;
					}
				}

				if(CurrentMessage() == msg)
				{
					bool noNeededToSendUpdate = false;
					EMessage *aMsg = NULL;
					MessageQueue()->Lock();
					if((aMsg = MessageQueue()->FindMessage((eint32)0)) != NULL)
					{
						if(aMsg->what == _UPDATE_IF_NEEDED_)
						{
							// Here we don't need to post _UPDATE_IF_NEEDED_
							noNeededToSendUpdate = true;
						}
						else if(aMsg->what == _UPDATE_)
						{
							// Here we don't post _UPDATE_IF_NEEDED_ because of
							// that it's a expose event next to handle, and
							// probably within the short time for switching
							// another or more expose events will need to be handle.
							noNeededToSendUpdate = true;
						}
					}
					MessageQueue()->Unlock();
					if(noNeededToSendUpdate) break;
				}

				if(fInUpdate == false) PostMessage(_UPDATE_IF_NEEDED_, this);
			}
			break;

		default:
			sendNotices = false;
			ELooper::DispatchMessage(msg, target);
	}

	if(sendNotices && IsWatched(msg->what)) SendNotices(msg->what, msg);
}


void
EWindow::Quit()
{
	if(!IsLockedByCurrentThread())
		ETK_ERROR("[INTERFACE]: %s --- Window must LOCKED before \"Quit()\" call!", __PRETTY_FUNCTION__);

	if(fWindow)
	{
		fWindow->Hide();
		fWindow->ContactTo(NULL);
	}

	if(fWindowFlags & E_QUIT_ON_WINDOW_CLOSE) etk_app->PostMessage(E_QUIT_REQUESTED);

	ELooper::Quit();
}


void
EWindow::Show()
{
	if(e_cast_as(fLayout, EWindowLayoutContainer)->TopItem()->IsHidden(false) == false) return;

	e_cast_as(fLayout, EWindowLayoutContainer)->TopItem()->Show();

	fMinimized = false;
	if(fWindow) fWindow->Show();

	if(fPulseRunner)
		fPulseRunner->SetCount((fPulseRate > 0 && fNeededToPulseViews.CountItems() > 0) ? -1 : 0);

	if(!(IsRunning() || Proxy() != this)) Run();

	if(fWindowFeel == E_MODAL_APP_WINDOW_FEEL)
	{
		EMessenger msgrSelf(this);
		etk_app->AddModalWindow(msgrSelf);
	}
}


void
EWindow::Hide()
{
	if(e_cast_as(fLayout, EWindowLayoutContainer)->TopItem()->IsHidden(false)) return;

	if(fPulseRunner) fPulseRunner->SetCount(0);

	if(fWindowFeel == E_MODAL_APP_WINDOW_FEEL)
	{
		EMessenger msgrSelf(this);
		etk_app->RemoveModalWindow(msgrSelf);
	}

	if(fMouseGrabCount > 0)
	{
		if(fWindow) fWindow->UngrabMouse();
		fMouseGrabCount = 0;

		for(eint32 i = 0; i < fMouseInterestedViews.CountItems(); i++)
		{
			EView *view = (EView*)fMouseInterestedViews.ItemAt(i);
			view->fMouseGrabbed = false;
		}
	}

	if(fKeyboardGrabCount > 0)
	{
		if(fWindow) fWindow->UngrabKeyboard();
		fKeyboardGrabCount = 0;

		for(eint32 i = 0; i < fKeyboardInterestedViews.CountItems(); i++)
		{
			EView *view = (EView*)fKeyboardInterestedViews.ItemAt(i);
			view->fKeyboardGrabbed = false;
		}
	}

	if(fWindow) fWindow->Hide();

	fMinimized = false;
	fBrokeOnExpose = false;

	if(IsWatched(E_MINIMIZED))
	{
		EMessage aMsg(E_MINIMIZED);
		aMsg.AddInt64("when", e_real_time_clock_usecs());
		aMsg.AddBool("minimize", false);
		SendNotices(E_MINIMIZED, &aMsg);
	}

	e_cast_as(fLayout, EWindowLayoutContainer)->TopItem()->Hide();
}


bool
EWindow::IsHidden() const
{
	return e_cast_as(fLayout, EWindowLayoutContainer)->TopItem()->IsHidden();
}


bool
EWindow::IsMinimized() const
{
	if(IsHidden()) return false;
	if(fMinimized) return true;
	return false;
}


void
EWindow::AddViewChildrenToHandlersList(EWindow *win, EView *child)
{
	if(win == NULL || child == NULL) return;
	for(EView *view = child->ChildAt(0); view != NULL; view = view->NextSibling())
	{
		win->AddHandler(view);

		if(view->Looper() != win)
		{
			ETK_WARNING("[INTERFACE]: %s --- Add child of the view added by \"AddChild()\" failed.", __PRETTY_FUNCTION__);
			continue;
		}

		view->AttachToWindow();
		view->AttachedToWindow();

		AddViewChildrenToHandlersList(win, view);
		view->AllAttached();
	}
}


void
EWindow::RemoveViewChildrenFromHandlersList(EWindow *win, EView *child)
{
	if(win == NULL || child == NULL || child->Looper() != win) return;
	for(EView *view = child->ChildAt(0); view != NULL; view = view->NextSibling())
	{
		RemoveViewChildrenFromHandlersList(win, view);
		view->AllDetached();

		view->DetachedFromWindow();

		view->DetachFromWindow();
		win->RemoveHandler(view);
	}
}


void
EWindow::AddChild(EView *child, EView *nextSibling)
{
	if(child == NULL || child->Looper() != NULL || child->Parent() != NULL ||
	   (nextSibling == NULL ? false : (nextSibling->Looper() != this || nextSibling->Parent() != NULL)))
	{
		ETK_WARNING("[INTERFACE]: %s --- Unable to add child.", __PRETTY_FUNCTION__);
		return;
	}

	AddHandler(child);
	if(child->Looper() != this)
	{
		ETK_WARNING("[INTERFACE]: %s --- Unable to attach child to window, abort to add child.", __PRETTY_FUNCTION__);
		return;
	}

	ELayoutItem *topItem = e_cast_as(fLayout, EWindowLayoutContainer)->TopItem();
	if(topItem->AddItem(child->fLayout, nextSibling == NULL ? -1 : topItem->IndexOf(nextSibling->fLayout)) == false)
	{
		RemoveHandler(child);
		ETK_WARNING("[INTERFACE]: %s --- Unable to add child to layout.", __PRETTY_FUNCTION__);
		return;
	}

	child->AttachToWindow();
	child->AttachedToWindow();

	AddViewChildrenToHandlersList(this, child);
	child->AllAttached();
}


bool
EWindow::RemoveChild(EView *child)
{
	if(child == NULL || child->Looper() != this || child->Parent() != NULL) return false;

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

	RemoveViewChildrenFromHandlersList(this, child);
	child->AllDetached();

	child->DetachedFromWindow();

	child->DetachFromWindow();
	RemoveHandler(child);

	e_cast_as(fLayout, EWindowLayoutContainer)->TopItem()->RemoveItem(child->fLayout);

	return true;
}


eint32
EWindow::CountChildren() const
{
	return e_cast_as(fLayout, EWindowLayoutContainer)->TopItem()->CountItems();
}


EView*
EWindow::ChildAt(eint32 index) const
{
	ELayoutItem *topItem = e_cast_as(fLayout, EWindowLayoutContainer)->TopItem();
	return(topItem->ItemAt(index) != NULL ? (EView*)topItem->ItemAt(index)->PrivateData() : NULL);
}


void
EWindow::ConvertToScreen(EPoint* pt) const
{
	if(!pt) return;
	*pt += e_cast_as(fLayout, EWindowLayoutContainer)->Origin();
}


EPoint
EWindow::ConvertToScreen(EPoint pt) const
{
	EPoint pt1 = pt;
	ConvertToScreen(&pt1);
	return pt1;
}


void
EWindow::ConvertFromScreen(EPoint* pt) const
{
	if(!pt) return;
	*pt -= e_cast_as(fLayout, EWindowLayoutContainer)->Origin();
}


EPoint
EWindow::ConvertFromScreen(EPoint pt) const
{
	EPoint pt1 = pt;
	ConvertFromScreen(&pt1);
	return pt1;
}


void
EWindow::ConvertToScreen(ERect *r) const
{
	if(!r) return;
	EPoint pt = ConvertToScreen(r->LeftTop());
	r->OffsetTo(pt);
}


ERect
EWindow::ConvertToScreen(ERect r) const
{
	ERect rect = r;
	ConvertToScreen(&rect);
	return rect;
}


void
EWindow::ConvertFromScreen(ERect *r) const
{
	if(!r) return;
	EPoint pt = ConvertFromScreen(E_ORIGIN);
	r->OffsetBy(pt);
}


ERect
EWindow::ConvertFromScreen(ERect r) const
{
	ERect rect = r;
	ConvertFromScreen(&rect);
	return rect;
}


void
EWindow::ConvertToScreen(ERegion *region) const
{
	if(!region || region->CountRects() <= 0) return;
	EPoint pt = ConvertToScreen(region->Frame().LeftTop());
	region->OffsetBy(pt - region->Frame().LeftTop());
}


ERegion
EWindow::ConvertToScreen(const ERegion &region) const
{
	ERegion aRegion(region);
	ConvertToScreen(&aRegion);
	return aRegion;
}


void
EWindow::ConvertFromScreen(ERegion *region) const
{
	if(!region || region->CountRects() <= 0) return;
	EPoint pt = ConvertFromScreen(E_ORIGIN);
	region->OffsetBy(pt);
}


ERegion
EWindow::ConvertFromScreen(const ERegion &region) const
{
	ERegion aRegion(region);
	ConvertFromScreen(&aRegion);
	return aRegion;
}


void
EWindow::FrameMoved(EPoint new_position)
{
}


void
EWindow::WorkspacesChanged(euint32 old_ws, euint32 new_ws)
{
}


void
EWindow::WorkspaceActivated(eint32 ws, bool state)
{
}


void
EWindow::FrameResized(float new_width, float new_height)
{
}


void
EWindow::Minimize(bool minimize)
{
	if(minimize)
	{
		if(fMouseGrabCount > 0)
		{
			if(fWindow) fWindow->UngrabMouse();
			fMouseGrabCount = 0;

			for(eint32 i = 0; i < fMouseInterestedViews.CountItems(); i++)
			{
				EView *view = (EView*)fMouseInterestedViews.ItemAt(i);
				view->fMouseGrabbed = false;
			}
		}

		if(fKeyboardGrabCount > 0)
		{
			if(fWindow) fWindow->UngrabKeyboard();
			fKeyboardGrabCount = 0;

			for(eint32 i = 0; i < fKeyboardInterestedViews.CountItems(); i++)
			{
				EView *view = (EView*)fKeyboardInterestedViews.ItemAt(i);
				view->fKeyboardGrabbed = false;
			}
		}
	}

	if(fMinimized == minimize) return;

	fMinimized = minimize;

	if(IsHidden() || fWindow == NULL) return;

	if(fMinimized)
		fWindow->Iconify();
	else
		fWindow->Show();
}


ERect
EWindow::Bounds() const
{
	return e_cast_as(fLayout, EWindowLayoutContainer)->TopItem()->Bounds();
}


ERect
EWindow::Frame() const
{
	ERect rect = e_cast_as(fLayout, EWindowLayoutContainer)->TopItem()->Frame();
	rect.OffsetTo(e_cast_as(fLayout, EWindowLayoutContainer)->Origin());
	return rect;
}


void
EWindow::Invalidate(ERect invalRect, bool redraw)
{
	if(IsHidden() || invalRect.IsValid() == false) return;

	if(redraw) fExposeRect |= invalRect;
	else fUpdateRect |= invalRect;

	if(fInUpdate == false)
	{
		if(fWindow == NULL)
		{
			// TODO
			UpdateIfNeeded();
		}
		else
		{
			PostMessage(_UPDATE_IF_NEEDED_, this);
		}
	}
}


void
EWindow::DisableUpdates()
{
	eint64 currentThread = etk_get_current_thread_id();

	if(fUpdateHolderThreadId != 0 && fUpdateHolderThreadId != currentThread)
		ETK_ERROR("[INTERFACE]: %s --- Invalid \"DisableUpdates()\" and \"EnableUpdates()\" call!", __PRETTY_FUNCTION__);

	if(fUpdateHolderThreadId == 0)
	{
		fUpdateHolderThreadId = currentThread;
		fUpdateHolderCount = 1;
	}
	else
	{
		if(E_MAXINT64 - 1 < fUpdateHolderCount)
			ETK_ERROR("[INTERFACE]: %s --- Call \"DisableUpdates()\" more than limited times!", __PRETTY_FUNCTION__);
		fUpdateHolderCount++;
	}
}


void
EWindow::EnableUpdates()
{
	eint64 currentThread = etk_get_current_thread_id();

	if(fUpdateHolderThreadId != 0 && fUpdateHolderThreadId != currentThread)
		ETK_ERROR("[INTERFACE]: %s --- Invalid \"DisableUpdates()\" and \"EnableUpdates()\" call!", __PRETTY_FUNCTION__);
	else if(fUpdateHolderThreadId == 0)
	{
		ETK_WARNING("[INTERFACE]: %s --- Please call \"DisableUpdates()\" before \"EnableUpdates()\"!", __PRETTY_FUNCTION__);
		return;
	}

	fUpdateHolderCount--;
	if(fUpdateHolderCount > 0) return;

	fUpdateHolderCount = 0;
	fUpdateHolderThreadId = 0;

	if(fWindow && fUpdateRect.IsValid() && !_HasResizeMessage(false))
	{
		fUpdateRect.Floor();
		fPixmap->CopyTo(fDC, fWindow,
				(eint32)fUpdateRect.left, (eint32)fUpdateRect.top,
				(euint32)fUpdateRect.Width(), (euint32)fUpdateRect.Height(),
				(eint32)fUpdateRect.left, (eint32)fUpdateRect.top,
				(euint32)fUpdateRect.Width(), (euint32)fUpdateRect.Height());
	}

	fUpdateRect = ERect();
}


bool
EWindow::NeedsUpdate() const
{
	return(fExposeRect.IsValid() || fUpdateRect.IsValid());
}


void
EWindow::_UpdateIfNeeded(e_bigtime_t when)
{
	if(_HasResizeMessage(false) || NeedsUpdate() == false) return;

	fBrokeOnExpose = false;
	ERect r = fExposeRect;
	if(r.IsValid())
	{
		bool saveInUpdate = fInUpdate;

		fExposeRect = ERect();

		fInUpdate = true;
		_Expose(r, when);
		if(fBrokeOnExpose)
		{
			fExposeRect |= r;
			fInUpdate = saveInUpdate;
			fBrokeOnExpose = false;
			if(fInUpdate == false) PostMessage(_UPDATE_IF_NEEDED_, this);
			return;
		}
		else if(fExposeRect.IsValid())
		{
			fUpdateRect |= r;
			fInUpdate = saveInUpdate;
			_UpdateIfNeeded(e_real_time_clock_usecs());
			return;
		}

		r |= fUpdateRect;

		fInUpdate = saveInUpdate;
	}
	else
	{
		r = fUpdateRect;
	}

	fUpdateRect = ERect();

	r &= Bounds();

	if(r.IsValid() == false || fWindow == NULL) return;

	r.Floor();
	fPixmap->CopyTo(fDC, fWindow,
			(eint32)r.left, (eint32)r.top, (euint32)r.Width(), (euint32)r.Height(),
			(eint32)r.left, (eint32)r.top, (euint32)r.Width(), (euint32)r.Height());
}


void
EWindow::UpdateIfNeeded()
{
	_UpdateIfNeeded(e_real_time_clock_usecs());
}


void
EWindow::_Update(ERect rect, bool force_update)
{
	if(rect.IsValid() == false) return;
	fUpdateRect |= rect;
	if(fInUpdate) return;
	if(fUpdateRect.IsValid() == false) return;
	if(fWindow && (force_update || fUpdateHolderThreadId == 0))
	{
		fUpdateRect.Floor();
		fPixmap->CopyTo(fDC, fWindow,
				(eint32)fUpdateRect.left, (eint32)fUpdateRect.top,
				(euint32)fUpdateRect.Width(), (euint32)fUpdateRect.Height(),
				(eint32)fUpdateRect.left, (eint32)fUpdateRect.top,
				(euint32)fUpdateRect.Width(), (euint32)fUpdateRect.Height());
	}
	fUpdateRect = ERect();
}


void
EWindow::SetBackgroundColor(e_rgb_color c)
{
	if(fDC->HighColor() != c)
	{
		if(fWindow) fWindow->SetBackgroundColor(c);
		fDC->SetHighColor(c);

		fExposeRect = Bounds();
		if(fInUpdate == false) PostMessage(_UPDATE_IF_NEEDED_, this);
	}
}


void
EWindow::SetBackgroundColor(euint8 r, euint8 g, euint8 b, euint8 a)
{
	e_rgb_color c;
	c.set_to(r, g, b, a);
	SetBackgroundColor(c);
}


e_rgb_color
EWindow::BackgroundColor() const
{
	return fDC->HighColor();
}


void
EWindow::_Expose(ERect rect, e_bigtime_t when)
{
	rect &= Bounds();
	if(rect.IsValid() == false) return;

	ERect r = rect.FloorCopy();
	fPixmap->FillRect(fDC, (eint32)r.left, (eint32)r.top, (euint32)r.Width(), (euint32)r.Height());

	ERegion region(rect);

	for(EView *child = ChildAt(0); child != NULL; child = child->NextSibling())
	{
		if(fBrokeOnExpose || _HasResizeMessage(true)) break;
		if(child->fLayout->VisibleRegion()->Intersects(child->ConvertFromParent(rect)) == false) continue;
		child->_Expose(child->ConvertFromParent(region), when);
	}
}


bool
EWindow::InUpdate() const
{
	return fInUpdate;
}


bool
EWindow::_HasResizeMessage(bool setBrokeOnExpose)
{
	bool retVal = false;
	ERect frame = Frame();

	MessageQueue()->Lock();
	EMessage *msg;
	eint32 fromIndex = 0;
	while(retVal == false && (msg = MessageQueue()->FindMessage(E_WINDOW_RESIZED, fromIndex, 20)) != NULL)
	{
		float w, h;
		if(msg->FindFloat("width", &w) == false || msg->FindFloat("height", &h) == false) break;
		fromIndex = MessageQueue()->IndexOfMessage(msg) + 1;
		retVal = frame.Width() != w || frame.Height() != h;
	}
	MessageQueue()->Unlock();

	if(retVal && setBrokeOnExpose) fBrokeOnExpose = true;

	return retVal;
}


void
EWindow::Activate(bool state)
{
	if(!(IsHidden() || fMinimized) || !state)
	{
		if(!(fWindow == NULL || fWindow->Activate(state) == E_OK))
		{
			ETK_DEBUG("[INTERFACE]: %s --- Unable to %s window.", __PRETTY_FUNCTION__, state ? "activate" : "inactivate");
			return;
		}

		fActivatedTimeStamp = e_real_time_clock_usecs();
		fActivated = state;
		if((state && !(fWindowFlags & E_AVOID_FRONT)) && fWindow) fWindow->Raise();
		WindowActivated(state);

		EMessage aMsg(E_WINDOW_ACTIVATED);
		aMsg.AddInt64("when", e_real_time_clock_usecs());
		for(EView *view = ChildAt(0); view != NULL; view = view->NextSibling()) PostMessage(&aMsg, view);
	}
}


bool
EWindow::IsActivate() const
{
	return fActivated;
}


void
EWindow::WindowActivated(bool state)
{
}


EView*
EWindow::FindView(const char *name) const
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


EView*
EWindow::FindView(EPoint where) const
{
	if(Bounds().Contains(where) == false) return NULL;

	for(EView *child = ChildAt(0); child != NULL; child = child->NextSibling())
	{
		if(child->fLayout->VisibleRegion()->Contains(child->fLayout->ConvertFromContainer(where))) return child;
	}

	return NULL;
}


EView*
EWindow::CurrentFocus() const
{
	return fFocus;
}


e_status_t
EWindow::SetType(e_window_type type)
{
	e_window_look look;
	e_window_feel feel;

	switch(type)
	{
		case E_MODAL_WINDOW:
			look = E_MODAL_WINDOW_LOOK;
			feel = E_MODAL_APP_WINDOW_FEEL;
			break;

		case E_DOCUMENT_WINDOW:
			look = E_DOCUMENT_WINDOW_LOOK;
			feel = E_NORMAL_WINDOW_FEEL;
			break;

		case E_BORDERED_WINDOW:
			look = E_BORDERED_WINDOW_LOOK;
			feel = E_NORMAL_WINDOW_FEEL;
			break;

		case E_FLOATING_WINDOW:
			look = E_FLOATING_WINDOW_LOOK;
			feel = E_FLOATING_APP_WINDOW_FEEL;
			break;

		default:
			return E_ERROR;
	}

	e_status_t status;

	e_window_look saveLook = fWindowLook;
	if((status = SetLook(look)) != E_OK) return status;
	if((status = SetFeel(feel)) != E_OK)
	{
		SetLook(saveLook);
		return status;
	}

	return E_OK;
}


e_window_type
EWindow::Type() const
{
	if(fWindowLook == E_TITLED_WINDOW_LOOK && fWindowFeel == E_NORMAL_WINDOW_FEEL)
		return E_TITLED_WINDOW;
	else if(fWindowLook == E_MODAL_WINDOW_LOOK && fWindowFeel == E_MODAL_APP_WINDOW_FEEL)
		return E_MODAL_WINDOW;
	else if(fWindowLook == E_DOCUMENT_WINDOW_LOOK && fWindowFeel == E_NORMAL_WINDOW_FEEL)
		return E_DOCUMENT_WINDOW;
	else if(fWindowLook == E_BORDERED_WINDOW_LOOK && fWindowFeel == E_NORMAL_WINDOW_FEEL)
		return E_BORDERED_WINDOW;
	else if(fWindowLook == E_FLOATING_WINDOW_LOOK && fWindowFeel == E_FLOATING_APP_WINDOW_FEEL)
		return E_FLOATING_WINDOW;
	else return E_UNTYPED_WINDOW;
}


e_status_t
EWindow::SetLook(e_window_look look)
{
	if(fWindowLook != look)
	{
		e_status_t status = fWindow == NULL ? E_OK : fWindow->SetLook(look);
		if(status != E_OK) return status;
		fWindowLook = look;
	}

	return E_OK;
}


e_window_look
EWindow::Look() const
{
	return fWindowLook;
}


e_status_t
EWindow::SetFeel(e_window_feel feel)
{
	if(fWindowFeel != feel)
	{
		e_status_t status = fWindow == NULL ? E_OK : fWindow->SetFeel(feel);
		if(status != E_OK) return status;

		e_window_feel oldFeel = fWindowFeel;
		fWindowFeel = feel;

		if((oldFeel == E_MODAL_APP_WINDOW_FEEL || feel == E_MODAL_APP_WINDOW_FEEL) && !IsHidden())
		{
			EMessenger msgrSelf(this);
			if(oldFeel == E_MODAL_APP_WINDOW_FEEL)
				etk_app->RemoveModalWindow(msgrSelf);
			else
				etk_app->AddModalWindow(msgrSelf);
		}
	}

	return E_OK;
}


e_window_feel
EWindow::Feel() const
{
	return fWindowFeel;
}


e_status_t
EWindow::SetFlags(euint32 flags)
{
	if(fWindowFlags != flags)
	{
		e_status_t status = fWindow == NULL ? E_OK : fWindow->SetFlags(flags);
		if(status != E_OK) return status;
		fWindowFlags = flags;
		if(flags & E_AVOID_FOCUS)
		{
			if(fActivated != false)
			{
				if(fWindow == NULL ? true : (fWindow->Activate(false) == E_OK))
				{
					fActivatedTimeStamp = e_real_time_clock_usecs();

					fActivated = false;
					WindowActivated(false);

					EMessage aMsg(E_WINDOW_ACTIVATED);
					aMsg.AddInt64("when", e_real_time_clock_usecs());
					for(EView *view = ChildAt(0); view != NULL; view = view->NextSibling()) PostMessage(&aMsg, view);
				}
			}
		}
	}

	return E_OK;
}


euint32
EWindow::Flags() const
{
	return fWindowFlags;
}


void
EWindow::SetWorkspaces(euint32 workspace)
{
	if(workspace == 0)
	{
		if(etk_app->fGraphicsEngine->GetCurrentWorkspace(&workspace) != E_OK || workspace == 0) return;
	}

	if(fWindow == NULL)
	{
		// TODO
		return;
	}

	if(fWindowWorkspaces != workspace || fWindowWorkspaces == 0)
	{
		if(fWindow->SetWorkspaces(workspace) == E_OK)
		{
			euint32 oldWorkspace = fWindowWorkspaces;
			fWindowWorkspaces = workspace;
			if(oldWorkspace != 0) WorkspacesChanged(oldWorkspace, workspace);
		}
	}
}


euint32
EWindow::Workspaces() const
{
	return fWindowWorkspaces;
}


void
EWindow::MoveBy(float dx, float dy)
{
	MoveTo(Frame().LeftTop() + EPoint(dx, dy));
}


void
EWindow::ResizeBy(float dx, float dy)
{
	ERect frame = Frame();
	ResizeTo(frame.Width() + dx, frame.Height() + dy);
}


void
EWindow::MoveTo(EPoint where)
{
	if(fWindow == NULL)
	{
		// TODO
		return;
	}

	if(Frame().LeftTop() != where)
	{
		EPoint pt = where.FloorCopy();
		if(fWindow->MoveTo((eint32)pt.x, (eint32)pt.y) != E_OK) return;

		fPositionChangedTimeStamp = e_real_time_clock_usecs();
		e_cast_as(fLayout, EWindowLayoutContainer)->MoveTo(where);
		FrameMoved(where);

		if(IsWatched(E_WINDOW_MOVED))
		{
			EMessage aMsg(E_WINDOW_MOVED);
			aMsg.AddInt64("when", fPositionChangedTimeStamp);
			aMsg.AddPoint("where", where);
			SendNotices(E_WINDOW_MOVED, &aMsg);
		}
	}
}


void
EWindow::MoveToCenter()
{
	EScreen scr(this);
	ERect r = scr.Frame();
	if(!r.IsValid()) return;
	MoveTo(EPoint((r.Width() - Frame().Width()) / 2, (r.Height() - Frame().Height()) / 2));
}


void
EWindow::ResizeTo(float w, float h)
{
	if(fWindow == NULL)
	{
		// TODO
		return;
	}

	euint32 min_h = E_MAXUINT32, max_h = E_MAXUINT32, min_v = E_MAXUINT32, max_v = E_MAXUINT32;
	fWindow->GetSizeLimits(&min_h, &max_h, &min_v, &max_v);

	if(w < (float)min_h && min_h != E_MAXUINT32) w = (float)min_h;
	else if(w > (float)max_h && max_h != E_MAXUINT32) w = (float)max_h;
	if(h < (float)min_v && min_v != E_MAXUINT32) h = (float)min_v;
	else if(h > (float)max_v && max_v != E_MAXUINT32) h = (float)max_v;

	ERect frame = Frame();
	if(frame.Width() != w || frame.Height() != h)
	{
		frame.right = frame.left + w;
		frame.bottom = frame.top + h;
		frame.Floor();
		if(fWindow->MoveAndResizeTo((eint32)frame.left, (eint32)frame.top,
					    (euint32)max_c(frame.Width(), 0),
					    (euint32)max_c(frame.Height(), 0)) != E_OK) return;
		fPixmap->ResizeTo((euint32)max_c(frame.Width(), 0), (euint32)max_c(frame.Height(), 0));
		fDC->SetClipping(ERegion(frame.OffsetToCopy(E_ORIGIN)));

		fSizeChangedTimeStamp = e_real_time_clock_usecs();

		fExposeRect = Bounds();
		if(fInUpdate == false) PostMessage(_UPDATE_IF_NEEDED_, this);

		// for disable update
		bool saveInUpdate = fInUpdate;
		fInUpdate = true;
		e_cast_as(fLayout, EWindowLayoutContainer)->ResizeTo(w, h);
		fInUpdate = saveInUpdate;

		FrameResized(w, h);

		if(IsWatched(E_WINDOW_RESIZED))
		{
			EMessage aMsg(E_WINDOW_RESIZED);
			aMsg.AddInt64("when", fSizeChangedTimeStamp);
			aMsg.AddFloat("width", w);
			aMsg.AddFloat("height", h);
			SendNotices(E_WINDOW_RESIZED, &aMsg);
		}
	}
}


void
EWindow::SetSizeLimits(float min_h, float max_h, float min_v, float max_v)
{
	if(fWindow == NULL)
	{
		// TODO
		return;
	}

	euint32 minH = E_MAXUINT32, maxH = E_MAXUINT32, minV = E_MAXUINT32, maxV = E_MAXUINT32;
	if(min_h >= 0) minH = (euint32)ceil((double)min_h);
	if(max_h >= 0) maxH = (euint32)ceil((double)max_h);
	if(min_v >= 0) minV = (euint32)ceil((double)min_v);
	if(max_v >= 0) maxV = (euint32)ceil((double)max_v);

	if(fWindow->SetSizeLimits(minH, maxH, minV, maxV) == E_OK)
	{
		if(min_h >= 0 || min_v >= 0)
		{
			ERect r = Frame();
			if(r.Width() < min_h && min_h >= 0) r.right = r.left + min_h;
			if(r.Height() < min_v && min_v >= 0) r.bottom = r.top + min_v;

			if(r != Frame()) ResizeTo(r.Width(), r.Height());
		}
	}
}


void
EWindow::GetSizeLimits(float *min_h, float *max_h, float *min_v, float *max_v) const
{
	euint32 minH = E_MAXUINT32, maxH = E_MAXUINT32, minV = E_MAXUINT32, maxV = E_MAXUINT32;
	if(fWindow) fWindow->GetSizeLimits(&minH, &maxH, &minV, &maxV);

	if(min_h) *min_h = minH != E_MAXUINT32 ? (float)minH : -1.f;
	if(max_h) *max_h = maxH != E_MAXUINT32 ? (float)maxH : -1.f;
	if(min_v) *min_v = minV != E_MAXUINT32 ? (float)minV : -1.f;
	if(max_v) *max_v = maxV != E_MAXUINT32 ? (float)maxV : -1.f;
}


e_status_t
EWindow::SendBehind(const EWindow *win)
{
	if(fWindow == NULL)
	{
		// TODO
		return E_ERROR;
	}

	if(win)
	{
		if(win->fWindow == NULL) return E_ERROR;
		return fWindow->Lower(win->fWindow);
	}

	return fWindow->Raise();
}


bool
EWindow::_GrabMouse()
{
	if(fWindow == NULL)
	{
		// TODO
		return false;
	}

	if(fMouseGrabCount == 0)
	{
		if(fWindow->GrabMouse() != E_OK)
		{
			ETK_DEBUG("[INTERFACE]: Mouse grab failed.");
			return false;
		}
	}

	if(fMouseGrabCount < E_MAXUINT32)
	{
		fMouseGrabCount++;
//		ETK_DEBUG("[INTERFACE]: Mouse grabbed (%u).", fMouseGrabCount);
		return true;
	}

	return false;
}


bool
EWindow::_GrabKeyboard()
{
	if(fWindow == NULL)
	{
		// TODO
		return false;
	}

	if(fKeyboardGrabCount == 0)
	{
		if(fWindow->GrabKeyboard() != E_OK)
		{
			ETK_DEBUG("[INTERFACE]: Keyboard grab failed.");
			return false;
		}
	}

	if(fKeyboardGrabCount < E_MAXUINT32)
	{
		fKeyboardGrabCount++;
//		ETK_DEBUG("[INTERFACE]: Keyboard grabbed (%u).", fKeyboardGrabCount);
		return true;
	}

	return false;
}


void
EWindow::_UngrabMouse()
{
	if(fWindow == NULL)
	{
		// TODO
		return;
	}

	if(fMouseGrabCount == 0) return;
	fMouseGrabCount--;
	if(fMouseGrabCount == 0) fWindow->UngrabMouse();
//	ETK_DEBUG("[INTERFACE]: Mouse ungrabbed (%u).", fMouseGrabCount);
}


void
EWindow::_UngrabKeyboard()
{
	if(fWindow == NULL)
	{
		// TODO
		return;
	}

	if(fKeyboardGrabCount == 0) return;
	fKeyboardGrabCount--;
	if(fKeyboardGrabCount == 0) fWindow->UngrabKeyboard();
//	ETK_DEBUG("[INTERFACE]: Keyboard ungrabbed (%u).", fKeyboardGrabCount);
}


bool
EWindow::GrabMouse()
{
	return _GrabMouse();
}


bool
EWindow::GrabKeyboard()
{
	return _GrabKeyboard();
}


void
EWindow::UngrabMouse()
{
	_UngrabMouse();
}


void
EWindow::UngrabKeyboard()
{
	_UngrabKeyboard();
}


bool
EWindow::IsMouseGrabbed() const
{
	return(fMouseGrabCount > 0);
}


bool
EWindow::IsKeyboardGrabbed() const
{
	return(fKeyboardGrabCount > 0);
}


void
EWindow::SetPulseRate(e_bigtime_t rate)
{
	if(fPulseRunner->SetInterval(rate) == E_OK)
	{
		fPulseRate = rate;
		fPulseRunner->SetCount((rate > 0 && fNeededToPulseViews.CountItems() > 0 && !IsHidden()) ? -1 : 0);
	}
	else
	{
		ETK_DEBUG("[INTERFACE]: %s --- Unable to set pulse rate.", __PRETTY_FUNCTION__);
	}
}


e_bigtime_t
EWindow::PulseRate() const
{
	return fPulseRate;
}


const char*
EWindow::Title() const
{
	return fWindowTitle;
}


void
EWindow::SetTitle(const char *title)
{
	EString str(title);
	if(str != fWindowTitle)
	{
		if(fWindowTitle) delete[] fWindowTitle;
		fWindowTitle = EStrdup(str.String());
		if(fWindow) fWindow->SetTitle(fWindowTitle);
	}
}

