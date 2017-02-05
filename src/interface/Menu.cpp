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
 * File: Menu.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../support/ClassInfo.h>
#include "./../kernel/Kernel.h>

#include "Window.h"
#include "PopUpMenu.h"
#include "Menu.h"

#define ETK_MENU_ROW_SPACING	5
#define ETK_MENU_COLUMN_SPACING	2


class ESubmenuView;

class _LOCAL ESubmenuWindow : public EWindow {
public:
	ESubmenuWindow(EPoint where, EMenu *menu);
	virtual ~ESubmenuWindow();

	virtual void DispatchMessage(EMessage *msg, EHandler *target);
	virtual bool QuitRequested();
	virtual void FrameMoved(EPoint new_position);

private:
	friend class EMenu;
	friend class ESubmenuView;

	EMenu *fMenu;

	virtual bool IsDependsOnOthersWhenQuitRequested() const;
};


EMenu::EMenu(ERect frame, const char *title, euint32 resizeMode, euint32 flags, e_menu_layout layout, bool resizeToFit)
	: EView(frame, title, resizeMode, flags), fSuperitem(NULL),
	  fRadioMode(false), fLabelFromMarked(false),
	  fSelectedIndex(-1), fTrackingIndex(-1), fMarkedIndex(-1), fShowSubmenuByKeyDown(false)
{
	fMargins = ERect(0, 0, 0, 0);

	SetViewColor(e_ui_color(E_MENU_BACKGROUND_COLOR));

	SetFlags(flags | E_WILL_DRAW);

	SetLayout(layout, frame.Width(), frame.Height(), resizeToFit);
}


EMenu::EMenu(const char *title, e_menu_layout layout)
	: EView(ERect(0, 0, 10, 10), title, E_FOLLOW_NONE, E_WILL_DRAW), fSuperitem(NULL),
	  fRadioMode(false), fLabelFromMarked(false),
	  fSelectedIndex(-1), fTrackingIndex(-1), fMarkedIndex(-1), fShowSubmenuByKeyDown(false)
{
	fMargins = ERect(0, 0, 0, 0);

	SetViewColor(e_ui_color(E_MENU_BACKGROUND_COLOR));

	SetLayout(layout, 10, 10, (layout != E_ITEMS_IN_MATRIX ? true : false));
}


EMenu::EMenu(const char *title, float width, float height)
	: EView(ERect(0, 0, width > 10 ? width : 10, height > 10 ? height : 10), title, E_FOLLOW_NONE, E_WILL_DRAW), fSuperitem(NULL),
	  fRadioMode(false), fLabelFromMarked(false),
	  fSelectedIndex(-1), fTrackingIndex(-1), fMarkedIndex(-1), fShowSubmenuByKeyDown(false)
{
	fMargins = ERect(0, 0, 0, 0);

	SetViewColor(e_ui_color(E_MENU_BACKGROUND_COLOR));

	SetLayout(E_ITEMS_IN_MATRIX, width, height, false);
}


EMenu::~EMenu()
{
	for(eint32 i = 0; i < fMenuItems.CountItems(); i++)
	{
		EMenuItem *item = (EMenuItem*)fMenuItems.ItemAt(i);
		item->fMenu = NULL;
		delete item;
	}

	if(fSuperitem)
	{
		ETK_WARNING("[INTERFACE]: %s --- Menu still have super-item as deleting.", __PRETTY_FUNCTION__);
		fSuperitem->fSubmenu = NULL;
	}
}


bool
EMenu::AddItem(EMenuItem *item)
{
	return AddItem(item, fMenuItems.CountItems());
}


bool
EMenu::AddItem(EMenuItem *item, eint32 index)
{
	if(!item || item->Menu() != NULL || fLayout == E_ITEMS_IN_MATRIX) return false;

	if(fMenuItems.AddItem((void*)item, index) == false) return false;

	item->fMenu = this;
	if(fSelectedIndex == index) fSelectedIndex++;
	if(fMarkedIndex == index) fMarkedIndex++;

	if(Window() != NULL) Refresh();

	return true;
}


bool
EMenu::AddItem(EMenuItem *item, ERect frame)
{
	if(!item || item->Menu() != NULL || !frame.IsValid() || fLayout != E_ITEMS_IN_MATRIX) return false;

	if(fMenuItems.AddItem((void*)item) == false) return false;

	item->fMenu = this;

	item->fFrame = frame;

	if(Window() != NULL) Refresh();

	return true;
}


bool
EMenu::AddItem(EMenu *menu)
{
	return AddItem(menu, fMenuItems.CountItems());
}


bool
EMenu::AddItem(EMenu *menu, eint32 index)
{
	if(!menu || menu->Superitem() != NULL || fLayout == E_ITEMS_IN_MATRIX) return false;

	EMenuItem *item = new EMenuItem(menu, NULL);
	if(!item) return false;

	if(fMenuItems.AddItem((void*)item, index) == false)
	{
		item->fSubmenu = NULL;
		menu->fSuperitem = NULL;
		delete item;
		return false;
	}

	item->fMenu = this;

	if(fSelectedIndex == index) fSelectedIndex++;
	if(fMarkedIndex == index) fMarkedIndex++;

	if(Window() != NULL) Refresh();

	return true;
}


bool
EMenu::AddItem(EMenu *menu, ERect frame)
{
	if(!menu || menu->Superitem() != NULL || !frame.IsValid() || fLayout != E_ITEMS_IN_MATRIX) return false;

	EMenuItem *item = new EMenuItem(menu, NULL);
	if(!item) return false;

	if(fMenuItems.AddItem((void*)item) == false)
	{
		item->fSubmenu = NULL;
		menu->fSuperitem = NULL;
		delete item;
		return false;
	}

	item->fMenu = this;
	item->fFrame = frame;

	Invalidate(frame);

	return false;
}


bool
EMenu::AddSeparatorItem()
{
	if(fLayout == E_ITEMS_IN_MATRIX) return false;

	EMenuItem *item = new EMenuSeparatorItem();
	if(!item) return false;

	if(!AddItem(item))
	{
		delete item;
		return false;
	}

	return true;
}


bool
EMenu::RemoveItem(EMenuItem *item)
{
	eint32 index = IndexOf(item);
	return(RemoveItem(index) != NULL);
}


