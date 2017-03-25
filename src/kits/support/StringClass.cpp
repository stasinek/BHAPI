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
 * File: Strings.cpp
 * Description: Strings --- string allocation and manipulation
 *
 * --------------------------------------------------------------------------*/
/*
 * Copyright 2001-2014 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Stefano Ceccherini, burton666@libero.it
 *		Axel Dörfler, axeld@pinc-software.de
 *		Marc Flerackers, mflerackers@androme.be
 *		Julun, host.haiku@gmx.de
 *		Michael Lotz, mmlr@mlotz.ch
 *		Oliver Tappe, openbeos@hirschkaefer.de
 */
//-------------------------------------------------------------------------------------------------
#include <StringClass.h>
using namespace bhapi;
//-------------------------------------------------------------------------------------------------
#include <StringList.h>
#include <StringPrivate.h>
#include <Debug.h>
#include <Errors.h>
#include <UTF8.h>
//-------------------------------------------------------------------------------------------------
#include <strings.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
//-------------------------------------------------------------------------------------------------
// define proper names for case-option of __DoReplace()
#define KEEP_CASE false
#define IGNORE_CASE true
// define proper names for count-option of __DoReplace()
#define REPLACE_ALL 0x7FFFFFFF
static const uint32 kPrivateDataOffset = BString::Private::kPrivateDataOffset;
BHAPI_EXPORT const char* B_EMPTY_STRING = "";
// helper function, returns minimum of two given values (but clamps to 0):
static inline int32 min_clamp0(int32 num1, int32 num2)
{
    if (num1 < num2) return num1 > 0 ? num1 : 0;
    return num2 > 0 ? num2 : 0;
}

//! Returns length of given string (but clamps to given maximum).
static inline int32 strlen_clamp(const char* string, int32 max)
{
    // this should yield 0 for max<0:
    return max <= 0 ? 0 : strnlen(string, max);
}

//! Helper function for strlen() that can handle NULL strings.
static inline size_t string_length(const char* string)
{
    return string != NULL ? strlen(string) : 0;
}

//! helper function, massages given pointer into a legal c-string:
static inline const char* safestr(const char* string)
{
    return string != NULL ? string : "";
}
//	#pragma mark - PosVect

class BHAPI_EXPORT BString::PosVect {
public:
    PosVect()
        :
        fSize(0),
        fBufferSize(20),
        fBuffer(NULL)
    {
    }

    ~PosVect()
    {
        free(fBuffer);
    }

    bool Add(int32 pos)
    {
        if (fBuffer == NULL || fSize == fBufferSize) {
            if (fBuffer != NULL) fBufferSize *= 2;
            int32* newBuffer = (int32*)realloc(fBuffer, fBufferSize * sizeof(int32));
            if (newBuffer == NULL)  return false;
            fBuffer = newBuffer;
        }
        fBuffer[fSize++] = pos;
        return true;
    }

    inline int32 ItemAt(int32 index) const
    {
        return fBuffer[index];
    }

    inline int32 CountItems() const
    {
        return fSize;
    }
private:
    int32	fSize;
    int32	fBufferSize;
    int32*	fBuffer;
};
//	#pragma mark - BStringRef
//-------------------------------------------------------------------------------------------------

BStringRef::BStringRef(BString& string, int32 position)
    :
    fString(string), fPosition(position)
{
}
//-------------------------------------------------------------------------------------------------

BStringRef::operator char() const
{
    return fPosition < fString.Length() ? fString.fPrivateData[fPosition] : 0;
}
//-------------------------------------------------------------------------------------------------

BStringRef& BStringRef::operator=(char c)
{
    fString.__MakeWritable();
    fString.fPrivateData[fPosition] = c;
    return *this;
}
//-------------------------------------------------------------------------------------------------

BStringRef& BStringRef::operator=(const BStringRef &rc)
{
    return operator=(rc.fString.fPrivateData[rc.fPosition]);
}
//-------------------------------------------------------------------------------------------------

const char* BStringRef::operator&() const
{
    return &fString.fPrivateData[fPosition];
}
//-------------------------------------------------------------------------------------------------

char* BStringRef::operator&()
{
    if (fString.__MakeWritable() != B_OK) return NULL;
    fString.__ReferenceCount() = -1;
        // mark as unsharable
    return &fString.fPrivateData[fPosition];
}
//	#pragma mark - BString
//-------------------------------------------------------------------------------------------------

inline int32& BString::__ReferenceCount()
{
    return Private::DataRefCount(fPrivateData);
}
//-------------------------------------------------------------------------------------------------

inline const int32& BString::__ReferenceCount() const
{
    return Private::DataRefCount(fPrivateData);
}
//-------------------------------------------------------------------------------------------------

inline bool BString::__IsShareable() const
{
    return fPrivateData != NULL && __ReferenceCount() >= 0;
}
//-------------------------------------------------------------------------------------------------

BString::BString()
    :
    fPrivateData(NULL)
{
    __Init("", 0);
}
//-------------------------------------------------------------------------------------------------

BString::BString(const char* string)
    :
    fPrivateData(NULL)
{
    __Init(string, strlen(safestr(string)));
}
//-------------------------------------------------------------------------------------------------

BString::BString(const BString& string)
    :
    fPrivateData(NULL)
{
    // check if source is sharable - if so, share else clone
    if (string.__IsShareable()) {
        fPrivateData = string.fPrivateData;
        bhapi::atomic_add(&__ReferenceCount(), 1);
            // string cannot go away right now
    } else
        __Init(string.String(), string.Length());
}
//-------------------------------------------------------------------------------------------------

BString::BString(const char* string, int32 maxLength)
    : fPrivateData(NULL)
{
    __Init(string, strlen_clamp(safestr(string), maxLength));
}
//-------------------------------------------------------------------------------------------------

BString::~BString()
{
    if (!__IsShareable() || bhapi::atomic_add(&__ReferenceCount(), -1) == 1)
        __FreePrivateData();
}
//-------------------------------------------------------------------------------------------------
//	#pragma mark - Access

int32 BString::CountChars() const
{
    return UTF8CountChars(fPrivateData, Length());
}
//-------------------------------------------------------------------------------------------------

int32 BString::CountBytes(int32 fromCharOffset, int32 charCount) const
{
    return UTF8CountBytes(
        fPrivateData + UTF8CountBytes(fPrivateData, fromCharOffset), charCount);
}
//-------------------------------------------------------------------------------------------------

/*static*/ uint32 BString::HashValue(const char* string)
{
    // from the Dragon Book: a slightly modified hashpjw()
     if (string == NULL) return 0;
    uint32 h = 0;
    for (; *string; string++) {
         uint32 g = h & 0xf0000000;
         if (g) h ^= g >> 24;
         h = (h << 4) + *string;
        }
    return h;
}
//-------------------------------------------------------------------------------------------------
//	#pragma mark - Assignment

BString& BString::operator=(const BString& string)
{
    return SetTo(string);
}
//-------------------------------------------------------------------------------------------------

