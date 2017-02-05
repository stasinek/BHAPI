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
 * File: List.h
 * Description: EList --- ordered list of data pointers
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_LIST_H__
#define __ETK_LIST_H__

#include "./../support/SupportDefs.h"

#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_ETK EList {
public:
	// EList(eint32), EList(eint32, eint32):
	// 	The argument "initialAllocSize" is the minimum count to hold in memory.
	// 		Valid range: 1 ~ (E_MAXINT32 - 1)
	// 		When you pass invalid value to "initialAllocSize", the minimum count just equal to 0.
	// 	The argument "nullItems" is the count to preallocate NULL items for ReplaceItem().
	EList(eint32 initialAllocSize = 0);
	EList(eint32 initialAllocSize, eint32 nullItems);

	// EList(const EList&),operator=(const EList&):
	// 	The minimum count to hold same as "list" when possible.
	EList(const EList &list);
	EList &operator=(const EList &from);

	virtual ~EList();

	bool	AddItem(void *item);
	bool	AddItem(void *item, eint32 atIndex);
	bool	AddList(const EList *newItems);
	bool	AddList(const EList *newItems, eint32 atIndex);

	// RemoveItem(),RemoveItems(): the item WOULD NOT be destructed yet.
	bool	RemoveItem(void *item);
	void	*RemoveItem(eint32 index);
	bool	RemoveItems(eint32 index, eint32 count);

	// ReplaceItem(): the old item WOULD NOT be destructed yet.
	bool	ReplaceItem(eint32 index, void *newItem, void **oldItem = NULL);

	void	MakeEmpty();

	bool	SwapItems(eint32 indexA, eint32 indexB);
	bool	MoveItem(eint32 fromIndex, eint32 toIndex);

	void	SortItems(int (*cmp)(const void *a, const void *b));

	void	*ItemAt(eint32 index) const;
	void	*FirstItem() const;
	void	*LastItem() const;

	bool	HasItem(void *item) const;
	eint32	IndexOf(void *item) const;
	eint32	CountItems() const;
	bool	IsEmpty() const;

	void	DoForEach(bool (*func)(void *data));
	void	DoForEach(bool (*func)(void *data, void *user_data), void *user_data);

	// Items(): return the list, use it carefully please
	void	**Items() const;

private:
	void **fObjects;

	eint32 fItemCount;
	eint32 fItemReal;
	eint32 fMinimumCount;

	bool _Resize(eint32 count);
};

#endif /* __cplusplus */

#endif /* __ETK_LIST_H__ */