EMenuItem*
EMenu::RemoveItem(eint32 index)
{
	if(index < 0 || index >= fMenuItems.CountItems()) return NULL;

	EMenuItem *item = (EMenuItem*)fMenuItems.RemoveItem(index);
	if(!item) return NULL;

	item->fMenu = NULL;
	if(item->fSubmenu != NULL) item->fSubmenu->ClosePopUp();

	if(fSelectedIndex == index) fSelectedIndex = -1;
	if(fMarkedIndex == index)
	{
		fMarkedIndex = -1;
		if(fRadioMode) FindMarked(&fMarkedIndex);
	}

	if(Window() != NULL) Refresh();

	return item;
}


bool
EMenu::RemoveItem(EMenu *menu)
{
	if(!menu || menu->fSuperitem == NULL || menu->fSuperitem->fMenu != this) return false;

	eint32 index = IndexOf(menu->fSuperitem);
	EMenuItem *item = (EMenuItem*)fMenuItems.RemoveItem(index);
	if(!item) return false;

	menu->fSuperitem = NULL;
	menu->ClosePopUp();

	item->fMenu = NULL;
	item->fSubmenu = NULL;
	delete item;

	if(fSelectedIndex == index) fSelectedIndex = -1;
	if(fMarkedIndex == index)
	{
		fMarkedIndex = -1;
		if(fRadioMode) FindMarked(&fMarkedIndex);
	}

	if(Window() != NULL) Refresh();

	return true;
}


EMenuItem*
EMenu::ItemAt(eint32 index) const
{
	return (EMenuItem*)fMenuItems.ItemAt(index);
}


EMenu*
EMenu::SubmenuAt(eint32 index) const
{
	EMenuItem *item = (EMenuItem*)fMenuItems.ItemAt(index);
	if(!item) return NULL;

	return item->Submenu();
}


eint32
EMenu::CountItems() const
{
	return fMenuItems.CountItems();
}


eint32
EMenu::IndexOf(const EMenuItem *item) const
{
	if(!item || item->fMenu != this) return -1;
	return fMenuItems.IndexOf((void*)item);
}


eint32
EMenu::IndexOf(const EMenu *menu) const
{
	if(!menu || menu->Superitem() == NULL) return -1;
	return fMenuItems.IndexOf((void*)menu->Superitem());
}


EMenuItem*
EMenu::FindItem(euint32 command) const
{
	for(eint32 i = 0; i < fMenuItems.CountItems(); i++)
	{
		EMenuItem *item = (EMenuItem*)fMenuItems.ItemAt(i);

		if(item->Command() == command) return item;
		if(item->fSubmenu == NULL) continue;

		EMenuItem *found = item->fSubmenu->FindItem(command);
		if(found != NULL) return found;
	}

	return NULL;
}


inline bool etk_comapre_menuitem_name(const char *name1, const char *name2)
{
	if(!name1 && !name2) return true;
	if(!name1 || !name2) return false;
	if(strlen(name1) != strlen(name2)) return false;
	if(strcmp(name1, name2) != 0) return false;
	return true;
}


EMenuItem*
EMenu::FindItem(const char *name) const
{
	for(eint32 i = 0; i < fMenuItems.CountItems(); i++)
	{
		EMenuItem *item = (EMenuItem*)fMenuItems.ItemAt(i);

		if(etk_comapre_menuitem_name(item->Label(), name)) return item;
		if(item->fSubmenu == NULL) continue;

		EMenuItem *found = item->fSubmenu->FindItem(name);
		if(found != NULL) return found;
	}

	return NULL;
}


EMenu*
EMenu::Supermenu() const
{
	if(!fSuperitem) return NULL;
	return fSuperitem->Menu();
}


EMenuItem*
EMenu::Superitem() const
{
	return fSuperitem;
}


e_status_t
EMenu::SetTargetForItems(EHandler *target)
{
	e_status_t status = E_OK;
	EMessenger msgr(target, NULL, &status);
	if(status != E_OK) return status;

	return SetTargetForItems(msgr);
}


e_status_t
EMenu::SetTargetForItems(EMessenger messenger)
{
	for(eint32 i = 0; i < fMenuItems.CountItems(); i++)
	{
		EMenuItem *item = (EMenuItem*)fMenuItems.ItemAt(i);
		e_status_t status = item->SetTarget(messenger);
		if(status != E_OK) return status;
	}

	return E_OK;
}


void
EMenu::SetEnabled(bool state)
{
	if(EView::IsEnabled() != state)
	{
		EView::SetEnabled(state);

		e_rgb_color vColor = e_ui_color(E_MENU_BACKGROUND_COLOR);

		if(!state)
		{
			fSelectedIndex = -1;
			fTrackingIndex = -1;

			vColor.mix(0, 0, 0, 20);
		}

		SetViewColor(vColor);

		if(!(fSuperitem  == NULL || fSuperitem->fEnabled == state)) fSuperitem->SetEnabled(state);

		if(e_is_instance_of(Window(), ESubmenuWindow)) Window()->SetBackgroundColor(vColor);
	}
}


bool
EMenu::IsEnabled() const
{
	if(EView::IsEnabled() == false) return false;
	if(fSuperitem == NULL || fSuperitem->fMenu == NULL) return true;
	return fSuperitem->fMenu->IsEnabled();
}


void
EMenu::AttachedToWindow()
{
	if(e_is_instance_of(Window(), ESubmenuWindow)) SetEventMask(E_POINTER_EVENTS | E_KEYBOARD_EVENTS);

	Window()->DisableUpdates();
	Refresh();
	Window()->EnableUpdates();

	if(!e_is_instance_of(Window(), ESubmenuWindow))
	{
		Window()->StartWatching(this, E_MINIMIZED);
		Window()->StartWatching(this, E_WINDOW_ACTIVATED);
	}

	Window()->StartWatching(this, E_WINDOW_MOVED);
	Window()->StartWatching(this, E_WINDOW_RESIZED);

	for(eint32 i = 0; i < fMenuItems.CountItems(); i++)
	{
		EMenu *menu = ((EMenuItem*)fMenuItems.ItemAt(i))->fSubmenu;
		if(menu == NULL) continue;
		if(menu->fRadioMode == false || menu->fLabelFromMarked == false || menu->fSuperitem == NULL) continue;

		EMenuItem *markedItem = (EMenuItem*)menu->fMenuItems.ItemAt(menu->fMarkedIndex);
		if(markedItem)
			menu->fSuperitem->SetLabel(markedItem->Label());
		else
			menu->fSuperitem->SetLabel(menu->Name());
	}
}


