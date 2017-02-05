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
 * File: MenuField.cpp
 * Description: EMenuField --- display a labeled pop-up menu
 * 
 * --------------------------------------------------------------------------*/

#include "./../support/String.h"
#include "./../support/ClassInfo.h"

#include "Window.h"
#include "MenuField.h"


EMenuField::EMenuField(ERect frame, const char *name,
		       const char *label, EMenu *menu, bool fixedSize,
		       euint32 resizeMode, euint32 flags)
	: EView(frame, name, resizeMode, flags),
	  fAlignment(E_ALIGN_LEFT), fDivider(-1),
	  fLabel(NULL), fMenu(NULL)
{
	fFixedSize = fixedSize;
	if(label != NULL) fLabel = EStrdup(label);
	AddChild(fMenuBar = new EMenuBar(frame, NULL, E_FOLLOW_NONE, E_ITEMS_IN_ROW, !fFixedSize));
	fMenuBar->SetEventMask(0);
	fMenuBar->SetBorder(E_BORDER_FRAME);
	SetMenu(menu);
}


EMenuField::~EMenuField()
{
	if(fLabel != NULL) delete[] fLabel;
}


void
EMenuField::ChildRemoving(EView *child)
{
	if(child == fMenuBar) {fMenuBar = NULL; fMenu = NULL;}
	EView::ChildRemoving(child);
}


void
EMenuField::SetLabel(const char *label)
{
	if(fLabel != NULL) delete[] fLabel;
	fLabel = (label == NULL ? NULL : EStrdup(label));
	Invalidate();
}


const char*
EMenuField::Label() const
{
	return fLabel;
}


void
EMenuField::SetAlignment(e_alignment alignment)
{
	if(alignment != fAlignment)
	{
		fAlignment = alignment;
		if(fLabel != NULL) Invalidate();
	}
}


e_alignment
EMenuField::Alignment() const
{
	return fAlignment;
}


void
EMenuField::SetDivider(float divider)
{
	if(fDivider < 0 && divider < 0) return;

	if(fDivider != divider)
	{
		fDivider = divider;
		EMenuField::FrameResized(Frame().Width(), Frame().Height());
		Invalidate();
	}
}


float
EMenuField::Divider() const
{
	return fDivider;
}


bool
EMenuField::SetMenu(EMenu *menu)
{
	if(fMenuBar == NULL || (menu == NULL ? false : (fMenuBar->AddItem(menu) == false))) return false;

	if(fMenu)
	{
		fMenuBar->RemoveItem(fMenu);
		delete fMenu;
	}

	fMenu = menu;

	EMenuField::FrameResized(Frame().Width(), Frame().Height());

	Invalidate();

	return true;
}


EMenu*
EMenuField::Menu() const
{
	return fMenu;
}


EMenuBar*
EMenuField::MenuBar() const
{
	return fMenuBar;
}


EMenuItem*
EMenuField::MenuItem() const
{
	return(fMenu == NULL ? NULL : fMenu->Superitem());
}


void
EMenuField::Draw(ERect updateRect)
{
	if(Window() == NULL) return;

	if(fLabel != NULL && fDivider != 0)
	{
		EFont font;
		GetFont(&font);
		e_font_height fontHeight;
		font.GetHeight(&fontHeight);
		float sHeight = fontHeight.ascent + fontHeight.descent;

		ERect rect = Frame().OffsetToSelf(E_ORIGIN);
		rect.right = (fDivider >= 0 ? fDivider : max_c(font.StringWidth(fLabel), 0));

		EPoint penLocation;

		if(fAlignment == E_ALIGN_LEFT || fDivider < 0)
			penLocation.x = 0;
		else if(fAlignment == E_ALIGN_RIGHT)
			penLocation.x = rect.right - max_c(font.StringWidth(fLabel), 0);
		else
			penLocation.x = (rect.right - max_c(font.StringWidth(fLabel), 0)) / 2;

		penLocation.y = rect.Center().y - sHeight / 2.f;
		penLocation.y += fontHeight.ascent + 1;

		PushState();
		ConstrainClippingRegion(rect);
		SetHighColor(IsEnabled() ? e_ui_color(E_PANEL_TEXT_COLOR) : e_ui_color(E_SHINE_COLOR).disable(ViewColor()));
		SetLowColor(ViewColor());
		DrawString(Label(), penLocation);
		if(!IsEnabled())
		{
			SetHighColor(e_ui_color(E_SHADOW_COLOR).disable(ViewColor()));
			DrawString(Label(), penLocation - EPoint(1, 1));
		}
		PopState();
	}

#if 0
	if(IsFocus() && Window()->IsActivate())
	{
		PushState();
		SetHighColor(e_ui_color(E_NAVIGATION_BASE_COLOR));
		StrokeRect(Frame().OffsetToSelf(E_ORIGIN));
		PopState();
	}
#endif
}


