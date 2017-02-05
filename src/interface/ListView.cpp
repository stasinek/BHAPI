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
 * File: ListView.cpp
 * Description: EListView --- Displays a list of items the user can select and invoke
 *
 * --------------------------------------------------------------------------*/

#include "./../support/ClassInfo.h>

#include "ListView.h"
#include "ScrollView.h"
#include "Window.h"


EListView::EListView(ERect frame,
		     const char *name,
		     e_list_view_type type,
		     euint32 resizingMode,
		     euint32 flags)
	: EView(frame, name, resizingMode, flags), EInvoker(),
	  fFirstSelected(-1), fLastSelected(-1), fPos(-1),
	  fSelectionMessage(NULL)
{
	fListType = type;
}


EListView::~EListView()
{
	EListView::MakeEmpty();
	if(fSelectionMessage != NULL) delete fSelectionMessage;
}


void
EListView::MakeFocus(bool focusState)
{
	if(IsFocus() != focusState)
	{
		EView::MakeFocus(focusState);

		if(IsVisible() && (Flags() & E_WILL_DRAW))
		{
			PushState();
			SetHighColor(IsFocus() ? e_ui_color(E_NAVIGATION_BASE_COLOR) : ViewColor());
			StrokeRect(Bounds());
			PopState();

			InvalidateItem(fPos);
		}
	}
}


void
EListView::WindowActivated(bool state)
{
	InvalidateItem(fPos);
	if(!(IsFocus() && (Flags() & E_WILL_DRAW))) return;
	PushState();
	SetHighColor(state ? e_ui_color(E_NAVIGATION_BASE_COLOR) : ViewColor());
	StrokeRect(Bounds());
	PopState();
}


bool
EListView::AddItem(EListItem *item)
{
	return EListView::AddItem(item, fItems.CountItems());
}


bool
EListView::AddItem(EListItem *item, eint32 atIndex)
{
	if(item == NULL || item->fOwner != NULL) return false;
	if(fItems.AddItem(item, atIndex) == false) return false;
	item->fOwner = this;
	item->fSelected = false;

	if(fFirstSelected >= 0 && atIndex <= fLastSelected)
	{
		if(atIndex <= fFirstSelected) fFirstSelected++;
		fLastSelected++;
		SelectionChanged();
	}

	EFont font;
	GetFont(&font);
	item->Update(this, &font);

	if(atIndex <= fPos) fPos++;

	return true;
}


bool
EListView::RemoveItem(EListItem *item, bool auto_destruct_item)
{
	if(EListView::RemoveItem(IndexOf(item)) == NULL) return false;
	if(auto_destruct_item) delete item;

	return true;
}


EListItem*
EListView::RemoveItem(eint32 index)
{
	EListItem *item = (EListItem*)fItems.RemoveItem(index);
	if(item == NULL) return NULL;

	item->fOwner = NULL;
	if(item->fSelected)
	{
		if(index == fFirstSelected)
		{
			fFirstSelected = -1;
			fLastSelected--;

			while(fListType == E_MULTIPLE_SELECTION_LIST && index < min_c(fItems.CountItems(), fLastSelected + 1))
			{
				item = (EListItem*)fItems.ItemAt(index);
				if(item->fSelected) {fFirstSelected = index; break;}
				index++;
			}

			if(fFirstSelected < 0) fLastSelected = -1;
		}
		else if(index == fLastSelected)
		{
			fLastSelected = -1;

			while(index >= max_c(0, fFirstSelected))
			{
				item = (EListItem*)fItems.ItemAt(index);
				if(item->fSelected) {fLastSelected = index; break;}
				index--;
			}

			if(fLastSelected < 0) fFirstSelected = -1;
		}
		else
		{
			fLastSelected--;
		}

		SelectionChanged();
	}
	else if(index < fFirstSelected)
	{
		fFirstSelected--;
		fLastSelected--;
		SelectionChanged();
	}

	if(index == fPos) fPos = -1;
	else if(index < fPos) fPos--;

	return item;
}


bool
EListView::RemoveItems(eint32 index, eint32 count, bool auto_destruct_items)
{
	if(index < 0 || index >= fItems.CountItems()) return false;

	if(count < 0) count = fItems.CountItems() - index;
	else count = min_c(fItems.CountItems() - index, count);

	// TODO: remove at once
	while(count-- > 0)
	{
		EListItem *item = EListView::RemoveItem(index);
		if(item == NULL) return false;
		if(auto_destruct_items) delete item;
	}

	return true;
}


