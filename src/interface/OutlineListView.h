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
 * File: OutlineListView.h
 * Description: EOutlineListView --- Displays a list of items that can be structured like an outline
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_OUTLINE_LIST_VIEW_H__
#define __ETK_OUTLINE_LIST_VIEW_H__

#include "./../interface/ListView.h>

#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_ETK EOutlineListView : public EListView {
public:
	EOutlineListView(ERect frame,
			 const char *name,
			 e_list_view_type type = E_SINGLE_SELECTION_LIST,
			 euint32 resizingMode = E_FOLLOW_LEFT | E_FOLLOW_TOP,
			 euint32 flags = E_WILL_DRAW | E_NAVIGABLE | E_FRAME_EVENTS);
	virtual ~EOutlineListView();

	virtual bool		AddUnder(EListItem *item, EListItem *superitem);

	virtual bool		AddItem(EListItem *item);
	virtual bool		AddItem(EListItem *item, eint32 fullListIndex);
	virtual bool		RemoveItem(EListItem *item, bool auto_destruct_item_and_subitems = true);
	virtual bool		RemoveItems(eint32 fullListIndex, eint32 count, bool auto_destruct_items = true);

	virtual EListItem	*RemoveItem(eint32 fullListIndex, bool auto_destruct_subitems, eint32 *count);
	virtual EListItem	*RemoveItem(eint32 fullListIndex); // same as RemoveItem(fullListIndex, true, NULL)

	EListItem		*FullListItemAt(eint32 fullListIndex) const;
	eint32			FullListIndexOf(const EListItem *item) const;
	EListItem		*FullListFirstItem() const;
	EListItem		*FullListLastItem() const;
	bool			FullListHasItem(const EListItem *item) const;
	eint32			FullListCountItems() const;
	eint32			FullListCurrentSelection(eint32 index = 0) const;
	virtual void		MakeEmpty();
	bool			FullListIsEmpty() const;

	void			FullListDoForEach(bool (*func)(EListItem *item));
	void			FullListDoForEach(bool (*func)(EListItem *item, void *user_data), void *user_data);

	EListItem		*Superitem(const EListItem *item) const;
	bool			HasSubitems(const EListItem *item) const;

	eint32			CountItemsUnder(EListItem *item, bool oneLevelOnly) const;
	EListItem		*EachItemUnder(EListItem *item, bool oneLevelOnly,
					       EListItem *(*eachFunc)(EListItem *item, void *user_data), void *user_data);
	EListItem		*ItemUnderAt(EListItem *item, bool oneLevelOnly, eint32 index) const;

	void			Expand(EListItem *item);
	void			Collapse(EListItem *item);
	bool			IsExpanded(eint32 fullListIndex) const;

	// FullListItems(): return the list, use it carefully please
	const EListItem		**FullListItems() const;

	virtual void		MouseDown(EPoint where);
	virtual void		MouseUp(EPoint where);
	virtual void		MouseMoved(EPoint where, euint32 code, const EMessage *a_message);
	virtual void		KeyDown(const char *bytes, eint32 numBytes);
	virtual void		KeyUp(const char *bytes, eint32 numBytes);

private:
	EList fFullItems;
};

#endif /* __cplusplus */

#endif /* __ETK_OUTLINE_LIST_VIEW_H__ */

