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
 * File: OutlineListView.cpp
 * Description: BOutlineListView --- Displays a list of items that can be structured like an outline
 *
 * --------------------------------------------------------------------------*/

#include <OutlineListView.h>

#include <ListItem.h>
#include <ListView.h>
#include <os/debug.h>

BOutlineListView::BOutlineListView(BRect frame, const char *name, bhapi::list_view_type type,
				    __be_uint32_t resizingMode,  __be_uint32_t flags)
	: BListView(frame, name, type, resizingMode, flags)
{
}


BOutlineListView::~BOutlineListView()
{
	BOutlineListView::MakeEmpty();
}


bool BOutlineListView::AddUnder(BListItem *item, BListItem *superitem)
{
	if(item == NULL || item->fOwner != NULL || item->fFullOwner != NULL ||
	   superitem == NULL || superitem->fFullOwner != this || superitem->fLevel == B_MAXUINT32) return false;

	__be_int32_t index = fFullItems.IndexOf(superitem);
	if(index < 0) return false;

	__be_uint32_t tmp = item->fLevel;
	item->fLevel = superitem->fLevel + 1;

	bool retVal = BOutlineListView::AddItem(item, index + 1);
	if(retVal == false) item->fLevel = tmp;

	return retVal;
}


bool BOutlineListView::AddItem(BListItem *item)
{
	return BOutlineListView::AddItem(item, fFullItems.CountItems());
}


bool BOutlineListView::AddItem(BListItem *item,  __be_int32_t fullListIndex)
{
	if(item == NULL || item->fOwner != NULL || item->fFullOwner != NULL) return false;
	if(fFullItems.AddItem(item, fullListIndex) == false) return false;

	item->fFullOwner = this;

	if(item->IsVisible())
	{
		__be_int32_t index = fullListIndex;

		if(index == 0) BListView::AddItem(item, 0);
		else while(index > 0)
		{
			BListItem *aItem = (BListItem*)fFullItems.ItemAt(--index);
			if((aItem->fLevel == item->fLevel && item->fLevel > 0) || aItem->fOwner == NULL) continue;

			index = IndexOf(aItem);
			BListView::AddItem(item, ++index);
			break;
		}

		__be_int32_t tIndex = fullListIndex;
		while(tIndex < fFullItems.CountItems() - 1)
		{
			BListItem *aItem = (BListItem*)fFullItems.ItemAt(++tIndex);
			if(aItem->fLevel <= item->fLevel) break;

			if(item->fExpanded)
			{
				if(aItem->fOwner != NULL)
				{
					__be_int32_t aIndex = IndexOf(aItem);
					BListView::RemoveItem(aItem, false);
					if(aIndex < index) index--;
				}

				if(aItem->IsVisible() == false) continue;

				BListView::AddItem(aItem, ++index);
			}
			else
			{
				if(aItem->fOwner == NULL) continue;
				BListView::RemoveItem(aItem, false);
			}
		}
	}

	return true;
}


bool BOutlineListView::RemoveItem(BListItem *item, bool auto_destruct_item_and_subitems)
{
	if(BOutlineListView::RemoveItem(FullListIndexOf(item), auto_destruct_item_and_subitems, NULL) == NULL) return false;
	if(auto_destruct_item_and_subitems) delete item;

	return true;
}


BListItem*
BOutlineListView::RemoveItem(__be_int32_t fullListIndex, bool auto_destruct_subitems,  __be_int32_t *count)
{
	BListItem *item = (BListItem*)fFullItems.RemoveItem(fullListIndex);
	if(item == NULL) return NULL;

	item->fFullOwner = NULL;
	if(item->fOwner != NULL) BListView::RemoveItem(item, false);

	if(count) *count = 0;

	__be_int32_t tIndex = fullListIndex;
	while(tIndex >= 0 && tIndex <= fFullItems.CountItems() - 1)
	{
		BListItem *aItem = (BListItem*)fFullItems.ItemAt(tIndex);
		if(aItem->fLevel <= item->fLevel) break;

		fFullItems.RemoveItem(tIndex);

		if(count) (*count) += 1;

		aItem->fFullOwner = NULL;
		if(aItem->fOwner != NULL) BListView::RemoveItem(aItem, auto_destruct_subitems);
	}

	return item;
}


BListItem*
BOutlineListView::RemoveItem(__be_int32_t fullListIndex)
{
	return RemoveItem(fullListIndex, true, NULL);
}