void
EListView::SetListType(e_list_view_type type)
{
	if(fListType != type)
	{
		fListType = type;
		if(fListType == E_SINGLE_SELECTION_LIST) Select(CurrentSelection(0), false);
	}
}


e_list_view_type
EListView::ListType() const
{
	return fListType;
}


EListItem*
EListView::ItemAt(eint32 index) const
{
	return (EListItem*)fItems.ItemAt(index);
}


EListItem*
EListView::FirstItem() const
{
	return (EListItem*)fItems.FirstItem();
}


EListItem*
EListView::LastItem() const
{
	return (EListItem*)fItems.LastItem();
}


eint32
EListView::IndexOf(const EListItem *item) const
{
	if(item == NULL || item->fOwner != this) return -1;
	return fItems.IndexOf((void*)item);
}


eint32
EListView::IndexOf(EPoint where, bool mustVisible) const
{
	float boundsBottom = -1;

	if(mustVisible)
	{
		ERect vRect = VisibleBounds();
		if(vRect.Contains(where) == false) return -1;
		boundsBottom = vRect.bottom;
	}

	ERect rect(1, 1, Frame().Width() - 1, 1);

	eint32 retVal = -1;

	for(eint32 i = 0; i < fItems.CountItems(); i++)
	{
		EListItem *item = (EListItem*)fItems.ItemAt(i);
		if(item->Height() < 0) continue;

		rect.top = rect.bottom;
		rect.bottom = rect.top + item->Height();
		if(rect.top > 1) rect.OffsetBy(0, 1);

		if(rect.Contains(where))
		{
			retVal = i;
			break;
		}

		if(boundsBottom < 0) continue;
		else if(rect.top > boundsBottom) break;
	}

	return retVal;
}


bool
EListView::HasItem(const EListItem *item) const
{
	return((item == NULL || item->fOwner != this) ? false : true);
}


eint32
EListView::CountItems() const
{
	return fItems.CountItems();
}


void
EListView::MakeEmpty()
{
	while(fItems.CountItems() > 0)
	{
		EListItem *item = (EListItem*)fItems.RemoveItem((eint32)0);
		item->fOwner = NULL;
		delete item;
	}

	fFirstSelected = fLastSelected = -1;
}


bool
EListView::IsEmpty() const
{
	return fItems.IsEmpty();
}


void
EListView::SelectionChanged()
{
}


void
EListView::Draw(ERect updateRect)
{
	if(Window() == NULL) return;
	ERect bounds = Bounds();
	bool winActivated = Window()->IsActivate();

	if(IsFocus() && winActivated)
	{
		PushState();
		SetPenSize(0);
		SetDrawingMode(E_OP_COPY);
		SetHighColor(e_ui_color(E_NAVIGATION_BASE_COLOR));
		StrokeRect(bounds);
		PopState();
	}

	bounds.InsetBy(1, 1);
	ERect rect(1, 1, bounds.right - 1, 1);

	for(eint32 i = 0; i < fItems.CountItems(); i++)
	{
		EListItem *item = (EListItem*)fItems.ItemAt(i);
		if(item->Height() < 0) continue;

		rect.top = rect.bottom;
		rect.bottom = rect.top + item->Height();
		if(rect.top > 1) rect.OffsetBy(0, 1);
		if(rect.Intersects(updateRect) == false) continue;
		if(rect.top >= bounds.bottom) break;
		if(rect.Intersects(bounds) == false) continue;

		PushState();
		ConstrainClippingRegion(rect & bounds);
		item->DrawItem(this, rect, true);
		if(!(i != fPos || !IsEnabled() || !IsFocus() || !winActivated))
		{
			SetPenSize(0);
			SetDrawingMode(E_OP_COPY);
			SetHighColor(0, 0, 0);
			StrokeRect(rect);
		}
		PopState();
	}
}


