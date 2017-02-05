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
 * File: PopUpMenu.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../app/Application.h"
#include "./../support/ClassInfo.h"
#include "./../kernel/Kernel.h"

#include "Window.h"
#include "PopUpMenu.h"


class EPopUpMenuView;

class EPopUpMenuWindow : public EWindow {
public:
	EPopUpMenuWindow(EPoint where, EPopUpMenu *menu, bool delivers_message, bool open_anyway, bool async, bool could_proxy);

	virtual bool QuitRequested();

	void WaitToClose();

private:
	friend class EPopUpMenu;
	friend class EPopUpMenuView;
	EPopUpMenu *fMenu;
	bool fAsync;
	bool fOpenAnyway;
	bool fDeliversMessage;
};


class EPopUpMenuView : public EView {
public:
	EPopUpMenuView(ERect frame);
	virtual void Draw(ERect updateRect);
};


EPopUpMenuView::EPopUpMenuView(ERect frame)
	: EView(frame, NULL, E_FOLLOW_ALL, E_WILL_DRAW)
{
}


void
EPopUpMenuView::Draw(ERect updateRect)
{
	if(!(Bounds().InsetByCopy(1, 1).Contains(updateRect)))
	{
		SetDrawingMode(E_OP_COPY);
		SetPenSize(1);
		e_rgb_color borderColor = e_ui_color(E_MENU_BORDER_COLOR);

		EPopUpMenuWindow *win = e_cast_as(Window(), EPopUpMenuWindow);
		if(win->fMenu == NULL || win->fMenu->IsEnabled() == false) borderColor.mix(0, 0, 0, 20);

		SetHighColor(borderColor);
		StrokeRect(Bounds());
	}
}


EPopUpMenuWindow::EPopUpMenuWindow(EPoint where, EPopUpMenu *menu, bool delivers_message, bool open_anyway, bool async, bool could_proxy)
	: EWindow(ERect(0, 0, 1, 1), NULL, E_NO_BORDER_WINDOW_LOOK, E_MODAL_APP_WINDOW_FEEL, E_AVOID_FOCUS), fMenu(NULL)
{
	Lock();

	fAsync = async;
	fOpenAnyway = open_anyway;
	fDeliversMessage = (async ? true : delivers_message);

	EPopUpMenuView *topView = new EPopUpMenuView(Bounds());
	AddChild(topView);
	if(topView->Window() != this) {delete topView; return;}
	topView->AddChild(menu);
	if(menu->Window() != this) return;

	fMenu = menu;

	euint32 oldResizingMode = fMenu->ResizingMode();

	fMenu->SetResizingMode(E_FOLLOW_NONE);
	fMenu->ResizeToPreferred();
	fMenu->MoveTo(EPoint(2, 2));
	ResizeTo(fMenu->Frame().Width() + 4, fMenu->Frame().Height() + 4);
	MoveTo(where);

	e_rgb_color bkColor = e_ui_color(E_MENU_BACKGROUND_COLOR);
	if(fMenu->IsEnabled() == false) bkColor.mix(0, 0, 0, 20);
	SetBackgroundColor(bkColor);

	fMenu->SetResizingMode(oldResizingMode);

	if(async == false && could_proxy)
	{
		ELooper *looper = ELooper::LooperForThread(etk_get_current_thread_id());
		if(looper)
		{
			looper->Lock();
			ProxyBy(looper);
			looper->Unlock();
		}
	}

	if(Proxy() == this) Run();
}


bool
EPopUpMenuWindow::QuitRequested()
{
	if(!(fMenu == NULL || fMenu->Window() != this))
	{
		EPopUpMenu *menu = fMenu;

		Hide();
		fMenu = NULL;

		menu->RemoveSelf();

		if(fAsync && menu->AsyncAutoDestruct()) delete menu;
	}
	return true;
}


void
EPopUpMenuWindow::WaitToClose()
{
	if(fAsync || Proxy() == this || !IsLockedByCurrentThread())
		ETK_ERROR("[INTERFACE]: %s --- Usage error!!!", __PRETTY_FUNCTION__);

	while(true)
	{
		EMessage *aMsg = NextLooperMessage(E_INFINITE_TIMEOUT);
		DispatchLooperMessage(aMsg);
		if(aMsg == NULL) break;
	}

	if(!(fMenu == NULL || fMenu->Window() != this))
	{
		EPopUpMenu *menu = fMenu;

		Hide();
		fMenu = NULL;

		menu->RemoveSelf();
	}

	Quit();
}



EPopUpMenu::EPopUpMenu(const char *title, bool radioMode, bool labelFromMarked, e_menu_layout layout)
	: EMenu(title, layout), fAutoDestruct(false)
{
	SetEventMask(E_POINTER_EVENTS);
	SetRadioMode(radioMode);
	if(radioMode) SetLabelFromMarked(labelFromMarked);
}


EPopUpMenu::~EPopUpMenu()
{
}


