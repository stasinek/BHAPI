/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku source-code, All Rights Reserved
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
 * File: StringList.h
 *
 * --------------------------------------------------------------------------*/
/*
 * Copyright 2011-2013, Ingo Weinhold, ingo_weinhold@gmx.de
 * Copyright 2011, Clemens Zeidler <haiku@clemens-zeidler.de>
 *
 * Distributed under the terms of the MIT License.
 */
//-----------------------------------------------------------------------------
#include <kits/support/StringList.h>
using namespace bhapi;
//-------------------------------------------------------------------------------------------------
#include <kits/debug/Debug.h>
#include <kits/support/List.h>
#include <kits/support/String.h>
#include <kits/support/private/StringPrivate.h>
//-----------------------------------------------------------------------------

static int compare_private_data(const void *a, const void *b)
{
    return BString::Private::StringFromData(*(char **)a).Compare(BString::Private::StringFromData(*(char **)b));
}
//-----------------------------------------------------------------------------

static int compare_private_data_ignore_case(const void *a, const void *b)
{
    return BString::Private::StringFromData(*(char **)a).ICompare(BString::Private::StringFromData(*(char **)b));
}
//-------------------------------------------------------------------------------------------------
// #pragma mark - BStringList
BStringList::BStringList(int32_t count)
    :
    fStrings(count)
{
}
//-----------------------------------------------------------------------------

BStringList::BStringList(const BStringList &other)
    :
    fStrings(other.fStrings)
{
    __IncrementRefCounts();
}

//-----------------------------------------------------------------------------

BStringList::~BStringList(void)
{
    __DecrementRefCounts();
}

//-----------------------------------------------------------------------------

bool BStringList::Add(const BString &_string, int32_t index)
{
    BString string(_string);
    char    *privateData = BString::Private(string).Data();

    // makes sure the string is shareable
    if (string.Length() != _string.Length()) return false;
    if (!fStrings.AddItem(privateData, index)) return false;

    BString::Private::IncrementDataRefCount(privateData);
    return true;
}
//-----------------------------------------------------------------------------

bool BStringList::Add(const BString &_string)
{
    BString string(_string);
    char    *privateData = BString::Private(string).Data();

    // makes sure the string is shareable
    if (string.Length() != _string.Length()) return false;
    if (!fStrings.AddItem(privateData)) return false;

    BString::Private::IncrementDataRefCount(privateData);
    return true;
}
//-----------------------------------------------------------------------------

bool BStringList::Add(const BStringList &list, int32_t index)
{
    if (!fStrings.AddList(&list.fStrings, index)) return false;

    list.__IncrementRefCounts();
    return true;
}
//-----------------------------------------------------------------------------

bool BStringList::Add(const BStringList &list)
{
    if (!fStrings.AddList(&list.fStrings)) return false;

    list.__IncrementRefCounts();
    return true;
}

//-----------------------------------------------------------------------------

bool BStringList::Remove(const BString &string, bool ignoreCase)
{
    bool    result = false;
    int32_t   count = fStrings.CountItems();

    if (ignoreCase)
    {
        int32_t   length = string.Length();

        for (int32_t i = count - 1; i >= 0; i--)
        {
            BString element(StringAt(i));
            if (length == element.Length() && string.ICompare(element) == 0)
            {
                Remove(i);
                result = true;
            }
        }
    }
    else
    {
        for (int32_t i = count - 1; i >= 0; i--)
        {
            if (string == StringAt(i))
            {
                Remove(i);
                result = true;
            }
        }
    }

    return result;
}

//-----------------------------------------------------------------------------

bool BStringList::Remove(const BStringList &list, bool ignoreCase)
{
    bool    removedAnything = false;
    int32_t   stringCount = list.CountStrings();
    for (int32_t i = 0; i < stringCount; i++) removedAnything |= Remove(list.StringAt(i), ignoreCase);

    return removedAnything;
}

//-----------------------------------------------------------------------------