void
EListView::KeyDown(const char *bytes, eint32 numBytes)
{
	if(!IsEnabled() || !IsFocus() || numBytes != 1) return;

	eint32 oldPos = -1, newPos = -1;
	bool doDown = false;

	switch(bytes[0])
	{
		case E_ENTER:
			if(fPos >= 0 && fPos >= fFirstSelected && fPos <= fLastSelected)
			{
				EListItem *item = (EListItem*)fItems.ItemAt(fPos);
				if(!(item == NULL || item->fSelected == false))
				{
					Invoke();
					break;
				}
			}
		case E_SPACE:
			if(fPos >= 0 && fPos < fItems.CountItems())
			{
				if(((EListItem*)fItems.ItemAt(fPos))->fEnabled == false) break;
				if(((EListItem*)fItems.ItemAt(fPos))->fSelected)
					Deselect(fPos);
				else
					Select(fPos, fListType != E_SINGLE_SELECTION_LIST);

				Invalidate();
			}
			break;

		case E_ESCAPE:
			if(fPos >= 0)
			{
				InvalidateItem(fPos);
				fPos = -1;
			}
			else if(fFirstSelected >= 0)
			{
				DeselectAll();
				Invalidate();
			}
			break;

		case E_UP_ARROW:
			{
				if(fPos <= 0) break;
				oldPos = fPos;
				newPos = --fPos;
			}
			break;

		case E_DOWN_ARROW:
			{
				if(fPos >= fItems.CountItems() - 1) break;
				oldPos = fPos;
				newPos = ++fPos;
				doDown = true;
			}
			break;

		case E_PAGE_UP:
		case E_PAGE_DOWN:
			{
				// TODO
			}
			break;

		case E_HOME:
			{
				if(fPos <= 0) break;
				oldPos = fPos;
				newPos = fPos = 0;
			}
			break;

		case E_END:
			{
				if(fPos >= fItems.CountItems() - 1) break;
				oldPos = fPos;
				newPos = fPos = fItems.CountItems() - 1;
				doDown = true;
			}
			break;

		default:
			break;
	}

	if(oldPos >= 0 || newPos >= 0)
	{
		ERect rect = ItemFrame(newPos);
		if(rect.IsValid() == false ||
		   e_is_kind_of(Parent(), EScrollView) == false || e_cast_as(Parent(), EScrollView)->Target() != this)
		{
			InvalidateItem(oldPos);
			if(rect.IsValid()) Invalidate(rect);
		}
		else
		{
			ERect vRect = ConvertFromParent(e_cast_as(Parent(), EScrollView)->TargetFrame());
			if(vRect.top <= rect.top && vRect.bottom >= rect.bottom)
			{
				InvalidateItem(oldPos);
				if(rect.IsValid()) Invalidate(rect);
			}
			else
			{
				float xOffset = Frame().left - ConvertToParent(EPoint(0, 0)).x;
				if(doDown == false)
					ScrollTo(xOffset, rect.top);
				else
					ScrollTo(xOffset, rect.bottom - vRect.Height());
			}
		}
	}

	if(!(!(bytes[0] == E_LEFT_ARROW || bytes[0] == E_RIGHT_ARROW) ||
	     e_is_kind_of(Parent(), EScrollView) == false ||
	     e_cast_as(Parent(), EScrollView)->Target() != this))
	{
		float visibleWidth = e_cast_as(Parent(), EScrollView)->TargetFrame().Width();
		ERect frame = Frame();

		if(visibleWidth >= frame.Width()) return;

		EScrollBar *hsb = e_cast_as(Parent(), EScrollView)->ScrollBar(E_HORIZONTAL);
		if(hsb == NULL) return;

		EPoint originOffset = frame.LeftTop() - ConvertToParent(EPoint(0, 0));

		float step = 0;
		hsb->GetSteps(NULL, &step);

		if(bytes[0] == E_LEFT_ARROW) originOffset.x -= step;
		else originOffset.x += step;

		if(originOffset.x > frame.Width() - visibleWidth) originOffset.x = frame.Width() - visibleWidth;
		else if(originOffset.x < 0) originOffset.x = 0;

		ScrollTo(originOffset);
	}
}


void
EListView::KeyUp(const char *bytes, eint32 numBytes)
{
}


void
EListView::MouseDown(EPoint where)
{
	eint32 btnClicks = 1;

	if(!IsEnabled() || !QueryCurrentMouse(true, E_PRIMARY_MOUSE_BUTTON, true, &btnClicks) || btnClicks > 2) return;

	if((Flags() & E_NAVIGABLE) && !IsFocus()) MakeFocus();

	if(btnClicks == 2)
	{
		if(fFirstSelected >= 0) Invoke();
		return;
	}

	eint32 selectIndex = IndexOf(where, true);
	EListItem *item = ItemAt(selectIndex);
	if(item == NULL || item->fEnabled == false) return;

	if(item->fSelected)
		Deselect(selectIndex);
	else
		Select(selectIndex, fListType != E_SINGLE_SELECTION_LIST);

	fPos = selectIndex;
	Invalidate();
}