void
EPopUpMenu::MessageReceived(EMessage *msg)
{
	switch(msg->what)
	{
		case E_MOUSE_MOVED:
			{
				EPopUpMenuWindow *win = e_cast_as(Window(), EPopUpMenuWindow);
				if(win == NULL || win->fOpenAnyway == true) break;

				eint32 buttons;
				EPoint where;

				if(!(msg->FindInt32("buttons", &buttons) == false || buttons > 0))
				{
					Window()->PostMessage(E_QUIT_REQUESTED);
					return;
				}
				if(msg->FindPoint("where", &where) == false || VisibleBounds().Contains(where)) break;
				ConvertToScreen(&where);

				EMenu *submenu = this;
				while(submenu != NULL)
				{
					EMenu *aMenu = submenu->SubmenuAt(submenu->fSelectedIndex);
					if(aMenu == NULL) break;
					if(aMenu->Window() == NULL) break;
					if(aMenu->Window()->Frame().Contains(where))
					{
						msg->RemovePoint("where");
						EMessenger msgr(aMenu->Window());
						msgr.SendMessage(msg);
						break;
					}
					submenu = aMenu;
				}
			}
			break;

		case _MENU_EVENT_:
			{
				if(Window() == NULL || !e_is_instance_of(Window(), EPopUpMenuWindow)) break;

				EMenuItem *item = NULL;
				if(msg->FindPointer("source", (void**)&item) == false || item == NULL) break;

				EPopUpMenuWindow *win = e_cast_as(Window(), EPopUpMenuWindow);
				fSelectedItem = item;
				if(win->fDeliversMessage)
				{
					euint32 what;
					if(msg->FindInt32("etk:menu_orig_what", (eint32*)&what))
					{
						EMessage aMsg = *msg;
						aMsg.what = what;
						item->EInvoker::Invoke(&aMsg);
					}
				}

				win->PostMessage(E_QUIT_REQUESTED);

				return;
			}
			break;

		default:
			break;
	}

	EMenu::MessageReceived(msg);
}


void
EPopUpMenu::MouseUp(EPoint where)
{
	EPopUpMenuWindow *win = e_cast_as(Window(), EPopUpMenuWindow);
	EMessage *msg = Window()->CurrentMessage();

	if(!(win == NULL || win->fOpenAnyway == true || msg == NULL || msg->what != E_MOUSE_UP))
	{
		EPoint mousePos = where;
		EMenu *submenu = this;

		ConvertToScreen(&mousePos);

		if(!VisibleBounds().Contains(where))
		{
			while(submenu != NULL)
			{
				EMenu *aMenu = submenu->SubmenuAt(submenu->fSelectedIndex);
				if(aMenu == NULL) break;
				if(aMenu->Window() == NULL) break;
				if(aMenu->Window()->Frame().Contains(mousePos))
				{
					aMenu->ConvertFromScreen(&mousePos);
					aMenu->fTrackingIndex = aMenu->fSelectedIndex;
					aMenu->MouseUp(mousePos);
					break;
				}
				submenu = aMenu;
			}
		}
		else
		{
			fTrackingIndex = fSelectedIndex;
		}
	}

	EMenu::MouseUp(where);

	if(!(win == NULL || win->fOpenAnyway == true || msg == NULL || msg->what != E_MOUSE_UP))
	{
		eint32 buttons;
		if(msg->FindInt32("buttons", &buttons) == false) return;
		if(buttons > 0) return;

		Window()->PostMessage(E_QUIT_REQUESTED);
	}
}


EMenuItem*
EPopUpMenu::Go(EPoint where, bool delivers_message, bool open_anyway, bool async, bool could_proxy)
{
	if(Window() != NULL || Supermenu() != NULL)
	{
		ETK_WARNING("[INTERFACE]: %s --- Menu already pop-up or attached to others.", __PRETTY_FUNCTION__);
		return NULL;
	}

	SelectItem(NULL);
	fSelectedItem = NULL;

	EPopUpMenuWindow *win = new EPopUpMenuWindow(where, this, delivers_message, open_anyway, async, could_proxy);
	void *trackingThread = NULL;

	if(win == NULL || win->IsRunning() == false || Window() != win ||
	   (win->Proxy() == win ? ((trackingThread = etk_open_thread(win->Thread())) == NULL) : false))
	{
		ETK_WARNING("[INTERFACE]: %s --- Unable to create pop-up window.", __PRETTY_FUNCTION__);
		if(win != NULL)
		{
			if(Window() == win)
			{
				win->fMenu = NULL;
				RemoveSelf();
			}
			win->Quit();
		}
		return NULL;
	}

	win->Show();
	win->SendBehind(NULL);

	if(win->Proxy() != win)
	{
		win->WaitToClose();
	}
	else
	{
		win->Unlock();

		if(trackingThread)
		{
			if(!async)
			{
				e_status_t status;
				etk_wait_for_thread(trackingThread, &status);
			}
			etk_delete_thread(trackingThread);
		}
	}

	return(async ? NULL : fSelectedItem);
}


void
EPopUpMenu::SetAsyncAutoDestruct(bool state)
{
	fAutoDestruct = state;
}


bool
EPopUpMenu::AsyncAutoDestruct() const
{
	return fAutoDestruct;
}


bool
EPopUpMenu::IsPopUpByGo() const
{
	return(e_is_instance_of(Window(), EPopUpMenuWindow) != 0);
}

