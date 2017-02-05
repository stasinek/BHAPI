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
 * File: StringArray.cpp
 *
 * --------------------------------------------------------------------------*/

#include "StringArray.h"


typedef struct __string_node__ {
	EString *str;
	void *data;

	__string_node__()
	{
		str = new EString();
		data = NULL;
	}

	~__string_node__()
	{
		if(str) delete str;
	}
} __string_node__;


EStringArray::EStringArray()
{
}


EStringArray::EStringArray(const char *string, void *attach_data)
{
	AddItem(string, attach_data);
}


EStringArray::EStringArray(const EString &string, void *attach_data)
{
	AddItem(string, attach_data);
}


EStringArray::EStringArray(const char **array)
{
	operator=(array);
}


EStringArray::EStringArray(const EStringArray &array)
{
	operator=(array);
}


EStringArray::~EStringArray()
{
	MakeEmpty();
}


void
EStringArray::MakeEmpty()
{
	if(!list.IsEmpty())
	{
		for(eint32 i = 0; i < list.CountItems(); i++) delete (__string_node__*)list.ItemAt(i);
		list.MakeEmpty();
	}
}


bool
EStringArray::IsEmpty() const
{
	return list.IsEmpty();
}


eint32
EStringArray::CountItems() const
{
	return list.CountItems();
}


EStringArray&
EStringArray::operator=(const EStringArray &array)
{
	MakeEmpty();
	AddArray(array);
	return *this;
}


EStringArray&
EStringArray::operator=(const char **array)
{
	MakeEmpty();

	if(array)
	{
		while(*array)
		{
			AddItem(*array);
			array++;
		}
	}

	return *this;
}


EStringArray&
EStringArray::operator+=(const char *string)
{
	AddItem(string);
	return *this;
}


EStringArray&
EStringArray::operator+=(const EString &string)
{
	AddItem(string);
	return *this;
}


EStringArray&
EStringArray::operator+=(const EStringArray &array)
{
	AddArray(array);
	return *this;
}


EStringArray&
EStringArray::operator+=(const char **array)
{
	if(array)
	{
		while(*array)
		{
			AddItem(*array);
			array++;
		}
	}

	return *this;
}


bool
EStringArray::AddItem(const char *item, void *attach_data)
{
	__string_node__ *data = new __string_node__;
	if(!data || !data->str) {if(data) delete data; return false;}

	data->str->SetTo(item);
	data->data = attach_data;

	if(!list.AddItem((void*)data))
	{
		delete data;
		return false;
	}

	return true;
}


bool
EStringArray::AddItem(const char *item, eint32 atIndex, void *attach_data)
{
	__string_node__ *data = new __string_node__;
	if(!data || !data->str) {if(data) delete data; return false;}

	data->str->SetTo(item);
	data->data = attach_data;

	if(!list.AddItem((void*)data, atIndex))
	{
		delete data;
		return false;
	}

	return true;
}


bool
EStringArray::AddItem(const EString &item, void *attach_data)
{
	return AddItem(item.String(), attach_data);
}


bool
EStringArray::AddItem(const EString &item, eint32 atIndex, void *attach_data)
{
	return AddItem(item.String(), atIndex, attach_data);
}


bool
EStringArray::AddArray(const EStringArray &array)
{
	if(array.list.IsEmpty()) return false;

	EStringArray _array;
	for(eint32 i = 0; i < array.list.CountItems(); i++)
	{
		const __string_node__ *node = (const __string_node__*)array.list.ItemAt(i);
		if(!node || !node->str) continue;
		if(_array.AddItem(node->str->String(), node->data) == false) return false;
	}

	if(list.AddList(&_array.list))
	{
		_array.list.MakeEmpty();
		return true;
	}

	return false;
}


bool
EStringArray::AddArray(const EStringArray &array, eint32 atIndex)
{
	if(array.list.IsEmpty()) return false;
	if(atIndex < 0 || atIndex >= list.CountItems()) return false;

	EStringArray _array;
	for(eint32 i = 0; i < array.list.CountItems(); i++)
	{
		const __string_node__ *node = (const __string_node__*)array.list.ItemAt(i);
		if(!node || !node->str) continue;
		if(_array.AddItem(node->str->String(), node->data) == false) return false;
	}

	if(list.AddList(&_array.list, atIndex))
	{
		_array.list.MakeEmpty();
		return true;
	}

	return false;
}


const EString*
EStringArray::ItemAt(eint32 index, void **attach_data) const
{
	const __string_node__ *node = (const __string_node__*)list.ItemAt(index);
	if(!node) return NULL;
	if(attach_data) *attach_data = node->data;
	return node->str;
}