void
EMenu::DetachedFromWindow()
{
	Window()->StopWatchingAll(this);

	for(eint32 i = 0; i < fMenuItems.CountItems(); i++)
	{
		EMenuItem *item = (EMenuItem*)fMenuItems.ItemAt(i);
		if(!(item == NULL || item->fSubmenu == NULL)) item->fSubmenu->ClosePopUp();
	}

	fTrackingIndex = -1;
}


void
EMenu::MessageReceived(EMessage *msg)
{
	bool processed = false;

	switch(msg->what)
	{
		case E_OBSERVER_NOTICE_CHANGE:
			{
				if(Window() == NULL) break;

				euint32 what;
				if(msg->FindInt32(E_OBSERVE_ORIGINAL_WHAT, (eint32*)&what) == false ||
				   !(what == E_WINDOW_MOVED ||
				     what == E_WINDOW_RESIZED ||
				     what == E_MINIMIZED ||
				     what == E_WINDOW_ACTIVATED)) break;

				processed = true;

				if(fSelectedIndex < 0) break;
				EMenuItem *item = (EMenuItem*)fMenuItems.ItemAt(fSelectedIndex);
				if(item == NULL || item->fSubmenu == NULL || item->fSubmenu->Window() == NULL) break;

				if((e_is_instance_of(Window(), ESubmenuWindow) == false &&
				    Window()->IsActivate() == false &&
				    item->fSubmenu->Window()->IsActivate() == false) ||
				   Window()->IsHidden() || Window()->IsMinimized())
				{
					item->fSubmenu->ClosePopUp();
				}
				else if(e_is_instance_of(item->fSubmenu->Window(), ESubmenuWindow))
				{
#if 0
					// notice submenu to move
					item->fSubmenu->Window()->PostMessage(_MENU_EVENT_, item->fSubmenu->Window());
#else
					item->fSubmenu->ClosePopUp();
#endif
				}
			}
			break;

		case _MENU_EVENT_:
			{
				if(Window() == NULL) break;

				EMenuItem *item = NULL;
				if(msg->FindPointer("source", (void**)&item) == false || item == NULL) break;

				processed = true;

				if(e_is_instance_of(Window(), ESubmenuWindow))
				{
					if(!(Supermenu() == NULL || Supermenu()->Window() == NULL))
						Supermenu()->Window()->PostMessage(msg, Supermenu());
					Window()->PostMessage(E_QUIT_REQUESTED);
				}
				else
				{
					euint32 what;
					if(msg->FindInt32("etk:menu_orig_what", (eint32*)&what))
					{
						EMessage aMsg = *msg;
						aMsg.what = what;
						item->EInvoker::Invoke(&aMsg);
					}
				}
			}
			break;

		default:
			break;
	}

	if(!processed) EView::MessageReceived(msg);
}


void
EMenu::MouseDown(EPoint where)
{
	if(!IsEnabled() || Window() == NULL || fMenuItems.CountItems() <= 0 || !QueryCurrentMouse(true, E_PRIMARY_MOUSE_BUTTON)) return;

	ERect rect = VisibleBounds();
	if(!rect.Contains(where)) return;

	if(fTrackingIndex >= 0) return;

	eint32 newIndex = FindItem(where);
	if(newIndex < 0)return;

	if(fSelectedIndex != newIndex)
	{
		EMenuItem *oldItem = (EMenuItem*)fMenuItems.ItemAt(fSelectedIndex);
		EMenuItem *newItem = (EMenuItem*)fMenuItems.ItemAt(newIndex);

		eint32 oldSelectedIndex = fSelectedIndex;
		fSelectedIndex = newIndex;

		Window()->DisableUpdates();
		if(newItem->SelectChanged())
		{
			if(oldItem) oldItem->SelectChanged();
		}
		else
		{
			fSelectedIndex = oldSelectedIndex;
		}
		Window()->EnableUpdates();
	}

	fShowSubmenuByKeyDown = false;

	fTrackingIndex = fSelectedIndex;
}


void
EMenu::MouseUp(EPoint where)
{
	if(fTrackingIndex >= 0)
	{
		eint32 trackingIndex = fTrackingIndex;
		fTrackingIndex = -1;

		if(!IsEnabled() || Window() == NULL || fMenuItems.CountItems() <= 0) return;
		if(trackingIndex != fSelectedIndex || ItemFrame(trackingIndex).Contains(where) == false) return;

		EMenuItem *item = (EMenuItem*)fMenuItems.ItemAt(fSelectedIndex);
		if(item == NULL || item->IsEnabled() == false) return;

		if(item->fSubmenu)
		{
			item->ShowSubmenu(true);
		}
		else
		{
			if(e_is_instance_of(Window(), ESubmenuWindow))
			{
				if(!(Supermenu() == NULL || Supermenu()->Window() == NULL))
				{
					EMessage msg(_MENU_EVENT_);
					if(item->Message() != NULL)
					{
						msg = *(item->Message());
						msg.AddInt32("etk:menu_orig_what", msg.what);
						msg.what = _MENU_EVENT_;
					}
					msg.AddInt64("when", e_real_time_clock_usecs());
					msg.AddPointer("source", item);
					msg.AddInt32("index", fSelectedIndex);

					Supermenu()->Window()->PostMessage(&msg, Supermenu());
				}
				Window()->PostMessage(E_QUIT_REQUESTED);
			}
			else if(e_is_kind_of(this, EPopUpMenu))
			{
				EMessage msg(_MENU_EVENT_);
				if(item->Message() != NULL)
				{
					msg = *(item->Message());
					msg.AddInt32("etk:menu_orig_what", msg.what);
					msg.what = _MENU_EVENT_;
				}
				msg.AddInt64("when", e_real_time_clock_usecs());
				msg.AddPointer("source", item);
				msg.AddInt32("index", fSelectedIndex);

				Window()->PostMessage(&msg, this);
			}
			else
			{
				item->Invoke();
			}

			ItemInvoked(item);
		}
	}
}


void
EMenu::MouseMoved(EPoint where, euint32 code, const EMessage *a_message)
{
	if(!IsEnabled() || Window() == NULL || fMenuItems.CountItems() <= 0 || !VisibleBounds().Contains(where)) return;

	eint32 newIndex = FindItem(where);
	if(newIndex < 0) return;

	if(fSelectedIndex != newIndex)
	{
		EMenuItem *oldItem = (EMenuItem*)fMenuItems.ItemAt(fSelectedIndex);
		EMenuItem *newItem = (EMenuItem*)fMenuItems.ItemAt(newIndex);

		eint32 oldSelectedIndex = fSelectedIndex;
		fSelectedIndex = newIndex;

		Window()->DisableUpdates();
		if(newItem->SelectChanged())
		{
			if(oldItem) oldItem->SelectChanged();
		}
		else
		{
			fSelectedIndex = oldSelectedIndex;
		}
		Window()->EnableUpdates();
	}

	fShowSubmenuByKeyDown = false;
}


