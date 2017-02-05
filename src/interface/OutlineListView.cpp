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
 * File: OutlineListView.cpp
 * Description: EOutlineListView --- Displays a list of items that can be structured like an outline
 *
 * --------------------------------------------------------------------------*/

#include "OutlineListView.h"

EOutlineListView::EOutlineListView(ERect frame, const char *name, e_list_view_type type,
				   euint32 resizingMode, euint32 flags)
	: EListView(frame, name, type, resizingMode, flags)
{
}


EOutlineListView::~EOutlineListView()
{
	EOutlineListView::MakeEmpty();
}


bool
EOutlineListView::AddUnder(EListItem *item, EListItem *superitem)
{
	if(item == NULL || item->fOwner != NULL || item->fFullOwner != NULL ||
	   superitem == NULL || superitem->fFullOwner != this || superitem->fLevel == E_MAXUINT32) return false;

	eint32 index = fFullItems.IndexOf(superitem);
	if(index < 0) return false;

	euint32 tmp = item->fLevel;
	item->fLevel = superitem->fLevel + 1;

	bool retVal = EOutlineListView::AddItem(item, index + 1);
	if(retVal == false) item->fLevel = tmp;

	return retVal;
}


bool
EOutlineListView::AddItem(EListItem *item)
{
	return EOutlineListView::AddItem(item, fFullItems.CountItems());
}


bool
EOutlineListView::AddItem(EListItem *item, eint32 fullListIndex)
{
	if(item == NULL || item->fOwner != NULL || item->fFullOwner != NULL) return false;
	if(fFullItems.AddItem(item, fullListIndex) == false) return false;

	item->fFullOwner = this;

	if(item->IsVisible())
	{
		eint32 index = fullListIndex;

		if(index == 0) EListView::AddItem(item, 0);
		else while(index > 0)
		{
			EListItem *aItem = (EListItem*)fFullItems.ItemAt(--index);
			if((aItem->fLevel == item->fLevel && item->fLevel > 0) || aItem->fOwner == NULL) continue;

			index = IndexOf(aItem);
			EListView::AddItem(item, ++index);
			break;
		}

		eint32 tIndex = fullListIndex;
		while(tIndex < fFullItems.CountItems() - 1)
		{
			EListItem *aItem = (EListItem*)fFullItems.ItemAt(++tIndex);
			if(aItem->fLevel <= item->fLevel) break;

			if(item->fExpanded)
			{
				if(aItem->fOwner != NULL)
				{
					eint32 aIndex = IndexOf(aItem);
					EListView::RemoveItem(aItem, false);
					if(aIndex < index) index--;
				}

				if(aItem->IsVisible() == false) continue;

				EListView::AddItem(aItem, ++index);
			}
			else
			{
				if(aItem->fOwner == NULL) continue;
				EListView::RemoveItem(aItem, false);
			}
		}
	}

	return true;
}


bool
EOutlineListView::RemoveItem(EListItem *item, bool auto_destruct_item_and_subitems)
{
	if(EOutlineListView::RemoveItem(FullListIndexOf(item), auto_destruct_item_and_subitems, NULL) == NULL) return false;
	if(auto_destruct_item_and_subitems) delete item;

	return true;
}


EListItem*
EOutlineListView::RemoveItem(eint32 fullListIndex, bool auto_destruct_subitems, eint32 *count)
{
	EListItem *item = (EListItem*)fFullItems.RemoveItem(fullListIndex);
	if(item == NULL) return NULL;

	item->fFullOwner = NULL;
	if(item->fOwner != NULL) EListView::RemoveItem(item, false);

	if(count) *count = 0;

	eint32 tIndex = fullListIndex;
	while(tIndex >= 0 && tIndex <= fFullItems.CountItems() - 1)
	{
		EListItem *aItem = (EListItem*)fFullItems.ItemAt(tIndex);
		if(aItem->fLevel <= item->fLevel) break;

		fFullItems.RemoveItem(tIndex);

		if(count) (*count) += 1;

		aItem->fFullOwner = NULL;
		if(aItem->fOwner != NULL) EListView::RemoveItem(aItem, auto_destruct_subitems);
	}

	return item;
}


EListItem*
EOutlineListView::RemoveItem(eint32 fullListIndex)
{
	return RemoveItem(fullListIndex, true, NULL);
}