void
EListView::MouseUp(EPoint where)
{
}


void
EListView::MouseMoved(EPoint where, euint32 code, const EMessage *a_message)
{
}


void
EListView::SetFont(const EFont *font, euint8 mask)
{
	if(font == NULL) return;

	EView::SetFont(font, mask);

	EFont aFont;
	GetFont(&aFont);
	for(eint32 i = 0; i < fItems.CountItems(); i++)
	{
		EListItem *item = (EListItem*)fItems.ItemAt(i);
		item->Update(this, &aFont);
	}
}


void
EListView::SetSelectionMessage(EMessage *message)
{
	if(message == fSelectionMessage) return;
	if(fSelectionMessage != NULL) delete fSelectionMessage;
	fSelectionMessage = message;
}


void
EListView::SetInvocationMessage(EMessage *message)
{
	SetMessage(message);
}


EMessage*
EListView::SelectionMessage() const
{
	return fSelectionMessage;
}


euint32
EListView::SelectionCommand() const
{
	return(fSelectionMessage ? fSelectionMessage->what : 0);
}


EMessage*
EListView::InvocationMessage() const
{
	return Message();
}


euint32
EListView::InvocationCommand() const
{
	return Command();
}


e_status_t
EListView::Invoke(const EMessage *msg)
{
	if(fFirstSelected < 0)
	{
		return EInvoker::Invoke(msg);
	}
	else
	{
		const EMessage *message = (msg ? msg : Message());
		if(!message) return E_BAD_VALUE;

		EMessage aMsg(*message);

		for(eint32 i = fFirstSelected; i <= fLastSelected; i++)
		{
			EListItem *item = (EListItem*)fItems.ItemAt(i);
			if(item == NULL) continue;
			if(item->fSelected) aMsg.AddInt32("index", i);
		}

		return EInvoker::Invoke(&aMsg);
	}
}


void
EListView::Select(eint32 index, bool extend)
{
	Select(index, index, extend);
}


void
EListView::Select(eint32 start, eint32 finish, bool extend)
{
	if(extend == false)
	{
		for(eint32 i = fFirstSelected; i <= fLastSelected; i++)
		{
			EListItem *item = (EListItem*)fItems.ItemAt(i);
			if(item == NULL) continue;
			item->fSelected = false;
		}
		fFirstSelected = fLastSelected = -1;
	}

	if(start >= 0)
	{
		bool hasNewSelection = false;

		if(finish < 0 || finish >= fItems.CountItems()) finish = fItems.CountItems() - 1;

		for(eint32 index = start; index <= finish; index++)
		{
			EListItem *item = (EListItem*)fItems.ItemAt(index);
			if(item == NULL || item->fSelected == true) continue;

			item->fSelected = true;
			hasNewSelection = true;

			fFirstSelected = (fFirstSelected < 0 ? index : min_c(fFirstSelected, index));
			fLastSelected = (fLastSelected < 0 ? index : max_c(fLastSelected, index));
		}

		if(hasNewSelection && fSelectionMessage != NULL) Invoke(fSelectionMessage);
	}

	SelectionChanged();
}


bool
EListView::IsItemSelected(eint32 index) const
{
	EListItem *item = (EListItem*)fItems.ItemAt(index);
	return(item == NULL ? false : item->fSelected);
}


eint32
EListView::CurrentSelection(eint32 index) const
{
	if(fFirstSelected < 0 || index < 0) return -1;

	// TODO: speed up
	for(eint32 i = fFirstSelected; i <= fLastSelected; i++)
	{
		EListItem *item = (EListItem*)fItems.ItemAt(i);
		if(item == NULL) continue;
		if(item->fSelected) index--;
		if(index < 0) return i;
	}

	return -1;
}


void
EListView::Deselect(eint32 index)
{
	EListItem *item = (EListItem*)fItems.ItemAt(index);
	if(item == NULL || item->fSelected == false) return;

	item->fSelected = false;

	if(index == fFirstSelected)
	{
		fFirstSelected = -1;

		while((++index) < min_c(fItems.CountItems(), fLastSelected + 1))
		{
			item = (EListItem*)fItems.ItemAt(index);
			if(item->fSelected) {fFirstSelected = index; break;}
		}

		if(fFirstSelected < 0) fLastSelected = -1;
	}
	else if(index == fLastSelected)
	{
		fLastSelected = -1;

		while((--index) >= max_c(0, fFirstSelected))
		{
			item = (EListItem*)fItems.ItemAt(index);
			if(item->fSelected) {fLastSelected = index; break;}
		}

		if(fLastSelected < 0) fFirstSelected = -1;
	}

	SelectionChanged();
}