bool BOutlineListView::RemoveItems(__be_int32_t fullListIndex,  __be_int32_t count, bool auto_destruct_items)
{
	if(fullListIndex < 0 || fullListIndex >= fFullItems.CountItems()) return false;

	if(count < 0) count = fFullItems.CountItems() - fullListIndex;
	else count = min_c(fFullItems.CountItems() - fullListIndex, count);

	// TODO: remove at once
	while(count-- > 0)
	{
		__be_int32_t tmp = 0;

		BListItem *item = BOutlineListView::RemoveItem(fullListIndex, auto_destruct_items, &tmp);
		if(item == NULL) return false;

		if(auto_destruct_items) delete item;
		if(tmp > 0) count -= tmp;
	}

	return true;
}


BListItem*
BOutlineListView::FullListItemAt(__be_int32_t fullListIndex) const
{
	return (BListItem*)fFullItems.ItemAt(fullListIndex);
}


__be_int32_t BOutlineListView::FullListIndexOf(const BListItem *item) const
{
	if(item == NULL || item->fFullOwner != this) return -1;
	return fFullItems.IndexOf((void*)item);
}


BListItem*
BOutlineListView::FullListFirstItem() const
{
	return (BListItem*)fFullItems.FirstItem();
}


BListItem*
BOutlineListView::FullListLastItem() const
{
	return (BListItem*)fFullItems.LastItem();
}


bool BOutlineListView::FullListHasItem(const BListItem *item) const
{
	return(!(item == NULL || item->fFullOwner != this));
}


__be_int32_t BOutlineListView::FullListCountItems() const
{
	return fFullItems.CountItems();
}


__be_int32_t BOutlineListView::FullListCurrentSelection(__be_int32_t index) const
{
	__be_int32_t tIndex = CurrentSelection(index);
	BListItem *item = ItemAt(tIndex);
	return(item == NULL ? -1 : fFullItems.IndexOf(item));
}


void BOutlineListView::MakeEmpty()
{
	while(fFullItems.CountItems() > 0)
	{
		BListItem *item = (BListItem*)fFullItems.RemoveItem((__be_int32_t)0);
		item->fFullOwner = NULL;
		delete item;
	}
}


bool BOutlineListView::FullListIsEmpty() const
{
	return fFullItems.IsEmpty();
}


void BOutlineListView::FullListDoForEach(bool (*func)(BListItem *item))
{
	fFullItems.DoForEach((bool (*)(void*))func);
}


void BOutlineListView::FullListDoForEach(bool (*func)(BListItem *item, void *user_data), void *user_data)
{
	fFullItems.DoForEach((bool (*)(void*, void*))func, user_data);
}


BListItem*
BOutlineListView::Superitem(const BListItem *item) const
{
	if(item == NULL || item->fFullOwner != this) return NULL;

	__be_int32_t index = fFullItems.IndexOf((void*)item);
	if(index < 0) return NULL;

	while(index > 0)
	{
		BListItem *aItem = (BListItem*)fFullItems.ItemAt(--index);
		if(aItem->fLevel < item->fLevel) return aItem;
	}

	return NULL;
}


bool BOutlineListView::HasSubitems(const BListItem *item) const
{
	if(item == NULL || item->fFullOwner != this) return false;

	__be_int32_t index = fFullItems.IndexOf((void*)item);
	if(index < 0 || index == fFullItems.CountItems() - 1) return false;

	BListItem *aItem = (BListItem*)fFullItems.ItemAt(index + 1);
	return(aItem->fLevel > item->fLevel);
}


__be_int32_t BOutlineListView::CountItemsUnder(BListItem *item, bool oneLevelOnly) const
{
	if(item == NULL || item->fFullOwner != this) return 0;

	__be_int32_t retVal = 0;
	__be_int32_t tIndex = fFullItems.IndexOf((void*)item);

	while(tIndex >= 0 && tIndex < fFullItems.CountItems() - 1)
	{
		BListItem *aItem = (BListItem*)fFullItems.ItemAt(++tIndex);
		if(aItem->fLevel <= item->fLevel) break;
		if(!oneLevelOnly || aItem->fLevel == item->fLevel + 1) retVal++;
	}

	return retVal;
}


BListItem*
BOutlineListView::ItemUnderAt(BListItem *item, bool oneLevelOnly,  __be_int32_t index) const
{
	if(item == NULL || item->fFullOwner != this || index < 0) return NULL;

	__be_int32_t tIndex = fFullItems.IndexOf((void*)item);

	while(tIndex >= 0 && tIndex < fFullItems.CountItems() - 1)
	{
		BListItem *aItem = (BListItem*)fFullItems.ItemAt(++tIndex);
		if(aItem->fLevel <= item->fLevel) break;
		if(!oneLevelOnly || aItem->fLevel == item->fLevel + 1)
		{
			if(index == 0) return aItem;
			index--;
		}
	}

	return NULL;
}


