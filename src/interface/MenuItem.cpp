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
 * File: MenuItem.cpp
 *
 * --------------------------------------------------------------------------*/

#include <ctype.h>
#include <string.h>

#include "./../support/String.h>
#include "./../support/ClassInfo.h>

#include "Window.h"
#include "Menu.h"
#include "MenuItem.h"


EMenuItem::EMenuItem(const char *label, EMessage *message, char shortcut, euint32 modifiers)
	: EArchivable(), EInvoker(message, NULL, NULL),
	  fShortcut(0), fModifiers(0), fMarked(false), fEnabled(true),
	  fLabel(NULL), fShortcuts(NULL), fSubmenu(NULL), fMenu(NULL)
{
	SetShortcut(shortcut, modifiers);
	if(label) fLabel = EStrdup(label);
}


EMenuItem::EMenuItem(EMenu *menu, EMessage *message)
	: EArchivable(), EInvoker(message, NULL, NULL),
	  fShortcut(0), fModifiers(0), fMarked(false), fEnabled(true),
	  fLabel(NULL), fShortcuts(NULL), fSubmenu(NULL), fMenu(NULL)
{
	if(menu)
	{
		if(menu->fSuperitem == NULL)
		{
			fSubmenu = menu;
			if(menu->Name() != NULL) fLabel = EStrdup(menu->Name());
			menu->fSuperitem = this;
			if(menu->EView::IsEnabled() == false) fEnabled = false;
		}
		else
		{
			ETK_ERROR("[INTERFACE]: %s --- The menu already attached to other item.", __PRETTY_FUNCTION__);
		}
	}
}


EMenuItem::~EMenuItem()
{
	if(fMenu)
	{
		ETK_WARNING("[INTERFACE]: %s --- Item still attach to menu, detaching from menu automatically.", __PRETTY_FUNCTION__);
		if(fMenu->RemoveItem(this) == false)
			ETK_ERROR("[INTERFACE]: %s --- Detaching from menu failed.", __PRETTY_FUNCTION__);
	}

	if(fLabel) delete[] fLabel;
	if(fShortcuts) delete[] fShortcuts;

	if(fSubmenu)
	{
		fSubmenu->fSuperitem = NULL;
		fSubmenu->ClosePopUp();
		delete fSubmenu;
	}
}


void
EMenuItem::SetLabel(const char *label)
{
	if((fLabel == NULL || strlen(fLabel) == 0) && (label == NULL || strlen(label) == 0)) return;
	if(!((fLabel == NULL || strlen(fLabel) == 0) || (label == NULL || strlen(label) == 0) || strcmp(fLabel, label) != 0)) return;

	ERect oldFrame = Frame();

	if(fLabel)
	{
		delete[] fLabel;
		fLabel = NULL;
	}

	if(label) fLabel = EStrdup(label);

	if(fMenu == NULL) return;

	if(fMenu->fResizeToFit)
		fMenu->Refresh();
	else
		fMenu->Invalidate(oldFrame | Frame());
}


void
EMenuItem::SetEnabled(bool state)
{
	if(fEnabled != state)
	{
		fEnabled = state;
		if(!(fSubmenu == NULL || fSubmenu->EView::IsEnabled() != state)) fSubmenu->SetEnabled(state);
		if(fMenu) fMenu->Invalidate(Frame());
	}
}


void
EMenuItem::SetMarked(bool state)
{
	if(fMarked != state || !(fMenu == NULL || fMenu->fRadioMode == false))
	{
		fMarked = state;
		if(fMenu)
		{
			eint32 index = fMenu->IndexOf(this);

			if(fMenu->fRadioMode)
			{
				if(fMarked)
				{
					if(fMenu->fMarkedIndex != index)
					{
						eint32 oldIndex = fMenu->fMarkedIndex;
						fMenu->fMarkedIndex = index;
						if(oldIndex >= 0) fMenu->Invalidate(fMenu->ItemFrame(oldIndex));
					}
					if(fMenu->fLabelFromMarked && fMenu->fSuperitem) fMenu->fSuperitem->SetLabel(Label());
				}
				else if(fMenu->fMarkedIndex == index)
				{
					fMenu->fMarkedIndex = -1;
					fMenu->Invalidate(fMenu->ItemFrame(index));
					if(fMenu->fLabelFromMarked && fMenu->fSuperitem) fMenu->fSuperitem->SetLabel(fMenu->Name());
				}
			}

			fMenu->Invalidate(fMenu->ItemFrame(index));
		}
	}
}