BString BStringList::Remove(int32_t index)
{
    char    *privateData = (char *)fStrings.RemoveItem(index);
    BString string(BString::Private::StringFromData(privateData));
    if (index < 0 || index >= fStrings.CountItems()) return BString();

    BString::Private::DecrementDataRefCount(privateData);
    return string;
}

//-----------------------------------------------------------------------------

bool BStringList::Remove(int32_t index, int32_t count)
{
    int32_t   stringCount = fStrings.CountItems();
    int32_t   end = index + min_c(stringCount - index, count);
    if (index < 0 || index > stringCount) return false;
    for (int32_t i = index; i < end; i++) BString::Private::DecrementDataRefCount((char *)fStrings.ItemAt(i));

    fStrings.RemoveItems(index, end - index);
    return true;
}

//-----------------------------------------------------------------------------

bool BStringList::Replace(int32_t index, const BString &string)
{
    char    *privateData = BString::Private(string).Data();
    if (index < 0 || index >= fStrings.CountItems()) return false;

    BString::Private::DecrementDataRefCount((char *)fStrings.ItemAt(index));

    BString::Private::IncrementDataRefCount(privateData);
    fStrings.ReplaceItem(index, privateData);

    return true;
}

//-----------------------------------------------------------------------------

void BStringList::MakeEmpty(void)
{
    __DecrementRefCounts();
    fStrings.MakeEmpty();
}

//-----------------------------------------------------------------------------

void BStringList::Sort(bool ignoreCase)
{
    fStrings.SortItems(ignoreCase ? compare_private_data_ignore_case : compare_private_data);
}

//-----------------------------------------------------------------------------

bool BStringList::Swap(int32_t indexA, int32_t indexB)
{
    return fStrings.SwapItems(indexA, indexB);
}

//-----------------------------------------------------------------------------

bool BStringList::Move(int32_t fromIndex, int32_t toIndex)
{
    return fStrings.MoveItem(fromIndex, toIndex);
}

//-----------------------------------------------------------------------------

BString BStringList::StringAt(int32_t index) const
{
    return BString::Private::StringFromData((char *)fStrings.ItemAt(index));
}

//-----------------------------------------------------------------------------

BString BStringList::First(void) const
{
    return BString::Private::StringFromData((char *)fStrings.FirstItem());
}

//-----------------------------------------------------------------------------

BString BStringList::Last(void) const
{
    return BString::Private::StringFromData((char *)fStrings.LastItem());
}

//-----------------------------------------------------------------------------

int32_t BStringList::IndexOf(const BString &string, bool ignoreCase) const
{
    int32_t   count = fStrings.CountItems();

    if (ignoreCase)
    {
        int32_t   length = string.Length();

        for (int32_t i = 0; i < count; i++)
        {
            BString element(StringAt(i));
            if (length == element.Length() && string.ICompare(element) == 0) return i;
        }
    }
    else
    {
        for (int32_t i = 0; i < count; i++)
        {
            if (string == StringAt(i)) return i;
        }
    }

    return -1;
}

//-----------------------------------------------------------------------------

int32_t BStringList::CountStrings(void) const
{
    return fStrings.CountItems();
}

//-----------------------------------------------------------------------------

bool BStringList::IsEmpty(void) const
{
    return fStrings.IsEmpty();
}

//-----------------------------------------------------------------------------

BString BStringList::Join(const char *separator, int32_t length) const
{
    return __Join(separator, length >= 0 ? strnlen(separator, length) : strlen(separator));
}

//-----------------------------------------------------------------------------

void BStringList::DoForEach(bool (*func) (const BString &string))
{
    int32_t   count = fStrings.CountItems();
    for (int32_t i = 0; i < count; i++) func(StringAt(i));
}

//-----------------------------------------------------------------------------

void BStringList::DoForEach(bool (*func) (const BString &string, void *arg2), void *arg2)
{
    int32_t   count = fStrings.CountItems();
    for (int32_t i = 0; i < count; i++) func(StringAt(i), arg2);
}
//-----------------------------------------------------------------------------