void
EMenu::KeyDown(const char *bytes, eint32 numBytes)
{
	if(!IsEnabled() || Window() == NULL || fMenuItems.CountItems() <= 0 || numBytes != 1 || bytes == NULL) return;
	if(Window()->CurrentMessage() == NULL) return;
	if(!(Window()->CurrentMessage()->what == E_KEY_DOWN || Window()->CurrentMessage()->what == E_UNMAPPED_KEY_DOWN)) return;

	EMenuItem *selectedItem = (EMenuItem*)fMenuItems.ItemAt(fSelectedIndex);

	if(!(selectedItem == NULL || selectedItem->fSubmenu == NULL || selectedItem->fSubmenu->Window() == NULL ||
	     Window()->CurrentMessage()->HasBool("_MENU_EVENT_")))
	{
		selectedItem->fSubmenu->Window()->PostMessage(Window()->CurrentMessage(), selectedItem->fSubmenu);
		return;
	}

	if(!fShowSubmenuByKeyDown)
		fShowSubmenuByKeyDown = ((selectedItem->fSubmenu == NULL || selectedItem->fSubmenu->Window() == NULL) ? false : true);

	eint32 oldIndex = fSelectedIndex;
	bool doRewind = false;
	bool doInvert = false;

	char ch = *bytes;
	switch(ch)
	{
		case E_UP_ARROW:
			{
				if(fLayout != E_ITEMS_IN_ROW)
				{
					fSelectedIndex--;
					if(fSelectedIndex < 0) fSelectedIndex = fMenuItems.CountItems() - 1;
					doRewind = true;
					doInvert = true;
				}
			}
			break;

		case E_DOWN_ARROW:
			{
				if(fLayout != E_ITEMS_IN_ROW)
				{
					fSelectedIndex++;
					if(fSelectedIndex >= fMenuItems.CountItems()) fSelectedIndex = 0;
					doRewind = true;
					doInvert = false;
				}
			}
			break;

		case E_LEFT_ARROW:
			{
				if(fLayout != E_ITEMS_IN_COLUMN)
				{
					fSelectedIndex--;
					if(fSelectedIndex < 0) fSelectedIndex = fMenuItems.CountItems() - 1;
					doRewind = true;
					doInvert = true;
				}
			}
			break;

		case E_RIGHT_ARROW:
			{
				if(fLayout != E_ITEMS_IN_COLUMN)
				{
					fSelectedIndex++;
					if(fSelectedIndex >= fMenuItems.CountItems()) fSelectedIndex = 0;
					doRewind = true;
					doInvert = false;
				}
			}
			break;

		case E_ENTER:
		case E_SPACE:
			{
				if(fSelectedIndex < 0 || fTrackingIndex >= 0) break;
				fTrackingIndex = fSelectedIndex;
			}
			break;

		case E_ESCAPE:
			if(!e_is_instance_of(Window(), ESubmenuWindow)) fTrackingIndex = fSelectedIndex = -1;
			break;

		case E_HOME:
			fSelectedIndex = 0;
			doRewind = false;
			doInvert = false;
			break;

		case E_END:
			fSelectedIndex = fMenuItems.CountItems() - 1;
			doRewind = false;
			doInvert = true;
			break;

		default:
			break;
	}

	if(oldIndex != fSelectedIndex && fSelectedIndex >= 0)
	{
		EMenuItem *oldItem = (EMenuItem*)fMenuItems.ItemAt(oldIndex);

		Window()->DisableUpdates();

		if(oldItem) oldItem->SelectChanged();

		eint32 tmp = fSelectedIndex;
		fSelectedIndex = -1;

		if(!doInvert)
		{
			for(eint32 i = tmp; i < fMenuItems.CountItems(); i++)
			{
				EMenuItem *newItem = (EMenuItem*)fMenuItems.ItemAt(i);
				fSelectedIndex = i;
				if(newItem->SelectChanged()) break;
				fSelectedIndex = -1;
			}
			if(fSelectedIndex < 0 && doRewind)
			{
				for(eint32 i = 0; i < tmp; i++)
				{
					EMenuItem *newItem = (EMenuItem*)fMenuItems.ItemAt(i);
					fSelectedIndex = i;
					if(newItem->SelectChanged()) break;
					fSelectedIndex = -1;
				}
			}
		}
		else
		{
			for(eint32 i = tmp; i >= 0; i--)
			{
				EMenuItem *newItem = (EMenuItem*)fMenuItems.ItemAt(i);
				fSelectedIndex = i;
				if(newItem->SelectChanged()) break;
				fSelectedIndex = -1;
			}
			if(fSelectedIndex < 0 && doRewind)
			{
				for(eint32 i = fMenuItems.CountItems() - 1; i > tmp; i--)
				{
					EMenuItem *newItem = (EMenuItem*)fMenuItems.ItemAt(i);
					fSelectedIndex = i;
					if(newItem->SelectChanged()) break;
					fSelectedIndex = -1;
				}
			}
		}

		Window()->EnableUpdates();
	}
	else if(fSelectedIndex < 0)
	{
		EMenuItem *oldItem = (EMenuItem*)fMenuItems.ItemAt(oldIndex);
		if(oldItem) oldItem->SelectChanged();
		fShowSubmenuByKeyDown = false;
	}
}