void
EListView::DeselectAll()
{
	if(fFirstSelected < 0) return;

	for(eint32 i = fFirstSelected; i <= fLastSelected; i++)
	{
		EListItem *item = (EListItem*)fItems.ItemAt(i);
		if(item == NULL) continue;
		item->fSelected = false;
	}

	fFirstSelected = fLastSelected = -1;

	SelectionChanged();
}


void
EListView::DeselectExcept(eint32 start, eint32 finish)
{
	if(fFirstSelected < 0) return;

	if(start >= 0 && (finish < 0 || finish >= fItems.CountItems())) finish = fItems.CountItems() - 1;

	for(eint32 i = fFirstSelected; i <= fLastSelected; i++)
	{
		if(start >= 0 && start <= finish && i >= start && i <= finish) continue;

		EListItem *item = (EListItem*)fItems.ItemAt(i);
		if(item == NULL) continue;
		item->fSelected = false;
	}

	if(start >= 0 && start <= finish && !(start > fLastSelected || finish < fFirstSelected))
	{
		fFirstSelected = max_c(fFirstSelected, start);
		fLastSelected = min_c(fLastSelected, finish);
	}
	else
	{
		fFirstSelected = fLastSelected = -1;
	}

	SelectionChanged();
}


bool
EListView::SwapItems(eint32 indexA, eint32 indexB)
{
	bool retVal = false;

	do
	{
		if((retVal = fItems.SwapItems(indexA, indexB)) == false) break;

		if(fFirstSelected < 0) break;
		if(indexA >= fFirstSelected && indexA <= fLastSelected &&
		   indexB >= fFirstSelected && indexB <= fLastSelected) break;

		eint32 newIn = -1;

		if(indexA >= fFirstSelected && indexA <= fLastSelected) newIn = indexB;
		else if(indexB >= fFirstSelected && indexB <= fLastSelected) newIn = indexA;

		if(newIn < 0) break;

		fFirstSelected = min_c(fFirstSelected, newIn);
		fLastSelected = max_c(fLastSelected, newIn);

		for(eint32 i = fFirstSelected; i <= fLastSelected; i++)
		{
			EListItem *item = (EListItem*)fItems.ItemAt(i);
			if(item->fSelected == true) break;
			fFirstSelected++;
		}

		for(eint32 i = fLastSelected; i >= fFirstSelected; i--)
		{
			EListItem *item = (EListItem*)fItems.ItemAt(i);
			if(item->fSelected == true) break;
			fLastSelected--;
		}

		if(fLastSelected < fFirstSelected) fFirstSelected = fLastSelected = -1;

		SelectionChanged();
	}while(false);

	return retVal;
}


bool
EListView::MoveItem(eint32 fromIndex, eint32 toIndex)
{
	bool retVal = false;

	do
	{
		if((retVal = fItems.MoveItem(fromIndex, toIndex)) == false) break;

		if(fFirstSelected < 0) break;
		if((fromIndex < fFirstSelected && toIndex < fFirstSelected) ||
		   (fromIndex > fLastSelected && toIndex > fLastSelected)) break;

		// TODO: speed up
		fFirstSelected = fLastSelected = -1;
		for(eint32 i = 0; i < fItems.CountItems(); i++)
		{
			EListItem *item = (EListItem*)fItems.ItemAt(i);
			if(item->fSelected == false) continue;
			fFirstSelected = (fFirstSelected < 0 ? i : min_c(fFirstSelected, i));
			fLastSelected = (fLastSelected < 0 ? i : max_c(fLastSelected, i));
		}
		SelectionChanged();
	}while(false);

	return retVal;
}