void
EMenuItem::SetShortcut(char ch, euint32 modifiers)
{
	if(fShortcut != ch || fModifiers != modifiers)
	{
		// TODO: update shortcut when window attached
		ERect oldFrame = Frame();

		fShortcut = ch;
		fModifiers = modifiers;

		if(fShortcuts) delete[] fShortcuts;
		fShortcuts = NULL;

		if(fShortcut != 0 && fModifiers != 0)
		{
			EString str;
			if(fModifiers & E_CONTROL_KEY) str << "Ctrl+";
			if(fModifiers & E_SHIFT_KEY) str << "Shift+";
			if(fModifiers & E_COMMAND_KEY) str << "Alt+";

			if(fModifiers & E_FUNCTIONS_KEY)
			{
				switch(fShortcut)
				{
					case E_F1_KEY: str << "F1"; break;
					case E_F2_KEY: str << "F2"; break;
					case E_F3_KEY: str << "F3"; break;
					case E_F4_KEY: str << "F4"; break;
					case E_F5_KEY: str << "F5"; break;
					case E_F6_KEY: str << "F6"; break;
					case E_F7_KEY: str << "F7"; break;
					case E_F8_KEY: str << "F8"; break;
					case E_F9_KEY: str << "F9"; break;
					case E_F10_KEY: str << "F10"; break;
					case E_F11_KEY: str << "F11"; break;
					case E_F12_KEY: str << "F12"; break;
					case E_PRINT_KEY: str << "Print"; break;
					case E_SCROLL_KEY: str << "Scroll"; break;
					case E_PAUSE_KEY: str << "Pause"; break;
					default: str.MakeEmpty(); break; // here don't support
				}
			}
			else switch(fShortcut)
			{
				case E_ENTER: str << "Enter"; break;
				case E_BACKSPACE: str << "Backspace"; break;
				case E_SPACE: str << "Space"; break;
				case E_TAB: str << "Tab"; break;
				case E_ESCAPE: str << "Esc"; break;
				case E_LEFT_ARROW: str << "Left"; break;
				case E_RIGHT_ARROW: str << "Right"; break;
				case E_UP_ARROW: str << "Up"; break;
				case E_DOWN_ARROW: str << "Down"; break;
				case E_INSERT: str << "Insert"; break;
				case E_DELETE: str << "Delete"; break;
				case E_HOME: str << "Home"; break;
				case E_END: str << "End"; break;
				case E_PAGE_UP: str << "PageUp"; break;
				case E_PAGE_DOWN: str << "PageDown"; break;
				default: str.Append((char)toupper(fShortcut), 1);
			}

			if(str.Length() > 0) fShortcuts = EStrdup(str.String());
		}

		if(fMenu == NULL) return;

		if(fMenu->fResizeToFit)
			fMenu->Refresh();
		else
			fMenu->Invalidate(oldFrame | Frame());
	}
}


const char*
EMenuItem::Label() const
{
	return fLabel;
}


bool
EMenuItem::IsEnabled() const
{
	return fEnabled;
}


bool
EMenuItem::IsMarked() const
{
	return fMarked;
}


char
EMenuItem::Shortcut(euint32 *modifiers) const
{
	if(modifiers) *modifiers = fModifiers;
	return fShortcut;
}


EMenu*
EMenuItem::Submenu() const
{
	return fSubmenu;
}


EMenu*
EMenuItem::Menu() const
{
	return fMenu;
}


ERect
EMenuItem::Frame() const
{
	if(!fMenu) return ERect();
	eint32 index = fMenu->IndexOf(this);
	return fMenu->ItemFrame(index);
}


