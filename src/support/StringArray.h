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
 * File: StringArray.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_STRING_ARRAY_H__
#define __ETK_STRING_ARRAY_H__

#include "List.h"
#include "String.h"

#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_ETK EStringArray {
public:
	EStringArray();

	EStringArray(const char *string, void *attach_data = NULL); // string: first item
	EStringArray(const EString &string, void *attach_data = NULL);

	EStringArray(const char **array); // array: NULL-terminated array of strings
	EStringArray(const EStringArray &array);

	~EStringArray();

	EStringArray&	operator=(const char **array);
	EStringArray&	operator=(const EStringArray &array);

	EStringArray&	operator+=(const char *string);
	EStringArray&	operator+=(const EString &string);
	EStringArray&	operator+=(const char **array);
	EStringArray&	operator+=(const EStringArray &array);

	bool		AddItem(const char *item, void *attach_data = NULL);
	bool		AddItem(const char *item, eint32 atIndex, void *attach_data = NULL);
	bool		AddItem(const EString &item, void *attach_data = NULL);
	bool		AddItem(const EString &item, eint32 atIndex, void *attach_data = NULL);
	bool		AddArray(const EStringArray &array);
	bool		AddArray(const EStringArray &array, eint32 atIndex);

	bool		RemoveItem(eint32 index);
	bool		RemoveItems(eint32 index, eint32 count);

	bool		ReplaceItem(eint32 index, const char *string, void *attach_data = NULL);
	bool		ReplaceItem(eint32 index, const EString &string, void *attach_data = NULL);

	EStringArray&	SortItems(int (*cmp)(const EString**, const EString**));
	bool		SwapItems(eint32 indexA, eint32 indexB);
	bool		MoveItem(eint32 fromIndex, eint32 toIndex);

	bool		IsEmpty() const;
	void		MakeEmpty();

	const EString*	ItemAt(eint32 index, void **attach_data = NULL) const;
	const EString*	FirstItem(void **attach_data = NULL) const;
	const EString*	LastItem(void **attach_data = NULL) const;

	eint32		CountItems() const;

	// return value: string index if found, else return -1
	eint32		FindString(const char *string, eint32 startIndex = 0, bool all_equal = true, bool invert = false) const;
	eint32		FindString(const EString &string, eint32 startIndex = 0, bool all_equal = true, bool invert = false) const;
	eint32		IFindString(const char *string, eint32 startIndex = 0, bool all_equal = true, bool invert = false) const;
	eint32		IFindString(const EString &string, eint32 startIndex = 0, bool all_equal = true, bool invert = false) const;

private:
	EList list;
};

#endif /* __cplusplus */

#endif /* __ETK_STRING_ARRAY_H__ */