void
EMenu::KeyUp(const char *bytes, eint32 numBytes)
{
	if(!IsEnabled() || Window() == NULL || fMenuItems.CountItems() <= 0 || numBytes != 1 || bytes == NULL) return;
	if(Window()->CurrentMessage() == NULL) return;
	if(!(Window()->CurrentMessage()->what == E_KEY_UP || Window()->CurrentMessage()->what == E_UNMAPPED_KEY_UP)) return;

	EMenuItem *selectedItem = (EMenuItem*)fMenuItems.ItemAt(fSelectedIndex);
	if(!(selectedItem == NULL || selectedItem->fSubmenu == NULL || selectedItem->fSubmenu->Window() == NULL ||
	     Window()->CurrentMessage()->HasBool("_MENU_EVENT_")))
	{
		selectedItem->fSubmenu->Window()->PostMessage(Window()->CurrentMessage(), selectedItem->fSubmenu);
		return;
	}

	char ch = *bytes;
	switch(ch)
	{
		case E_ENTER:
		case E_SPACE:
			{
				if(fTrackingIndex < 0 || fTrackingIndex != fSelectedIndex) break;
				if(selectedItem == NULL || selectedItem->IsEnabled() == false) break;
				if(selectedItem->fSubmenu)
				{
					if(selectedItem->fSubmenu->Window() == NULL) selectedItem->ShowSubmenu(true);
					break;
				}

				if(e_is_instance_of(Window(), ESubmenuWindow))
				{
					if(!(Supermenu() == NULL || Supermenu()->Window() == NULL))
					{
						EMessage msg(_MENU_EVENT_);
						if(selectedItem->Message() != NULL)
						{
							msg = *(selectedItem->Message());
							msg.AddInt32("etk:menu_orig_what", msg.what);
							msg.what = _MENU_EVENT_;
						}
						msg.AddInt64("when", e_real_time_clock_usecs());
						msg.AddPointer("source", selectedItem);
						msg.AddInt32("index", fSelectedIndex);

						Supermenu()->Window()->PostMessage(&msg, Supermenu());
					}
					Window()->PostMessage(E_QUIT_REQUESTED);
				}
				else if(e_is_kind_of(this, EPopUpMenu))
				{
					EMessage msg(_MENU_EVENT_);
					if(selectedItem->Message() != NULL)
					{
						msg = *(selectedItem->Message());
						msg.AddInt32("etk:menu_orig_what", msg.what);
						msg.what = _MENU_EVENT_;
					}
					msg.AddInt64("when", e_real_time_clock_usecs());
					msg.AddPointer("source", selectedItem);
					msg.AddInt32("index", fSelectedIndex);

					Window()->PostMessage(&msg, this);
				}
				else
				{
					selectedItem->Invoke();
				}

				ItemInvoked(selectedItem);
			}
			break;

		case E_UP_ARROW:
			{
				if(fLayout == E_ITEMS_IN_ROW)
				{
					if(!e_is_instance_of(Window(), ESubmenuWindow)) break;
					if(Supermenu()->Layout() != E_ITEMS_IN_ROW && Supermenu()->Window() != NULL)
					{
						EMessage aMsg = *(Window()->CurrentMessage());
						aMsg.AddBool("_MENU_EVENT_", true);
						aMsg.ReplaceInt64("when", e_real_time_clock_usecs());
						aMsg.what = E_KEY_DOWN;
						Supermenu()->Window()->PostMessage(&aMsg, Supermenu());
						aMsg.what = E_KEY_UP;
						Supermenu()->Window()->PostMessage(&aMsg, Supermenu());
					}
					else
					{
						Window()->PostMessage(E_QUIT_REQUESTED);
					}
				}
				else if(fShowSubmenuByKeyDown)
				{
					if(selectedItem == NULL || selectedItem->fSubmenu == NULL) break;
					if(selectedItem->fSubmenu->Window() == NULL) selectedItem->ShowSubmenu(true);
				}
			}
			break;

		case E_DOWN_ARROW:
			{
				if(fLayout == E_ITEMS_IN_ROW)
				{
					if(selectedItem)
					{
						if(e_is_instance_of(Window(), ESubmenuWindow))
						{
							if(Supermenu()->Layout() != E_ITEMS_IN_ROW && selectedItem->fSubmenu == NULL)
							{
								if(Supermenu()->Window() != NULL)
								{
									EMessage aMsg = *(Window()->CurrentMessage());
									aMsg.AddBool("_MENU_EVENT_", true);
									aMsg.ReplaceInt64("when", e_real_time_clock_usecs());
									aMsg.what = E_KEY_DOWN;
									Supermenu()->Window()->PostMessage(&aMsg, Supermenu());
									aMsg.what = E_KEY_UP;
									Supermenu()->Window()->PostMessage(&aMsg, Supermenu());
								}
								break;
							}
						}

						if(selectedItem->fSubmenu != NULL) selectedItem->ShowSubmenu(true);
					}
				}
				else if(fShowSubmenuByKeyDown)
				{
					if(selectedItem == NULL || selectedItem->fSubmenu == NULL) break;
					if(selectedItem->fSubmenu->Window() == NULL) selectedItem->ShowSubmenu(true);
				}
			}
			break;

		case E_LEFT_ARROW:
			{
				if(fLayout == E_ITEMS_IN_COLUMN)
				{
					if(!e_is_instance_of(Window(), ESubmenuWindow)) break;
					if(Supermenu()->Layout() != E_ITEMS_IN_COLUMN && Supermenu()->Window() != NULL)
					{
						EMessage aMsg = *(Window()->CurrentMessage());
						aMsg.AddBool("_MENU_EVENT_", true);
						aMsg.ReplaceInt64("when", e_real_time_clock_usecs());
						aMsg.what = E_KEY_DOWN;
						Supermenu()->Window()->PostMessage(&aMsg, Supermenu());
						aMsg.what = E_KEY_UP;
						Supermenu()->Window()->PostMessage(&aMsg, Supermenu());
					}
					else
					{
						Window()->PostMessage(E_QUIT_REQUESTED);
					}
				}
				else if(fShowSubmenuByKeyDown)
				{
					if(selectedItem == NULL || selectedItem->fSubmenu == NULL) break;
					if(selectedItem->fSubmenu->Window() == NULL) selectedItem->ShowSubmenu(true);
				}
			}
			break;

		case E_RIGHT_ARROW:
			{
				if(fLayout == E_ITEMS_IN_COLUMN)
				{
					if(selectedItem)
					{
						if(e_is_instance_of(Window(), ESubmenuWindow))
						{
							if(Supermenu()->Layout() != E_ITEMS_IN_COLUMN && selectedItem->fSubmenu == NULL)
							{
								if(Supermenu()->Window() != NULL)
								{
									EMessage aMsg = *(Window()->CurrentMessage());
									aMsg.AddBool("_MENU_EVENT_", true);
									aMsg.ReplaceInt64("when", e_real_time_clock_usecs());
									aMsg.what = E_KEY_DOWN;
									Supermenu()->Window()->PostMessage(&aMsg, Supermenu());
									aMsg.what = E_KEY_UP;
									Supermenu()->Window()->PostMessage(&aMsg, Supermenu());
								}
								break;
							}
						}

						if(selectedItem->fSubmenu != NULL) selectedItem->ShowSubmenu(true);
					}
				}
				else if(fShowSubmenuByKeyDown)
				{
					if(selectedItem == NULL || selectedItem->fSubmenu == NULL) break;
					if(selectedItem->fSubmenu->Window() == NULL) selectedItem->ShowSubmenu(true);
				}
			}
			break;

		case E_ESCAPE:
			{
				if(e_is_instance_of(Window(), ESubmenuWindow)) Window()->PostMessage(E_QUIT_REQUESTED);
				if(Supermenu()) Supermenu()->fShowSubmenuByKeyDown = false;
			}
			break;

		case E_HOME:
		case E_END:
			{
				if(!fShowSubmenuByKeyDown) break;
				if(selectedItem == NULL || selectedItem->fSubmenu == NULL) break;
				if(selectedItem->fSubmenu->Window() == NULL) selectedItem->ShowSubmenu(true);
			}
			break;

		default:
			break;
	}

	fTrackingIndex = -1;
}