bool
EMenuItem::IsSelected() const
{
	if(!fMenu) return false;
	return(fMenu->CurrentSelection() == this);
}


void
EMenuItem::GetContentSize(float *width, float *height) const
{
	if(!width && !height || !fMenu) return;

	EFont font(etk_plain_font);

	if(width)
	{
		*width = Label() ? (float)ceil((double)font.StringWidth(Label())) : 0;

		if(fMenu->Layout() == E_ITEMS_IN_COLUMN)
		{
			if(fSubmenu)
			{
				e_font_height fontHeight;
				font.GetHeight(&fontHeight);
				*width += (float)ceil((double)(fontHeight.ascent + fontHeight.descent) / 2.f) + 10;
			}
			else if(fShortcuts)
				*width += (float)ceil((double)font.StringWidth(fShortcuts)) + 30;

			*width += 20;
		}
		else
		{
			*width += 10;
		}
	}

	if(height)
	{
		e_font_height fontHeight;
		font.GetHeight(&fontHeight);
		*height = (float)ceil((double)(fontHeight.ascent + fontHeight.descent));
		*height += 4;
	}
}


void
EMenuItem::DrawContent()
{
	if(fMenu == NULL || fMenu->Window() == NULL) return;

	fMenu->PushState();

	EFont font(etk_plain_font);
	e_font_height fontHeight;
	font.GetHeight(&fontHeight);
	fMenu->SetFont(&font, E_FONT_ALL);

	ERect rect = Frame().InsetByCopy(2, 2);
	if(fMenu->Layout() == E_ITEMS_IN_COLUMN) rect.left += 16;
	if(fMenu->PenLocation().x > rect.left && fMenu->PenLocation().x < rect.right) rect.left = fMenu->PenLocation().x;

	float sHeight = fontHeight.ascent + fontHeight.descent;
	EPoint location;
	if(fMenu->Layout() == E_ITEMS_IN_COLUMN)
		location.x = rect.left;
	else
		location.x = rect.Center().x - font.StringWidth(Label()) / 2.f;
	location.y = rect.Center().y - sHeight / 2.f;
	location.y += fontHeight.ascent + 1;

	e_rgb_color bkColor, textColor;

	if(fEnabled && fMenu->IsEnabled())
	{
		if(IsSelected())
		{
			bkColor = e_ui_color(E_MENU_SELECTED_BACKGROUND_COLOR);
			textColor = e_ui_color(E_MENU_SELECTED_ITEM_TEXT_COLOR);
		}
		else
		{
			bkColor = e_ui_color(E_MENU_BACKGROUND_COLOR);
			textColor = e_ui_color(E_MENU_ITEM_TEXT_COLOR);
		}
	}
	else
	{
		bkColor = e_ui_color(E_MENU_BACKGROUND_COLOR);
		if(fMenu->IsEnabled() == false) bkColor.mix(0, 0, 0, 20);

		textColor = e_ui_color(E_MENU_ITEM_TEXT_COLOR);
		e_rgb_color color = bkColor;
		color.alpha = 127;
		textColor.mix(color);
	}

	fMenu->SetDrawingMode(E_OP_COPY);
	fMenu->SetHighColor(textColor);
	fMenu->SetLowColor(bkColor);
	fMenu->DrawString(Label(), location);

	if(fMenu->Layout() == E_ITEMS_IN_COLUMN)
	{
		if(fSubmenu != NULL)
		{
			ERect r = Frame().InsetByCopy(5, 2);
			EPoint pt1, pt2, pt3;
			pt1.x = r.right;
			pt1.y = r.Center().y;
			pt2.x = pt3.x = pt1.x - (float)ceil((double)(fontHeight.ascent + fontHeight.descent) / 2.f);
			pt2.y = pt1.y - (float)ceil((double)(fontHeight.ascent + fontHeight.descent) / 3.5f);
			pt3.y = pt1.y + (float)ceil((double)(fontHeight.ascent + fontHeight.descent) / 3.5f);
			fMenu->FillTriangle(pt1, pt2, pt3);
		}
		else if(fShortcuts != NULL)
		{
			location.x = rect.right - font.StringWidth(fShortcuts);
			fMenu->DrawString(fShortcuts, location);
		}
	}

	fMenu->PopState();
}