bool
EOutlineListView::RemoveItems(eint32 fullListIndex, eint32 count, bool auto_destruct_items)
{
	if(fullListIndex < 0 || fullListIndex >= fFullItems.CountItems()) return false;

	if(count < 0) count = fFullItems.CountItems() - fullListIndex;
	else count = min_c(fFullItems.CountItems() - fullListIndex, count);

	// TODO: remove at once
	while(count-- > 0)
	{
		eint32 tmp = 0;

		EListItem *item = EOutlineListView::RemoveItem(fullListIndex, auto_destruct_items, &tmp);
		if(item == NULL) return false;

		if(auto_destruct_items) delete item;
		if(tmp > 0) count -= tmp;
	}

	return true;
}


EListItem*
EOutlineListView::FullListItemAt(eint32 fullListIndex) const
{
	return (EListItem*)fFullItems.ItemAt(fullListIndex);
}


eint32
EOutlineListView::FullListIndexOf(const EListItem *item) const
{
	if(item == NULL || item->fFullOwner != this) return -1;
	return fFullItems.IndexOf((void*)item);
}


EListItem*
EOutlineListView::FullListFirstItem() const
{
	return (EListItem*)fFullItems.FirstItem();
}


EListItem*
EOutlineListView::FullListLastItem() const
{
	return (EListItem*)fFullItems.LastItem();
}


bool
EOutlineListView::FullListHasItem(const EListItem *item) const
{
	return(!(item == NULL || item->fFullOwner != this));
}


eint32
EOutlineListView::FullListCountItems() const
{
	return fFullItems.CountItems();
}


eint32
EOutlineListView::FullListCurrentSelection(eint32 index) const
{
	eint32 tIndex = CurrentSelection(index);
	EListItem *item = ItemAt(tIndex);
	return(item == NULL ? -1 : fFullItems.IndexOf(item));
}


void
EOutlineListView::MakeEmpty()
{
	while(fFullItems.CountItems() > 0)
	{
		EListItem *item = (EListItem*)fFullItems.RemoveItem((eint32)0);
		item->fFullOwner = NULL;
		delete item;
	}
}


bool
EOutlineListView::FullListIsEmpty() const
{
	return fFullItems.IsEmpty();
}


void
EOutlineListView::FullListDoForEach(bool (*func)(EListItem *item))
{
	fFullItems.DoForEach((bool (*)(void*))func);
}


void
EOutlineListView::FullListDoForEach(bool (*func)(EListItem *item, void *user_data), void *user_data)
{
	fFullItems.DoForEach((bool (*)(void*, void*))func, user_data);
}


EListItem*
EOutlineListView::Superitem(const EListItem *item) const
{
	if(item == NULL || item->fFullOwner != this) return NULL;

	eint32 index = fFullItems.IndexOf((void*)item);
	if(index < 0) return NULL;

	while(index > 0)
	{
		EListItem *aItem = (EListItem*)fFullItems.ItemAt(--index);
		if(aItem->fLevel < item->fLevel) return aItem;
	}

	return NULL;
}


bool
EOutlineListView::HasSubitems(const EListItem *item) const
{
	if(item == NULL || item->fFullOwner != this) return false;

	eint32 index = fFullItems.IndexOf((void*)item);
	if(index < 0 || index == fFullItems.CountItems() - 1) return false;

	EListItem *aItem = (EListItem*)fFullItems.ItemAt(index + 1);
	return(aItem->fLevel > item->fLevel);
}


eint32
EOutlineListView::CountItemsUnder(EListItem *item, bool oneLevelOnly) const
{
	if(item == NULL || item->fFullOwner != this) return 0;

	eint32 retVal = 0;
	eint32 tIndex = fFullItems.IndexOf((void*)item);

	while(tIndex >= 0 && tIndex < fFullItems.CountItems() - 1)
	{
		EListItem *aItem = (EListItem*)fFullItems.ItemAt(++tIndex);
		if(aItem->fLevel <= item->fLevel) break;
		if(!oneLevelOnly || aItem->fLevel == item->fLevel + 1) retVal++;
	}

	return retVal;
}


EListItem*
EOutlineListView::ItemUnderAt(EListItem *item, bool oneLevelOnly, eint32 index) const
{
	if(item == NULL || item->fFullOwner != this || index < 0) return NULL;

	eint32 tIndex = fFullItems.IndexOf((void*)item);

	while(tIndex >= 0 && tIndex < fFullItems.CountItems() - 1)
	{
		EListItem *aItem = (EListItem*)fFullItems.ItemAt(++tIndex);
		if(aItem->fLevel <= item->fLevel) break;
		if(!oneLevelOnly || aItem->fLevel == item->fLevel + 1)
		{
			if(index == 0) return aItem;
			index--;
		}
	}

	return NULL;
}