bool
EListView::ReplaceItem(eint32 index, EListItem *newItem, EListItem **oldItem)
{
	bool retVal = false;
	EListItem *old_item = NULL;

	do
	{
		if(newItem == NULL) break;
		if((retVal = fItems.ReplaceItem(index, (void*)newItem, (void**)old_item)) == false) break;

		bool oldItemSelected = false;
		if(old_item) {old_item->fOwner = NULL; oldItemSelected = old_item->fSelected;}
		if(oldItem != NULL) {*oldItem = old_item; old_item = NULL;}

		newItem->fSelected = false;
		newItem->fOwner = this;

		if(oldItemSelected == false || fFirstSelected < 0) break;

		for(eint32 i = fFirstSelected; i <= fLastSelected; i++)
		{
			EListItem *item = (EListItem*)fItems.ItemAt(i);
			if(item->fSelected == true) break;
			fFirstSelected++;
		}

		for(eint32 i = fLastSelected; i >= fFirstSelected; i--)
		{
			EListItem *item = (EListItem*)fItems.ItemAt(i);
			if(item->fSelected == true) break;
			fLastSelected--;
		}

		if(fLastSelected < fFirstSelected) fFirstSelected = fLastSelected = -1;

		SelectionChanged();
	}while(false);

	if(old_item != NULL && retVal == true) delete old_item;

	return retVal;
}


void
EListView::SortItems(int (*cmp)(const EListItem **a, const EListItem **b))
{
	if(cmp == NULL) return;

	fItems.SortItems((int (*)(const void*, const void*))cmp);

	if(fFirstSelected > 0)
	{
		fFirstSelected = fLastSelected = -1;
		for(eint32 i = 0; i < fItems.CountItems(); i++)
		{
			EListItem *item = (EListItem*)fItems.ItemAt(i);
			if(item->fSelected == false) continue;
			fFirstSelected = (fFirstSelected < 0 ? i : min_c(fFirstSelected, i));
			fLastSelected = (fLastSelected < 0 ? i : max_c(fLastSelected, i));
		}
		SelectionChanged();
	}
}


void
EListView::DoForEach(bool (*func)(EListItem *item))
{
	fItems.DoForEach((bool (*)(void*))func);
}


void
EListView::DoForEach(bool (*func)(EListItem *item, void *user_data), void *user_data)
{
	fItems.DoForEach((bool (*)(void*, void*))func, user_data);
}


const EListItem**
EListView::Items() const
{
	return (const EListItem**)fItems.Items();
}


ERect
EListView::ItemFrame(eint32 index) const
{
	ERect r;

	if(index >= 0 && index < fItems.CountItems())
	{
		EListItem *item = (EListItem*)fItems.ItemAt(index);
		if(item->Height() >= 0)
		{
			r.Set(1, 1, Frame().Width() - 1, 1);

			for(eint32 i = 0; i <= index; i++)
			{
				EListItem *item = (EListItem*)fItems.ItemAt(i);
				if(item->Height() < 0) continue;

				r.top = r.bottom;
				r.bottom = r.top + item->Height();
				if(r.top > 1) r.OffsetBy(0, 1);
			}
		}
	}

	return r;
}


void
EListView::InvalidateItem(eint32 index)
{
	ERect r = ItemFrame(index);
	if(r.IsValid()) Invalidate(r, true);
}


void
EListView::ScrollToItem(eint32 index)
{
	ERect rect = ItemFrame(index);
	if(rect.IsValid() == false) return;

	ERect vRect = Bounds();
	if(vRect.top <= rect.top && vRect.bottom >= rect.bottom) return;

	EPoint pt = rect.LeftTop();
	pt.ConstrainTo(vRect);

	ScrollTo(Frame().left - ConvertToParent(EPoint(0, 0)).x,
		 pt.y == vRect.top ? rect.top : rect.bottom - vRect.Height());
}


void
EListView::ScrollToSelection()
{
	EScrollView *scrollView = e_cast_as(Parent(), EScrollView);
	if(scrollView == NULL || scrollView->Target() != this) return;

	ERect rect = ItemFrame(fFirstSelected);
	if(rect.IsValid() == false) return;

	ERect vRect = ConvertFromParent(scrollView->TargetFrame());
	if(vRect.top <= rect.top && vRect.bottom >= rect.bottom) return;

	EPoint pt = rect.LeftTop();
	pt.ConstrainTo(vRect);

	ScrollTo(Frame().left - ConvertToParent(EPoint(0, 0)).x,
		 pt.y == vRect.top ? rect.top : rect.bottom - vRect.Height());
}


void
EListView::AttachedToWindow()
{
	if(Target() == NULL) SetTarget(Window());
}


void
EListView::DetachedFromWindow()
{
	if(Target() == Window()) SetTarget(NULL);
}


void
EListView::SetPosition(eint32 pos)
{
	if(pos >= fItems.CountItems()) pos = -1;
	fPos = pos;
}


eint32
EListView::Position() const
{
	return fPos;
}

