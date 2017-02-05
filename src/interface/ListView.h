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
 * File: ListView.h
 * Description: EListView --- Displays a list of items the user can select and invoke
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_LIST_VIEW_H__
#define __ETK_LIST_VIEW_H__

#include "./../support/List.h>
#include "./../app/Invoker.h>
#include "./../interface/ListItem.h>

typedef enum e_list_view_type {
	E_SINGLE_SELECTION_LIST,
	E_MULTIPLE_SELECTION_LIST,
} e_list_view_type;


#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_ETK EListView : public EView, public EInvoker {
public:
	EListView(ERect frame,
		  const char *name,
		  e_list_view_type type = E_SINGLE_SELECTION_LIST,
		  euint32 resizingMode = E_FOLLOW_LEFT | E_FOLLOW_TOP,
		  euint32 flags = E_WILL_DRAW | E_NAVIGABLE | E_FRAME_EVENTS);
	virtual ~EListView();

	virtual bool		AddItem(EListItem *item);
	virtual bool		AddItem(EListItem *item, eint32 atIndex);
	virtual bool		RemoveItem(EListItem *item, bool auto_destruct_item = true);
	virtual EListItem	*RemoveItem(eint32 index);
	virtual bool		RemoveItems(eint32 index, eint32 count, bool auto_destruct_items = true);

	virtual void		SetListType(e_list_view_type type);
	e_list_view_type	ListType() const;

	virtual void		SetSelectionMessage(EMessage *message);
	virtual void		SetInvocationMessage(EMessage *message);
	EMessage		*SelectionMessage() const;
	euint32			SelectionCommand() const;
	EMessage		*InvocationMessage() const;
	euint32			InvocationCommand() const;
	virtual e_status_t	Invoke(const EMessage *message = NULL);

	// Empty functions BEGIN --- just for derivative class
	virtual void		SelectionChanged();
	// Empty functions END

	void			Select(eint32 index, bool extend = false);
	void			Select(eint32 start, eint32 finish, bool extend = false);
	bool			IsItemSelected(eint32 index) const;
	eint32			CurrentSelection(eint32 index = 0) const;

	void			Deselect(eint32 index);
	void			DeselectAll();
	void			DeselectExcept(eint32 start, eint32 finish);

	ERect			ItemFrame(eint32 index) const;
	void			InvalidateItem(eint32 index);
	void			ScrollToSelection();

	EListItem		*ItemAt(eint32 index) const;
	EListItem		*FirstItem() const;
	EListItem		*LastItem() const;
	eint32			IndexOf(const EListItem *item) const;
	eint32			IndexOf(EPoint where, bool mustVisible = false) const;
	bool			HasItem(const EListItem *item) const;
	eint32			CountItems() const;
	virtual void		MakeEmpty();
	bool			IsEmpty() const;
	bool			SwapItems(eint32 indexA, eint32 indexB);
	bool			MoveItem(eint32 fromIndex, eint32 toIndex);

	// ReplaceItem(): when "oldItem" assigned to NULL, the old item will be destructed automatically.
	bool			ReplaceItem(eint32 index, EListItem *newItem, EListItem **oldItem = NULL);

	void			SortItems(int (*cmp)(const EListItem **a, const EListItem **b));
	void			DoForEach(bool (*func)(EListItem *item));
	void			DoForEach(bool (*func)(EListItem *item, void *user_data), void *user_data);

	// Items(): return the list, use it carefully please
	const EListItem		**Items() const;

	virtual void		Draw(ERect updateRect);
	virtual void		MouseDown(EPoint where);
	virtual void		MouseUp(EPoint where);
	virtual void		MouseMoved(EPoint where, euint32 code, const EMessage *a_message);
	virtual void		KeyDown(const char *bytes, eint32 numBytes);
	virtual void		KeyUp(const char *bytes, eint32 numBytes);
	virtual void		SetFont(const EFont *font, euint8 mask = E_FONT_ALL);
	virtual void		MakeFocus(bool focusState = true);
	virtual void		WindowActivated(bool state);
	virtual void		AttachedToWindow();
	virtual void		DetachedFromWindow();

protected:
	void			SetPosition(eint32 pos);
	eint32			Position() const;
	void			ScrollToItem(eint32 index);

private:
	EList fItems;
	e_list_view_type fListType;
	eint32 fFirstSelected;
	eint32 fLastSelected;
	eint32 fPos;

	EMessage *fSelectionMessage;
};

#endif /* __cplusplus */

#endif /* __ETK_LIST_VIEW_H__ */

