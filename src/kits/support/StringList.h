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
 * Copyright 2011 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
//-------------------------------------------------------------------------------------------------
#ifndef BHAPI_STRING_LIST_H
#define BHAPI_STRING_LIST_H
//-------------------------------------------------------------------------------------------------
#ifdef __cplusplus
#ifndef BSTRING_DEF
#define BSTRING_DEF
class BString;
#endif
#include <kits/support/List.h>
#include <kits/support/Flattenable.h>
#include <Haiku.h>
//-------------------------------------------------------------------------------------------------
class BHAPI_IMPEXP BStringList : public BFlattenable {
public:
                BStringList(int32 count = 20);
                BStringList(const BStringList& other);
    virtual	   ~BStringList();
    // Adding and removing items.
    bool				Add(const BString& string, int32 index);
    bool				Add(const BString& string);
    bool				Add(const BStringList& list, int32 index);
    bool				Add(const BStringList& list);

    bool				Remove(const BString& string, bool ignoreCase = false);
    bool				Remove(const BStringList& list, bool ignoreCase = false);
    BString				Remove(int32 index);
    bool				Remove(int32 index, int32 count);
    bool				Replace(int32 index, const BString& string);

    void				MakeEmpty();
    // Reorder items
    void				Sort(bool ignoreCase = false);
            // TODO: Sort() with custom sort function.
    bool				Swap(int32 indexA, int32 indexB);
    bool				Move(int32 fromIndex, int32 toIndex);
    // Retrieve items
    BString				StringAt(int32 index) const;
    BString				First() const;
    BString				Last() const;
    // Query
    bool				HasString(const BString& string, bool ignoreCase = false) const;
    int32				IndexOf(const BString& string, bool ignoreCase = false) const;
    int32				CountStrings() const;
    bool				IsEmpty() const;

    BString				Join(const char* separator, int32 length = -1) const;
    // Iteration
    void				DoForEach(bool (*func)(const BString& string));
    void				DoForEach(bool (*func)(const BString& string, void* arg2), void* arg2);

    BStringList&		operator=(const BStringList& other);
    bool				operator==(const BStringList& other) const;
    bool				operator!=(const BStringList& other) const;
    // BFlattenable
    virtual	bool				IsFixedSize() const;
    virtual	type_code			TypeCode() const;
    virtual	bool				AllowsTypeCode(type_code code) const;
    virtual	ssize_t				FlattenedSize() const;
    virtual	status_t			Flatten(void* buffer, ssize_t size) const;
    virtual	status_t			Unflatten(type_code code, const void* buffer, ssize_t size);
private:
    void				__IncrementRefCounts() const;
    void				__DecrementRefCounts() const;

    BString				__Join(const char* separator, int32 length) const;
private:
    BList				fStrings;
};
//-------------------------------------------------------------------------------------------------

inline bool BStringList::HasString(const BString& string, bool ignoreCase) const
{
    return IndexOf(string, ignoreCase) >= 0;
}
//-------------------------------------------------------------------------------------------------

inline bool BStringList::operator!=(const BStringList& other) const
{
    return !(*this == other);
}
//-------------------------------------------------------------------------------------------------
#define BSTRINGLIST_DEF
#endif  // C++
//-------------------------------------------------------------------------------------------------
#endif  // BHAPI_STRING_LIST_H
//-------------------------------------------------------------------------------------------------