BString& BString::operator=(const char* string)
{
    if (!string) string = "";
    if (string != String()) SetTo(string, strlen(string));
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::operator=(char c)
{
    return SetTo(c, 1);
}
//-------------------------------------------------------------------------------------------------

BString& BString::SetTo(const char* string, int32 maxLength)
{
    if (maxLength < 0) maxLength = INT32_MAX;
    maxLength = strlen_clamp(safestr(string), maxLength);
    if (__MakeWritable(maxLength, false) == B_OK) memcpy(fPrivateData, string, maxLength);

    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::SetTo(const BString& string)
{
    // we share the information already
    if (fPrivateData == string.fPrivateData)  return *this;

    bool freeData = true;
    if (__IsShareable() && bhapi::atomic_add(&__ReferenceCount(), -1) > 1) {
        // there is still someone who shares our data
        freeData = false;
    }
    if (freeData) __FreePrivateData();

    // if source is sharable share, otherwise clone
    if (string.__IsShareable()) {
        fPrivateData = string.fPrivateData;
        bhapi::atomic_add(&__ReferenceCount(), 1);
            // the string cannot go away right now
    } else
        __Init(string.String(), string.Length());

    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::Adopt(BString& from)
{
    SetTo(from);
    from.SetTo("");
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::SetTo(const BString& string, int32 maxLength)
{
    if (maxLength < 0)
        maxLength = INT32_MAX;
    if (fPrivateData != string.fPrivateData
        // make sure we reassing in case length is different
        || (fPrivateData == string.fPrivateData && Length() > maxLength)) {
        maxLength = min_clamp0(maxLength, string.Length());
        if (__MakeWritable(maxLength, false) == B_OK)
            memcpy(fPrivateData, string.String(), maxLength);
    }
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::Adopt(BString& from, int32 maxLength)
{
    SetTo(from, maxLength);
    from.SetTo("");

    return *this;
}

BString& BString::SetTo(char c, int32 count)
{
    if (count < 0) count = 0;
    if (__MakeWritable(count, false) == B_OK) memset(fPrivateData, c, count);
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::SetToChars(const char* string, int32 charCount)
{
    return SetTo(string, UTF8CountBytes(string, charCount));
}
//-------------------------------------------------------------------------------------------------

BString& BString::SetToChars(const BString& string, int32 charCount)
{
    return SetTo(string, UTF8CountBytes(string.String(), charCount));
}
//-------------------------------------------------------------------------------------------------

BString& BString::AdoptChars(BString& string, int32 charCount)
{
    return Adopt(string, UTF8CountBytes(string.String(), charCount));
}
//-------------------------------------------------------------------------------------------------

BString& BString::SetToFormat(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    SetToFormatVarArgs(format, args);
    va_end(args);

    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::SetToFormatVarArgs(const char* format, va_list args)
{
    // Use a small on-stack buffer to save a second vsnprintf() call for most
    // use cases.
    int32 bufferSize = 1024;
    char buffer[bufferSize];

    va_list clonedArgs;
#if __GNUC__ == 2
    __va_copy(clonedArgs, args);
#else
    va_copy(clonedArgs, args);
#endif
    int32 bytes = vsnprintf(buffer, bufferSize, format, clonedArgs);
    va_end(clonedArgs);

    if (bytes < 0) return Truncate(0);
    if (bytes < bufferSize) {
        SetTo(buffer);
        return *this;
    }
    bytes = vsnprintf(LockBuffer(bytes), bytes + 1, format, args);
    if (bytes < 0)
        bytes = 0;

    UnlockBuffer(bytes);
    return *this;
}
//-------------------------------------------------------------------------------------------------

int BString::ScanWithFormat(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int result = ScanWithFormatVarArgs(format, args);
    va_end(args);

    return result;
}
//-------------------------------------------------------------------------------------------------

int BString::ScanWithFormatVarArgs(const char* format, va_list args)
{
    return vsscanf(fPrivateData, format, args);
}
//-------------------------------------------------------------------------------------------------
//	#pragma mark - Substring copying

BString& BString::CopyInto(BString& into, int32 fromOffset, int32 length) const
{
    if (this != &into) into.SetTo(fPrivateData + fromOffset, length);
    return into;
}
//-------------------------------------------------------------------------------------------------

void BString::CopyInto(char* into, int32 fromOffset, int32 length) const
{
    if (into==0) return;
    length = min_clamp0(length, Length() - fromOffset);
    memcpy(into, fPrivateData + fromOffset, length);
}
//-------------------------------------------------------------------------------------------------

BString& BString::CopyCharsInto(BString& into, int32 fromCharOffset, int32 charCount) const
{
    int32 fromOffset = UTF8CountBytes(fPrivateData, fromCharOffset);
    int32 length = UTF8CountBytes(fPrivateData + fromOffset, charCount);
    return CopyInto(into, fromOffset, length);
}
//-------------------------------------------------------------------------------------------------

bool BString::CopyCharsInto(char* into, int32* intoLength, int32 fromCharOffset, int32 charCount) const
{
    if (into == NULL) return false;
    int32 fromOffset = UTF8CountBytes(fPrivateData, fromCharOffset);
    int32 length = UTF8CountBytes(fPrivateData + fromOffset, charCount);
    length = min_clamp0(length, Length() - fromOffset);
    if (intoLength != NULL) {
        if (*intoLength < length) return false;
        *intoLength = length;
    }
    memcpy(into, fPrivateData + fromOffset, length);
    return true;
}
//-------------------------------------------------------------------------------------------------

bool BString::Split(const char* separator, bool noEmptyStrings, BStringList& _list) const
{
    int32 separatorLength = strlen(separator);
    int32 length = Length();
    if (separatorLength == 0 || length == 0 || separatorLength > length) {
        if (length == 0 && noEmptyStrings)
            return true;
        return _list.Add(*this);
    }
    int32 index = 0;
    for (;;) {
        int32 endIndex = index < length ? FindFirst(separator, index) : length;
        if (endIndex < 0)
            endIndex = length;

        if (endIndex > index || !noEmptyStrings) {
            BString toAppend(String() + index, endIndex - index);
            if (toAppend.Length() != endIndex - index
                || !_list.Add(toAppend)) {
                return false;
            }
        }

        if (endIndex == length)
            break;
        index = endIndex + 1;
    }
    return true;
}
//-------------------------------------------------------------------------------------------------
//	#pragma mark - Appending

BString& BString::operator+=(const char* string)
{
    if (string) {
        int32 length = strlen(string);
        if (length > 0)
            __DoAppend(string, length);
    }
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::operator+=(char c)
{
    __DoAppend(&c, 1);
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::Append(const BString& string, int32 length)
{
    if (&string != this) {
        length = min_clamp0(length, string.Length());
        if (length > 0)
            __DoAppend(string.fPrivateData, length);
    }
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::Append(const char* string, int32 length)
{
    if (string) {
        length = strlen_clamp(string, length);
        if (length > 0)
            __DoAppend(string, length);
    }
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::Append(char c, int32 count)
{
    int32 oldLength = Length();
    if (count > 0 && __DoAppend("", count))
        memset(fPrivateData + oldLength, c, count);
    return *this;
}

BString& BString::AppendChars(const BString& string, int32 charCount)
{
    return Append(string, UTF8CountBytes(string.String(), charCount));
}
//-------------------------------------------------------------------------------------------------

BString& BString::AppendChars(const char* string, int32 charCount)
{
    return Append(string, UTF8CountBytes(string, charCount));
}
//-------------------------------------------------------------------------------------------------
//	#pragma mark - Prepending

BString& BString::Prepend(const char* string)
{
    if (string)
        __DoPrepend(string, strlen(string));
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::Prepend(const BString& string)
{
    if (&string != this)
        __DoPrepend(string.String(), string.Length());
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::Prepend(const char* string, int32 length)
{
    if (string)
        __DoPrepend(string, strlen_clamp(string, length));
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::Prepend(const BString& string, int32 length)
{
    if (&string != this)
        __DoPrepend(string.fPrivateData, min_clamp0(length, string.Length()));
    return *this;
}

BString& BString::Prepend(char c, int32 count)
{
    if (count > 0 && __DoPrepend("", count)) memset(fPrivateData, c, count);
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::PrependChars(const char* string, int32 charCount)
{
    return Prepend(string, UTF8CountBytes(string, charCount));
}

BString& BString::PrependChars(const BString& string, int32 charCount)
{
    return Prepend(string, UTF8CountBytes(string.String(), charCount));
}
//-------------------------------------------------------------------------------------------------
//	#pragma mark - Inserting

BString& BString::Insert(const char* string, int32 position)
{
    if (string != NULL && position <= Length()) {
        int32 len = int32(strlen(string));
        if (position < 0) {
            int32 skipLen = min_clamp0(-1 * position, len);
            string += skipLen;
            len -= skipLen;
            position = 0;
        } else {
            position = min_clamp0(position, Length());
        }
        __DoInsert(string, position, len);
    }
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::Insert(const char* string, int32 length, int32 position)
{
    if (string != NULL && position <= Length()) {
        int32 len = strlen_clamp(string, length);
        if (position < 0) {
            int32 skipLen = min_clamp0(-1 * position, len);
            string += skipLen;
            len -= skipLen;
            position = 0;
        } else {
            position = min_clamp0(position, Length());
        }
        __DoInsert(string, position, len);
    }
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::Insert(const char* string, int32 fromOffset, int32 length,
    int32 position)
{
    if (string)
        Insert(string + fromOffset, length, position);
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::Insert(const BString& string, int32 position)
{
    if (&string != this && string.Length() > 0)
        Insert(string.fPrivateData, position);
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::Insert(const BString& string, int32 length, int32 position)
{
    if (&string != this && string.Length() > 0)
        Insert(string.String(), length, position);
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::Insert(const BString& string, int32 fromOffset, int32 length,
    int32 position)
{
    if (&string != this && string.Length() > 0)
        Insert(string.String() + fromOffset, length, position);
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::Insert(char c, int32 count, int32 position)
{
    if (position < 0) {
        count = max_c(count + position, 0);
        position = 0;
    } else
        position = min_clamp0(position, Length());

    if (count > 0 && __DoInsert("", position, count))
        memset(fPrivateData + position, c, count);

    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::InsertChars(const char* string, int32 charPosition)
{
    return Insert(string, UTF8CountBytes(fPrivateData, charPosition));
}
//-------------------------------------------------------------------------------------------------

BString& BString::InsertChars(const char* string, int32 charCount, int32 charPosition)
{
    return Insert(string, UTF8CountBytes(string, charCount),
        UTF8CountBytes(fPrivateData, charPosition));
}
//-------------------------------------------------------------------------------------------------

BString& BString::InsertChars(const char* string, int32 fromCharOffset,
    int32 charCount, int32 charPosition)
{
    int32 fromOffset = UTF8CountBytes(string, fromCharOffset);
    return Insert(string, fromOffset,
        UTF8CountBytes(string + fromOffset, charCount),
        UTF8CountBytes(fPrivateData, charPosition));
}
//-------------------------------------------------------------------------------------------------

BString& BString::InsertChars(const BString& string, int32 charPosition)
{
    return Insert(string, UTF8CountBytes(fPrivateData, charPosition));
}
//-------------------------------------------------------------------------------------------------

BString& BString::InsertChars(const BString& string, int32 charCount, int32 charPosition)
{
    return Insert(string, UTF8CountBytes(string.String(), charCount),
        UTF8CountBytes(fPrivateData, charPosition));
}
//-------------------------------------------------------------------------------------------------

BString& BString::InsertChars(const BString& string, int32 fromCharOffset,
    int32 charCount, int32 charPosition)
{
    int32 fromOffset = UTF8CountBytes(string.String(), fromCharOffset);
    return Insert(string, fromOffset,
        UTF8CountBytes(string.String() + fromOffset, charCount),
        UTF8CountBytes(fPrivateData, charPosition));
}
//-------------------------------------------------------------------------------------------------
//	#pragma mark - Removing

BString& BString::Truncate(int32 newLength, bool lazy)
{
    if (newLength < 0)
        newLength = 0;

    if (newLength < Length()) {
        // ignore lazy, since we might detach
        __MakeWritable(newLength, true);
    }

    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::TruncateChars(int32 newCharCount, bool lazy)
{
    return Truncate(UTF8CountBytes(fPrivateData, newCharCount));
}
//-------------------------------------------------------------------------------------------------

BString& BString::Remove(int32 from, int32 length)
{
    if (length > 0 && from < Length())
        __ShrinkAtBy(from, min_clamp0(length, (Length() - from)));
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::RemoveChars(int32 fromCharOffset, int32 charCount)
{
    int32 fromOffset = UTF8CountBytes(fPrivateData, fromCharOffset);
    return Remove(fromOffset,
        UTF8CountBytes(fPrivateData + fromOffset, charCount));
}
//-------------------------------------------------------------------------------------------------

BString& BString::RemoveFirst(const BString& string)
{
    if (string.Length() > 0) {
        int32 pos = __ShortFindAfter(string.String(), string.Length());
        if (pos >= 0)
            __ShrinkAtBy(pos, string.Length());
    }
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::RemoveLast(const BString& string)
{
    int32 pos = __FindBefore(string.String(), Length(), string.Length());
    if (pos >= 0)
        __ShrinkAtBy(pos, string.Length());

    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::RemoveAll(const BString& string)
{
    if (string.Length() == 0 || Length() == 0 || FindFirst(string) < 0)
        return *this;

    if (__MakeWritable() != B_OK)
        return *this;

    return __DoReplace(string.String(), "", REPLACE_ALL, 0, KEEP_CASE);
}
//-------------------------------------------------------------------------------------------------

BString& BString::RemoveFirst(const char* string)
{
    int32 length = string ? strlen(string) : 0;
    if (length > 0) {
        int32 pos = __ShortFindAfter(string, length);
        if (pos >= 0)
            __ShrinkAtBy(pos, length);
    }
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::RemoveLast(const char* string)
{
    int32 length = string ? strlen(string) : 0;
    if (length > 0) {
        int32 pos = __FindBefore(string, Length(), length);
        if (pos >= 0)
            __ShrinkAtBy(pos, length);
    }
    return *this;
}


BString&
BString::RemoveAll(const char* string)
{
    if (!string || Length() == 0 || FindFirst(string) < 0)
        return *this;

    if (__MakeWritable() != B_OK)
        return *this;

    return __DoReplace(string, "", REPLACE_ALL, 0, KEEP_CASE);
}


BString&
BString::RemoveSet(const char* setOfBytesToRemove)
{
    return ReplaceSet(setOfBytesToRemove, "");
}


BString&
BString::RemoveCharsSet(const char* setOfCharsToRemove)
{
    return ReplaceCharsSet(setOfCharsToRemove, "");
}


BString&
BString::MoveInto(BString& into, int32 from, int32 length)
{
    if (length) {
        CopyInto(into, from, length);
        Remove(from, length);
    }
    return into;
}


void
BString::MoveInto(char* into, int32 from, int32 length)
{
    if (into) {
        CopyInto(into, from, length);
        Remove(from, length);
    }
}


BString&
BString::MoveCharsInto(BString& into, int32 fromCharOffset, int32 charCount)
{
    if (charCount > 0) {
        CopyCharsInto(into, fromCharOffset, charCount);
        RemoveChars(fromCharOffset, charCount);
    }

    return into;
}


bool
BString::MoveCharsInto(char* into, int32* intoLength, int32 fromCharOffset,
    int32 charCount)
{
    if (!CopyCharsInto(into, intoLength, fromCharOffset, charCount))
        return false;

    RemoveChars(fromCharOffset, charCount);
    return true;
}


//	#pragma mark - Compare functions


bool
BString::operator<(const char* string) const
{
    return strcmp(String(), safestr(string)) < 0;
}


bool
BString::operator<=(const char* string) const
{
    return strcmp(String(), safestr(string)) <= 0;
}


bool
BString::operator==(const char* string) const
{
    return strcmp(String(), safestr(string)) == 0;
}


bool
BString::operator>=(const char* string) const
{
    return strcmp(String(), safestr(string)) >= 0;
}


bool
BString::operator>(const char* string) const
{
    return strcmp(String(), safestr(string)) > 0;
}


//	#pragma mark - strcmp()-style compare functions


int
BString::Compare(const BString& string) const
{
    return strcmp(String(), string.String());
}


int
BString::Compare(const char* string) const
{
    return strcmp(String(), safestr(string));
}


int
BString::Compare(const BString& string, int32 length) const
{
    return strncmp(String(), string.String(), length);
}


int
BString::Compare(const char* string, int32 length) const
{
    return strncmp(String(), safestr(string), length);
}


int
BString::CompareChars(const BString& string, int32 charCount) const
{
    return Compare(string, UTF8CountBytes(fPrivateData, charCount));
}


int
BString::CompareChars(const char* string, int32 charCount) const
{
    return Compare(string, UTF8CountBytes(fPrivateData, charCount));
}


int
BString::ICompare(const BString& string) const
{
    return strcasecmp(String(), string.String());
}


int
BString::ICompare(const char* string) const
{
    return strcasecmp(String(), safestr(string));
}


int
BString::ICompare(const BString& string, int32 length) const
{
    return strncasecmp(String(), string.String(), length);
}


int
BString::ICompare(const char* string, int32 length) const
{
    return strncasecmp(String(), safestr(string), length);
}


//	#pragma mark - Searching


int32
BString::FindFirst(const BString& string) const
{
    return __ShortFindAfter(string.String(), string.Length());
}
//-------------------------------------------------------------------------------------------------

int32 BString::FindFirst(const char* string) const
{
    if (string == NULL)
        return B_BAD_VALUE;

    return __ShortFindAfter(string, strlen(string));
}
//-------------------------------------------------------------------------------------------------

int32 BString::FindFirst(const BString& string, int32 fromOffset) const
{
    if (fromOffset < 0)
        return B_ERROR;

    return __FindAfter(string.String(), min_clamp0(fromOffset, Length()),
        string.Length());
}
//-------------------------------------------------------------------------------------------------

int32 BString::FindFirst(const char* string, int32 fromOffset) const
{
    if (string == NULL)
        return B_BAD_VALUE;

    if (fromOffset < 0)
        return B_ERROR;

    return __FindAfter(string, min_clamp0(fromOffset, Length()),
        strlen(safestr(string)));
}
//-------------------------------------------------------------------------------------------------

int32 BString::FindFirst(char c) const
{
    const char* start = String();
    const char* end = String() + Length();

    // Scans the string until we found the
    // character, or we reach the string's start
    while (start != end && *start != c) {
        start++;
    }

    if (start == end)
        return B_ERROR;

    return start - String();
}
//-------------------------------------------------------------------------------------------------

int32 BString::FindFirst(char c, int32 fromOffset) const
{
    if (fromOffset < 0)
        return B_ERROR;

    const char* start = String() + min_clamp0(fromOffset, Length());
    const char* end = String() + Length();

    // Scans the string until we found the
    // character, or we reach the string's start
    while (start < end && *start != c) {
        start++;
    }

    if (start >= end)
        return B_ERROR;

    return start - String();
}
//-------------------------------------------------------------------------------------------------

int32 BString::FindFirstChars(const BString& string, int32 fromCharOffset) const
{
    return FindFirst(string, UTF8CountBytes(fPrivateData, fromCharOffset));
}
//-------------------------------------------------------------------------------------------------

int32 BString::FindFirstChars(const char* string, int32 fromCharOffset) const
{
    return FindFirst(string, UTF8CountBytes(fPrivateData, fromCharOffset));
}
//-------------------------------------------------------------------------------------------------

int32 BString::FindLast(const BString& string) const
{
    return __FindBefore(string.String(), Length(), string.Length());
}
//-------------------------------------------------------------------------------------------------

int32 BString::FindLast(const char* string) const
{
    if (string == NULL)
        return B_BAD_VALUE;

    return __FindBefore(string, Length(), strlen(safestr(string)));
}
//-------------------------------------------------------------------------------------------------

int32 BString::FindLast(const BString& string, int32 beforeOffset) const
{
    if (beforeOffset < 0)
        return B_ERROR;

    return __FindBefore(string.String(), min_clamp0(beforeOffset, Length()),
        string.Length());
}
//-------------------------------------------------------------------------------------------------

int32 BString::FindLast(const char* string, int32 beforeOffset) const
{
    if (string == NULL)
        return B_BAD_VALUE;

    if (beforeOffset < 0)
        return B_ERROR;

    return __FindBefore(string, min_clamp0(beforeOffset, Length()),
        strlen(safestr(string)));
}
//-------------------------------------------------------------------------------------------------

int32 BString::FindLast(char c) const
{
    const char* const start = String();
    const char* end = String() + Length() - 1;

    // Scans the string backwards until we found
    // the character, or we reach the string's start
    while (end >= start && *end != c) {
        end--;
    }

    if (end < start)
        return B_ERROR;

    return end - start;
}
//-------------------------------------------------------------------------------------------------

int32 BString::FindLast(char c, int32 beforeOffset) const
{
    if (beforeOffset < 0)
        return B_ERROR;

    const char* const start = String();
    const char* end = String() + min_clamp0(beforeOffset + 1, Length()) - 1;

    // Scans the string backwards until we found
    // the character, or we reach the string's start
    while (end >= start && *end != c) {
        end--;
    }

    if (end < start)
        return B_ERROR;

    return end - start;
}
//-------------------------------------------------------------------------------------------------

int32 BString::FindLastChars(const BString& string, int32 beforeCharOffset) const
{
    return FindLast(string, UTF8CountBytes(fPrivateData, beforeCharOffset));
}
//-------------------------------------------------------------------------------------------------

int32 BString::FindLastChars(const char* string, int32 beforeCharOffset) const
{
    return FindLast(string, UTF8CountBytes(fPrivateData, beforeCharOffset));
}
//-------------------------------------------------------------------------------------------------

int32 BString::IFindFirst(const BString& string) const
{
    return __IFindAfter(string.String(), 0, string.Length());
}


int32
BString::IFindFirst(const char* string) const
{
    if (string == NULL)
        return B_BAD_VALUE;

    return __IFindAfter(string, 0, strlen(safestr(string)));
}


int32
BString::IFindFirst(const BString& string, int32 fromOffset) const
{
    if (fromOffset < 0)
        return B_ERROR;

    return __IFindAfter(string.String(), min_clamp0(fromOffset, Length()),
        string.Length());
}
//-------------------------------------------------------------------------------------------------

int32 BString::IFindFirst(const char* string, int32 fromOffset) const
{
    if (string == NULL)
        return B_BAD_VALUE;

    if (fromOffset < 0)
        return B_ERROR;

    return __IFindAfter(string, min_clamp0(fromOffset,Length()),
        strlen(safestr(string)));
}
//-------------------------------------------------------------------------------------------------

int32 BString::IFindLast(const BString& string) const
{
    return __IFindBefore(string.String(), Length(), string.Length());
}
//-------------------------------------------------------------------------------------------------

int32 BString::IFindLast(const char* string) const
{
    if (string == NULL)
        return B_BAD_VALUE;

    return __IFindBefore(string, Length(), strlen(safestr(string)));
}
//-------------------------------------------------------------------------------------------------

int32 BString::IFindLast(const BString& string, int32 beforeOffset) const
{
    if (beforeOffset < 0)
        return B_ERROR;

    return __IFindBefore(string.String(), min_clamp0(beforeOffset, Length()),
        string.Length());
}
//-------------------------------------------------------------------------------------------------

int32 BString::IFindLast(const char* string, int32 beforeOffset) const
{
    if (string == NULL)
        return B_BAD_VALUE;

    if (beforeOffset < 0)
        return B_ERROR;

    return __IFindBefore(string, min_clamp0(beforeOffset, Length()),
        strlen(safestr(string)));
}
//-------------------------------------------------------------------------------------------------

bool BString::StartsWith(const BString& string) const
{
    return StartsWith(string.String(), string.Length());
}
//-------------------------------------------------------------------------------------------------

bool BString::StartsWith(const char* string) const
{
    return StartsWith(string, strlen(string));
}
//-------------------------------------------------------------------------------------------------

bool BString::StartsWith(const char* string, int32 length) const
{
    if (length > Length())
        return false;

    return memcmp(String(), string, length) == 0;
}
//-------------------------------------------------------------------------------------------------

bool BString::EndsWith(const BString& string) const
{
    return EndsWith(string.String(), string.Length());
}
//-------------------------------------------------------------------------------------------------

bool BString::EndsWith(const char* string) const
{
    return EndsWith(string, strlen(string));
}
//-------------------------------------------------------------------------------------------------

bool BString::EndsWith(const char* string, int32 length) const
{
    int32 offset = Length() - length;
    if (offset < 0)
        return false;

    return memcmp(String() + offset, string, length) == 0;
}
//-------------------------------------------------------------------------------------------------
//	#pragma mark - Replacing

BString& BString::ReplaceFirst(char replaceThis, char withThis)
{
    int32 pos = FindFirst(replaceThis);
    if (pos >= 0 && __MakeWritable() == B_OK)
        fPrivateData[pos] = withThis;
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::ReplaceLast(char replaceThis, char withThis)
{
    int32 pos = FindLast(replaceThis);
    if (pos >= 0 && __MakeWritable() == B_OK)
        fPrivateData[pos] = withThis;
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::ReplaceAll(char replaceThis, char withThis, int32 fromOffset)
{
    fromOffset = min_clamp0(fromOffset, Length());
    int32 pos = FindFirst(replaceThis, fromOffset);

    // detach and set first match
    if (pos >= 0 && __MakeWritable() == B_OK) {
        for( ; pos >= 0; pos = FindFirst(replaceThis, pos + 1))
            fPrivateData[pos] = withThis;
    }
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::Replace(char replaceThis, char withThis, int32 maxReplaceCount,
    int32 fromOffset)
{
    fromOffset = min_clamp0(fromOffset, Length());
    int32 pos = FindFirst(replaceThis, fromOffset);

    if (maxReplaceCount > 0 && pos >= 0 && __MakeWritable() == B_OK) {
        for( ; maxReplaceCount > 0 && pos >= 0;
            pos = FindFirst(replaceThis, pos + 1)) {
            fPrivateData[pos] = withThis;
            maxReplaceCount--;
        }
    }
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::ReplaceFirst(const char* replaceThis, const char* withThis)
{
    if (!replaceThis || !withThis || FindFirst(replaceThis) < 0)
        return *this;

    if (__MakeWritable() != B_OK)
        return *this;

    return __DoReplace(replaceThis, withThis, 1, 0, KEEP_CASE);
}
//-------------------------------------------------------------------------------------------------

BString& BString::ReplaceLast(const char* replaceThis, const char* withThis)
{
    if (!replaceThis || !withThis)
        return *this;

    int32 replaceThisLength = strlen(replaceThis);
    int32 pos = __FindBefore(replaceThis, Length(), replaceThisLength);

    if (pos >= 0) {
        int32 withThisLength =  strlen(withThis);
        int32 difference = withThisLength - replaceThisLength;

        if (difference > 0) {
            if (!__OpenAtBy(pos, difference))
                return *this;
        } else if (difference < 0) {
            if (!__ShrinkAtBy(pos, -difference))
                return *this;
        } else {
            if (__MakeWritable() != B_OK)
                return *this;
        }
        memcpy(fPrivateData + pos, withThis, withThisLength);
    }

    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::ReplaceAll(const char* replaceThis, const char* withThis, int32 fromOffset)
{
    if (!replaceThis || !withThis || FindFirst(replaceThis) < 0)
        return *this;

    if (__MakeWritable() != B_OK)
        return *this;

    return __DoReplace(replaceThis, withThis, REPLACE_ALL,
        min_clamp0(fromOffset, Length()), KEEP_CASE);
}
//-------------------------------------------------------------------------------------------------

BString& BString::Replace(const char* replaceThis, const char* withThis, int32 maxReplaceCount, int32 fromOffset)
{
    if (!replaceThis || !withThis || maxReplaceCount <= 0
        || FindFirst(replaceThis) < 0)
        return *this;

    if (__MakeWritable() != B_OK)
        return *this;

    return __DoReplace(replaceThis, withThis, maxReplaceCount,
        min_clamp0(fromOffset, Length()), KEEP_CASE);
}
//-------------------------------------------------------------------------------------------------

BString& BString::ReplaceAllChars(const char* replaceThis, const char* withThis, int32 fromCharOffset)
{
    return ReplaceAll(replaceThis, withThis,
        UTF8CountBytes(fPrivateData, fromCharOffset));
}
//-------------------------------------------------------------------------------------------------

BString& BString::ReplaceChars(const char* replaceThis, const char* withThis, int32 maxReplaceCount, int32 fromCharOffset)
{
    return Replace(replaceThis, withThis, maxReplaceCount,
        UTF8CountBytes(fPrivateData, fromCharOffset));
}
//-------------------------------------------------------------------------------------------------

BString& BString::IReplaceFirst(char replaceThis, char withThis)
{
    char tmp[2] = { replaceThis, '\0' };

    int32 pos = __IFindAfter(tmp, 0, 1);
    if (pos >= 0 && __MakeWritable() == B_OK)
        fPrivateData[pos] = withThis;
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::IReplaceLast(char replaceThis, char withThis)
{
    char tmp[2] = { replaceThis, '\0' };

    int32 pos = __IFindBefore(tmp, Length(), 1);
    if (pos >= 0 && __MakeWritable() == B_OK)
        fPrivateData[pos] = withThis;
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::IReplaceAll(char replaceThis, char withThis, int32 fromOffset)
{
    char tmp[2] = { replaceThis, '\0' };
    fromOffset = min_clamp0(fromOffset, Length());
    int32 pos = __IFindAfter(tmp, fromOffset, 1);

    if (pos >= 0 && __MakeWritable() == B_OK) {
        for( ; pos >= 0; pos = __IFindAfter(tmp, pos + 1, 1))
            fPrivateData[pos] = withThis;
    }
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::IReplace(char replaceThis, char withThis, int32 maxReplaceCount, int32 fromOffset)
{
    char tmp[2] = { replaceThis, '\0' };
    fromOffset = min_clamp0(fromOffset, Length());
    int32 pos = __IFindAfter(tmp, fromOffset, 1);

    if (maxReplaceCount > 0 && pos >= 0 && __MakeWritable() == B_OK) {
        for( ; maxReplaceCount > 0 && pos >= 0;
            pos = __IFindAfter(tmp, pos + 1, 1)) {
            fPrivateData[pos] = withThis;
            maxReplaceCount--;
        }
    }

    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::IReplaceFirst(const char* replaceThis, const char* withThis)
{
    if (!replaceThis || !withThis || IFindFirst(replaceThis) < 0)
        return *this;

    if (__MakeWritable() != B_OK)
        return *this;
    return __DoReplace(replaceThis, withThis, 1, 0, IGNORE_CASE);
}
//-------------------------------------------------------------------------------------------------

BString& BString::IReplaceLast(const char* replaceThis, const char* withThis)
{
    if (!replaceThis || !withThis)
        return *this;

    int32 replaceThisLength = strlen(replaceThis);
    int32 pos = __IFindBefore(replaceThis, Length(), replaceThisLength);

    if (pos >= 0) {
        int32 withThisLength = strlen(withThis);
        int32 difference = withThisLength - replaceThisLength;

        if (difference > 0) {
            if (!__OpenAtBy(pos, difference))
                return *this;
        } else if (difference < 0) {
            if (!__ShrinkAtBy(pos, -difference))
                return *this;
        } else {
            if (__MakeWritable() != B_OK)
                return *this;
        }
        memcpy(fPrivateData + pos, withThis, withThisLength);
    }

    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::IReplaceAll(const char* replaceThis, const char* withThis, int32 fromOffset)
{
    if (!replaceThis || !withThis || IFindFirst(replaceThis) < 0)
        return *this;

    if (__MakeWritable() != B_OK)
        return *this;

    return __DoReplace(replaceThis, withThis, REPLACE_ALL,
        min_clamp0(fromOffset, Length()), IGNORE_CASE);
}
//-------------------------------------------------------------------------------------------------

BString& BString::IReplace(const char* replaceThis, const char* withThis, int32 maxReplaceCount, int32 fromOffset)
{
    if (!replaceThis || !withThis || maxReplaceCount <= 0
        || FindFirst(replaceThis) < 0)
        return *this;

    if (__MakeWritable() != B_OK)
        return *this;

    return __DoReplace(replaceThis, withThis, maxReplaceCount,
        min_clamp0(fromOffset, Length()), IGNORE_CASE);
}
//-------------------------------------------------------------------------------------------------

BString& BString::ReplaceSet(const char* setOfBytes, char with)
{
    if (!setOfBytes || strcspn(fPrivateData, setOfBytes) >= uint32(Length()))
        return *this;

    if (__MakeWritable() != B_OK)
        return *this;

    int32 offset = 0;
    int32 length = Length();
    for (int32 pos;;) {
        pos = strcspn(fPrivateData + offset, setOfBytes);

        offset += pos;
        if (offset >= length)
            break;

        fPrivateData[offset] = with;
        offset++;
    }

    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::ReplaceSet(const char* setOfBytes, const char* with)
{
    if (!setOfBytes || !with
        || strcspn(fPrivateData, setOfBytes) >= uint32(Length()))
        return *this;

    // delegate simple case
    int32 withLen = strlen(with);
    if (withLen == 1)
        return ReplaceSet(setOfBytes, *with);

    if (__MakeWritable() != B_OK)
        return *this;

    int32 pos = 0;
    int32 searchLen = 1;
    int32 len = Length();

    PosVect positions;
    for (int32 offset = 0; offset < len; offset += (pos + searchLen)) {
        pos = strcspn(fPrivateData + offset, setOfBytes);
        if (pos + offset >= len)
            break;
        if (!positions.Add(offset + pos))
            return *this;
    }

    __ReplaceAtPositions(&positions, searchLen, with, withLen);
    return *this;
}


BString&
BString::ReplaceCharsSet(const char* setOfChars, const char* with)
{
    if (!setOfChars || !with)
        return *this;

    int32 setCharCount = UTF8CountChars(setOfChars, -1);
    if ((uint32)setCharCount == strlen(setOfChars)) {
        // no multi-byte chars at all
        return ReplaceSet(setOfChars, with);
    }

    BString setString(setOfChars);
    BString result;

    int32 withLength = strlen(with);
    int32 charCount = CountChars();
    for (int32 i = 0; i < charCount; i++) {
        int32 charLength;
        const char* sourceChar = CharAt(i, &charLength);
        bool match = false;

        for (int32 j = 0; j < setCharCount; j++) {
            int32 setCharLength;
            const char* setChar = setString.CharAt(j, &setCharLength);
            if (charLength == setCharLength
                && memcmp(sourceChar, setChar, charLength) == 0) {
                match = true;
                break;
            }
        }

        if (match)
            result.Append(with, withLength);
        else
            result.Append(sourceChar, charLength);
    }

    *this = result;
    return *this;
}


//	#pragma mark - Unchecked char access


#if __GNUC__ > 3
BStringRef
BString::operator[](int32 index)
{
    return BStringRef(*this, index);
}
#else
char&
BString::operator[](int32 index)
{
    if (__MakeWritable() != B_OK) {
        static char invalid;
        return invalid;
    }

    __ReferenceCount() = -1;
        // mark string as unshareable

    return fPrivateData[index];
}
#endif


const char*
BString::CharAt(int32 charIndex, int32* bytes) const
{
    int32 offset = UTF8CountBytes(fPrivateData, charIndex);
    if (bytes != NULL)
        *bytes = UTF8NextCharLen(fPrivateData + offset);
    return fPrivateData + offset;
}


bool
BString::CharAt(int32 charIndex, char* buffer, int32* bytes) const
{
    int32 length;
    const char* charAt = CharAt(charIndex, &length);
    if (bytes != NULL) {
        if (*bytes < length)
            return false;
        *bytes = length;
    }

    memcpy(buffer, charAt, length);
    return true;
}
//-------------------------------------------------------------------------------------------------
//	#pragma mark - Fast low-level manipulation

char* BString::LockBuffer(int32 maxLength)
{
    int32 length = Length();
    if (maxLength > length)
        length = maxLength;

    if (__MakeWritable(length, true) != B_OK)
        return NULL;

    __ReferenceCount() = -1;
        // mark unshareable

    return fPrivateData;
}
//-------------------------------------------------------------------------------------------------

BString& BString::UnlockBuffer(int32 length)
{
    if (length > 0)
        length = min_clamp0(length, Length());
    else
        length = fPrivateData == NULL ? 0 : strlen(fPrivateData);

    if (__Resize(length) != NULL) {
        fPrivateData[length] = '\0';
        __ReferenceCount() = 1;
            // mark shareable again
    }

    return *this;
}
//-------------------------------------------------------------------------------------------------
//	#pragma mark - Uppercase <-> Lowercase


BString& BString::ToLower()
{
    int32 length = Length();
    if (length > 0 && __MakeWritable() == B_OK) {
        for (int32 count = 0; count < length; count++)
            fPrivateData[count] = tolower(fPrivateData[count]);
    }
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::ToUpper()
{
    int32 length = Length();
    if (length > 0 && __MakeWritable() == B_OK) {
        for (int32 count = 0; count < length; count++)
            fPrivateData[count] = toupper(fPrivateData[count]);
    }
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::Capitalize()
{
    int32 length = Length();

    if (length > 0 && __MakeWritable() == B_OK) {
        fPrivateData[0] = toupper(fPrivateData[0]);
        for (int32 count = 1; count < length; count++)
            fPrivateData[count] = tolower(fPrivateData[count]);
    }
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::CapitalizeEachWord()
{
    int32 length = Length();

    if (length > 0 && __MakeWritable() == B_OK) {
        int32 count = 0;
        do {
            // Find the first alphabetical character...
            for (; count < length; count++) {
                if (isalpha(fPrivateData[count])) {
                    // ...found! Convert it to uppercase.
                    fPrivateData[count] = toupper(fPrivateData[count]);
                    count++;
                    break;
                }
            }

            // Now find the first non-alphabetical character,
            // and meanwhile, turn to lowercase all the alphabetical ones
            for (; count < length; count++) {
                if (isalpha(fPrivateData[count]))
                    fPrivateData[count] = tolower(fPrivateData[count]);
                else
                    break;
            }
        } while (count < length);
    }
    return *this;
}
//-------------------------------------------------------------------------------------------------
//	#pragma mark - Escaping and De-escaping

BString& BString::CharacterEscape(const char* original,
                         const char* setOfCharsToEscape, char escapeWith)
{
    if (setOfCharsToEscape)
        __DoCharacterEscape(original, setOfCharsToEscape, escapeWith);
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::CharacterEscape(const char* setOfCharsToEscape, char escapeWith)
{
    if (setOfCharsToEscape && Length() > 0)
        __DoCharacterEscape(fPrivateData, setOfCharsToEscape, escapeWith);
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::CharacterDeescape(const char* original, char escapeChar)
{
    return __DoCharacterDeescape(original, escapeChar);
}
//-------------------------------------------------------------------------------------------------

BString& BString::CharacterDeescape(char escapeChar)
{
    if (Length() > 0)
        __DoCharacterDeescape(fPrivateData, escapeChar);
    return *this;
}
//-------------------------------------------------------------------------------------------------
//	#pragma mark - Trimming

BString& BString::Trim()
{
    if (Length() <= 0)
        return *this;

    const char* string = String();

    // string is \0 terminated thus we don't need to check if we reached the end
    int32 startCount = 0;
    while (isspace(string[startCount]))
        startCount++;

    int32 endIndex = Length() - 1;
    while (endIndex >= startCount && isspace(string[endIndex]))
        endIndex--;

    if (startCount == 0 && endIndex == Length() - 1)
        return *this;

    // We actually need to trim

    ssize_t length = endIndex + 1 - startCount;
    ASSERT(length >= 0);
    if (startCount == 0 || length == 0) {
        __MakeWritable(length, true);
    } else if (__MakeWritable() == B_OK) {
        memmove(fPrivateData, fPrivateData + startCount, length);
        fPrivateData[length] = '\0';
        __SetLength(length);
    }

    return *this;
}
//-------------------------------------------------------------------------------------------------
//	#pragma mark - Insert

BString&
BString::operator<<(const char* string)
{
    if (string != NULL) {
        int32 length = strlen(string);
        if (length > 0)
            __DoAppend(string, length);
    }
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::operator<<(const BString& string)
{
    if (string.Length() > 0)
        __DoAppend(string.String(), string.Length());
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::operator<<(char c)
{
    __DoAppend(&c, 1);
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::operator<<(bool value)
{
    if (value)
        __DoAppend("true", 4);
    else
        __DoAppend("false", 5);

    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::operator<<(int i)
{
    char num[32];
    int32 length = snprintf(num, sizeof(num), "%d", i);

    __DoAppend(num, length);
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::operator<<(unsigned int i)
{
    char num[32];
    int32 length = snprintf(num, sizeof(num), "%u", i);

    __DoAppend(num, length);
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::operator<<(unsigned long i)
{
    char num[32];
    int32 length = snprintf(num, sizeof(num), "%lu", i);

    __DoAppend(num, length);
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::operator<<(long i)
{
    char num[32];
    int32 length = snprintf(num, sizeof(num), "%ld", i);
    __DoAppend(num, length);
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::operator<<(unsigned long long i)
{
    char num[64];
    int32 length = snprintf(num, sizeof(num), "%llu", i);
    __DoAppend(num, length);
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::operator<<(long long i)
{
    char num[64];
    int32 length = snprintf(num, sizeof(num), "%lld", i);
    __DoAppend(num, length);
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::operator<<(float f)
{
    char num[64];
    int32 length = snprintf(num, sizeof(num), "%.2f", f);
    __DoAppend(num, length);
    return *this;
}
//-------------------------------------------------------------------------------------------------

BString& BString::operator<<(double value)
{
    char num[64];
    int32 length = snprintf(num, sizeof(num), "%.2f", value);
    __DoAppend(num, length);
    return *this;
}
//-------------------------------------------------------------------------------------------------
//	#pragma mark - Private or reserved

BString::BString(char* privateData, PrivateDataTag tag)
    :
    fPrivateData(privateData)
{
    if (fPrivateData != NULL) bhapi::atomic_add(&__ReferenceCount(), 1);
}
//-------------------------------------------------------------------------------------------------
/*!	Detaches this string from an eventually shared fPrivateData, ie. this makes
    this string writable.
*/
status_t BString::__MakeWritable()
{
    if (bhapi::atomic_get(&__ReferenceCount()) > 1) {
        // It might be shared, and this requires special treatment
        char* newData = __Clone(fPrivateData, Length());
        if (bhapi::atomic_add(&__ReferenceCount(), -1) == 1) {
            // someone else left, we were the last owner
            __FreePrivateData();
        }
        if (newData == NULL)  return B_NO_MEMORY;
        fPrivateData = newData;
    }
    return B_OK;
}
//-------------------------------------------------------------------------------------------------
/*!	Makes this string writable, and resizes the buffer to \a length bytes (not
    including the terminating null).

    @param length The length of the new buffer in bytes.
    @param copy If true, the current string will be copied into the new string.
*/
status_t BString::__MakeWritable(int32 length, bool copy)
{
    char* newData = NULL;

    if (bhapi::atomic_get(&__ReferenceCount()) > 1) {
        // we might share our data with someone else
        if (copy)
            newData = __Clone(fPrivateData, length);
        else
            newData = __Allocate(length);

        if (newData == NULL)
            return B_NO_MEMORY;

        if (bhapi::atomic_add(&__ReferenceCount(), -1) == 1) {
            // someone else left, we were the last owner
            __FreePrivateData();
        }
    } else {
        // we don't share our data with someone else
        newData = __Resize(length);
        if (newData == NULL) return B_NO_MEMORY;
    }
    fPrivateData = newData;
    return B_OK;
}
//-------------------------------------------------------------------------------------------------
/*!	Allocates a new private data buffer with the space to store \a length bytes
    (not including the terminating null).
*/
/*static*/ char* BString::__Allocate(int32 length)
{
    if (length < 0) return NULL;

    char* newData = (char*)malloc(length + kPrivateDataOffset + 1);
    if (newData == NULL) return NULL;

    newData += kPrivateDataOffset;
    newData[length] = '\0';
    // initialize reference count & length
    Private::DataRefCount(newData) = 1;
    Private::DataLength(newData) = length & 0x7fffffff;
    return newData;
}
//-------------------------------------------------------------------------------------------------
/*!	Resizes the private data buffer. You must already have a writable buffer
    when you call this method.
*/
char* BString::__Resize(int32 length)
{
    ASSERT(__ReferenceCount() == 1 || __ReferenceCount() == -1);

    if (length == Length()) return fPrivateData;
    char* data = fPrivateData ? fPrivateData - kPrivateDataOffset : NULL;
    if (length < 0) length = 0;

    data = (char*)realloc(data, length + kPrivateDataOffset + 1);
    if (data == NULL) return NULL;
    data += kPrivateDataOffset;
    fPrivateData = data;
    fPrivateData[length] = '\0';
    __SetLength(length);
    __ReferenceCount() = 1;
    return data;
}
//-------------------------------------------------------------------------------------------------

void BString::__Init(const char* src, int32 length)
{
    fPrivateData = __Clone(src, length);
    if (fPrivateData == NULL) fPrivateData = __Clone(NULL, 0);
}
//-------------------------------------------------------------------------------------------------

char* BString::__Clone(const char* data, int32 length)
{
    char* newData = __Allocate(length);
    if (newData == NULL)
        return NULL;
    if (data != NULL && length > 0) {
        // "data" may not span over the whole length
        strncpy(newData, data, length);
    }
    return newData;
}
//-------------------------------------------------------------------------------------------------

char* BString::__OpenAtBy(int32 offset, int32 length)
{
    int32 oldLength = Length();
    if (__MakeWritable() != B_OK)
        return NULL;
    memmove(fPrivateData + offset + length, fPrivateData + offset,
        oldLength - offset);
    return __Resize(oldLength + length);
}
//-------------------------------------------------------------------------------------------------

char* BString::__ShrinkAtBy(int32 offset, int32 length)
{
    int32 oldLength = Length();
    if (__MakeWritable() != B_OK)
        return NULL;
    memmove(fPrivateData + offset, fPrivateData + offset + length,
        oldLength - offset - length);
    return __Resize(oldLength - length);
}
//-------------------------------------------------------------------------------------------------

void BString::__SetLength(int32 length)
{
    Private::DataLength(fPrivateData) = length & 0x7fffffff;
}
//-------------------------------------------------------------------------------------------------

void BString::__FreePrivateData()
{
    if (fPrivateData != NULL) {
        free(fPrivateData - kPrivateDataOffset);
        fPrivateData = NULL;
    }
}
//-------------------------------------------------------------------------------------------------

bool BString::__DoAppend(const char* string, int32 length)
{
    int32 oldLength = Length();
    if (__MakeWritable(oldLength + length, true) == B_OK) {
        strncpy(fPrivateData + oldLength, string, length);
        return true;
    }
    return false;
}
//-------------------------------------------------------------------------------------------------

bool BString::__DoPrepend(const char* string, int32 length)
{
    // TODO: this could be optimized (allocate a new buffer, use memcpy())
    int32 oldLength = Length();
    if (__MakeWritable(oldLength + length, true) == B_OK) {
        memmove(fPrivateData + length, fPrivateData, oldLength);
        if (string && length)
            strncpy(fPrivateData, string, length);
        return true;
    }
    return false;
}
//-------------------------------------------------------------------------------------------------

bool BString::__DoInsert(const char* string, int32 offset, int32 length)
{
    int32 oldLength = Length();
    if (__MakeWritable(oldLength + length, true) == B_OK) {
        memmove(fPrivateData + offset + length, fPrivateData + offset,
            oldLength - offset);
        if (string != NULL && length)
            strncpy(fPrivateData + offset, string, length);
        return true;
    }
    return false;
}
//-------------------------------------------------------------------------------------------------

int32 BString::__ShortFindAfter(const char* string, int32 len) const
{
    const char* ptr = strstr(String(), string);
    if (ptr != NULL)
        return ptr - String();
    return B_ERROR;
}
//-------------------------------------------------------------------------------------------------

int32 BString::__FindAfter(const char* string, int32 offset, int32 length) const
{
    const char* ptr = strstr(String() + offset, string);
    if (ptr != NULL)
        return ptr - String();
    return B_ERROR;
}
//-------------------------------------------------------------------------------------------------

int32 BString::__IFindAfter(const char* string, int32 offset, int32 length) const
{
    const char* ptr = strcasestr(String() + offset, string);
    if (ptr != NULL)
        return ptr - String();
    return B_ERROR;
}
//-------------------------------------------------------------------------------------------------

int32 BString::__FindBefore(const char* string, int32 offset, int32 length) const
{
    if (fPrivateData != NULL) {
        const char* ptr = fPrivateData + offset - length;

        while (ptr >= fPrivateData) {
            if (!memcmp(ptr, string, length))
                return ptr - fPrivateData;
            ptr--;
        }
    }
    return B_ERROR;
}
//-------------------------------------------------------------------------------------------------

int32 BString::__IFindBefore(const char* string, int32 offset, int32 length) const
{
    if (fPrivateData != NULL) {
        char* ptr1 = fPrivateData + offset - length;

        while (ptr1 >= fPrivateData) {
            if (!strncasecmp(ptr1, string, length))
                return ptr1 - fPrivateData;
            ptr1--;
        }
    }
    return B_ERROR;
}
//-------------------------------------------------------------------------------------------------

BString& BString::__DoCharacterEscape(const char* string, const char* setOfCharsToEscape,
    char escapeChar)
{
    if (__MakeWritable(string_length(string), false) != B_OK)
        return *this;

    memcpy(fPrivateData, string, Length());

    PosVect positions;
    int32 length = Length();
    int32 pos;
    for (int32 offset = 0; offset < length; offset += pos + 1) {
        pos = strcspn(fPrivateData + offset, setOfCharsToEscape);
        if (pos < length - offset && !positions.Add(offset + pos))
            return *this;
    }

    uint32 count = positions.CountItems();
    int32 newLength = length + count;
    if (!newLength) {
        __Resize(0);
        return *this;
    }

    char* newData = __Allocate(newLength);
    if (newData) {
        char* oldString = fPrivateData;
        char* newString = newData;
        int32 lastPos = 0;

        for (uint32 i = 0; i < count; ++i) {
            pos = positions.ItemAt(i);
            length = pos - lastPos;
            if (length > 0) {
                memcpy(newString, oldString, length);
                oldString += length;
                newString += length;
            }
            *newString++ = escapeChar;
            *newString++ = *oldString++;
            lastPos = pos + 1;
        }

        length = Length() + 1 - lastPos;
        if (length > 0)
            memcpy(newString, oldString, length);

        __FreePrivateData();
        fPrivateData = newData;
    }
    return *this;
}


BString&
BString::__DoCharacterDeescape(const char* string, char escapeChar)
{
    if (__MakeWritable(string_length(string), false) != B_OK)
        return *this;

    memcpy(fPrivateData, string, Length());
    const char escape[2] = { escapeChar, '\0' };
    return __DoReplace(escape, "", REPLACE_ALL, 0, KEEP_CASE);
}


BString&
BString::__DoReplace(const char* findThis, const char* replaceWith,
    int32 maxReplaceCount, int32 fromOffset, bool ignoreCase)
{
    if (findThis == NULL || maxReplaceCount <= 0
        || fromOffset < 0 || fromOffset >= Length())
        return *this;

    int32 findLen = strlen(findThis);
    if (findLen == 0)
        return *this;

    typedef int32 (BString::*TFindMethod)(const char*, int32, int32) const;
    TFindMethod findMethod = ignoreCase
        ? &BString::__IFindAfter : &BString::__FindAfter;

    if (!replaceWith)
        replaceWith = "";

    int32 replaceLen = strlen(replaceWith);
    int32 lastSrcPos = fromOffset;
    PosVect positions;
    for (int32 srcPos = 0; maxReplaceCount > 0
        && (srcPos = (this->*findMethod)(findThis, lastSrcPos, findLen)) >= 0;
            maxReplaceCount--) {
        positions.Add(srcPos);
        lastSrcPos = srcPos + findLen;
    }
    __ReplaceAtPositions(&positions, findLen, replaceWith, replaceLen);
    return *this;
}
//-------------------------------------------------------------------------------------------------

void BString::__ReplaceAtPositions(const PosVect* positions, int32 searchLength, const char* with, int32 withLength)
{
    int32 length = Length();
    uint32 count = positions->CountItems();
    int32 newLength = length + count * (withLength - searchLength);
    if (!newLength) {
        __Resize(0);
        return;
    }

    char* newData = __Allocate(newLength);
    if (newData == NULL)
        return;

    char* oldString = fPrivateData;
    char* newString = newData;
    int32 lastPos = 0;

    for (uint32 i = 0; i < count; ++i) {
        int32 pos = positions->ItemAt(i);
        length = pos - lastPos;
        if (length > 0) {
            memcpy(newString, oldString, length);
            oldString += length;
            newString += length;
        }
        memcpy(newString, with, withLength);
        oldString += searchLength;
        newString += withLength;
        lastPos = pos + searchLength;
    }

    length = Length() + 1 - lastPos;
    if (length > 0)
        memcpy(newString, oldString, length);

    __FreePrivateData();
    fPrivateData = newData;
}
//-------------------------------------------------------------------------------------------------
//	#pragma mark - backwards compatibility

/*!	Translates to (missing const):
    BString& BString::operator<<(BString& string)
*/
extern "C" BString& __ls__7BStringR7BString(BString* self, BString& string)
{
    return self->operator<<(string);
}
//-------------------------------------------------------------------------------------------------
//	#pragma mark - Non-member compare for sorting, etc.

int Compare(const BString& string1, const BString& string2)
{
    return strcmp(string1.String(), string2.String());
}

int ICompare(const BString& string1, const BString& string2)
{
    return strcasecmp(string1.String(), string2.String());
}

int Compare(const BString* string1, const BString* string2)
{
    return strcmp(string1->String(), string2->String());
}

int ICompare(const BString* string1, const BString* string2)
{
    return strcasecmp(string1->String(), string2->String());
}
#undef __STRICT_ANSI__

#ifndef HAVE_VA_COPY
    #ifdef HAVE__VA_COPY
        #define va_copy(a, b)   __va_copy(a, b)
    #else
        #if defined(_MSC_VER) || defined(__MWERKS__)
            #define va_copy(a, b)   ((a) = (b))
        #endif
    #endif // HAVE__VA_COPY
#endif // HAVE_VA_COPY
#ifndef HAVE_ISFINITE
    #ifdef HAVE_FPCLASSIFY
        #define isfinite(a)     ((fpclassify(a) != FP_NAN && fpclassify(a) != FP_INFINITE) ? 1 : 0)
        #define HAVE_ISFINITE   1
    #else
        #ifdef _MSC_VER
            #define isfinite(a)     _finite(a)
            #define HAVE_ISFINITE   1
        #endif // _MSC_VER
    #endif
#endif
#ifndef HAVE_ISNAN
    #ifdef HAVE_FPCLASSIFY
        #define isnan(a)    ((fpclassify(a) == FP_NAN) ? 1 : 0)
        #define HAVE_ISNAN  1
    #else
        #ifdef _MSC_VER
            #define isnan(a)    _isnan(a)
            #define HAVE_ISNAN  1
        #endif // _MSC_VER
    #endif
#endif
#ifndef HAVE_STRCASECMP
    #ifdef _MSC_VER
    #define bhapi   ::strcasecmp(a, b) _stricmp(a, b)
    #endif
#else
    #define bhapi   ::strcasecmp(a, b) strcasecmp(a, b)
#endif // HAVE_STRCASECMP
#ifndef HAVE_STRNCASECMP
    #ifdef _MSC_VER
    #define bhapi   ::strncasecmp(a, b, c) _strnicmp(a, b, c)
    #define bhapi   ::strcasecmp(a, b) _stricmp(a, b)
    #endif
#else
    #define bhapi   ::strncasecmp(a, b, c) strncasecmp(a, b, c)
    #define bhapi   ::strcasecmp(a, b) strcasecmp(a, b)
#endif // HAVE_STRNCASECMP

//-----------------------------------------------------------------------------

int __cdecl strncasecmp(const char *a, const char *b, size_t c)
{
    return _strnicmp(a, b, c);
}

//-----------------------------------------------------------------------------

int __cdecl strcasecmp(const char *a, const char *b)
{
    return _stricmp(a, b);
}

// long double
#undef BHAPI_SUPPORT_LONG_DOUBLE
#ifdef BHAPI_SUPPORT_LONG_DOUBLE
    #ifndef HAVE_STRTOLD
        #ifdef _MSC_VER
            #ifdef HAVE__STRTOLD
                #define strtold(a, b)   _strtold(a, b)
            #else
                #define strtold(a, b)   strtod(a, b)
            #endif // HAVE__STRTOLD
        #endif
    #endif
    #ifdef HAVE_FMODL
        #define b_fmodl(a, b)   fmodl(a, b)
    #endif
    #ifdef HAVE_MODFL
        #define b_modfl(a, b)   modfl(a, b)
    #endif
    #if !defined(e_fmodl) && defined(HAVE_FLOORL)

//-----------------------------------------------------------------------------

static long double fmodl(long double x, long double y)
{
    long double remain = x / y;
    if (x == y) return 0;

    return(remain - floorl(remain));
}
    #endif
    #if !defined(e_modfl) && defined(HAVE_FLOORL)

//-----------------------------------------------------------------------------

static long double modfl(long double value, long double *iptr)
{
    long double iValue = floorl(value);
    long double fValue = value - iValue;

    if (iptr != NULL) *iptr = iValue;
    return fValue;
}
    #endif
#endif // BHAPI_SUPPORT_LONG_DOUBLE

//-----------------------------------------------------------------------------

BHAPI_EXPORT char *strdup_dirty(const char *src,  __be_int32 length)
{
    char    *dest;

     __be_int32 len = 0;

    if (src == NULL || *src == 0 || length == 0) return NULL;

    if (length < 0)
        len = (__be_int32) strlen(src);
    else
        len = min_c(length, (__be_int32) strlen(src));

    dest = new char[len + 1];

    if (!dest) return NULL;
    if (len != 0) memcpy(dest, src, len);
    dest[len] = 0;

    return dest;
}

#ifndef HAVE_STRCASESTR
//-----------------------------------------------------------------------------

BHAPI_EXPORT char* strcasestr(const char *haystack, const char *needle)
{
    size_t needle_len = strlen(needle);
    size_t haystack_len = strlen(haystack);
    const char  *tmp = haystack;
    const char  *end = haystack + haystack_len - needle_len;

    if (!haystack || !needle) return NULL;
    if (needle_len == 0) return (char *)haystack;
    if (haystack_len < needle_len) return NULL;

    while (*tmp && tmp <= end)
    {
        if (strncasecmp(tmp, needle, needle_len) == 0) return (char *)tmp;
        tmp++;
    }

    return NULL;
}

#else
    #define bhapi   ::strcasestr(a, b) strcasestr(a, b)
#endif
#ifndef HAVE_STRRSTR

//-----------------------------------------------------------------------------

BHAPI_EXPORT char* strrstr(const char *haystack, const char *needle)
{
    size_t needle_len = strlen(needle);
    size_t haystack_len = strlen(haystack);
    const char  *tmp = haystack + haystack_len - needle_len;

    if (!haystack || !needle) return NULL;
    if (needle_len == 0) return (char *)haystack;
    if (haystack_len < needle_len) return NULL;

    while (*tmp && tmp >= haystack)
    {
        if (strncmp(tmp, needle, needle_len) == 0) return (char *)tmp;
        tmp--;
    }

    return NULL;
}

#else
    #define bhapi   ::strrstr(a, b) strrstr(a, b)
#endif
#ifndef HAVE_STRRCASESTR

//-----------------------------------------------------------------------------

BHAPI_EXPORT char *strrcasestr(const char *haystack, const char *needle)
{
    size_t needle_len = strlen(needle);
    size_t haystack_len = strlen(haystack);
    const char  *tmp = haystack + haystack_len - needle_len;

    if (!haystack || !needle) return NULL;
    if (needle_len == 0) return (char *)haystack;
    if (haystack_len < needle_len) return NULL;

    while (*tmp && tmp >= haystack)
    {
        if (strncasecmp(tmp, needle, needle_len) == 0) return (char *)tmp;
        tmp--;
    }

    return NULL;
}

#else
    #define bhapi   ::strrcasestr(a, b) strrcasestr(a, b)
#endif
#ifndef HAVE_STRCASECHR

//-----------------------------------------------------------------------------

BHAPI_EXPORT char *strcasechr(const char *s, int c)
{
    const char  *tmp = strchr(s, tolower(c));
    if (s == NULL || *s == 0 || c == 0) return NULL;
    if (tmp == NULL) tmp = strchr(s, toupper(c));

    return (char *)tmp;
}

#else
    #define bhapi   ::strcasechr(a, b) strcasechr(a, b)
#endif
#ifndef HAVE_STRRCASECHR

//-----------------------------------------------------------------------------

BHAPI_EXPORT char *strrcasechr(const char *s, int c)
{
    const char  *tmp = strrchr(s, tolower(c));
    if (s == NULL || *s == 0 || c == 0) return NULL;
    if (tmp == NULL) tmp = strrchr(s, toupper(c));

    return (char *)tmp;
}
//-------------------------------------------------------------------------------------------------
#else
    #define bhapi   ::strrcasechr(a, b) strrcasechr(a, b)
#endif
#define MAX_STRING_LENGTH   (B_MAXINT32 - 1)
//-------------------------------------------------------------------------------------------------
