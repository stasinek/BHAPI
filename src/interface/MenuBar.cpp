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
 * File: MenuBar.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../support/ClassInfo.h"

#include "Window.h"
#include "MenuBar.h"
#include "MenuField.h"


EMenuBar::EMenuBar(ERect frame, const char *title, euint32 resizeMode, e_menu_layout layout, bool resizeToFit)
	: EMenu(frame, title, resizeMode, E_WILL_DRAW, layout, resizeToFit), fBorder(E_BORDER_FRAME)
{
	SetEventMask(E_POINTER_EVENTS);
}


EMenuBar::~EMenuBar()
{
}


void
EMenuBar::MakeFocus(bool state)
{
	EMenuField *parent = e_cast_as(Parent(), EMenuField);
	if(!(state == false || parent == NULL || parent->MenuBar() != this)) return;

	EMenu::MakeFocus(state);
	if(!IsFocus() && CurrentSelection() != NULL) SelectItem(NULL);
}


void
EMenuBar::MouseDown(EPoint where)
{
	EMenu::MouseDown(where);

	if(Window() == NULL) return;

	EMenuItem *item = CurrentSelection();
	if(item != NULL)
	{
		if(VisibleBounds().Contains(where) == false)
		{
			ConvertToScreen(&where);
			EMenu *submenu;
			while(true)
			{
				submenu = (item ? item->Submenu() : NULL);
				if(submenu == NULL || submenu->Window() == NULL || submenu->Window()->Frame().Contains(where))
				{
					if(submenu == NULL || submenu->Window() == NULL)
					{
						SelectItem(NULL);
						MakeFocus(false);
					}
					break;
				}

				item = submenu->CurrentSelection();
			}
		}
		else
		{
			bool found = false;
			for(eint32 i = 0; i < CountItems(); i++)
				if(ItemFrame(i).Contains(where)) found = true;
			if(found == false)
			{
				SelectItem(NULL);
				MakeFocus(false);
			}
		}
	}

	if(CurrentSelection() != NULL) MakeFocus(true);
}


void
EMenuBar::MouseUp(EPoint where)
{
	EMenu::MouseUp(where);

	if(Window() == NULL) return;

	EMenuItem *item = CurrentSelection();
	if(item != NULL)
	{
		if(VisibleBounds().Contains(where) == false)
		{
			ConvertToScreen(&where);
			EMenu *submenu;
			while(true)
			{
				submenu = (item ? item->Submenu() : NULL);
				if(submenu == NULL || submenu->Window() == NULL || submenu->Window()->Frame().Contains(where))
				{
					if(submenu == NULL || submenu->Window() == NULL)
					{
						SelectItem(NULL);
						MakeFocus(false);
					}
					break;
				}

				item = submenu->CurrentSelection();
			}
		}
		else
		{
			bool found = false;
			for(eint32 i = 0; i < CountItems(); i++)
				if(ItemFrame(i).Contains(where)) found = true;
			if(found == false)
			{
				SelectItem(NULL);
				MakeFocus(false);
			}
		}
	}

	if(CurrentSelection() != NULL) MakeFocus(true);
}


void
EMenuBar::ItemInvoked(EMenuItem *item)
{
	EMenu::ItemInvoked(item);
	SelectItem(NULL);
	MakeFocus(false);
}


void
EMenuBar::MouseMoved(EPoint where, euint32 code, const EMessage *a_message)
{
	if(CurrentSelection() == NULL) return;
	EMenu::MouseMoved(where, code, a_message);
}


void
EMenuBar::KeyDown(const char *bytes, eint32 numBytes)
{
	if(CurrentSelection() == NULL) return;
	EMenu::KeyDown(bytes, numBytes);
}


void
EMenuBar::KeyUp(const char *bytes, eint32 numBytes)
{
	if(CurrentSelection() == NULL) return;
	EMenu::KeyUp(bytes, numBytes);
}


void
EMenuBar::MessageReceived(EMessage *msg)
{
	EMenu::MessageReceived(msg);

	if(Window() == NULL) return;

	switch(msg->what)
	{
		case E_OBSERVER_NOTICE_CHANGE:
			{
				euint32 what;
				if(msg->FindInt32(E_OBSERVE_ORIGINAL_WHAT, (eint32*)&what) == false ||
				   !(what == E_MINIMIZED || what == E_WINDOW_ACTIVATED)) break;

				if(Window()->IsActivate() == false || Window()->IsHidden() || Window()->IsMinimized())
				{
					SelectItem(NULL);
					MakeFocus(false);
				}
			}
			break;

		case _MENU_EVENT_:
			{
				EMenuItem *item = NULL;
				if(msg->FindPointer("source", (void**)&item) == false || item == NULL) break;
				SelectItem(NULL);
				MakeFocus(false);
			}
			break;

		default:
			break;
	}
}


void
EMenuBar::Draw(ERect updateRect)
{
	if(!IsVisible()) return;

	EMenu::Draw(updateRect);
	if(fBorder == E_BORDER_NONE) return;

	e_rgb_color shineColor = e_ui_color(E_SHINE_COLOR);
	e_rgb_color shadowColor = e_ui_color(E_SHADOW_COLOR);

	if(!IsEnabled())
	{
		shineColor.disable(ViewColor());
		shadowColor.disable(ViewColor());
	}

	switch(fBorder)
	{
		case E_BORDER_FRAME:
			{
				ERect rect = Frame().OffsetToSelf(E_ORIGIN);

				PushState();
				SetHighColor(shadowColor);
				StrokeRect(rect);
				SetHighColor(shineColor);
				StrokeLine(rect.LeftBottom(), rect.LeftTop());
				StrokeLine(rect.RightTop());
				PopState();
			}
			break;

		default:
			// TODO
			break;
	}
}


void
EMenuBar::SetBorder(e_menu_bar_border border)
{
	if(fBorder != border)
	{
		fBorder = border;
		Invalidate();
	}
}


e_menu_bar_border
EMenuBar::Border() const
{
	return fBorder;
}