ERect
EMenu::ItemFrame(eint32 index) const
{
	if(index < 0 || index >= fMenuItems.CountItems()) return ERect();

	if(fLayout == E_ITEMS_IN_MATRIX)
	{
		EMenuItem *item = (EMenuItem*)fMenuItems.ItemAt(index);
		return item->fFrame;
	}

	float left = fMargins.left, top = fMargins.top, w, h;

	for(eint32 i = 0; i <= index; i++)
	{
		EMenuItem *item = (EMenuItem*)fMenuItems.ItemAt(i);

		w = 0; h = 0;
		item->GetContentSize(&w, &h);
		if(w < 4) w = 4;
		if(h < 4) h = 4;

		if(i == index) break;

		if(fLayout == E_ITEMS_IN_ROW) left += w + ETK_MENU_ROW_SPACING;
		else top += h + ETK_MENU_COLUMN_SPACING;
	}

	if(fLayout == E_ITEMS_IN_ROW)
		return ERect(left, fMargins.top, left + w, Frame().Height() - fMargins.bottom);
	else
		return ERect(fMargins.left, top, Frame().Width() - fMargins.right, top + h);
}


eint32
EMenu::FindItem(EPoint where)
{
	if(!VisibleBounds().Contains(where)) return false;

	if(fLayout == E_ITEMS_IN_MATRIX)
	{
		for(eint32 i = 0; i < fMenuItems.CountItems(); i++)
		{
			EMenuItem *item = (EMenuItem*)fMenuItems.ItemAt(i);
			if(item->fFrame.Contains(where)) return i;
		}

		return -1;
	}

	float left = fMargins.left, top = fMargins.top, w, h;

	for(eint32 i = 0; i < fMenuItems.CountItems(); i++)
	{
		EMenuItem *item = (EMenuItem*)fMenuItems.ItemAt(i);

		w = 0; h = 0;
		item->GetContentSize(&w, &h);
		if(w < 4) w = 4;
		if(h < 4) h = 4;

		ERect r;
		if(fLayout == E_ITEMS_IN_ROW)
			r = ERect(left, fMargins.top, left + w, Frame().Height() - fMargins.bottom);
		else
			r = ERect(fMargins.left, top, Frame().Width() - fMargins.right, top + h);

		if(r.Contains(where)) return i;

		if(fLayout == E_ITEMS_IN_ROW) left += w + ETK_MENU_ROW_SPACING;
		else top += h + ETK_MENU_COLUMN_SPACING;
	}

	return -1;
}


void
EMenu::Refresh()
{
	if(e_is_instance_of(Window(), ESubmenuWindow) || fResizeToFit)
	{
		ERect rect = Frame();
		ResizeToPreferred();
		if(rect != Frame() && e_is_instance_of(Window(), ESubmenuWindow))
		{
			EMessage msg(_MENU_EVENT_);
			msg.AddInt64("when", e_real_time_clock_usecs());
			msg.AddRect("frame", Frame());
			Window()->PostMessage(&msg, Window());
		}
	}

	Invalidate();
}


void
EMenu::Draw(ERect updateRect)
{
	if(!IsVisible()) return;

	ERect bounds = Frame().OffsetToSelf(E_ORIGIN);
	bounds.left += fMargins.left;
	bounds.top += fMargins.top;
	bounds.right -= fMargins.right;
	bounds.bottom -= fMargins.bottom;
	bounds &= updateRect;
	if(!bounds.IsValid()) return;

	PushState();

	if(fLayout == E_ITEMS_IN_MATRIX)
	{
		for(eint32 i = 0; i < fMenuItems.CountItems(); i++)
		{
			EMenuItem *item = (EMenuItem*)fMenuItems.ItemAt(i);
			ERect r = item->fFrame;
			r &= bounds;
			if(!r.IsValid()) continue;
			ConstrainClippingRegion(r);
			item->Draw();
		}
	}
	else
	{
		float left = fMargins.left, top = fMargins.top, w, h;

		for(eint32 i = 0; i < fMenuItems.CountItems(); i++)
		{
			EMenuItem *item = (EMenuItem*)fMenuItems.ItemAt(i);

			w = 0; h = 0;
			item->GetContentSize(&w, &h);
			if(w < 4) w = 4;
			if(h < 4) h = 4;

			ERect r;
			if(fLayout == E_ITEMS_IN_ROW)
				r = ERect(left, fMargins.top, left + w, Frame().Height() - fMargins.bottom);
			else
				r = ERect(fMargins.left, top, Frame().Width() - fMargins.right, top + h);

			r &= bounds;
			if(r.IsValid())
			{
				ConstrainClippingRegion(r);
				item->Draw();
			}

			if(fLayout == E_ITEMS_IN_ROW) left += w + ETK_MENU_ROW_SPACING;
			else top += h + ETK_MENU_COLUMN_SPACING;
		}
	}

	PopState();
}


e_menu_layout
EMenu::Layout() const
{
	return fLayout;
}


void
EMenu::SetLayout(e_menu_layout layout, float width, float height, bool resizeToFit)
{
	fLayout = layout;
	fResizeToFit = resizeToFit;

	if(layout == E_ITEMS_IN_MATRIX)
	{
		if(width < fMargins.left + fMargins.right + 10) width = fMargins.left + fMargins.right + 10;
		if(height < fMargins.top + fMargins.bottom + 10) height = fMargins.top + fMargins.bottom + 10;

		ResizeTo(width, height);
	}

	Refresh();
}