EListItem*
EOutlineListView::EachItemUnder(EListItem *item, bool oneLevelOnly,
				EListItem *(*eachFunc)(EListItem *item, void *user_data), void *user_data)
{
	if(item == NULL || item->fFullOwner != this || eachFunc == NULL) return NULL;

	EListItem *retVal = NULL;
	eint32 tIndex = fFullItems.IndexOf((void*)item);

	while(retVal == NULL && tIndex >= 0 && tIndex < fFullItems.CountItems() - 1)
	{
		EListItem *aItem = (EListItem*)fFullItems.ItemAt(++tIndex);
		if(aItem->fLevel <= item->fLevel) break;
		if(!oneLevelOnly || aItem->fLevel == item->fLevel + 1) retVal = (*eachFunc)(aItem, user_data);
		if(retVal == NULL) continue;
		if(retVal->fFullOwner != this)
		{
			ETK_WARNING("[INTERFACE]: %s --- \"eachFunc\" shouldn't return item which owner not this!", __PRETTY_FUNCTION__);
			retVal = NULL;
		}
	}

	return retVal;
}


void
EOutlineListView::Expand(EListItem *item)
{
	if(item == NULL || item->fFullOwner != this || item->fExpanded) return;

	item->fExpanded = true;
	if(item->IsVisible())
	{
		eint32 tIndex = fFullItems.IndexOf(item);
		eint32 index = IndexOf(item);
		while(tIndex < fFullItems.CountItems() - 1)
		{
			EListItem *aItem = (EListItem*)fFullItems.ItemAt(++tIndex);
			if(aItem->fLevel <= item->fLevel) break;

			if(aItem->fOwner != NULL)
			{
				eint32 aIndex = IndexOf(aItem);
				EListView::RemoveItem(aItem, false);
				if(aIndex < index) index--;
			}

			if(aItem->IsVisible() == false) continue;

			EListView::AddItem(aItem, ++index);
		}
	}
}


void
EOutlineListView::Collapse(EListItem *item)
{
	if(item == NULL || item->fFullOwner != this || item->fExpanded == false) return;

	item->fExpanded = false;
	if(item->IsVisible())
	{
		eint32 tIndex = fFullItems.IndexOf(item);
		while(tIndex < fFullItems.CountItems() - 1)
		{
			EListItem *aItem = (EListItem*)fFullItems.ItemAt(++tIndex);
			if(aItem->fLevel <= item->fLevel) break;
			if(aItem->fOwner == NULL) continue;
			EListView::RemoveItem(aItem, false);
		}
	}
}


bool
EOutlineListView::IsExpanded(eint32 fullListIndex) const
{
	EListItem *item = (EListItem*)fFullItems.ItemAt(fullListIndex);
	return(item == NULL ? false : item->fExpanded);
}


const EListItem**
EOutlineListView::FullListItems() const
{
	return (const EListItem**)fFullItems.Items();
}


void
EOutlineListView::MouseDown(EPoint where)
{
	eint32 btnClicks = 1;
	if(!IsEnabled() || !QueryCurrentMouse(true, E_PRIMARY_MOUSE_BUTTON, true, &btnClicks) || btnClicks > 1)
	{
		EListView::MouseDown(where);
		return;
	}

	eint32 index = IndexOf(where, true);
	EListItem *item = ItemAt(index);
	if(item == NULL || item->fEnabled == false || item->HasSubitems() == false)
	{
		EListView::MouseDown(where);
		return;
	}

	ERect rect = ItemFrame(index);
	if(item->fLevel > 0) rect.left += rect.Height() * 2.f * (float)item->fLevel;
	rect.right = rect.left + rect.Height();
	if(rect.Contains(where) == false)
	{
		EListView::MouseDown(where);
		return;
	}

	item->SetExpanded(!(item->fExpanded));
	Invalidate();
}


void
EOutlineListView::MouseUp(EPoint where)
{
	EListView::MouseUp(where);
}


void
EOutlineListView::MouseMoved(EPoint where, euint32 code, const EMessage *a_message)
{
	EListView::MouseMoved(where, code, a_message);
}


void
EOutlineListView::KeyDown(const char *bytes, eint32 numBytes)
{
	EListItem *item = ItemAt(Position());

	if(!(item == NULL || IsEnabled() == false || IsFocus() == false ||
	     numBytes != 1 || !(bytes[0] == E_LEFT_ARROW || bytes[0] == E_RIGHT_ARROW)))
	{
		if(item->fExpanded ? (bytes[0] == E_LEFT_ARROW) : (bytes[0] == E_RIGHT_ARROW))
		{
			item->SetExpanded(!(item->fExpanded));
			Invalidate();
			return;
		}
	}

	EListView::KeyDown(bytes, numBytes);
}


void
EOutlineListView::KeyUp(const char *bytes, eint32 numBytes)
{
	EListView::KeyUp(bytes, numBytes);
}

