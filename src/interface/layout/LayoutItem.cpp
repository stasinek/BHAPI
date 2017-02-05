/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
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
 * File: LayoutItem.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../support/ClassInfo.h>

#include "Layout.h"


ELayoutItem::ELayoutItem(ERect frame, euint32 resizingMode)
	: ELayoutContainer(), fContainer(NULL),
	  fIndex(-1),
	  fLocalOrigin(E_ORIGIN),
	  fFrame(frame), fResizingMode(resizingMode),
	  fHidden(false), fUpdating(false)
{
}


ELayoutItem::~ELayoutItem()
{
	RemoveSelf();
}


ELayoutContainer*
ELayoutItem::Container() const
{
	return fContainer;
}


ELayoutContainer*
ELayoutItem::Ancestor() const
{
	if(fContainer == NULL) return e_cast_as((ELayoutItem*)this, ELayoutContainer);
	if(e_is_kind_of(fContainer, ELayoutItem) == false) return fContainer;
	return e_cast_as(fContainer, ELayoutItem)->Ancestor();
}


ELayoutItem*
ELayoutItem::PreviousSibling() const
{
	return(fContainer == NULL ? NULL : (ELayoutItem*)fContainer->ItemAt(fIndex - 1));
}


ELayoutItem*
ELayoutItem::NextSibling() const
{
	return(fContainer == NULL ? NULL : (ELayoutItem*)fContainer->ItemAt(fIndex + 1));
}


bool
ELayoutItem::RemoveSelf()
{
	if(fContainer == NULL) return false;
	return fContainer->RemoveItem(this);
}


void
ELayoutItem::SetResizingMode(euint32 mode)
{
	fResizingMode = mode;
}


euint32
ELayoutItem::ResizingMode() const
{
	return fResizingMode;
}


void
ELayoutItem::Show()
{
	if(fHidden == false) return;

	fHidden = false;

	if(fUpdating || fContainer == NULL || fFrame.IsValid() == false) return;
	for(ELayoutItem *item = this; item != NULL; item = item->NextSibling()) item->UpdateVisibleRegion();
	fContainer->Invalidate(fFrame);
}


void
ELayoutItem::Hide()
{
	if(fHidden == true) return;

	fHidden = true;

	if(fUpdating || fContainer == NULL || fFrame.IsValid() == false) return;
	for(ELayoutItem *item = this; item != NULL; item = item->NextSibling()) item->UpdateVisibleRegion();
	fContainer->Invalidate(fFrame);
}


bool
ELayoutItem::IsHidden(bool check_containers) const
{
	if(check_containers == false) return fHidden;
	if(fHidden || fContainer == NULL) return true;
	if(e_is_kind_of(fContainer, ELayoutItem) == false) return false;
	return e_cast_as(fContainer, ELayoutItem)->IsHidden(true);
}


void
ELayoutItem::SendBehind(ELayoutItem *item)
{
	if(fContainer == NULL || item == NULL || !(item->fContainer == fContainer && item->fIndex > fIndex)) return;

	eint32 index = (item == NULL ? 0 : item->fIndex);
	if(fContainer->fItems.MoveItem(fIndex, index) == false) return;

	for(eint32 i = min_c(index, fIndex); i < fContainer->fItems.CountItems(); i++)
		((ELayoutItem*)fContainer->fItems.ItemAt(i))->fIndex = i;

	if(fUpdating || fHidden || fFrame.IsValid() == false) return;
	ERect updateRect = fFrame | item->fFrame;
	for(eint32 i = min_c(index, fIndex); i < fContainer->fItems.CountItems(); i++)
		((ELayoutItem*)fContainer->fItems.ItemAt(i))->UpdateVisibleRegion();
	fContainer->Invalidate(updateRect);
}


void
ELayoutItem::MoveTo(EPoint where)
{
	if(fFrame.LeftTop() == where) return;

	ERect oldFrame = fFrame;
	fFrame.OffsetTo(where);

	if(fUpdating || fContainer == NULL || fHidden || fFrame.IsValid() == false) return;
	for(ELayoutItem *item = this; item != NULL; item = item->NextSibling()) item->UpdateVisibleRegion();
	fContainer->Invalidate(oldFrame | fFrame);
}


void
ELayoutItem::ScrollTo(EPoint where)
{
	if(where.x < 0) where.x = 0;
	if(where.y < 0) where.y = 0;

	if(fLocalOrigin == where) return;

	fLocalOrigin = where;

	if(fUpdating || fContainer == NULL || fHidden || fFrame.IsValid() == false) return;
	UpdateVisibleRegion();
	fContainer->Invalidate(fFrame);
}