void
EMenuField::GetPreferredSize(float *width, float *height)
{
	if(width) *width = 0;
	if(height) *height = 0;

	if(fMenuBar != NULL) fMenuBar->GetPreferredSize(width, height);

	if(width != NULL)
	{
		EFont font;
		GetFont(&font);
		*width += (fDivider >= 0 ? fDivider : max_c(font.StringWidth(fLabel), 0)) + UnitsPerPixel() * 2;
	}

	if(height != NULL)
	{
		e_font_height fontHeight;
		GetFontHeight(&fontHeight);
		float sHeight = fontHeight.ascent + fontHeight.descent;
		if(sHeight + 10 > *height) *height = sHeight + 10;
	}
}


void
EMenuField::FrameResized(float new_width, float new_height)
{
	if(fMenuBar == NULL) return;

	EFont font;
	GetFont(&font);

	fMenuBar->ResizeToPreferred();
	fMenuBar->MoveTo((fDivider >= 0 ? fDivider : max_c(font.StringWidth(fLabel), 0)) + UnitsPerPixel(),
			 (new_height - fMenuBar->Frame().Height()) / 2);
	if(fFixedSize)
		fMenuBar->ResizeTo(new_width - fMenuBar->Frame().left, fMenuBar->Frame().Height());
}


void
EMenuField::FrameMoved(EPoint new_position)
{
	EMenuField::FrameResized(Frame().Width(), Frame().Height());
}


void
EMenuField::WindowActivated(bool state)
{
	if(!state && fMenuBar) fMenuBar->SelectItem(NULL);

#if 0
	if(!(IsFocus() && (Flags() & E_WILL_DRAW))) return;
	PushState();
	SetHighColor(state ? e_ui_color(E_NAVIGATION_BASE_COLOR) : ViewColor());
	StrokeRect(Frame().OffsetToSelf(E_ORIGIN));
	PopState();
#endif
}


void
EMenuField::MakeFocus(bool focusState)
{
	if(IsFocus() != focusState)
	{
		EView::MakeFocus(focusState);

#if 0
		if(IsVisible() && (Flags() & E_WILL_DRAW))
		{
			PushState();
			SetHighColor(IsFocus() ? e_ui_color(E_NAVIGATION_BASE_COLOR) : ViewColor());
			StrokeRect(Frame().OffsetToSelf(E_ORIGIN));
			PopState();
		}
#endif

		if(!IsFocus() && fMenuBar) fMenuBar->SelectItem(NULL);
	}
}


void
EMenuField::SetFont(const EFont *font, euint8 mask)
{
	EView::SetFont(font, mask);
	EMenuField::FrameResized(Frame().Width(), Frame().Height());
	Invalidate();
}


void
EMenuField::MouseDown(EPoint where)
{
	if((Flags() & E_NAVIGABLE) && !IsFocus()) MakeFocus();

	EMenuItem *item = NULL;
	if(fMenuBar == NULL || (item = fMenuBar->ItemAt(0)) == NULL) return;

	ERect itemFrame = item->Frame();
	if(itemFrame.Contains(fMenuBar->ConvertFromParent(where)) == false &&
	   fMenuBar->CurrentSelection() == NULL) where = fMenuBar->ConvertToParent(itemFrame.Center());

	fMenuBar->MouseDown(fMenuBar->ConvertFromParent(where));
}


void
EMenuField::MouseUp(EPoint where)
{
	EMenuItem *item = NULL;
	if(fMenuBar == NULL || (item = fMenuBar->ItemAt(0)) == NULL) return;

	ERect itemFrame = item->Frame();
	if(itemFrame.Contains(fMenuBar->ConvertFromParent(where)) == false) where = fMenuBar->ConvertToParent(itemFrame.Center());

	fMenuBar->MouseUp(fMenuBar->ConvertFromParent(where));
}


void
EMenuField::MouseMoved(EPoint where, euint32 code, const EMessage *a_message)
{
	EMenuItem *item = NULL;
	if(fMenuBar == NULL || (item = fMenuBar->ItemAt(0)) == NULL) return;

	ERect itemFrame = item->Frame();
	if(itemFrame.Contains(fMenuBar->ConvertFromParent(where)) == false &&
	   fMenuBar->CurrentSelection() == NULL) where = fMenuBar->ConvertToParent(itemFrame.Center());

	fMenuBar->MouseMoved(fMenuBar->ConvertFromParent(where), code, a_message);
}


void
EMenuField::KeyDown(const char *bytes, eint32 numBytes)
{
	EMenuItem *item = NULL;
	if(bytes == NULL || fMenuBar == NULL || (item = fMenuBar->ItemAt(0)) == NULL) return;

	if(numBytes == 1 && bytes[0] == E_DOWN_ARROW && fMenuBar->CurrentSelection() == NULL) fMenuBar->SelectItem(item);
	fMenuBar->KeyDown(bytes, numBytes);
}


void
EMenuField::KeyUp(const char *bytes, eint32 numBytes)
{
	EMenuItem *item = NULL;
	if(bytes == NULL || fMenuBar == NULL || (item = fMenuBar->ItemAt(0)) == NULL) return;

	fMenuBar->KeyUp(bytes, numBytes);
}

