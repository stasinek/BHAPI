/* --------------------------------------------------------------------------
 *
 * BHAPI++ previously named ETK++, The Easy Toolkit for C++ programing
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
 * File: StringArray.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_STRING_ARRAY__H
#define BHAPI_STRING_ARRAY__H
#include "./SupportDefs.h"
#include "./List.h"
#ifdef __cplusplus /* Just for C++ */
class BString;
class IMPEXP_BHAPI BStringArray {
public:
    BStringArray();
    BStringArray(const char *string, void *attach_data = NULL); // string: first item
    BStringArray(const BString &string, void *attach_data = NULL);
    BStringArray(const char **array); // array: NULL-terminated array of strings
    BStringArray(const BStringArray &array);

   ~BStringArray();

    BStringArray&	operator=(const char **array);
    BStringArray&	operator=(const BStringArray &array);

    BStringArray&	operator+=(const char *string);
    BStringArray&	operator+=(const BString &string);
    BStringArray&	operator+=(const char **array);
    BStringArray&	operator+=(const BStringArray &array);

    bool            AddItem(const char *item, void *attach_data = NULL);
    bool            AddItem(const char *item, b_int32 atIndex, void *attach_data = NULL);
    bool            AddItem(const BString &item, void *attach_data = NULL);
    bool            AddItem(const BString &item, b_int32 atIndex, void *attach_data = NULL);
    bool            AddArray(const BStringArray &array);
    bool            AddArray(const BStringArray &array, b_int32 atIndex);

    bool            RemoveItem(b_int32 index);
    bool            RemoveItems(b_int32 index, b_int32 count);

    bool            ReplaceItem(b_int32 index, const char *string, void *attach_data = NULL);
    bool            ReplaceItem(b_int32 index, const BString &string, void *attach_data = NULL);

    BStringArray&	SortItems(int (*cmp)(const BString**, const BString**));
    bool            SwapItems(b_int32 indexA, b_int32 indexB);
    bool            MoveItem(b_int32 fromIndex, b_int32 toIndex);

    bool            IsEmpty() const;
    void            MakeEmpty();

    const BString*	ItemAt(b_int32 index, void **attach_data = NULL) const;
    const BString*	FirstItem(void **attach_data = NULL) const;
    const BString*	LastItem(void **attach_data = NULL) const;

    b_int32         CountItems() const;

	// return value: string index if found, else return -1
    b_int32         FindString(const char *string, b_int32 startIndex = 0, bool all_equal = true, bool invert = false) const;
    b_int32         FindString(const BString &string, b_int32 startIndex = 0, bool all_equal = true, bool invert = false) const;
    b_int32         IFindString(const char *string, b_int32 startIndex = 0, bool all_equal = true, bool invert = false) const;
    b_int32         IFindString(const BString &string, b_int32 startIndex = 0, bool all_equal = true, bool invert = false) const;

private:
    BList list;
};
#endif /* __cplusplus */
#endif /* BHAPI_STRING_ARRAY__H */