void
EMenuItem::Draw()
{
	if(fMenu == NULL || fMenu->Window() == NULL || fMenu->IsVisible() == false) return;

	eint32 index = fMenu->IndexOf(this);
	ERect frame = fMenu->ItemFrame(index);
	if(index < 0 || frame.IsValid() == false) return;

	e_rgb_color bkColor, textColor;

	if(fEnabled && fMenu->IsEnabled())
	{
		if(IsSelected())
		{
			bkColor = e_ui_color(E_MENU_SELECTED_BACKGROUND_COLOR);
			textColor = e_ui_color(E_MENU_SELECTED_ITEM_TEXT_COLOR);
		}
		else
		{
			bkColor = e_ui_color(E_MENU_BACKGROUND_COLOR);
			textColor = e_ui_color(E_MENU_ITEM_TEXT_COLOR);
		}
	}
	else
	{
		bkColor = e_ui_color(E_MENU_BACKGROUND_COLOR);
		if(fMenu->IsEnabled() == false) bkColor.mix(0, 0, 0, 20);

		textColor = e_ui_color(E_MENU_ITEM_TEXT_COLOR);
		e_rgb_color color = bkColor;
		color.alpha = 127;
		textColor.mix(color);
	}

	fMenu->PushState();
	fMenu->SetDrawingMode(E_OP_COPY);
	fMenu->SetPenSize(1);
	fMenu->SetHighColor(bkColor);
	fMenu->FillRect(frame);

	if(fEnabled && fMenu->IsEnabled() && IsSelected())
	{
		fMenu->SetHighColor(e_ui_color(E_MENU_SELECTED_BORDER_COLOR));
		fMenu->StrokeRect(frame);
	}

	frame.InsetBy(2, 2);

	bool drawMarked = (fMenu->fRadioMode ? fMenu->fMarkedIndex == index : fMarked);

	if(fMenu->Layout() == E_ITEMS_IN_COLUMN)
	{
		if(drawMarked)
		{
			fMenu->SetHighColor(textColor);
			ERect rect = frame;
			rect.right = rect.left + 15;
			rect.InsetBy(3, 3);
			fMenu->MovePenTo(rect.LeftTop() + EPoint(0, rect.Height() / 2.f));
			fMenu->StrokeLine(rect.LeftBottom() + EPoint(rect.Width() / 2.f, 0));
			fMenu->StrokeLine(rect.RightTop());
		}

		frame.left += 16;
	}
	else
	{
		// TODO: draw marked
	}

	fMenu->MovePenTo(frame.LeftTop());
	fMenu->ConstrainClippingRegion(frame);

	DrawContent();

	fMenu->PopState();

	Highlight(IsSelected());
}


void
EMenuItem::Highlight(bool on)
{
}


void
EMenuItem::ShowSubmenu(bool selectFirstItem)
{
	if(fSubmenu == NULL) return;
	if(fSubmenu->Window() != NULL)
	{
		if(!(fMenu == NULL || fMenu->Window() == NULL)) fMenu->Window()->SendBehind(fSubmenu->Window());
		return;
	}

	EPoint where;
	if(fSubmenu->GetPopUpWhere(&where) == false) return;

	if(fSubmenu->PopUp(where, selectFirstItem) && Message() != NULL)
	{
		EMessage aMsg = *(Message());

		aMsg.AddInt64("when", e_real_time_clock_usecs());
		aMsg.AddPointer("source", this);
		if(fMenu) aMsg.AddInt32("index", fMenu->IndexOf(this));

		EInvoker::Invoke(&aMsg);
	}
}


