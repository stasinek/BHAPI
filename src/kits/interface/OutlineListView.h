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
 * File: OutlineListView.h
 * Description: BOutlineListView --- Displays a list of items that can be structured like an outline
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_OUTLINE_LIST_VIEW_H
#define BHAPI_OUTLINE_LIST_VIEW_H

#ifdef __cplusplus /* Just for C++ */
#include "../interface/ListView.h"
class IMPEXPBHAPI BOutlineListView : public BListView {
public:
    BOutlineListView(BRect frame,
             const char *name,
             bhapi::list_view_type type = B_SINGLE_SELECTION_LIST,
              __be_uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
              __be_uint32 flags = B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS);
    virtual ~BOutlineListView();

    virtual bool		AddUnder(BListItem *item, BListItem *superitem);

    virtual bool		AddItem(BListItem *item);
    virtual bool		AddItem(BListItem *item,  __be_int32 fullListIndex);
    virtual bool		RemoveItem(BListItem *item, bool auto_destruct_item_and_subitems = true);
    virtual bool		RemoveItems(__be_int32 fullListIndex,  __be_int32 count, bool auto_destruct_items = true);

    virtual BListItem	*RemoveItem(__be_int32 fullListIndex, bool auto_destruct_subitems,  __be_int32 *count);
    virtual BListItem	*RemoveItem(__be_int32 fullListIndex); // same as RemoveItem(fullListIndex, true, NULL)

    BListItem		*FullListItemAt(__be_int32 fullListIndex) const;
     __be_int32			FullListIndexOf(const BListItem *item) const;
    BListItem		*FullListFirstItem() const;
    BListItem		*FullListLastItem() const;
    bool			FullListHasItem(const BListItem *item) const;
     __be_int32			FullListCountItems() const;
     __be_int32			FullListCurrentSelection(__be_int32 index = 0) const;
    virtual void		MakeEmpty();
    bool			FullListIsEmpty() const;

    void			FullListDoForEach(bool (*func)(BListItem *item));
    void			FullListDoForEach(bool (*func)(BListItem *item, void *user_data), void *user_data);

    BListItem		*Superitem(const BListItem *item) const;
    bool			HasSubitems(const BListItem *item) const;

     __be_int32			CountItemsUnder(BListItem *item, bool oneLevelOnly) const;
    BListItem		*EachItemUnder(BListItem *item, bool oneLevelOnly,
                           BListItem *(*eachFunc)(BListItem *item, void *user_data), void *user_data);
    BListItem		*ItemUnderAt(BListItem *item, bool oneLevelOnly,  __be_int32 index) const;

    void			Expand(BListItem *item);
    void			Collapse(BListItem *item);
    bool			IsExpanded(__be_int32 fullListIndex) const;

    // FullListItems(): return the list, use it carefully please
    const BListItem		**FullListItems() const;

    virtual void		MouseDown(BPoint where);
    virtual void		MouseUp(BPoint where);
    virtual void		MouseMoved(BPoint where,  __be_uint32 code, const BMessage *a_message);
    virtual void		KeyDown(const char *bytes,  __be_int32 numBytes);
    virtual void		KeyUp(const char *bytes,  __be_int32 numBytes);

private:
    BList fFullItems;
};

#endif /* __cplusplus */

#endif /* BHAPI_OUTLINE_LIST_VIEW_H */