BStringList &BStringList::operator=(const BStringList &other)
{
    if (this != &other)
    {
        __DecrementRefCounts();
        fStrings = other.fStrings;
        __IncrementRefCounts();
    }

    return *this;
}
//-----------------------------------------------------------------------------

bool BStringList::operator==(const BStringList &other) const
{
    int32_t   count = fStrings.CountItems();
    if (this == &other) return true;
    if (count != other.fStrings.CountItems()) return false;

    for (int32_t i = 0; i < count; i++)
    {
        if (StringAt(i) != other.StringAt(i)) return false;
    }

    return true;
}
//-----------------------------------------------------------------------------

bool BStringList::IsFixedSize(void) const
{
    return false;
}
//-----------------------------------------------------------------------------

type_code BStringList::TypeCode(void) const
{
    return B_STRING_LIST_TYPE;
}
//-----------------------------------------------------------------------------

bool BStringList::AllowsTypeCode(type_code code) const
{
    return code == B_STRING_LIST_TYPE;
}
//-----------------------------------------------------------------------------

ssize_t BStringList::FlattenedSize(void) const
{
    ssize_t size = 0;
    int32_t   count = CountStrings();
    for (int32_t i = 0; i < count; i++) size += StringAt(i).Length() + 1;

    return size;
}
//-----------------------------------------------------------------------------

status_t BStringList::Flatten(void *buf, ssize_t size) const
{
    const char  *buffer = (const char *)buf;

    int32_t       count = CountStrings();
    if (size < FlattenedSize()) return B_NO_MEMORY;

    for (int32_t i = 0; i < count; i++)
    {
        BString item = StringAt(i);
        ssize_t storeSize = item.Length() + 1;
        memcpy((void *)buffer, (const void *)item.String(), storeSize);
        buffer += storeSize;
    }

    return B_OK;
}
//-----------------------------------------------------------------------------

status_t BStringList::Unflatten(type_code code, const void *buffer, ssize_t size)
{
    off_t   offset = 0;
    const char *bufferStart = (const char *)buffer;

    if (code != B_STRING_LIST_TYPE) return B_ERROR;
    MakeEmpty();

    while (offset < size)
    {
        const char  *string = bufferStart + offset;
        size_t      restSize = size - offset;
        size_t      read = strnlen(string, restSize);
        if (read == restSize) return B_BAD_VALUE;
        if (!Add(string)) return B_NO_MEMORY;
        offset += read + 1;
    }

    return B_OK;
}
//-----------------------------------------------------------------------------

void BStringList::__IncrementRefCounts(void) const
{
    int32_t   count = fStrings.CountItems();
    for (int32_t i = 0; i < count; i++)
    {
        BString::Private::IncrementDataRefCount((char *)fStrings.ItemAt(i));
    }
}
//-----------------------------------------------------------------------------

void BStringList::__DecrementRefCounts(void) const
{
    int32_t   count = fStrings.CountItems();
    for (int32_t i = 0; i < count; i++) BString::Private::DecrementDataRefCount((char *)fStrings.ItemAt(i));
}
//-----------------------------------------------------------------------------

BString BStringList::__Join(const char *separator, int32_t length) const
{

    // handle simple cases (0 or 1 element)
    int32_t   count = CountStrings();
    int32_t   totalLength = length * (count - 1);
    BString result;
    char    *buffer = result.LockBuffer(totalLength);
    if (count == 0) return BString();
    if (count == 1) return StringAt(0);

    // determine the total length
    for (int32_t i = 0; i < count; i++) totalLength += StringAt(i).Length();

    // compose the result string
    if (buffer == NULL) return result;

    for (int32_t i = 0; i < count; i++)
    {
        BString string = StringAt(i);
        if (i > 0 && length > 0)
        {
            memcpy(buffer, separator, length);
            buffer += length;
        }

        memcpy(buffer, string.String(), string.Length());
        buffer += string.Length();
    }

    return result.UnlockBuffer(totalLength);
}
//-----------------------------------------------------------------------------
