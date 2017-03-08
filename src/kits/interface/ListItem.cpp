/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
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
 * File: ListItem.cpp
 * Description: BListItem --- item for BListView/BOutlineListView
 *
 * --------------------------------------------------------------------------*/

#include "ListItem.h"
#include "OutlineListView.h"
#include "ListView.h"
#include "InterfaceDefs.h"

#include "../app/Message.h"
#include "../interface/View.h"
#include "../support/Errors.h"
#include "../support/List.h"


BListItem::BListItem(__be_uint32 outlineLevel, bool expanded,  __be_uint32 flags)
	: BArchivable(), fOwner(NULL), fFullOwner(NULL),
	  fWidth(-1), fHeight(-1), fSelected(false), fEnabled(true)
{
	fLevel = outlineLevel;
	fExpanded = expanded;
	fFlags = flags;
}


BListItem::~BListItem()
{
	if(fFullOwner != NULL) fFullOwner->BOutlineListView::RemoveItem(this, false);
	else if(fOwner != NULL) fOwner->BListView::RemoveItem(this, false);
}


BListItem::BListItem(BMessage *from)
	: BArchivable(from), fOwner(NULL), fFullOwner(NULL),
	  fLevel(0), fExpanded(true), fWidth(-1), fHeight(-1), fSelected(false), fEnabled(true)
{
}


status_t
BListItem::Archive(BMessage *into, bool deep) const
{
	if(!into) return B_ERROR;

	BArchivable::Archive(into, deep);
	into->AddString("class", "BListItem");

	// TODO

	return B_OK;
}


float
BListItem::Height() const
{
	return fHeight;
}


float
BListItem::Width() const
{
	return fWidth;
}


bool
BListItem::IsSelected() const
{
	return fSelected;
}


void
BListItem::Select()
{
	if(fOwner == NULL) return;
	fOwner->Select(fOwner->IndexOf(this));
}


void
BListItem::Deselect()
{
	if(fOwner == NULL) return;
	fOwner->Deselect(fOwner->IndexOf(this));
}


void
BListItem::SetEnabled(bool state)
{
	fEnabled = state;
}


bool
BListItem::IsEnabled() const
{
	return fEnabled;
}


void
BListItem::SetHeight(float height)
{
	if(height < 0) return;
	fHeight = height;
}


void
BListItem::SetWidth(float width)
{
	if(width < 0) return;
	fWidth = width;
}


bool
BListItem::IsExpanded() const
{
	return fExpanded;
}


void
BListItem::SetExpanded(bool state)
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


be_uint32
BListItem::OutlineLevel() const
{
	return fLevel;
}


void
BListItem::Invalidate()
{
	if(fOwner == NULL) return;
	fOwner->InvalidateItem(fOwner->IndexOf(this));
}


bool
BListItem::IsVisible() const
{
	if(fFullOwner == NULL) return false;
	BListItem *superitem = fFullOwner->Superitem(this);
	if(superitem == NULL) return true;
	if(superitem->fExpanded == false) return false;
	return superitem->IsVisible();
}


BListItem*
BListItem::SuperItem() const
{
	return(fFullOwner == NULL ? NULL : fFullOwner->Superitem(this));
}


bool
BListItem::HasSubitems() const
{
	return(fFullOwner == NULL ? false : fFullOwner->HasSubitems(this));
}


void
BListItem::DrawLeader(BView *owner, BRect *itemRect)
{
	if(fFullOwner == NULL || owner == NULL || itemRect == NULL || itemRect->IsValid() == false) return;

	if(fLevel > 0) itemRect->left += itemRect->Height() * 2.f * (float)fLevel;
	if(itemRect->IsValid() == false) return;

	if(HasSubitems())
	{
		BRect rect(*itemRect);
		rect.right = rect.left + rect.Height();
		rect.InsetBy(2.f, 2.f);

		if(fExpanded)
			owner->FillTriangle(rect.LeftTop(), rect.RightTop(), rect.LeftBottom() + BPoint(rect.Width() / 2.f, 0));
		else
			owner->FillTriangle(rect.LeftTop(), rect.LeftBottom(), rect.RightTop() + BPoint(0, rect.Height() / 2.f));

		itemRect->left += itemRect->Height();
	}
}


void
BListItem::GetLeaderSize(float *width, float *height) const
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
BListItem::SetFlags(__be_uint32 flags)
{
	fFlags = flags;
}


be_uint32
BListItem::Flags() const
{
	return fFlags;
}


void
BListItem::MouseDown(BView *owner, BPoint where)
{
}


void
BListItem::MouseUp(BView *owner, BPoint where)
{
}


void
BListItem::MouseMoved(BView *owner, BPoint where,  __be_uint32 code, const BMessage *a_message)
{
}


void
BListItem::KeyDown(BView *owner, const char *bytes,  __be_int32 numBytes)
{
}


void
BListItem::KeyUp(BView *owner, const char *bytes,  __be_int32 numBytes)
{
}


StringItem::StringItem(const char *text,  __be_uint32 outlineLevel, bool expanded)
	: BListItem(outlineLevel, expanded, 0), fText(NULL)
{
	if(text) fText = bhapi::strdup(text);
}


StringItem::~StringItem()
{
	if(fText) delete[] fText;
}


void
StringItem::DrawItem(BView *owner, BRect itemRect, bool drawEverything)
{
	bhapi::rgb_color bkColor = (IsSelected() ? bhapi::ui_color(B_DOCUMENT_HIGHLIGHT_COLOR): owner->ViewColor());
	bhapi::rgb_color fgColor = bhapi::ui_color(B_DOCUMENT_TEXT_COLOR);

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
		bhapi::font_height fontHeight;
		owner->GetFontHeight(&fontHeight);

		float sHeight = fontHeight.ascent + fontHeight.descent;

		BPoint penLocation;
		penLocation.x = itemRect.left;
		penLocation.y = itemRect.Center().y - sHeight / 2.f;
		penLocation.y += fontHeight.ascent + 1;

		owner->DrawString(fText, penLocation);
	}
}


void
StringItem::Update(BView *owner, const BFont *font)
{
	bhapi::font_height fontHeight;
	font->GetHeight(&fontHeight);
	SetHeight(fontHeight.ascent + fontHeight.descent);

	float width = 0;
	GetLeaderSize(&width, NULL);
	width += font->StringWidth(fText);
	SetWidth(width);
}


void
StringItem::SetText(const char *text)
{
	if(fText) delete[] fText;
	fText = (text == NULL ? NULL : bhapi::strdup(text));
}


const char*
StringItem::Text() const
{
	return fText;
}