void
ELayoutItem::ResizeTo(float width, float height)
{
	if(fFrame.Width() == width && fFrame.Height() == height) return;

	float width_ext = width - fFrame.Width();
	float height_ext = height - fFrame.Height();
	ERect oldFrame = fFrame;
	EPoint center_offset = fFrame.Center() - oldFrame.Center();

	fFrame.right += width_ext;
	fFrame.bottom += height_ext;

	for(ELayoutItem *item = ItemAt(0); item != NULL; item = item->NextSibling())
	{
		euint32 iMode = item->fResizingMode;
		ERect iFrame = item->fFrame;

		if(iMode == E_FOLLOW_NONE || iMode == (E_FOLLOW_LEFT | E_FOLLOW_TOP)) continue;

		if((iMode & E_FOLLOW_H_CENTER) && (iMode & E_FOLLOW_LEFT_RIGHT) != E_FOLLOW_LEFT_RIGHT)
		{
			float newCenterX = iFrame.Center().x + center_offset.x;
			if(iMode & E_FOLLOW_RIGHT)
			{
				iFrame.right += width_ext;
				iFrame.left = iFrame.right - 2.f * (iFrame.right - newCenterX);
			}
			else if(iMode & E_FOLLOW_LEFT)
			{
				iFrame.right = iFrame.left + 2.f * (newCenterX - iFrame.left);
			}
			else
			{
				float iWidth = iFrame.Width();
				iFrame.left = newCenterX - iWidth / 2.f;
				iFrame.right = newCenterX + iWidth / 2.f;
			}
		}
		else if(iMode & E_FOLLOW_RIGHT)
		{
			iFrame.right += width_ext;
			if(!(iMode & E_FOLLOW_LEFT)) iFrame.left += width_ext;
		}

		if((iMode & E_FOLLOW_V_CENTER) && (iMode & E_FOLLOW_TOP_BOTTOM) != E_FOLLOW_TOP_BOTTOM)
		{
			float newCenterY = iFrame.Center().y + center_offset.y;
			if(iMode & E_FOLLOW_TOP_BOTTOM)
			{
				iFrame.bottom += height_ext;
				iFrame.top = iFrame.bottom - 2.f * (iFrame.bottom - newCenterY);
			}
			else if(iMode & E_FOLLOW_TOP)
			{
				iFrame.bottom = iFrame.top + 2.f * (newCenterY - iFrame.top);
			}
			else
			{
				float iHeight = iFrame.Height();
				iFrame.top = newCenterY - iHeight / 2.f;
				iFrame.bottom = newCenterY + iHeight / 2.f;
			}
		}
		else if(iMode & E_FOLLOW_BOTTOM)
		{
			iFrame.bottom += height_ext;
			if(!(iMode & E_FOLLOW_TOP)) iFrame.top += height_ext;
		}

		item->fFrame.OffsetTo(iFrame.LeftTop());
		item->fUpdating = true;
		item->ResizeTo(iFrame.Width(), iFrame.Height());
		item->fUpdating = false;
	}

	if(fUpdating || fContainer == NULL || fHidden || (oldFrame.IsValid() == false && fFrame.IsValid() == false)) return;
	for(ELayoutItem *item = this; item != NULL; item = item->NextSibling()) item->UpdateVisibleRegion();
	fContainer->Invalidate(oldFrame | fFrame);
}


void
ELayoutItem::MoveAndResizeTo(EPoint where, float width, float height)
{
	ERect oldFrame = fFrame;

	bool saveUpdating = fUpdating;
	fUpdating = true;
	MoveTo(where);
	ResizeTo(width, height);
	fUpdating = saveUpdating;

	if(oldFrame == fFrame) return;
	if(fUpdating || fContainer == NULL || fHidden || (oldFrame.IsValid() == false && fFrame.IsValid() == false)) return;
	for(ELayoutItem *item = this; item != NULL; item = item->NextSibling()) item->UpdateVisibleRegion();
	fContainer->Invalidate(oldFrame | fFrame);
}


void
ELayoutItem::GetPreferredSize(float *width, float *height)
{
	if(width == NULL && height == NULL) return;

	float w = 0, h = 0;
	ERect rect = fFrame.OffsetToCopy(E_ORIGIN);

	for(ELayoutItem *item = ItemAt(0); item != NULL; item = item->NextSibling())
	{
		if(item->fHidden) continue;

		float iW = 0, iH = 0;
		item->GetPreferredSize(&iW, &iH);

		euint32 iMode = item->fResizingMode;
		if((iMode & E_FOLLOW_LEFT) || (iMode & E_FOLLOW_NONE)) iW += item->fFrame.left;
		if(iMode & E_FOLLOW_RIGHT) iW += rect.right - item->fFrame.right;
		if((iMode & E_FOLLOW_TOP) || (iMode & E_FOLLOW_NONE)) iH += item->fFrame.top;
		if(iMode & E_FOLLOW_BOTTOM) iH += rect.bottom - item->fFrame.bottom;

		w = max_c(w, iW);
		h = max_c(h, iH);
	}

	if(width) *width = w;
	if(height) *height = h;
}