BListItem*
BOutlineListView::EachItemUnder(BListItem *item, bool oneLevelOnly,
				BListItem *(*eachFunc)(BListItem *item, void *user_data), void *user_data)
{
	if(item == NULL || item->fFullOwner != this || eachFunc == NULL) return NULL;

	BListItem *retVal = NULL;
	__be_int32_t tIndex = fFullItems.IndexOf((void*)item);

	while(retVal == NULL && tIndex >= 0 && tIndex < fFullItems.CountItems() - 1)
	{
		BListItem *aItem = (BListItem*)fFullItems.ItemAt(++tIndex);
		if(aItem->fLevel <= item->fLevel) break;
		if(!oneLevelOnly || aItem->fLevel == item->fLevel + 1) retVal = (*eachFunc)(aItem, user_data);
		if(retVal == NULL) continue;
		if(retVal->fFullOwner != this)
		{
			BHAPI_WARNING("[INTERFACE]: %s --- \"eachFunc\" shouldn't return item which owner not this!", __PRETTY_FUNCTION__);
			retVal = NULL;
		}
	}

	return retVal;
}


void BOutlineListView::Expand(BListItem *item)
{
	if(item == NULL || item->fFullOwner != this || item->fExpanded) return;

	item->fExpanded = true;
	if(item->IsVisible())
	{
		__be_int32_t tIndex = fFullItems.IndexOf(item);
		__be_int32_t index = IndexOf(item);
		while(tIndex < fFullItems.CountItems() - 1)
		{
			BListItem *aItem = (BListItem*)fFullItems.ItemAt(++tIndex);
			if(aItem->fLevel <= item->fLevel) break;

			if(aItem->fOwner != NULL)
			{
				__be_int32_t aIndex = IndexOf(aItem);
				BListView::RemoveItem(aItem, false);
				if(aIndex < index) index--;
			}

			if(aItem->IsVisible() == false) continue;

			BListView::AddItem(aItem, ++index);
		}
	}
}


void BOutlineListView::Collapse(BListItem *item)
{
	if(item == NULL || item->fFullOwner != this || item->fExpanded == false) return;

	item->fExpanded = false;
	if(item->IsVisible())
	{
		__be_int32_t tIndex = fFullItems.IndexOf(item);
		while(tIndex < fFullItems.CountItems() - 1)
		{
			BListItem *aItem = (BListItem*)fFullItems.ItemAt(++tIndex);
			if(aItem->fLevel <= item->fLevel) break;
			if(aItem->fOwner == NULL) continue;
			BListView::RemoveItem(aItem, false);
		}
	}
}


bool BOutlineListView::IsExpanded(__be_int32_t fullListIndex) const
{
	BListItem *item = (BListItem*)fFullItems.ItemAt(fullListIndex);
	return(item == NULL ? false : item->fExpanded);
}


const BListItem**
BOutlineListView::FullListItems() const
{
	return (const BListItem**)fFullItems.Items();
}


void BOutlineListView::MouseDown(BPoint where)
{
	__be_int32_t btnClicks = 1;
	if(!IsEnabled() || !QueryCurrentMouse(true, B_PRIMARY_MOUSE_BUTTON, true, &btnClicks) || btnClicks > 1)
	{
		BListView::MouseDown(where);
		return;
	}

	__be_int32_t index = IndexOf(where, true);
	BListItem *item = ItemAt(index);
	if(item == NULL || item->fEnabled == false || item->HasSubitems() == false)
	{
		BListView::MouseDown(where);
		return;
	}

	BRect rect = ItemFrame(index);
	if(item->fLevel > 0) rect.left += rect.Height() * 2.f * (float)item->fLevel;
	rect.right = rect.left + rect.Height();
	if(rect.Contains(where) == false)
	{
		BListView::MouseDown(where);
		return;
	}

	item->SetExpanded(!(item->fExpanded));
	Invalidate();
}


void BOutlineListView::MouseUp(BPoint where)
{
	BListView::MouseUp(where);
}


void BOutlineListView::MouseMoved(BPoint where,  __be_uint32_t code, const BMessage *a_message)
{
	BListView::MouseMoved(where, code, a_message);
}


void BOutlineListView::KeyDown(const char *bytes,  __be_int32_t numBytes)
{
	BListItem *item = ItemAt(Position());

	if(!(item == NULL || IsEnabled() == false || IsFocus() == false ||
	     numBytes != 1 || !(bytes[0] == B_LEFT_ARROW || bytes[0] == B_RIGHT_ARROW)))
	{
		if(item->fExpanded ? (bytes[0] == B_LEFT_ARROW) : (bytes[0] == B_RIGHT_ARROW))
		{
			item->SetExpanded(!(item->fExpanded));
			Invalidate();
			return;
		}
	}

	BListView::KeyDown(bytes, numBytes);
}


void BOutlineListView::KeyUp(const char *bytes,  __be_int32_t numBytes)
{
	BListView::KeyUp(bytes, numBytes);
}