const EString*
EStringArray::FirstItem(void **attach_data) const
{
	const __string_node__ *node = (const __string_node__*)list.FirstItem();
	if(!node) return NULL;
	if(attach_data) *attach_data = node->data;
	return node->str;
}


const EString*
EStringArray::LastItem(void **attach_data) const
{
	const __string_node__ *node = (const __string_node__*)list.LastItem();
	if(!node) return NULL;
	if(attach_data) *attach_data = node->data;
	return node->str;
}


bool
EStringArray::RemoveItem(eint32 index)
{
	__string_node__ *node = (__string_node__*)list.RemoveItem(index);

	if(node)
	{
		delete node;
		return true;
	}

	return false;
}


bool
EStringArray::RemoveItems(eint32 index, eint32 count)
{
	if(index < 0 || index >= list.CountItems()) return false;

	if(count < 0) count = list.CountItems() - index;
	else count = min_c(list.CountItems() - index, count);

	if(count == 0) return true;

	EList list_store(list);
	if(list_store.RemoveItems(0, index) == false) return false;
	if(list_store.CountItems() < count) return false;
	if(list_store.CountItems() > count)
		if(list_store.RemoveItems(count, -1) == false) return false;

	if(list.RemoveItems(index, count))
	{
		for(eint32 i = 0; i < list_store.CountItems(); i++) delete (__string_node__*)list_store.ItemAt(i);
		list_store.MakeEmpty();
		return true;
	}

	return false;
}


bool
EStringArray::ReplaceItem(eint32 index, const char *string, void *attach_data)
{
	__string_node__ *node = (__string_node__*)list.ItemAt(index);

	if(node && node->str)
	{
		node->str->SetTo(string);
		node->data = attach_data;
		return true;
	}

	return false;
}


bool
EStringArray::ReplaceItem(eint32 index, const EString &string, void *attach_data)
{
	__string_node__ *node = (__string_node__*)list.ItemAt(index);

	if(node && node->str)
	{
		node->str->SetTo(string);
		node->data = attach_data;
		return true;
	}

	return false;
}


EStringArray&
EStringArray::SortItems(int (*cmp)(const EString**, const EString**))
{
	list.SortItems((int (*)(const void*, const void*))cmp);

	return *this;
}


bool
EStringArray::SwapItems(eint32 indexA, eint32 indexB)
{
	if(indexA != indexB) return list.SwapItems(indexA, indexB);

	return true;
}


bool
EStringArray::MoveItem(eint32 fromIndex, eint32 toIndex)
{
	if(fromIndex != toIndex) return list.MoveItem(fromIndex, toIndex);

	return true;
}


eint32
EStringArray::FindString(const char *string, eint32 startIndex, bool all_equal, bool invert) const
{
	if(startIndex < 0 || startIndex >= list.CountItems()) return -1;

	eint32 i = startIndex;

	while(i >= 0 && i < list.CountItems())
	{
		const EString *str = ItemAt(i);

		if(str)
		{
			if(string == NULL || *string == 0)
			{
				if(str->Length() <= 0) return i;
			}
			else if(!all_equal)
			{
				if(str->FindFirst(string) >= 0) return i;
			}
			else
			{
				if(*str == string) return i;
			}
		}
		else if(string == NULL || *string == 0)
		{
			return i;
		}

		i += (invert ? -1 : 1);
	}

	return -1;
}


eint32
EStringArray::FindString(const EString &string, eint32 startIndex, bool all_equal, bool invert) const
{
	return FindString(string.String(), startIndex, all_equal, invert);
}


eint32
EStringArray::IFindString(const char *string, eint32 startIndex, bool all_equal, bool invert) const
{
	if(startIndex < 0 || startIndex >= list.CountItems()) return -1;

	eint32 i = startIndex;

	while(i >= 0 && i < list.CountItems())
	{
		const EString *str = ItemAt(i);

		if(str)
		{
			if(string == NULL || *string == 0)
			{
				if(str->Length() == 0) return i;
			}
			else if(!all_equal)
			{
				if(str->IFindFirst(string) >= 0) return i;
			}
			else
			{
				if(str->ICompare(string) == 0) return i;
			}
		}
		else if(string == NULL || *string == 0)
		{
			return i;
		}

		i += (invert ? -1 : 1);
	}

	return -1;
}


eint32
EStringArray::IFindString(const EString &string, eint32 startIndex, bool all_equal, bool invert) const
{
	return IFindString(string.String(), startIndex, all_equal, invert);
}