void
ELayoutItem::ResizeToPreferred()
{
	float w = -1, h = -1;
	GetPreferredSize(&w, &h);
	if(w < 0) w = fFrame.Width();
	if(h < 0) h = fFrame.Height();
	if(w == fFrame.Width() && h == fFrame.Height()) return;

	ERect iFrame = fFrame;
	euint32 iMode = fResizingMode;

	if((iMode & E_FOLLOW_H_CENTER) && (iMode & E_FOLLOW_LEFT_RIGHT) != E_FOLLOW_LEFT_RIGHT)
	{
		float centerX = fFrame.Center().x;
		iFrame.left = centerX - w / 2.f;
		iFrame.right = centerX + w / 2.f;
	}
	else if((iMode & E_FOLLOW_LEFT_RIGHT) != E_FOLLOW_LEFT_RIGHT)
	{
		if(iMode & E_FOLLOW_RIGHT)
			iFrame.left = iFrame.right - w;
		else
			iFrame.right = iFrame.left + w;
	}

	if((iMode & E_FOLLOW_V_CENTER) && (iMode & E_FOLLOW_TOP_BOTTOM) != E_FOLLOW_TOP_BOTTOM)
	{
		float centerY = fFrame.Center().y;
		iFrame.top = centerY - h / 2.f;
		iFrame.bottom = centerY + h / 2.f;
	}
	else if((iMode & E_FOLLOW_TOP_BOTTOM) != E_FOLLOW_TOP_BOTTOM)
	{
		if(iMode & E_FOLLOW_BOTTOM)
			iFrame.top = iFrame.bottom - h;
		else
			iFrame.bottom = iFrame.top + h;
	}

	if(iFrame == fFrame) return;

	bool saveUpdating = fUpdating;
	ERect oldFrame = fFrame;

	fUpdating = true;
	MoveAndResizeTo(iFrame.LeftTop(), iFrame.Width(), iFrame.Height());
	fUpdating = saveUpdating;

	if(fUpdating || fContainer == NULL || fHidden || (oldFrame.IsValid() == false && fFrame.IsValid() == false)) return;
	for(ELayoutItem *item = this; item != NULL; item = item->NextSibling()) item->UpdateVisibleRegion();
	fContainer->Invalidate(oldFrame | fFrame);
}


ERect
ELayoutItem::Bounds() const
{
	return fFrame.OffsetToCopy(fLocalOrigin);
}


ERect
ELayoutItem::Frame() const
{
	return fFrame;
}


const ERegion*
ELayoutItem::VisibleRegion() const
{
	return &fVisibleRegion;
}


void
ELayoutItem::GetVisibleRegion(ERegion **region)
{
	if(region) *region = &fVisibleRegion;
}


EPoint
ELayoutItem::LeftTop() const
{
	return fLocalOrigin;
}


float
ELayoutItem::Width() const
{
	return fFrame.Width();
}


float
ELayoutItem::Height() const
{
	return fFrame.Height();
}


void
ELayoutItem::ConvertToContainer(EPoint *pt) const
{
	if(pt == NULL) return;

	*pt -= fLocalOrigin;
	*pt += fFrame.LeftTop();
}


EPoint
ELayoutItem::ConvertToContainer(EPoint pt) const
{
	ConvertToContainer(&pt);
	return pt;
}


void
ELayoutItem::ConvertFromContainer(EPoint *pt) const
{
	if(pt == NULL) return;

	*pt -= fFrame.LeftTop();
	*pt += fLocalOrigin;
}


EPoint
ELayoutItem::ConvertFromContainer(EPoint pt) const
{
	ConvertFromContainer(&pt);
	return pt;
}


void
ELayoutItem::UpdateVisibleRegion()
{
	ELayoutItem *item;

	if(fUpdating) return;

	if(fContainer == NULL || fHidden || fFrame.IsValid() == false)
	{
		fVisibleRegion.MakeEmpty();
	}
	else
	{
		if(e_is_kind_of(fContainer, ELayoutItem) == false)
		{
			fVisibleRegion = fFrame;
		}
		else
		{
			fVisibleRegion = e_cast_as(fContainer, ELayoutItem)->fVisibleRegion;
			fVisibleRegion &= fFrame;
		}

		for(item = fContainer->ItemAt(0);
		    item != NULL && item != this && fVisibleRegion.CountRects() > 0;
		    item = item->NextSibling())
		{
			if(item->fHidden || item->fFrame.IsValid() == false) continue;
			fVisibleRegion.Exclude(item->fFrame.InsetByCopy(-fContainer->UnitsPerPixel(), -fContainer->UnitsPerPixel()));
		}

		fVisibleRegion.OffsetBy(E_ORIGIN - fFrame.LeftTop() + fLocalOrigin);
	}

	for(item = ItemAt(0); item != NULL; item = item->NextSibling()) item->UpdateVisibleRegion();
}