void
EMenu::SetRadioMode(bool state)
{
	if(fRadioMode != state)
	{
		fRadioMode = state;

		if(state)
		{
			FindMarked(&fMarkedIndex);

			if(fLabelFromMarked && fSuperitem != NULL)
			{
				EMenuItem *markedItem = ItemAt(fMarkedIndex);
				if(markedItem)
					fSuperitem->SetLabel(markedItem->Label());
				else
					fSuperitem->SetLabel(Name());
			}
		}
		else
		{
			fMarkedIndex = -1;
			SetLabelFromMarked(false);
			if(fLabelFromMarked)
			{
				fLabelFromMarked = false;
				if(fSuperitem) fSuperitem->SetLabel(Name());
			}
		}

		Invalidate();
	}
}


bool
EMenu::IsRadioMode() const
{
	return fRadioMode;
}


EMenuItem*
EMenu::FindMarked(eint32 *index) const
{
	if(fRadioMode && fMarkedIndex >= 0)
	{
		EMenuItem *item = (EMenuItem*)fMenuItems.ItemAt(fMarkedIndex);
		if(!(item == NULL || item->IsMarked() == false))
		{
			if(index) *index = fMarkedIndex;
			return item;
		}
	}

	for(eint32 i = 0; i < fMenuItems.CountItems(); i++)
	{
		EMenuItem *item = (EMenuItem*)fMenuItems.ItemAt(i);

		if(item->IsMarked())
		{
			if(index) *index = i;
			return item;
		}
	}

	if(index) *index = -1;
	return NULL;
}


void
EMenu::SetLabelFromMarked(bool state)
{
	if(fLabelFromMarked != state)
	{
		if(state)
		{
			SetRadioMode(true);
			if(!fRadioMode) return;
		}

		fLabelFromMarked = state;
		if(fSuperitem)
		{
			if(state)
			{
				EMenuItem *markedItem = FindMarked();
				if(markedItem)
					fSuperitem->SetLabel(markedItem->Label());
				else
					fSuperitem->SetLabel(Name());
			}
			else
			{
					fSuperitem->SetLabel(Name());
			}
		}
	}
}


bool
EMenu::IsLabelFromMarked() const
{
	return(fRadioMode ? fLabelFromMarked : false);
}


void
EMenu::GetPreferredSize(float *width, float *height)
{
	if(!width && !height) return;

	if(fLayout == E_ITEMS_IN_MATRIX)
	{
		ERect rect(0, 0, 0, 0);

		for(eint32 i = 0; i < fMenuItems.CountItems(); i++)
		{
			EMenuItem *item = (EMenuItem*)fMenuItems.ItemAt(i);

			ERect r = item->fFrame;
			if(r.IsValid()) rect |= r;
		}

		if(width) *width = rect.Width();
		if(height) *height = rect.Height();
	}
	else
	{
		ERect rect(0, 0, 0, 0);

		for(eint32 i = 0; i < fMenuItems.CountItems(); i++)
		{
			EMenuItem *item = (EMenuItem*)fMenuItems.ItemAt(i);

			float w = 0, h = 0;
			item->GetContentSize(&w, &h);
			if(w < 4) w = 4;
			if(h < 4) h = 4;

			if(fLayout == E_ITEMS_IN_ROW)
			{
				w += (i == fMenuItems.CountItems() - 1 ? 0 : ETK_MENU_ROW_SPACING);
				rect.right += w;
				rect.bottom = max_c(rect.bottom, h);
			}
			else
			{
				h += (i == fMenuItems.CountItems() - 1 ? 0 : ETK_MENU_COLUMN_SPACING);
				rect.bottom += h;
				rect.right = max_c(rect.right, w);
			}
		}

		if(width) *width = rect.Width();
		if(height) *height = rect.Height();
	}

	if(width) *width += fMargins.left + fMargins.right;
	if(height) *height += fMargins.top + fMargins.bottom;
}


class ESubmenuView : public EView {
public:
	ESubmenuView(ERect frame);
	virtual ~ESubmenuView();

	virtual void Draw(ERect updateRect);
};


ESubmenuView::ESubmenuView(ERect frame)
	: EView(frame, NULL, E_FOLLOW_ALL, E_WILL_DRAW)
{
}


ESubmenuView::~ESubmenuView()
{
}


void
ESubmenuView::Draw(ERect updateRect)
{
	if(!(Bounds().InsetByCopy(1, 1).Contains(updateRect)))
	{
		SetDrawingMode(E_OP_COPY);
		SetPenSize(1);
		e_rgb_color borderColor = e_ui_color(E_MENU_BORDER_COLOR);

		ESubmenuWindow *win = e_cast_as(Window(), ESubmenuWindow);
		if(win->fMenu == NULL || win->fMenu->IsEnabled() == false) borderColor.mix(0, 0, 0, 20);

		SetHighColor(borderColor);
		StrokeRect(Bounds());
	}
}


ESubmenuWindow::ESubmenuWindow(EPoint where, EMenu *menu)
	: EWindow(ERect(0, 0, 1, 1), NULL, E_NO_BORDER_WINDOW_LOOK, E_FLOATING_APP_WINDOW_FEEL, E_AVOID_FOCUS), fMenu(NULL)
{
	Lock();
	if(!(menu == NULL || menu->Supermenu() == NULL || menu->Supermenu()->Window() == NULL))
	{
		if(ProxyBy(menu->Supermenu()->Window()))
		{
			ESubmenuView *topView = new ESubmenuView(Bounds());
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
		}
	}
}


ESubmenuWindow::~ESubmenuWindow()
{
}


bool
ESubmenuWindow::IsDependsOnOthersWhenQuitRequested() const
{
	return true;
}


void
ESubmenuWindow::DispatchMessage(EMessage *msg, EHandler *target)
{
	if(target == this && msg->what == _MENU_EVENT_)
	{
		if(fMenu)
		{
			EPoint where;
			ERect frame;
			if(!(msg->FindRect("frame", &frame) == false || frame.IsValid() == false))
				ResizeTo(frame.Width() + 4, frame.Height() + 4);
			if(fMenu->GetPopUpWhere(&where)) MoveTo(where);
			else PostMessage(E_QUIT_REQUESTED);
		}

		return;
	}

	EWindow::DispatchMessage(msg, target);
}


