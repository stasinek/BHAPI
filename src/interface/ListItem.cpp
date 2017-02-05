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
 * File: ListItem.cpp
 * Description: EListItem --- item for EListView/EOutlineListView
 *
 * --------------------------------------------------------------------------*/

#include "InterfaceDefs.h"
#include "ListItem.h"
#include "ListView.h"
#include "OutlineListView.h"


EListItem::EListItem(euint32 outlineLevel, bool expanded, euint32 flags)
	: EArchivable(), fOwner(NULL), fFullOwner(NULL),
	  fWidth(-1), fHeight(-1), fSelected(false), fEnabled(true)
{
	fLevel = outlineLevel;
	fExpanded = expanded;
	fFlags = flags;
}


EListItem::~EListItem()
{
	if(fFullOwner != NULL) fFullOwner->EOutlineListView::RemoveItem(this, false);
	else if(fOwner != NULL) fOwner->EListView::RemoveItem(this, false);
}


EListItem::EListItem(EMessage *from)
	: EArchivable(from), fOwner(NULL), fFullOwner(NULL),
	  fLevel(0), fExpanded(true), fWidth(-1), fHeight(-1), fSelected(false), fEnabled(true)
{
}


e_status_t
EListItem::Archive(EMessage *into, bool deep) const
{
	if(!into) return E_ERROR;

	EArchivable::Archive(into, deep);
	into->AddString("class", "EListItem");

	// TODO

	return E_OK;
}


float
EListItem::Height() const
{
	return fHeight;
}


float
EListItem::Width() const
{
	return fWidth;
}


bool
EListItem::IsSelected() const
{
	return fSelected;
}


void
EListItem::Select()
{
	if(fOwner == NULL) return;
	fOwner->Select(fOwner->IndexOf(this));
}


void
EListItem::Deselect()
{
	if(fOwner == NULL) return;
	fOwner->Deselect(fOwner->IndexOf(this));
}


void
EListItem::SetEnabled(bool state)
{
	fEnabled = state;
}


bool
EListItem::IsEnabled() const
{
	return fEnabled;
}


void
EListItem::SetHeight(float height)
{
	if(height < 0) return;
	fHeight = height;
}


void
EListItem::SetWidth(float width)
{
	if(width < 0) return;
	fWidth = width;
}


bool
EListItem::IsExpanded() const
{
	return fExpanded;
}


void
EListItem::SetExpanded(bool state)
{
	if(fFullOwner == NULL)
	{
		fExpanded = state;
		return;
	}
	else
	{
		if(state) fFullOwner->Expand(this);
		else fFullOwner->Collapse(this);
	}
}


euint32
EListItem::OutlineLevel() const
{
	return fLevel;
}


void
EListItem::Invalidate()
{
	if(fOwner == NULL) return;
	fOwner->InvalidateItem(fOwner->IndexOf(this));
}


bool
EListItem::IsVisible() const
{
	if(fFullOwner == NULL) return false;
	EListItem *superitem = fFullOwner->Superitem(this);
	if(superitem == NULL) return true;
	if(superitem->fExpanded == false) return false;
	return superitem->IsVisible();
}


EListItem*
EListItem::SuperItem() const
{
	return(fFullOwner == NULL ? NULL : fFullOwner->Superitem(this));
}


bool
EListItem::HasSubitems() const
{
	return(fFullOwner == NULL ? false : fFullOwner->HasSubitems(this));
}


void
EListItem::DrawLeader(EView *owner, ERect *itemRect)
{
	if(fFullOwner == NULL || owner == NULL || itemRect == NULL || itemRect->IsValid() == false) return;

	if(fLevel > 0) itemRect->left += itemRect->Height() * 2.f * (float)fLevel;
	if(itemRect->IsValid() == false) return;

	if(HasSubitems())
	{
		ERect rect(*itemRect);
		rect.right = rect.left + rect.Height();
		rect.InsetBy(2.f, 2.f);

		if(fExpanded)
			owner->FillTriangle(rect.LeftTop(), rect.RightTop(), rect.LeftBottom() + EPoint(rect.Width() / 2.f, 0));
		else
			owner->FillTriangle(rect.LeftTop(), rect.LeftBottom(), rect.RightTop() + EPoint(0, rect.Height() / 2.f));

		itemRect->left += itemRect->Height();
	}
}


void
EListItem::GetLeaderSize(float *width, float *height) const
{
	if(width) *width = 0;
	if(height) *height = fHeight;

	if(fFullOwner != NULL && width != NULL)
	{
		if(fLevel > 0) *width += fHeight * 2.f * (float)fLevel;
		if(HasSubitems()) *width += fHeight;
	}
}


void
EListItem::SetFlags(euint32 flags)
{
	fFlags = flags;
}


euint32
EListItem::Flags() const
{
	return fFlags;
}


void
EListItem::MouseDown(EView *owner, EPoint where)
{
}


void
EListItem::MouseUp(EView *owner, EPoint where)
{
}


void
EListItem::MouseMoved(EView *owner, EPoint where, euint32 code, const EMessage *a_message)
{
}


void
EListItem::KeyDown(EView *owner, const char *bytes, eint32 numBytes)
{
}


void
EListItem::KeyUp(EView *owner, const char *bytes, eint32 numBytes)
{
}


EStringItem::EStringItem(const char *text, euint32 outlineLevel, bool expanded)
	: EListItem(outlineLevel, expanded, 0), fText(NULL)
{
	if(text) fText = EStrdup(text);
}


EStringItem::~EStringItem()
{
	if(fText) delete[] fText;
}


void
EStringItem::DrawItem(EView *owner, ERect itemRect, bool drawEverything)
{
	e_rgb_color bkColor = (IsSelected() ? e_ui_color(E_DOCUMENT_HIGHLIGHT_COLOR): owner->ViewColor());
	e_rgb_color fgColor = e_ui_color(E_DOCUMENT_TEXT_COLOR);

	if(!IsEnabled())
	{
		bkColor.disable(owner->ViewColor());
		fgColor.disable(owner->ViewColor());
	}

	if(IsSelected() || !IsEnabled())
	{
		owner->SetHighColor(bkColor);
		owner->FillRect(itemRect);
	}

	owner->SetHighColor(fgColor);
	owner->SetLowColor(bkColor);

	DrawLeader(owner, &itemRect);
	if(itemRect.IsValid() == false) return;

	if(fText)
	{
		e_font_height fontHeight;
		owner->GetFontHeight(&fontHeight);

		float sHeight = fontHeight.ascent + fontHeight.descent;

		EPoint penLocation;
		penLocation.x = itemRect.left;
		penLocation.y = itemRect.Center().y - sHeight / 2.f;
		penLocation.y += fontHeight.ascent + 1;

		owner->DrawString(fText, penLocation);
	}
}


void
EStringItem::Update(EView *owner, const EFont *font)
{
	e_font_height fontHeight;
	font->GetHeight(&fontHeight);
	SetHeight(fontHeight.ascent + fontHeight.descent);

	float width = 0;
	GetLeaderSize(&width, NULL);
	width += font->StringWidth(fText);
	SetWidth(width);
}


void
EStringItem::SetText(const char *text)
{
	if(fText) delete[] fText;
	fText = (text == NULL ? NULL : EStrdup(text));
}


const char*
EStringItem::Text() const
{
	return fText;
}