bool
EMenuItem::SelectChanged()
{
	if(fMenu == NULL) return false;

	if(fMenu->Window())
	{
		if(fEnabled) fMenu->Invalidate(Frame());
		if(fSubmenu)
		{
			if(IsSelected() &&
			   !(fMenu->Window()->CurrentMessage() == NULL ||
			     fMenu->Window()->CurrentMessage()->what != E_MOUSE_MOVED))
				ShowSubmenu(true);
			else if(!IsSelected())
				fSubmenu->ClosePopUp();
		}
	}

	return fEnabled;
}


e_status_t
EMenuItem::Invoke(const EMessage *msg)
{
	if(!msg && Message() == NULL) return E_ERROR;

	EMessage aMsg = (msg ? *msg : *(Message()));

	aMsg.AddInt64("when", e_real_time_clock_usecs());
	aMsg.AddPointer("source", this);
	if(fMenu) aMsg.AddInt32("index", fMenu->IndexOf(this));

	return EInvoker::Invoke(&aMsg);
}


EMenuSeparatorItem::EMenuSeparatorItem()
	: EMenuItem(NULL, NULL, 0, 0)
{
}


EMenuSeparatorItem::~EMenuSeparatorItem()
{
}


void
EMenuSeparatorItem::GetContentSize(float *width, float *height) const
{
	if(Menu() == NULL || (!width && !height)) return;

	if(width) *width = 4;
	if(height) *height = 4;
}


bool
EMenuSeparatorItem::SelectChanged()
{
	return false;
}


void
EMenuSeparatorItem::Draw()
{
	ERect frame = Frame();
	if(Menu() == NULL || Menu()->Window() == NULL || !frame.IsValid()) return;

	e_rgb_color bkColor;

	if(IsEnabled() && Menu()->IsEnabled())
	{
		bkColor = e_ui_color(E_MENU_BACKGROUND_COLOR);
	}
	else
	{
		bkColor = e_ui_color(E_MENU_BACKGROUND_COLOR);
		if(Menu()->IsEnabled() == false) bkColor.mix(0, 0, 0, 20);
	}

	Menu()->PushState();

	Menu()->SetDrawingMode(E_OP_COPY);
	Menu()->SetPenSize(1);
	Menu()->SetHighColor(bkColor);
	Menu()->FillRect(frame);

	e_rgb_color shinerColor = Menu()->ViewColor();
	e_rgb_color darkerColor = shinerColor;
	shinerColor.mix(255, 255, 255, 100);
	darkerColor.mix(0, 0, 0, 100);

	if(Menu()->Layout() == E_ITEMS_IN_ROW || (Menu()->Layout() == E_ITEMS_IN_MATRIX && frame.Height() > frame.Width()))
	{
		Menu()->PushState();
		Menu()->ConstrainClippingRegion(frame);
		Menu()->SetDrawingMode(E_OP_COPY);
		Menu()->SetPenSize(1);
		Menu()->SetHighColor(shinerColor);
		frame.InsetBy(0, 2);
		EPoint pt1 = frame.Center();
		pt1.y = frame.top + 1;
		EPoint pt2 = pt1;
		pt2.y = frame.bottom - 1;
		Menu()->StrokeLine(pt1, pt2);
		pt1.x += 1; pt2.x += 1;
		Menu()->SetHighColor(darkerColor);
		Menu()->StrokeLine(pt1, pt2);
		Menu()->PopState();
	}
	else if(Menu()->Layout() == E_ITEMS_IN_COLUMN || (Menu()->Layout() == E_ITEMS_IN_MATRIX && frame.Width() >= frame.Height()))
	{
		Menu()->PushState();
		Menu()->ConstrainClippingRegion(frame);
		Menu()->SetDrawingMode(E_OP_COPY);
		Menu()->SetPenSize(1);
		Menu()->SetHighColor(shinerColor);
		frame.InsetBy(2, 0);
		EPoint pt1 = frame.Center();
		pt1.x = frame.left + 1;
		EPoint pt2 = pt1;
		pt2.x = frame.right - 1;
		Menu()->StrokeLine(pt1, pt2);
		pt1.y += 1; pt2.y += 1;
		Menu()->SetHighColor(darkerColor);
		Menu()->StrokeLine(pt1, pt2);
		Menu()->PopState();
	}

	Menu()->PopState();
}