void
ESubmenuWindow::FrameMoved(EPoint new_position)
{
	if(IsHidden() || fMenu == NULL || fMenu->Supermenu() == NULL || fMenu->Supermenu()->Window() == NULL) return;
	fMenu->Supermenu()->Window()->SendBehind(this);
}


bool
ESubmenuWindow::QuitRequested()
{
	if(!(fMenu == NULL || fMenu->Window() != this))
	{
		EMenu *menu = fMenu;

		Hide();
		fMenu = NULL;

		menu->RemoveSelf();
	}

	return true;
}


bool
EMenu::PopUp(EPoint where, bool selectFirstItem)
{
	if(Window() != NULL) return false;
	if(Supermenu() == NULL || Supermenu()->Window() == NULL) return false;

	ESubmenuWindow *win = new ESubmenuWindow(where, this);
	if(!win) return false;

	if(Window() != win)
	{
		win->Quit();
		return false;
	}

	fSelectedIndex = -1;
	if(selectFirstItem)
	{
		for(eint32 i = 0; i < fMenuItems.CountItems(); i++)
		{
			EMenuItem *newItem = (EMenuItem*)fMenuItems.ItemAt(i);
			fSelectedIndex = i;
			if(newItem->SelectChanged()) break;
			fSelectedIndex = -1;
		}
	}

	// TODO: show after a while
	win->Show();
	Supermenu()->Window()->SendBehind(win);

	win->Unlock();

	return true;
}


void
EMenu::ClosePopUp()
{
	ESubmenuWindow *win = e_cast_as(Window(), ESubmenuWindow);
	if(!(win == NULL || win->fMenu != this))
	{
		win->Hide();
		win->fMenu = NULL;

		RemoveSelf();

		if(win->Thread() == etk_get_current_thread_id())
		{
			win->Lock();
			win->Quit();
		}
		else
		{
			win->PostMessage(E_QUIT_REQUESTED);
		}
	}
}


//#define TEMP_DEBUG(Exp)	{if((Exp)) ETK_DEBUG("[INTERFACE]: %s --- %s", __PRETTY_FUNCTION__, #Exp);}

bool
EMenu::GetPopUpWhere(EPoint *where)
{
	if(!where) return false;

	if(Supermenu() == NULL || Supermenu()->Window() == NULL ||
	   Supermenu()->Window()->IsHidden() || Supermenu()->Window()->IsMinimized())
	{
//		TEMP_DEBUG(Supermenu() == NULL || Supermenu()->Window() == NULL);
//		if(!(Supermenu() == NULL || Supermenu()->Window() == NULL))
//		{
//			TEMP_DEBUG(Supermenu()->Window()->IsHidden());
//			TEMP_DEBUG(Supermenu()->Window()->IsMinimized());
//			TEMP_DEBUG(!e_is_instance_of(Supermenu()->Window(), ESubmenuWindow));
//		}

		return false;
	}

	if(Supermenu()->Window()->IsActivate() == false)
	{
		if(!(e_is_instance_of(Supermenu()->Window(), ESubmenuWindow) || e_is_instance_of(Supermenu(), EPopUpMenu))) return false;
		if(e_is_instance_of(Supermenu(), EPopUpMenu))
		{
			if(e_cast_as(Supermenu(), EPopUpMenu)->IsPopUpByGo() == false) return false;
		}
	}

	ERect frame = Superitem()->Frame();
	frame.left -= Supermenu()->fMargins.left;
	frame.top -= Supermenu()->fMargins.top;
	frame.right += Supermenu()->fMargins.right;
	frame.bottom += Supermenu()->fMargins.bottom;
	frame &= Supermenu()->VisibleBounds();

	if(frame.IsValid() == false) return false;

	if(Supermenu()->Layout() == E_ITEMS_IN_COLUMN)
		*where = frame.RightTop() + EPoint(1, 0);
	else
		*where = frame.LeftBottom() + EPoint(0, 1);
	Supermenu()->ConvertToScreen(where);

	return true;
}


void
EMenu::SetItemMargins(float left, float top, float right, float bottom)
{
	if(left < 0) left = 0;
	if(top < 0) top = 0;
	if(right < 0) right = 0;
	if(bottom < 0) bottom = 0;

	if(fMargins != ERect(left, top, right, bottom))
	{
		fMargins = ERect(left, top, right, bottom);
		Refresh();
	}
}


void
EMenu::GetItemMargins(float *left, float *top, float *right, float *bottom) const
{
	if(left) *left = fMargins.left;
	if(top) *top = fMargins.top;
	if(right) *right = fMargins.right;
	if(bottom) *bottom = fMargins.bottom;
}


EMenuItem*
EMenu::CurrentSelection() const
{
	if(Window() == NULL) return NULL;
	return (EMenuItem*)fMenuItems.ItemAt(fSelectedIndex);
}


void
EMenu::ItemInvoked(EMenuItem *item)
{
	if(fRadioMode == false || item == NULL || item->fEnabled == false) return;
	item->SetMarked(true);
}


void
EMenu::SelectItem(EMenuItem *item, bool showSubmenu, bool selectFirstItem)
{
	if(!(item == NULL || item->fMenu == this)) return;
	eint32 newIndex = (item ? fMenuItems.IndexOf((void*)item) : -1);

	if(fSelectedIndex != newIndex && newIndex >= 0)
	{
		EWindow *win = Window();

		EMenuItem *oldItem = (EMenuItem*)fMenuItems.ItemAt(fSelectedIndex);
		EMenuItem *newItem = (EMenuItem*)fMenuItems.ItemAt(newIndex);

		eint32 oldSelectedIndex = fSelectedIndex;
		fSelectedIndex = newIndex;

		if(win) win->DisableUpdates();
		if(newItem->SelectChanged())
		{
			if(oldItem) oldItem->SelectChanged();
		}
		else
		{
			fSelectedIndex = oldSelectedIndex;
		}
		if(win) win->EnableUpdates();
	}
	else
	{
		EMenuItem *oldItem = (EMenuItem*)fMenuItems.ItemAt(fSelectedIndex);
		fSelectedIndex = -1;
		if(oldItem) oldItem->SelectChanged();
	}

	fTrackingIndex = -1;
}


void
EMenu::Hide()
{
	EView::Hide();
	if(IsHidden())
	{
		if(fSelectedIndex >= 0)
		{
			EMenuItem *item = (EMenuItem*)fMenuItems.ItemAt(fSelectedIndex);
			fSelectedIndex = -1;
			if(item) item->SelectChanged();
		}

		fTrackingIndex = -1;
	}
}

