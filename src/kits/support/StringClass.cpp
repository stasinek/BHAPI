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
#include "StringClass.h"
#include "StringArray.h"
#include "../../private/support/StringPrivate.h"
#include "Errors.h"
#include "UTF8.h"
#include "../kernel/Debug.h"

#undef __STRICT_ANSI__


#ifndef HAVE_VA_COPY
    #ifdef HAVE___VA_COPY
        #define va_copy(a, b)   __va_copy(a, b)
    #else
        #if defined(_MSC_VER) || defined(__MWERKS__)
            #define va_copy(a, b)   ((a) = (b))
        #endif
    #endif // HAVE___VA_COPY
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

int __cdecl bhapi::strncasecmp(const char *a, const char *b, size_t c)
{
    return _strnicmp(a, b, c);
}

//-----------------------------------------------------------------------------

int __cdecl bhapi::strcasecmp(const char *a, const char *b)
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

BHAPI_EXPORT char *bhapi::strdup_dirty(const char *src,  __be_int32 length)
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

char *bhapi::strcasestr(const char *haystack, const char *needle)
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
        if (bhapi::strncasecmp(tmp, needle, needle_len) == 0) return (char *)tmp;
        tmp++;
    }

    return NULL;
}

#else
    #define bhapi   ::strcasestr(a, b) strcasestr(a, b)
#endif
#ifndef HAVE_STRRSTR

//-----------------------------------------------------------------------------

char *bhapi::strrstr(const char *haystack, const char *needle)
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

char *bhapi::strrcasestr(const char *haystack, const char *needle)
{
    size_t needle_len = strlen(needle);
    size_t haystack_len = strlen(haystack);
    const char  *tmp = haystack + haystack_len - needle_len;

    if (!haystack || !needle) return NULL;
    if (needle_len == 0) return (char *)haystack;
    if (haystack_len < needle_len) return NULL;

    while (*tmp && tmp >= haystack)
    {
        if (bhapi::strncasecmp(tmp, needle, needle_len) == 0) return (char *)tmp;
        tmp--;
    }

    return NULL;
}

#else
    #define bhapi   ::strrcasestr(a, b) strrcasestr(a, b)
#endif
#ifndef HAVE_STRCASECHR

//-----------------------------------------------------------------------------

char *bhapi::strcasechr(const char *s, int c)
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

char *bhapi::strrcasechr(const char *s, int c)
{
    const char  *tmp = strrchr(s, tolower(c));
    if (s == NULL || *s == 0 || c == 0) return NULL;
    if (tmp == NULL) tmp = strrchr(s, toupper(c));

    return (char *)tmp;
}

#else
    #define bhapi   ::strrcasechr(a, b) strrcasechr(a, b)
#endif
#define MAX_STRING_LENGTH   (B_MAXINT32 - 1)

//-----------------------------------------------------------------------------

class BString::PosVect {
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
            if (fBuffer != NULL)
                fBufferSize *= 2;

            int32* newBuffer = NULL;
            newBuffer = (int32*)realloc(fBuffer, fBufferSize * sizeof(int32));
            if (newBuffer == NULL)
                return false;

            fBuffer = newBuffer;
        }

        fBuffer[fSize++] = pos;

        return true;
    }

    BHAPI_EXPORT int32 ItemAt(int32 index) const
    {
        return fBuffer[index];
    }

    BHAPI_EXPORT int32 CountItems() const
    {
        return fSize;
    }

private:
    int32	fSize;
    int32	fBufferSize;
    int32*	fBuffer;
};


//	#pragma mark - BStringRef
//-----------------------------------------------------------------------------

BStringRef::BStringRef(BString& string, int32 position)
    :
    fString(string), fPosition(position)
{
}
//-----------------------------------------------------------------------------

BStringRef::operator char() const
{
    return fPosition < fString.Length() ? fString.fPrivateData[fPosition] : 0;
}
//-----------------------------------------------------------------------------

BStringRef& BStringRef::operator=(char c)
{
    fString._MakeWritable();
    fString.fPrivateData[fPosition] = c;
    return *this;
}
//-----------------------------------------------------------------------------

BStringRef& BStringRef::operator=(const BStringRef &rc)
{
    return operator=(rc.fString.fPrivateData[rc.fPosition]);
}
//-----------------------------------------------------------------------------

const char* BStringRef::operator&() const
{
    return &fString.fPrivateData[fPosition];
}
//-----------------------------------------------------------------------------

char* BStringRef::operator&()
{
    if (fString._MakeWritable() != B_OK)
        return NULL;

    fString._ReferenceCount() = -1;
        // mark as unsharable
    return &fString.fPrivateData[fPosition];
}
//-----------------------------------------------------------------------------

bool BString::SetMinimumBufferSize(__be_int32 length)
{
    char    *newData = (char *)realloc(fBuffer, length);
    if (length > MAX_STRING_LENGTH + 1) return false;

    if (length <= 0 || length <= fLenReal)
    {
        fMinBufferSize = length;
        return true;
    }

    if (!newData) return false;

    fMinBufferSize = length;
    fBuffer = newData;
    fBuffer[max_c(fLen, 0)] = 0;
    fLenReal = length;

    return true;
}

//-----------------------------------------------------------------------------

__be_int32 BString::MinimumBufferSize(void) const
{
    return fMinBufferSize;
}

//-----------------------------------------------------------------------------

bool BString::_ResizeBool(__be_int32 length)
{
     __be_int32 length_to_alloc = max_c(length + 1, fMinBufferSize);
    char    *newData = (char *)realloc(fBuffer, length_to_alloc);
    if (length <= 0)
    {
        if (fMinBufferSize > 0)
        {
            if (fLenReal != fMinBufferSize)
            {
                newData = (char *)realloc(fBuffer, fMinBufferSize);
                if (newData)
                {
                    fBuffer = newData;
                    fLenReal = fMinBufferSize;
                }
            }

            fLen = 0;
            if (fBuffer)
                fBuffer[0] = 0;
            else
                fLenReal = 0;
        }
        else
        {
            if (fBuffer) free(fBuffer);
            fBuffer = NULL;
            fLen = 0;
            fLenReal = 0;
        }

        return true;
    }
    else if (length == fLenReal - 1)
    {
        fLen = length;
        return true;
    }
    else if (length > MAX_STRING_LENGTH)
    {
        return false;
    }
    else if (length < fMinBufferSize && fMinBufferSize == fLenReal)
    {
        fLen = length;
        fBuffer[fLen] = 0;
        return true;
    }

    if (!newData && length + 1 < fMinBufferSize)
    {
        length_to_alloc = length + 1;
        newData = (char *)realloc(fBuffer, length_to_alloc);
    }

    if (!newData)
    {
        if (length < fLenReal)
        {
            fLen = length;
            fBuffer[fLen] = 0;
            return true;
        }
        else
        {
            return false;
        }
    }

    newData[length] = 0;
    fBuffer = newData;
    fLen = length;
    fLenReal = length_to_alloc;

    return true;
}

//-----------------------------------------------------------------------------

BString::BString(void) :
fLen(0),
fLenReal(0),
fMinBufferSize(0),
fBuffer(NULL)
{ }

//-----------------------------------------------------------------------------

BString::BString(const char *str) :
fLen(0),
fLenReal(0),
fMinBufferSize(0),
fBuffer(NULL)
{
    Append(str);
}

//-----------------------------------------------------------------------------

BString::BString(const BString &str) :
fLen(0),
fLenReal(0),
fMinBufferSize(0),
fBuffer(NULL)
{
    Append(str);
}

//-----------------------------------------------------------------------------

BString::BString(const char *str,  __be_int32 maxLength) :
fLen(0),
fLenReal(0),
fMinBufferSize(0),
fBuffer(NULL)
{
    Append(str, maxLength);
}

//-----------------------------------------------------------------------------

BString::~BString(void)
{
    if (fBuffer) free(fBuffer);
}

//-----------------------------------------------------------------------------

BString &BString::operator=(const BString &str)
{
    this->SetTo(str);
    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::operator=(const char *str)
{
    this->SetTo(str);
    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::operator=(char c)
{
    this->SetTo(c, 1);
    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::SetTo(const BString &str)
{
    MakeEmpty();
    return Append(str);
}

//-----------------------------------------------------------------------------

BString &BString::SetTo(const BString &str,  __be_int32 length)
{
    MakeEmpty();
    return Append(str, length);
}

//-----------------------------------------------------------------------------

BString &BString::SetTo(const char *str,  __be_int32 length)
{
    MakeEmpty();
    return Append(str, length);
}

//-----------------------------------------------------------------------------

BString &BString::SetTo(char c,  __be_int32 count)
{
    MakeEmpty();
    return Append(c, count);
}

//-----------------------------------------------------------------------------

BString &BString::Adopt(BString &from)
{
    Append(from);
    from.MakeEmpty();

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::Adopt(BString &from,  __be_int32 length)
{
    Append(from, length);
    from.MakeEmpty();

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::SetToChars(const char *string, int32 charCount)
{
    return SetTo(string, bhapi::UTF8CountBytes(string, charCount));
}

//-----------------------------------------------------------------------------

BString &BString::SetToChars(const BString &string, int32 charCount)
{
    return SetTo(string, bhapi::UTF8CountBytes(string.String(), charCount));
}

//-----------------------------------------------------------------------------

BString &BString::AdoptChars(BString &string, int32 charCount)
{
    return Adopt(string, bhapi::UTF8CountBytes(string.String(), charCount));
}

//-----------------------------------------------------------------------------

BString &BString::CopyInto(BString &into,  __be_int32 fromOffset,  __be_int32 length) const
{
    if (this == &into)
    {
        BString str = *this;
        if (str == *this) str.CopyInto(into, fromOffset, length);
    }
    else if (fromOffset >= 0 && fromOffset < fLen)
    {
        into.MakeEmpty();
        into.Append(fBuffer + fromOffset, length);
    }

    return into;
}

//-----------------------------------------------------------------------------

void BString::CopyInto(char *into, size_t into_size,  __be_int32 fromOffset,  __be_int32 length) const
{
    if (!into || into_size <= 0) return;
    if (fromOffset < 0 || fromOffset >= fLen) return;
    if (length < 0 || length > fLen - fromOffset) length = fLen - fromOffset;

    if (fBuffer && fromOffset >= 0 && fromOffset < fLen && length > 0)
        strncpy(into, fBuffer + fromOffset, min_c(into_size, (size_t) length));
    else
        into[0] = '\0';
}

//-----------------------------------------------------------------------------

BString &BString::MoveInto(BString &into,  __be_int32 from,  __be_int32 length)
{
    if (this == &into)
    {
        BString str = *this;
        if (str == *this) str.MoveInto(into, from, length);
    }
    else if (from >= 0 && from < fLen)
    {
        into.MakeEmpty();
        into.Append(fBuffer + from, length);

        Remove(from, length);
    }

    return into;
}

//-----------------------------------------------------------------------------

void BString::MoveInto(char *into, size_t into_size,  __be_int32 from,  __be_int32 length)
{
    if (!into || into_size <= 0) return;
    if (from < 0 || from >= fLen) return;
    if (length < 0 || length > fLen - from) length = fLen - from;

    if (fBuffer && from >= 0 && from < fLen)
    {
        strncpy(into, fBuffer + from, min_c(into_size, (size_t) length));
        Remove(from, length);
    }
    else
    {
        into[0] = '\0';
    }
}

//-----------------------------------------------------------------------------

bool BString::Split(const char *separator, bool noEmptyStrings, BStringArray &_list) const
{
    int32   separatorLength = strlen(separator);
    int32   length = Length();
    int32   index = 0;
    if (separatorLength == 0 || length == 0 || separatorLength > length)
    {
        if (length == 0 && noEmptyStrings) return true;
        return _list.Add(*this);
    }

    for (;;)
    {
        int32   endIndex = index < length ? FindFirst(separator, index) : length;
        if (endIndex < 0) endIndex = length;

        if (endIndex > index || !noEmptyStrings)
        {
            BString toAppend(String() + index, endIndex - index);
            if (toAppend.Length() != endIndex - index || !_list.Add(toAppend))
            {
                return false;
            }
        }

        if (endIndex == length) break;

        index = endIndex + 1;
    }

    return true;
}

BHAPI_EXPORT int32 BString::Length() const
{
    // the most significant bit is reserved; accessing
    // it in any way will cause the computer to explode
    return (int32)fPrivateData ? (*(((int32*)fPrivateData) - 1) & 0x7fffffff) : 0;
}

BHAPI_EXPORT bool BString::IsEmpty() const
{
    return !Length();
}

BHAPI_EXPORT const char* BString::String() const
{
    if (!fPrivateData)
        return "";
    return fPrivateData;
}

BHAPI_EXPORT uint32 BString::HashValue() const
{
    return HashValue(String());
}

BHAPI_EXPORT BString& BString::SetTo(const char* string)
{
    return operator=(string);
}

BHAPI_EXPORT BString::operator const char*() const
{
    return String();
}

BHAPI_EXPORT bool operator<(const char* str, const BString& string)
{
    return string > str;
}

BHAPI_EXPORT bool operator<=(const char* str, const BString& string)
{
    return string >= str;
}

BHAPI_EXPORT bool operator==(const char* str, const BString& string)
{
    return string == str;
}

BHAPI_EXPORT bool operator>(const char* str, const BString& string)
{
    return string < str;
}

BHAPI_EXPORT bool operator>=(const char* str, const BString& string)
{
    return string <= str;
}

BHAPI_EXPORT bool operator!=(const char* str, const BString& string)
{
    return string != str;
}
//-----------------------------------------------------------------------------

void BString::MakeEmpty(void)
{
    Remove(0, -1);
}

//-----------------------------------------------------------------------------

BString &BString::operator+=(const BString &str)
{
    return Append(str);
}

//-----------------------------------------------------------------------------

BString &BString::operator+=(const char *str)
{
    return Append(str);
}

//-----------------------------------------------------------------------------

BString &BString::operator+=(char c)
{
    return Append(c, 1);
}

//-----------------------------------------------------------------------------

BString &BString::Append(const BString &str)
{
    return Append(str, str.Length());
}

//-----------------------------------------------------------------------------

BString &BString::Append(const BString &str,  __be_int32 length)
{
    if (str.Length() < 1 || length == 0) return *this;
    if (length < 0) length = str.Length();

    return Append(str.String(), length);
}

//-----------------------------------------------------------------------------

BString &BString::Append(const char *str)
{
    if (str) Append(str, -1);

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::Append(const char *str,  __be_int32 length)
{
     __be_int32 strLen = (__be_int32) strlen(str);

    if (str == NULL || *str == 0 || length == 0) return *this;
    if (length < 0 || length > strLen) length = strLen;
    if (MAX_STRING_LENGTH - fLen < length) return *this;

    if (_Resize(fLen + length))
    {
        if (memcpy(fBuffer + fLen - length, str, length) == NULL)
        {
            fLen -= length;
        }
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::Append(char c,  __be_int32 count)
{
    if (c == 0 || MAX_STRING_LENGTH - fLen < count || count <= 0) return *this;

    if (_Resize(fLen + count))
    {
        if (memset(fBuffer + fLen - count, (int)c, count) == NULL)
        {
            fLen -= count;
        }
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::Prepend(const BString &str)
{
    return Prepend(str, str.Length());
}

//-----------------------------------------------------------------------------

BString &BString::Prepend(const BString &str,  __be_int32 length)
{
    if (str.Length() < 1 || length == 0) return *this;
    if (length < 0) length = str.Length();

    return Prepend(str.String(), length);
}

//-----------------------------------------------------------------------------

BString &BString::Prepend(const char *str)
{
    if (str) Prepend(str, -1);

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::Prepend(const char *str,  __be_int32 length)
{
    return Insert(str, length, 0);
}

//-----------------------------------------------------------------------------

BString &BString::Prepend(char c,  __be_int32 count)
{
    return Insert(c, count, 0);
}

//-----------------------------------------------------------------------------

BString &BString::Insert(const BString &str,  __be_int32 pos)
{
    return Insert(str, str.Length(), pos);
}

//-----------------------------------------------------------------------------

BString &BString::Insert(const BString &str,  __be_int32 length,  __be_int32 pos)
{
    return Insert(str, 0, length, pos);
}

//-----------------------------------------------------------------------------

BString &BString::Insert(const BString &str,  __be_int32 fromOffset,  __be_int32 length,  __be_int32 pos)
{
    return Insert(str.String(), fromOffset, length, pos);
}

//-----------------------------------------------------------------------------

BString &BString::Insert(const char *str,  __be_int32 pos)
{
    if (str) Insert(str, -1, pos);
    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::Insert(const char *str,  __be_int32 length,  __be_int32 pos)
{
    return Insert(str, 0, length, pos);
}

//-----------------------------------------------------------------------------

BString &BString::Insert(const char *str,  __be_int32 fromOffset,  __be_int32 length,  __be_int32 pos)
{
     __be_int32 strLen = (__be_int32) strlen(str);
    if (str == NULL || *str == 0 || length == 0 || fromOffset < 0) return *this;
    if (fromOffset >= strLen) return *this;
    if ((pos == 0 && fLen <= 0) || (pos > 0 && pos == fLen)) return Append(str + fromOffset, length);
    if (pos < 0 || pos >= fLen) return *this;
    if (length < 0 || length > strLen - fromOffset) length = strLen - fromOffset;
    if (MAX_STRING_LENGTH - fLen < length) return *this;
    if (!_Resize(fLen + length)) return *this;

    if (memmove(fBuffer + pos + length, fBuffer + pos, fLen - length - pos) == NULL)
    {
        fLen -= length;
        return *this;
    }
    else if (memcpy(fBuffer + pos, str + fromOffset, length) == NULL)
    {
        fLen -= length;
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::Insert(char c,  __be_int32 count,  __be_int32 pos)
{
    if ((pos == 0 && fLen <= 0) || (pos > 0 && pos == fLen)) return Append(c, count);
    if (pos < 0 || pos >= fLen) return *this;
    if (c == 0 || MAX_STRING_LENGTH - fLen < count || count <= 0) return *this;
    if (!_Resize(fLen + count)) return *this;

    if (memmove(fBuffer + pos + count, fBuffer + pos, fLen - count - pos) == NULL)
    {
        fLen -= count;
        return *this;
    }
    else if (memset(fBuffer + pos, (int)c, count) == NULL)
    {
        fLen -= count;
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::Truncate(__be_int32 newLength)
{
    if (!(newLength < 0 || newLength >= fLen)) _Resize(newLength);

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::Remove(__be_int32 from,  __be_int32 length)
{
    if (from < 0 || from >= fLen) return *this;

    if (length == 0)
        return *this;
    else if (length < 0)
        length = fLen - from;
    else
        length = min_c(fLen - from, length);

    if (from < (fLen - 1) && length != (fLen - from))
    {
        if (memmove(fBuffer + from, fBuffer + from + length, fLen - from - length) == NULL) return *this;
    }

    _Resize(fLen - length);

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::RemoveFirst(const BString &str)
{
    return RemoveFirst(str.String());
}

//-----------------------------------------------------------------------------

BString &BString::RemoveLast(const BString &str)
{
    return RemoveLast(str.String());
}

//-----------------------------------------------------------------------------

BString &BString::RemoveAll(const BString &str)
{
    return RemoveAll(str.String());
}

//-----------------------------------------------------------------------------

BString &BString::RemoveFirst(const char *str)
{
    const char  *s = this->String();
    if (str == NULL || *str == 0) return *this;

    if (s)
    {
        const char  *tmp = strstr(s, str);
        if (tmp)
        {
             __be_int32 start = (__be_int32) (tmp - s);
            Remove(start, (__be_int32) strlen(str));
        }
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::RemoveLast(const char *str)
{
    const char  *s = this->String();
    if (str == NULL || *str == 0) return *this;

    if (s)
    {
        const char  *tmp = bhapi::strrstr(s, str);
        if (tmp)
        {
             __be_int32 start = (__be_int32) (tmp - s);
            Remove(start, (__be_int32) strlen(str));
        }
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::RemoveAll(const char *str)
{
    const char  *s = this->String();
    if (str == NULL || *str == 0) return *this;

    if (s)
    {
        const char  *tmp;
        while ((tmp = strstr(s, str)) != NULL)
        {
             __be_int32 start = (__be_int32) (tmp - s);
            Remove(start, (__be_int32) strlen(str));
            s = this->String();

            if (!s) break;
        }
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::RemoveSet(const char *setOfCharsToRemove)
{
     __be_int32 len = (__be_int32) strlen(setOfCharsToRemove);

    char    str[2];
    if (setOfCharsToRemove == NULL || *setOfCharsToRemove == 0) return *this;

    str[1] = '\0';

    for (__be_int32 i = 0; i < len; i++)
    {
        str[0] = setOfCharsToRemove[i];
        RemoveAll(str);

        if (Length() == 0) break;
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::IRemoveFirst(const BString &str)
{
    return IRemoveFirst(str.String());
}

//-----------------------------------------------------------------------------

BString &BString::IRemoveLast(const BString &str)
{
    return IRemoveLast(str.String());
}

//-----------------------------------------------------------------------------

BString &BString::IRemoveAll(const BString &str)
{
    return IRemoveAll(str.String());
}

//-----------------------------------------------------------------------------

BString &BString::IRemoveFirst(const char *str)
{
    const char  *s = this->String();
    if (str == NULL || *str == 0) return *this;

    if (s)
    {
        const char  *tmp = bhapi::strcasestr(s, str);
        if (tmp)
        {
             __be_int32 start = (__be_int32) (tmp - s);
            Remove(start, (__be_int32) strlen(str));
        }
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::IRemoveLast(const char *str)
{
    const char  *s = this->String();
    if (str == NULL || *str == 0) return *this;

    if (s)
    {
        const char  *tmp = bhapi::strrcasestr(s, str);
        if (tmp)
        {
             __be_int32 start = (__be_int32) (tmp - s);
            Remove(start, (__be_int32) strlen(str));
        }
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::IRemoveAll(const char *str)
{
    const char  *s = this->String();
    if (str == NULL || *str == 0) return *this;

    if (s)
    {
        const char  *tmp;
        while ((tmp = bhapi::strcasestr(s, str)) != NULL)
        {
             __be_int32 start = (__be_int32) (tmp - s);
            Remove(start, (__be_int32) strlen(str));
            s = this->String();

            if (!s) break;
        }
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::IRemoveSet(const char *setOfCharsToRemove)
{
     __be_int32 len = (__be_int32) strlen(setOfCharsToRemove);

    char    str[2];
    if (setOfCharsToRemove == NULL || *setOfCharsToRemove == 0) return *this;

    str[1] = 0;

    for (__be_int32 i = 0; i < len; i++)
    {
        str[0] = setOfCharsToRemove[i];
        IRemoveAll(str);

        if (Length() == 0) break;
    }

    return *this;
}

//-----------------------------------------------------------------------------

bool BString::operator<(const BString &str) const
{
    return(Compare(str) < 0);
}

//-----------------------------------------------------------------------------

bool BString::operator<=(const BString &str) const
{
    return(Compare(str) <= 0);
}

//-----------------------------------------------------------------------------

bool BString::operator==(const BString &str) const
{
    const char  *a = this->String();
    const char  *b = str.String();

    if (a == NULL && b == NULL) return true;
    if (a == NULL || b == NULL) return false;

    return(!(strlen(a) != strlen(b) || strcmp(a, b) != 0));
}

//-----------------------------------------------------------------------------

bool BString::operator>=(const BString &str) const
{
    return(Compare(str) >= 0);
}

//-----------------------------------------------------------------------------

bool BString::operator>(const BString &str) const
{
    return(Compare(str) > 0);
}

//-----------------------------------------------------------------------------

bool BString::operator!=(const BString &str) const
{
    return !operator ==(str);
}

//-----------------------------------------------------------------------------

bool BString::operator<(const char *str) const
{
    return(Compare(str) < 0);
}

//-----------------------------------------------------------------------------

bool BString::operator<=(const char *str) const
{
    return(Compare(str) <= 0);
}

//-----------------------------------------------------------------------------

bool BString::operator==(const char *str) const
{
    const char  *a = this->String();
    const char  *b = str;

    if (a == NULL && b == NULL) return true;
    if (a == NULL || b == NULL) return false;

    return(!(strlen(a) != strlen(b) || strcmp(a, b) != 0));
}

//-----------------------------------------------------------------------------

bool BString::operator>=(const char *str) const
{
    return(Compare(str) >= 0);
}

//-----------------------------------------------------------------------------

bool BString::operator>(const char *str) const
{
    return(Compare(str) > 0);
}

//-----------------------------------------------------------------------------

bool BString::operator!=(const char *str) const
{
    return !operator ==(str);
}

//-----------------------------------------------------------------------------

int BString::Compare(const BString &str) const
{
    const char  *a = "";
    const char  *b = "";

    if (this->String()) a = this->String();
    if (str.String()) b = str.String();

    return strcmp(a, b);
}

//-----------------------------------------------------------------------------

int BString::Compare(const char *str) const
{
    const char  *a = "";
    const char  *b = "";

    if (this->String()) a = this->String();
    if (str) b = str;

    return strcmp(a, b);
}

//-----------------------------------------------------------------------------

int BString::Compare(const BString &str,  __be_int32 n) const
{
    const char  *a = "";
    const char  *b = "";

    if (this->String()) a = this->String();
    if (str.String()) b = str.String();

    return strncmp(a, b, (size_t) n);
}

//-----------------------------------------------------------------------------

int BString::Compare(const char *str,  __be_int32 n) const
{
    const char  *a = "";
    const char  *b = "";

    if (this->String()) a = this->String();
    if (str) b = str;

    return strncmp(a, b, (size_t) n);
}

//-----------------------------------------------------------------------------

int BString::ICompare(const BString &str) const
{
    const char  *a = "";
    const char  *b = "";

    if (this->String()) a = this->String();
    if (str.String()) b = str.String();

    return strcasecmp(a, b);
}

//-----------------------------------------------------------------------------

int BString::ICompare(const char *str) const
{
    const char  *a = "";
    const char  *b = "";

    if (this->String()) a = this->String();
    if (str) b = str;

    return strcasecmp(a, b);
}

//-----------------------------------------------------------------------------

int BString::ICompare(const BString &str,  __be_int32 n) const
{
    const char  *a = "";
    const char  *b = "";

    if (this->String()) a = this->String();
    if (str.String()) b = str.String();

    return strncasecmp(a, b, (size_t) n);
}

//-----------------------------------------------------------------------------

int BString::ICompare(const char *str,  __be_int32 n) const
{
    const char  *a = "";
    const char  *b = "";

    if (this->String()) a = this->String();
    if (str) b = str;

    return strncasecmp(a, b, (size_t) n);
}

//-----------------------------------------------------------------------------

__be_int32 BString::FindFirst(const BString &string) const
{
    const char  *tmp = strstr(this->String(), string.String());

    if (this->String() == NULL || string.String() == NULL) return -1;
    if (tmp == NULL) return -1;

    return((__be_int32) (tmp - this->String()));
}

//-----------------------------------------------------------------------------

__be_int32 BString::FindFirst(const char *string) const
{
    const char  *tmp = strstr(this->String(), string);

    if (this->String() == NULL || string == NULL || *string == 0) return -1;
    if (tmp == NULL) return -1;

    return((__be_int32) (tmp - this->String()));
}

//-----------------------------------------------------------------------------

__be_int32 BString::FindFirst(const BString &string,  __be_int32 fromOffset) const
{
    const char  *string_find = this->String() + fromOffset;
    const char  *tmp = strstr(string_find, string.String());

    if (this->String() == NULL || string.String() == NULL || fromOffset < 0 || fromOffset >= Length()) return -1;
    if (tmp == NULL) return -1;

    return((__be_int32) (tmp - this->String()));
}

//-----------------------------------------------------------------------------

__be_int32 BString::FindFirst(const char *string,  __be_int32 fromOffset) const
{
    const char  *string_find = this->String() + fromOffset;
    const char  *tmp = strstr(string_find, string);

    if (this->String() == NULL || string == NULL || *string == 0 || fromOffset < 0 || fromOffset >= Length())
        return -1;

    if (tmp == NULL) return -1;

    return((__be_int32) (tmp - this->String()));
}

//-----------------------------------------------------------------------------

__be_int32 BString::FindFirst(char c) const
{
    const char  *tmp = strchr(this->String(), c);

    if (this->String() == NULL || c == 0) return -1;
    if (tmp == NULL) return -1;

    return((__be_int32) (tmp - this->String()));
}

//-----------------------------------------------------------------------------

__be_int32 BString::FindFirst(char c,  __be_int32 fromOffset) const
{
    const char  *string_find = this->String() + fromOffset;
    const char  *tmp = strchr(string_find, c);

    if (this->String() == NULL || c == 0 || fromOffset < 0 || fromOffset >= Length()) return -1;
    if (tmp == NULL) return -1;

    return((__be_int32) (tmp - this->String()));
}

//-----------------------------------------------------------------------------

__be_int32 BString::FindLast(const BString &string) const
{
    const char  *tmp = bhapi::strrstr(this->String(), string.String());

    if (this->String() == NULL || string.String() == NULL) return -1;
    if (tmp == NULL) return -1;

    return((__be_int32) (tmp - this->String()));
}

//-----------------------------------------------------------------------------

__be_int32 BString::FindLast(const char *string) const
{
    const char  *tmp = bhapi::strrstr(this->String(), string);

    if (this->String() == NULL || string == NULL || *string == 0) return -1;
    if (tmp == NULL) return -1;

    return((__be_int32) (tmp - this->String()));
}

//-----------------------------------------------------------------------------

__be_int32 BString::FindLast(const BString &string,  __be_int32 beforeOffset) const
{
    char *string_find = bhapi::strndup(this->String(), beforeOffset + 1);
    const char  *tmp = bhapi::strrstr(string_find, string.String());
     __be_int32     ret = -1;

    if (this->String() == NULL || string.String() == NULL || beforeOffset < 0 || beforeOffset >= Length()) return -1;
    if (string_find == NULL) return -1;
    if (tmp != NULL) ret = (__be_int32) (tmp - string_find);

    free(string_find);

    return ret;
}

//-----------------------------------------------------------------------------

__be_int32 BString::FindLast(const char *string,  __be_int32 beforeOffset) const
{
    char *string_find = bhapi::strndup(this->String(), beforeOffset + 1);
     __be_int32     ret = -1;
    const char  *tmp = bhapi::strrstr(string_find, string);

    if (this->String() == NULL || string == NULL || *string == 0 || beforeOffset < 0 || beforeOffset >= Length())
        return -1;

    if (string_find == NULL) return -1;
    if (tmp != NULL) ret = (__be_int32) (tmp - string_find);

    free(string_find);

    return ret;
}

//-----------------------------------------------------------------------------

__be_int32 BString::FindLast(char c) const
{
    const char  *tmp = strrchr(this->String(), c);

    if (this->String() == 0 || c == 0) return -1;
    if (tmp == NULL) return -1;

    return((__be_int32) (tmp - this->String()));
}

//-----------------------------------------------------------------------------

__be_int32 BString::FindLast(char c,  __be_int32 beforeOffset) const
{
    char *string_find = bhapi::strndup(this->String(), beforeOffset + 1);
    const char  *tmp = strrchr(string_find, c);
     __be_int32     ret = -1;

    if (this->String() == NULL || c == 0 || beforeOffset < 0 || beforeOffset >= Length()) return -1;
    if (string_find == NULL) return -1;
    if (tmp != NULL) ret = (__be_int32) (tmp - string_find);

    free(string_find);
    return ret;
}

//-----------------------------------------------------------------------------

__be_int32 BString::IFindFirst(const BString &string) const
{
    const char  *tmp = bhapi::strcasestr(this->String(), string.String());

    if (this->String() == NULL || string.String() == NULL) return -1;
    if (tmp == NULL) return -1;

    return((__be_int32) (tmp - this->String()));
}

//-----------------------------------------------------------------------------

__be_int32 BString::IFindFirst(const char *string) const
{
    const char  *tmp = bhapi::strcasestr(this->String(), string);

    if (this->String() == NULL || string == NULL || *string == 0) return -1;
    if (tmp == NULL) return -1;

    return((__be_int32) (tmp - this->String()));
}

//-----------------------------------------------------------------------------

__be_int32 BString::IFindFirst(const BString &string,  __be_int32 fromOffset) const
{
    const char  *string_find = this->String() + fromOffset;
    const char  *tmp = bhapi::strcasestr(string_find, string.String());

    if (this->String() == NULL || string.String() == NULL || fromOffset < 0 || fromOffset >= Length()) return -1;
    if (tmp == NULL) return -1;

    return((__be_int32) (tmp - this->String()));
}

//-----------------------------------------------------------------------------

__be_int32 BString::IFindFirst(const char *string,  __be_int32 fromOffset) const
{
    const char  *string_find = this->String() + fromOffset;
    const char  *tmp = bhapi::strcasestr(string_find, string);

    if (this->String() == NULL || string == NULL || *string == 0 || fromOffset < 0 || fromOffset >= Length())
        return -1;

    if (tmp == NULL) return -1;

    return((__be_int32) (tmp - this->String()));
}

//-----------------------------------------------------------------------------

__be_int32 BString::IFindFirst(char c) const
{
    const char  *tmp = bhapi::strcasechr(this->String(), c);

    if (this->String() == NULL || c == 0) return -1;
    if (tmp == NULL) return -1;

    return((__be_int32) (tmp - this->String()));
}

//-----------------------------------------------------------------------------

__be_int32 BString::IFindFirst(char c,  __be_int32 fromOffset) const
{
    const char  *string_find = this->String() + fromOffset;
    const char  *tmp = bhapi::strcasechr(string_find, c);

    if (this->String() == NULL || c == 0 || fromOffset < 0 || fromOffset >= Length()) return -1;
    if (tmp == NULL) return -1;

    return((__be_int32) (tmp - this->String()));
}

//-----------------------------------------------------------------------------

__be_int32 BString::IFindLast(const BString &string) const
{
    const char  *tmp = bhapi::strrcasestr(this->String(), string.String());

    if (this->String() == NULL || string.String() == NULL) return -1;
    if (tmp == NULL) return -1;

    return((__be_int32) (tmp - this->String()));
}

//-----------------------------------------------------------------------------

__be_int32 BString::IFindLast(const char *string) const
{
    const char  *tmp = bhapi::strrcasestr(this->String(), string);

    if (this->String() == NULL || string == NULL || *string == 0) return -1;
    if (tmp == NULL) return -1;

    return((__be_int32) (tmp - this->String()));
}

//-----------------------------------------------------------------------------

__be_int32 BString::IFindLast(const BString &string,  __be_int32 beforeOffset) const
{
    char *string_find = bhapi::strndup(this->String(), beforeOffset + 1);
    const char  *tmp = bhapi::strrcasestr(string_find, string.String());
     __be_int32     ret = -1;
    if (this->String() == NULL || string.String() == NULL || beforeOffset < 0 || beforeOffset >= Length()) return -1;
    if (string_find == NULL) return -1;
    if (tmp != NULL) ret = (__be_int32) (tmp - string_find);

    free(string_find);

    return ret;
}

//-----------------------------------------------------------------------------

__be_int32 BString::IFindLast(const char *string,  __be_int32 beforeOffset) const
{
    char *string_find = bhapi::strndup(this->String(), beforeOffset + 1);
    const char  *tmp = bhapi::strrcasestr(string_find, string);
     __be_int32     ret = -1;
    if (this->String() == NULL || string == NULL || *string == 0 || beforeOffset < 0 || beforeOffset >= Length())
        return -1;

    if (string_find == NULL) return -1;
    if (tmp != NULL) ret = (__be_int32) (tmp - string_find);

    free(string_find);

    return ret;
}

//-----------------------------------------------------------------------------

__be_int32 BString::IFindLast(char c) const
{
    const char  *tmp = bhapi::strrcasechr(this->String(), c);

    if (this->String() == NULL || c == 0) return -1;
    if (tmp == NULL) return -1;

    return((__be_int32) (tmp - this->String()));
}

//-----------------------------------------------------------------------------

__be_int32 BString::IFindLast(char c,  __be_int32 beforeOffset) const
{
    char *string_find = bhapi::strndup(this->String(), beforeOffset + 1);
    const char  *tmp = bhapi::strrcasechr(string_find, c);
     __be_int32     ret = -1;
    if (this->String() == NULL || c == 0 || beforeOffset < 0 || beforeOffset >= Length()) return -1;
    if (string_find == NULL) return -1;
    if (tmp != NULL) ret = (__be_int32) (tmp - string_find);

    free(string_find);

    return ret;
}

//-----------------------------------------------------------------------------

BString &BString::ReplaceFirst(char replaceThis, char withThis)
{
     __be_int32 index = FindFirst(replaceThis);

    if (index >= 0) fBuffer[index] = withThis;

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::ReplaceLast(char replaceThis, char withThis)
{
     __be_int32 index = FindLast(replaceThis);

    if (index >= 0) fBuffer[index] = withThis;

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::ReplaceAll(char replaceThis, char withThis,  __be_int32 fromOffset)
{
    if (fromOffset < 0 || fromOffset >= fLen) return *this;

    while (fromOffset < fLen)
    {
         __be_int32 index = FindFirst(replaceThis, fromOffset);

        if (index >= 0)
        {
            fBuffer[index] = withThis;
            fromOffset = index + 1;
        }
        else
            break;
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::ReplaceFirst(const char *replaceThis, const char *withThis)
{
     __be_int32 index = FindFirst(replaceThis);

    if (replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;

    if (index >= 0)
    {
        Remove(index, (__be_int32) strlen(replaceThis));
        Insert(withThis, index);
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::ReplaceLast(const char *replaceThis, const char *withThis)
{
     __be_int32 index = FindLast(replaceThis);

    if (replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;

    if (index >= 0)
    {
        Remove(index, (__be_int32) strlen(replaceThis));
        Insert(withThis, index);
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::ReplaceAll(const char *replaceThis, const char *withThis)
{
    return BString::ReplaceAll(replaceThis, withThis, 0);
}

//-----------------------------------------------------------------------------

BString &BString::ReplaceAll(const char *replaceThis, const char *withThis,  __be_int32 fromOffset)
{
     __be_int32 strLenReplace = (__be_int32) strlen(replaceThis);
     __be_int32 strLenWith = (__be_int32) strlen(withThis);

    if (fromOffset < 0 || fromOffset >= fLen) return *this;
    if (replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;

    while (fromOffset < fLen)
    {
         __be_int32 index = FindFirst(replaceThis, fromOffset);

        if (index >= 0)
        {
            Remove(index, strLenReplace);
            Insert(withThis, index);
            fromOffset = index + strLenWith;
        }
        else
            break;
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::Replace(const char *replaceThis, const char *withThis,  __be_int32 maxReplaceCount,  __be_int32 fromOffset)
{
    int     count = 0;
     __be_int32 strLenReplace = (__be_int32) strlen(replaceThis);
     __be_int32 strLenWith = (__be_int32) strlen(withThis);

    if (fromOffset < 0 || fromOffset >= fLen) return *this;
    if (replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;
    if (maxReplaceCount == 0) return *this;
    if (maxReplaceCount < 0) return ReplaceAll(replaceThis, withThis, fromOffset);

    while (true)
    {
         __be_int32 index = FindFirst(replaceThis, fromOffset);

        if (index >= 0)
        {
            Remove(index, strLenReplace);
            Insert(withThis, index);
            fromOffset = index + strLenWith;
            count++;
        }
        else
            break;

        if (count >= maxReplaceCount) break;
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::IReplaceFirst(char replaceThis, char withThis)
{
     __be_int32 index = IFindFirst(replaceThis);

    if (index >= 0) fBuffer[index] = withThis;

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::IReplaceLast(char replaceThis, char withThis)
{
     __be_int32 index = IFindLast(replaceThis);

    if (index >= 0) fBuffer[index] = withThis;

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::IReplaceAll(char replaceThis, char withThis,  __be_int32 fromOffset)
{
    if (fromOffset < 0 || fromOffset >= fLen) return *this;

    while (fromOffset < fLen)
    {
         __be_int32 index = IFindFirst(replaceThis, fromOffset);

        if (index >= 0)
        {
            fBuffer[index] = withThis;
            fromOffset = index + 1;
        }
        else
            break;
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::IReplaceFirst(const char *replaceThis, const char *withThis)
{
     __be_int32 index = IFindFirst(replaceThis);

    if (replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;

    if (index >= 0)
    {
        Remove(index, (__be_int32) strlen(replaceThis));
        Insert(withThis, index);
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::IReplaceLast(const char *replaceThis, const char *withThis)
{
     __be_int32 index = IFindLast(replaceThis);

    if (replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;

    if (index >= 0)
    {
        Remove(index, (__be_int32) strlen(replaceThis));
        Insert(withThis, index);
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::IReplaceAll(const char *replaceThis, const char *withThis,  __be_int32 fromOffset)
{
     __be_int32 strLenReplace = (__be_int32) strlen(replaceThis);
     __be_int32 strLenWith = (__be_int32) strlen(withThis);

    if (fromOffset < 0 || fromOffset >= fLen) return *this;
    if (replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;

    while (fromOffset < fLen)
    {
         __be_int32 index = IFindFirst(replaceThis, fromOffset);

        if (index >= 0)
        {
            Remove(index, strLenReplace);
            Insert(withThis, index);
            fromOffset = index + strLenWith;
        }
        else
            break;
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::IReplace(const char *replaceThis, const char *withThis,  __be_int32 maxReplaceCount,  __be_int32 fromOffset)
{
    int     count = 0;
     __be_int32 strLenReplace = (__be_int32) strlen(replaceThis);
     __be_int32 strLenWith = (__be_int32) strlen(withThis);

    if (fromOffset < 0 || fromOffset >= fLen) return *this;
    if (replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;
    if (maxReplaceCount == 0) return *this;
    if (maxReplaceCount < 0) return IReplaceAll(replaceThis, withThis, fromOffset);

    while (true)
    {
         __be_int32 index = IFindFirst(replaceThis, fromOffset);

        if (index >= 0)
        {
            Remove(index, strLenReplace);
            Insert(withThis, index);
            fromOffset = index + strLenWith;
            count++;
        }
        else
            break;

        if (count >= maxReplaceCount) break;
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::ReplaceSet(const char *set_to_replace, char with)
{
     __be_int32 length = (__be_int32) strlen(set_to_replace);
     __be_int32 offset = 0;

    if (set_to_replace == NULL || *set_to_replace == 0 || with == 0) return *this;

    while (true)
    {
         __be_int32 index = -1;
         __be_int32 i;

        for (i = 0; i < length; i++)
        {
            index = FindFirst(set_to_replace[i], offset);
            if (index >= 0) break;
        }

        if (index >= 0)
        {
            Remove(index, 1);
            Insert(with, 1, index);
            offset = index + 1;
        }
        else
            break;
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::ReplaceSet(const char *set_to_replace, const char *with)
{
     __be_int32 length = (__be_int32) strlen(set_to_replace);
     __be_int32 strLenWith = (__be_int32) strlen(with);
     __be_int32 offset = 0;

    if (set_to_replace == NULL || *set_to_replace == 0 || with == NULL || *with == 0) return *this;

    while (true)
    {
         __be_int32 index = -1;
         __be_int32 i;

        for (i = 0; i < length; i++)
        {
            index = FindFirst(set_to_replace[i], offset);
            if (index >= 0) break;
        }

        if (index >= 0)
        {
            Remove(index, 1);
            Insert(with, index);
            offset = index + strLenWith;
        }
        else
            break;
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::IReplaceSet(const char *set_to_replace, char with)
{
     __be_int32 length = (__be_int32) strlen(set_to_replace);
     __be_int32 offset = 0;

    if (set_to_replace == NULL || *set_to_replace == 0 || with == 0) return *this;

    while (true)
    {
         __be_int32 index = -1;
         __be_int32 i;

        for (i = 0; i < length; i++)
        {
            index = IFindFirst(set_to_replace[i], offset);
            if (index >= 0) break;
        }

        if (index >= 0)
        {
            Remove(index, 1);
            Insert(with, 1, index);
            offset = index + 1;
        }
        else
            break;
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::IReplaceSet(const char *set_to_replace, const char *with)
{
     __be_int32 length = (__be_int32) strlen(set_to_replace);
     __be_int32 strLenWith = (__be_int32) strlen(with);
     __be_int32 offset = 0;

    if (set_to_replace == NULL || *set_to_replace == 0 || with == NULL || *with == 0) return *this;

    while (true)
    {
         __be_int32 index = -1;
         __be_int32 i;

        for (i = 0; i < length; i++)
        {
            index = IFindFirst(set_to_replace[i], offset);
            if (index >= 0) break;
        }

        if (index >= 0)
        {
            Remove(index, 1);
            Insert(with, index);
            offset = index + strLenWith;
        }
        else
            break;
    }

    return *this;
}

//-----------------------------------------------------------------------------

char BString::operator[](__be_int32 index) const
{
    return fBuffer[index];
}

//-----------------------------------------------------------------------------

char BString::ByteAt(__be_int32 index) const
{
    if (!fBuffer || index < 0 || index >= Length()) return 0;
    return fBuffer[index];
}

//-----------------------------------------------------------------------------

BString &BString::ToLower(void)
{
    for (__be_int32 i = 0; i < fLen; i++) fBuffer[i] = tolower(fBuffer[i]);

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::ToUpper(void)
{
    for (__be_int32 i = 0; i < fLen; i++) fBuffer[i] = toupper(fBuffer[i]);

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::CharacterEscape(const char *original, const char *setOfCharsToEscape, char escapeWith)
{
    SetTo(original);
    return CharacterEscape(setOfCharsToEscape, escapeWith);
}

//-----------------------------------------------------------------------------

BString &BString::CharacterEscape(const char *setOfCharsToEscape, char escapeWith)
{
     __be_int32 length = (__be_int32) strlen(setOfCharsToEscape);
     __be_int32 offset = 0;

    if (Length() <= 0 || setOfCharsToEscape == NULL || *setOfCharsToEscape == 0 || escapeWith == 0) return *this;

    while (true)
    {
         __be_int32 index = -1;
         __be_int32 i;

        for (i = 0; i < length; i++)
        {
            index = FindFirst(setOfCharsToEscape[i], offset);
            if (index >= 0) break;
        }

        if (index >= 0)
        {
            Insert(escapeWith, 1, index);
            offset = index + 2;
        }
        else
            break;
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::CharacterDeescape(const char *original, char escapeChar)
{
    SetTo(original);
    return CharacterDeescape(escapeChar);
}

//-----------------------------------------------------------------------------

BString &BString::CharacterDeescape(char escapeChar)
{
     __be_int32 offset = 0;

    if (Length() <= 0 || escapeChar == 0) return *this;

    while (true)
    {
         __be_int32 index = -1;

        index = FindFirst(escapeChar, offset);

        if (index >= 0)
        {
            Remove(index, 1);
            offset = index + 1;
        }
        else
            break;
    }

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::Capitalize(void)
{
    ToLower();

    if (Length() > 0) fBuffer[0] = toupper(fBuffer[0]);

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::CapitalizeEachWord(void)
{
     __be_int32 length = Length();
    ToLower();

    if (length > 0)
    {
        char    *src = fBuffer;

        if (fBuffer[0] != ' ') fBuffer[0] = toupper(fBuffer[0]);

        while (true)
        {
            char    *tmp = strchr(src, ' ');

            if (!tmp) break;
            if (tmp - fBuffer >= length - 1) break;

            tmp++;
            if (*tmp != ' ')
            {
                *tmp = toupper(*tmp);
                src = (tmp + 1);
            }
            else
            {
                src = (tmp + 2);
            }
        }
    }

    return *this;
}

//-----------------------------------------------------------------------------

bool BString::IsNumber(void) const
{
    const char  *s = this->String();
     __be_int32     length = Length();

    const char  *tmp = strchr(s, '.');
    if (s == NULL) return false;
    if (length < 1) return false;

    if (!(length < 2 || s[0] != '0' || !(s[1] == 'x' || s[1] == 'X')))
    {
         __be_int32 i;
        for (i = 2; i < length; i++)
        {
            if (s[i] >= '0' && s[i] <= '9') continue;
            if (s[i] >= 'a' && s[i] <= 'f') continue;
            if (s[i] >= 'A' && s[i] <= 'F') continue;
            break;
        }

        return(i != 2 && i >= length);
    }

    if (s[0] == '-' || s[0] == '+')
    {
        if (length < 2) return false;
        if (strchr(&s[1], '-') != NULL || strchr(&s[1], '+') != NULL) return false; // double '-' or '+'
    }

    if (tmp != NULL)
    {
        tmp++;
        if (strchr(tmp, '.') != NULL) return false; // double '.'
    }

    for (__be_int32 i = 0; i < length; i++)
    {
        if (!(isdigit(s[i]) || s[i] == '-' || s[i] == '.' || s[i] == '+'))
        {
            if (s[i] != 'e' || s[i] != 'E' || i != length - 4) return false;
            if (s[i + 1] != '+' || s[i + 1] != '-') return false;
            if (!isdigit(s[i + 2]) || !isdigit(s[i + 3])) return false;
            break;
        }
    }

    return true;
}

//-----------------------------------------------------------------------------

bool BString::IsInteger(void) const
{
    const char  *s = strchr(this->String(), '.');

    if (!IsNumber()) return false;
    if (s == NULL) return true;

    if (strchr(s, '.') != NULL)
    {
         __be_int32 length = (__be_int32) strlen(s);
        s++;
        if (length > 0)
        {
            for (__be_int32 i = 0; i < length; i++)
            {
                if (s[i] != '0') return false;
            }
        }
    }

    return true;
}

//-----------------------------------------------------------------------------

bool BString::IsDecimal(void) const
{
    if (!IsNumber()) return false;

    return(!IsInteger());
}

namespace   bhapi
{
template<class TYPE_INT>
bool get_hex(const BString &str, TYPE_INT *value);
}
//-----------------------------------------------------------------------------

template<class TYPE_INT>
bool bhapi::get_hex(const BString &str, TYPE_INT *value)
{
     __be_int32     maxLen = 2 + (__be_int32) sizeof(TYPE_INT) * 2;
     __be_uint32    bitOffset = 0;

    if (value == NULL || str.Length() < 2 || str[0] != '0' || !(str[1] == 'x' || str[1] == 'X')) return false;

    *value = (TYPE_INT) 0;

    for (__be_int32 i = min_c(str.Length(), maxLen) - 1; i >= 2; i--, bitOffset += 4)
    {
        int v = 0;
        if (str[i] >= '0' && str[i] <= '9')
            v = str[i] - '0';
        else if (str[i] >= 'a' && str[i] <= 'f')
            v = str[i] - 'a' + 10;
        else
            v = str[i] - 'A' + 10;
        *value |= (((TYPE_INT) v) << bitOffset);
    }

    return true;
}

//-----------------------------------------------------------------------------

bool BString::GetDecimal(float *value) const
{
     __be_uint64    tmp;
    #if !(defined(_MSC_VER) && _MSC_VER <= 0x4b0)
    if (!value || !IsNumber()) return false;

    if (bhapi::get_hex(*this, &tmp))
    {
        *value = (float)tmp;
        return true;
    }

        #ifdef HAVE_STRTOF
    * value = strtof(this->String(), NULL);
        #else
    *value = (float)strtod(this->String(), NULL);
        #endif
    return true;

    #else

    // TODO
    return false;
    #endif
}

//-----------------------------------------------------------------------------

bool BString::GetDecimal(double *value) const
{
     __be_uint64    tmp;
    #if !(defined(_MSC_VER) && _MSC_VER <= 0x4b0)
    if (!value || !IsNumber()) return false;

    if (bhapi::get_hex(*this, &tmp))
    {
        *value = (double)tmp;
        return true;
    }

    *value = strtod(this->String(), NULL);

    return true;

    #else

    // TODO
    return false;
    #endif
}

#ifdef BHAPI_SUPPORT_LONG_DOUBLE

//-----------------------------------------------------------------------------

bool BString::GetDecimal(long double *value) const
{
     __be_uint64    tmp;
    if (!value || !IsNumber()) return false;

    if (bhapi::get_hex(*this, &tmp))
    {
        *value = (long double)tmp;
        return true;
    }

    *value = strtold(this->String(), NULL);

    return true;
}
#endif // BHAPI_SUPPORT_LONG_DOUBLE

//-----------------------------------------------------------------------------

bool BString::GetInteger(__be_int8 *value) const
{
    if (!value || !IsNumber()) return false;

    *value = (__be_int8) strtol(this->String(), NULL, 10);

    return true;
}

//-----------------------------------------------------------------------------

bool BString::GetInteger(__be_uint8 *value) const
{
    if (!value || !IsNumber()) return false;
    if (bhapi::get_hex(*this, value)) return true;

    *value = (__be_uint8) strtoul(this->String(), NULL, 10);

    return true;
}

//-----------------------------------------------------------------------------

bool BString::GetInteger(__be_int16 *value) const
{
    if (!value || !IsNumber()) return false;

    *value = (__be_int16) strtol(this->String(), NULL, 10);

    return true;
}

//-----------------------------------------------------------------------------

bool BString::GetInteger(__be_uint16 *value) const
{
    if (!value || !IsNumber()) return false;
    if (bhapi::get_hex(*this, value)) return true;

    *value = (__be_uint16) strtoul(this->String(), NULL, 10);

    return true;
}

//-----------------------------------------------------------------------------

bool BString::GetInteger(__be_int32 *value) const
{
    if (!value || !IsNumber()) return false;
    if (bhapi::get_hex(*this, value)) return true;

    *value = (__be_int32) strtol(this->String(), NULL, 10);

    return true;
}

//-----------------------------------------------------------------------------

bool BString::GetInteger(__be_uint32 *value) const
{
    if (!value || !IsNumber()) return false;
    if (bhapi::get_hex(*this, value)) return true;

    *value = (__be_uint32) strtoul(this->String(), NULL, 10);

    return true;
}

//-----------------------------------------------------------------------------

bool BString::GetInteger(__be_int64 *value) const
{
    if (!value || !IsNumber()) return false;
    if (bhapi::get_hex(*this, value)) return true;

    #if SIZEOF_INT == 8 || SIZEOF_LONG == 8
    * value = (__be_int64) strtol(this->String(), NULL, 10);
    return true;
    #else
        #if SIZEOF_LONG_LONG == 8 && !defined(_MSC_VER)
    * value = (__be_int64) strtoll(this->String(), NULL, 10);
    return true;
        #else
            #if SIZEOF___INT64 == 8
                #if _MSC_VER <= 0x4b0
    * value = (__be_int64) _atoi64(this->String());
                #else
    *value = (__be_int64) _strtoi64(this->String(), NULL, 10);
                #endif
    return true;
            #else
    return false;
            #endif
        #endif
    #endif
}

//-----------------------------------------------------------------------------

bool BString::GetInteger(__be_uint64 *value) const
{
    #if !(defined(_MSC_VER) && _MSC_VER <= 0x4b0)
    if (!value || !IsNumber()) return false;
    if (bhapi::get_hex(*this, value)) return true;

        #if SIZEOF_INT == 8 || SIZEOF_LONG == 8
    * value = (__be_uint64) strtoul(this->String(), NULL, 10);
    return true;
        #else
            #if SIZEOF_LONG_LONG == 8 && !defined(_MSC_VER)
    * value = (__be_uint64) strtoull(this->String(), NULL, 10);
    return true;
            #else
                #if SIZEOF___INT64 == 8
    * value = (__be_uint64) strtoull(this->String(), NULL, 10);
    return true;
                #else
    return false;
                #endif
            #endif
        #endif
    #else

    // TODO
    return false;
    #endif
}

//-----------------------------------------------------------------------------

BString &BString::operator<<(const char *str)
{
    return Append(str);
}

//-----------------------------------------------------------------------------

BString &BString::operator<<(const BString &str)
{
    return Append(str);
}

//-----------------------------------------------------------------------------

BString &BString::operator<<(__be_int8 value)
{
    return AppendFormat("%I8i", value);
}

//-----------------------------------------------------------------------------

BString &BString::operator<<(__be_uint8 value)
{
    return AppendFormat("%I8u", value);
}

//-----------------------------------------------------------------------------

BString &BString::operator<<(__be_int16 value)
{
    return AppendFormat("%I16i", value);
}

//-----------------------------------------------------------------------------

BString &BString::operator<<(__be_uint16 value)
{
    return AppendFormat("%I16u", value);
}

//-----------------------------------------------------------------------------

BString &BString::operator<<(__be_int32 value)
{
    return AppendFormat("%I32i", value);
}

//-----------------------------------------------------------------------------

BString &BString::operator<<(__be_uint32 value)
{
    return AppendFormat("%I32u", value);
}

//-----------------------------------------------------------------------------

BString &BString::operator<<(__be_int64 value)
{
    return AppendFormat("%I64i", value);
}

//-----------------------------------------------------------------------------

BString &BString::operator<<(__be_uint64 value)
{
    return AppendFormat("%I64u", value);
}

//-----------------------------------------------------------------------------

BString &BString::operator<<(float value)
{
    return AppendFormat("%g", value);
}

//-----------------------------------------------------------------------------

BString &BString::operator<<(double value)
{
    return AppendFormat("%g", value);
}

#ifdef BHAPI_SUPPORT_LONG_DOUBLE

//-----------------------------------------------------------------------------

BString &BString::operator<<(long double value)
{
    return AppendFormat("%Lg", value);
}
#endif // BHAPI_SUPPORT_LONG_DOUBLE

//-----------------------------------------------------------------------------

BString &BString::AppendFormat(const char *format, ...)
{
    va_list args;

    char    *buffer = NULL;

    if (!format) return *this;

    va_start(args, format);

    buffer = bhapi::strdup_vprintf(format, args);
    if (buffer)
    {
        Append(buffer);
        free(buffer);
    }

    va_end(args);

    return *this;
}

//-----------------------------------------------------------------------------

BString &BString::PrependFormat(const char *format, ...)
{
    va_list args;

    char    *buffer = NULL;

    if (!format) return *this;

    va_start(args, format);

    buffer = bhapi::strdup_vprintf(format, args);
    if (buffer)
    {
        Prepend(buffer);
        free(buffer);
    }

    va_end(args);

    return *this;
}

//-----------------------------------------------------------------------------

BStringArray *BString::Split(const char *delimiter,  __be_uint32 max_tokens) const
{
     __be_int32     offset = 0;
     __be_int32     found = 0;
     __be_uint32    i = 0;

     __be_int32 delimiter_len = (__be_int32) strlen(delimiter);

    BStringArray *array = new BStringArray();
    if (delimiter == NULL || *delimiter == 0) return NULL;
    if (max_tokens == B_MAXUINT32) return NULL;
    if (!array) return NULL;

    while (true)
    {
        BString line;

         __be_int32 length = found - offset;

        if (offset >= Length() || i >= max_tokens) break;
        if ((found = FindFirst(delimiter, offset)) < 0) found = Length();
        i++;

        if (length > 0) line.SetTo(this->String() + offset, length);

        array->AddItem(line);

        offset = found + delimiter_len;
    }

    if (array->CountItems() <= 0)
    {
        delete array;
        array = NULL;
    }

    return array;
}

//-----------------------------------------------------------------------------

BStringArray *BString::Split(const char delimiter,  __be_uint32 max_tokens) const
{
    char    string[2];
    bzero(string, 2);
    string[0] = delimiter;
    return Split(string, max_tokens);
}

//-----------------------------------------------------------------------------

__be_int32 BString::CountChars(void) const
{
    if (fLen <= 0) return fLen;
    return bhapi::utf8_strlen(fBuffer);
}

//-----------------------------------------------------------------------------

const char *BString::CharAt(__be_int32 index,  __be_uint8 *length) const
{
    if (length) *length = 0;
    if (fLen <= index) return NULL;
    return bhapi::utf8_at(fBuffer, index, length);
}

//-----------------------------------------------------------------------------

template<class TYPE_INT>
void bhapi::printf_int(BString &str, TYPE_INT value,  __be_uint8 _base, int precision_width, bool upper_style)
{
    TYPE_INT    base = (TYPE_INT) _base;

    if (str.Length() > 0 || !(_base == 8 || _base == 10 || _base == 16)) return;

    if (value == (TYPE_INT) 0)
    {
        if (precision_width != 0) str.Append('0', max_c(precision_width, 1));
        return;
    }

    while (value != (TYPE_INT) 0)
    {
        int nIndex = (int)(value % base);
        if (nIndex < 0) nIndex = -nIndex;
        if (nIndex >= (int)_base) break;
        str.Prepend((char)(nIndex <= 9 ? '0' + nIndex : (upper_style ? 'A' : 'a') + nIndex - 10), 1);
        value /= base;
    }

    if (!(str.Length() < 0 || precision_width < 0 || str.Length() >= precision_width))
        str.Prepend('0', precision_width - str.Length());
}

//-----------------------------------------------------------------------------

__be_int32 bhapi::printf_double(BString &str, double value, int precision_width,  __be_int32 *exponent, bool g_style, bool upper_style)
{
    double n;
     __be_int32 exp = -1;
     __be_int32 digits = 0;
    double iValue = 0;
    double fValue = modf(value, &iValue);
    bool iValueZero = (iValue == 0);

    if (exponent) *exponent = -1;
    if (str.Length() > 0 || (!g_style && exponent == NULL)) return -1;

    #if defined(HAVE_ISFINITE) && defined(HAVE_ISNAN)
    if (isfinite(value) == 0)
    {
        if (upper_style)
            str = (isnan(value) ? "NAN" : "INF");
        else
            str = (isnan(value) ? "nan" : "inf");
        return -1;
    }
    #endif
    if (precision_width < 0 && !g_style)
        precision_width = 6;
    else if (precision_width <= 0 && g_style)
        precision_width = (precision_width < 0 ? 6 : 1);

    if (iValueZero)
    {
        str.Append("0");
        digits++;
    }
    else
    {
        while (iValue != 0 && precision_width > 0)
        {
            n = fmod(iValue, 10);
            int     nIndex = (int)n;
            if (nIndex < 0) nIndex = -nIndex;
            if (nIndex >= 10)
            {
                str = (upper_style ? "NAN" : "nan");
                return -1;
            }

            str.Prepend((char)('0' + nIndex), 1);
            modf(iValue / 10, &iValue);
            digits++;
        }
    }

    #if 0
    if (exponent != NULL)   // 'g' or 'G' or 'e' or 'E' style
        #else
        if (!g_style && exponent != NULL)   // 'e' or 'E' style
            #endif
        {
            exp = digits - 1;
            if (exp >= precision_width)
            {
                BString nStr;
                *exponent = precision_width + (g_style ? 0 : 1);
                str.Remove(*exponent, -1);

                str.Append(upper_style ? "E+" : "e+");
                bhapi::printf_int(nStr, exp, 10, 2, false);
                str.Append(nStr);

                return 1;
            }

            precision_width -= (g_style ? digits : exp);
            if (!g_style) digits = 1;
        }

    while (fValue != 0 && precision_width > 0)
    {
        int nIndex = (int)n;
        n = 0;
        fValue *= 10;
        fValue = modf(fValue, &n);
        if (nIndex < 0) nIndex = -nIndex;
        if (nIndex >= 10)
        {
            str = (upper_style ? "NAN" : "nan");
            return -1;
        }
        else if (g_style && iValueZero && exp <= 0)
        {
            if (nIndex == 0)
            {
                exp--;
                continue;
            }
            else
            {
                exp--;
                if (exp < -4)
                {
                    str = (char)('0' + nIndex);
                    iValueZero = false;
                    continue;
                }
                else
                {
                    if (exp < -1)
                    {
                        str.Append('0', min_c(-exp - 1, precision_width));
                        precision_width -= (-exp - 1);
                        if (precision_width <= 0) break;
                    }

                    iValueZero = false;
                }
            }
        }

        str.Append((char)('0' + nIndex), 1);
        precision_width--;
    }

    if ((g_style && exp < -4) || (!g_style && exponent != NULL))
    {
        BString nStr;
        *exponent = str.Length();

        str.Append(upper_style ? "E" : "e");
        str.Append(exp > 0 ? "+" : "-");
        bhapi::printf_int(nStr, exp, 10, 2, false);
        str.Append(nStr);
    }

    return digits;
}

#ifdef BHAPI_SUPPORT_LONG_DOUBLE

//-----------------------------------------------------------------------------

__be_int32 bhapi::printf_double(BString &str, long double value, int precision_width,  __be_int32 *exponent, bool g_style, bool upper_style)
{
     __be_int32 n;
     __be_int32 exp = -1;
     __be_int32 digits = 0;
    long double iValue = 0;
    long double fValue = b_modfl(value, &iValue);
    bool iValueZero = (iValue == 0);

    if (exponent) *exponent = -1;
    if (str.Length() > 0 || (!g_style && exponent == NULL)) return -1;

        #if defined(HAVE_ISFINITE) && defined(HAVE_ISNAN)
    if (isfinite(value) == 0)
    {
        if (upper_style)
            str = (isnan(value) ? "NAN" : "INF");
        else
            str = (isnan(value) ? "nan" : "inf");
        return -1;
    }
        #endif
    if (precision_width < 0 && !g_style)
        precision_width = 6;
    else if (precision_width <= 0 && g_style)
        precision_width = (precision_width < 0 ? 6 : 1);

    if (iValueZero)
    {
        str.Append("0");
        digits++;
    }
    else
    {
        while (iValue != 0 && precision_width > 0)
        {
            long double n = b_fmodl(iValue, 10);
            int         nIndex = (int)n;
            if (nIndex < 0) nIndex = -nIndex;
            if (nIndex >= 10)
            {
                str = (upper_style ? "NAN" : "nan");
                return -1;
            }

            str.Prepend((char)('0' + nIndex), 1);
            b_modfl(iValue / 10, &iValue);
            digits++;
        }
    }

        #if 0
    if (exponent != NULL)   // 'g' or 'G' or 'e' or 'E' style
            #else
        if (!g_style && exponent != NULL)   // 'e' or 'E' style
                #endif
        {
            exp = digits - 1;
            if (exp >= precision_width)
            {
                BString nStr;
                *exponent = precision_width + (g_style ? 0 : 1);
                str.Remove(*exponent, -1);

                str.Append(upper_style ? "E+" : "e+");
                bhapi::printf_int(nStr, exp, 10, 2, false);
                str.Append(nStr);

                return 1;
            }

            precision_width -= (g_style ? digits : exp);
            if (!g_style) digits = 1;
        }

    while (fValue != 0 && precision_width > 0)
    {
        int nIndex = (int)n;
        long double n = 0;
        fValue *= 10;
        fValue = b_modfl(fValue, &n);
        if (nIndex < 0) nIndex = -nIndex;
        if (nIndex >= 10)
        {
            str = (upper_style ? "NAN" : "nan");
            return -1;
        }
        else if (g_style && iValueZero && exp <= 0)
        {
            if (nIndex == 0)
            {
                exp--;
                continue;
            }
            else
            {
                exp--;
                if (exp < -4)
                {
                    str = (char)('0' + nIndex);
                    iValueZero = false;
                    continue;
                }
                else
                {
                    if (exp < -1)
                    {
                        str.Append('0', min_c(-exp - 1, precision_width));
                        precision_width -= (-exp - 1);
                        if (precision_width <= 0) break;
                    }

                    iValueZero = false;
                }
            }
        }

        str.Append((char)('0' + nIndex), 1);
        precision_width--;
    }

    if ((g_style && exp < -4) || (!g_style && exponent != NULL))
    {
        BString nStr;
        *exponent = str.Length();

        str.Append(upper_style ? "E" : "e");
        str.Append(exp > 0 ? "+" : "-");
        bhapi::printf_int(nStr, exp, 10, 2, false);
        str.Append(nStr);
    }

    return digits;
}
#endif // BHAPI_SUPPORT_LONG_DOUBLE
extern "C"
{
//-----------------------------------------------------------------------------

BHAPI_EXPORT char *bhapi::strndup(const char *src,  __be_int32 length)
{
    char    *dest;
     __be_int32 len = 0;

    if (src == NULL || *src == 0 || length == 0) return NULL;

    if (length < 0)
        len = (__be_int32) strlen(src);
    else
        len = min_c(length, (__be_int32) strlen(src));

    dest = (char *)malloc((size_t) (len + 1));

    if (!dest) return NULL;
    if (len != 0) memcpy(dest, src, (size_t) len);
    dest[len] = 0;

    return dest;
}

//-----------------------------------------------------------------------------

BHAPI_EXPORT char *bhapi::strdup(const char *src)
{
    return bhapi::strndup(src, -1);
}

//-----------------------------------------------------------------------------

BHAPI_EXPORT char *bhapi::strdup_vprintf(const char *format, va_list ap)
{
    va_list args;
    BString str(format);

     __be_int32 offset = 0;
    if (str.Length() <= 0) return NULL;
    str.SetMinimumBufferSize(1024);

    va_copy(args, ap);

    while (!(offset < 0 || offset >= str.Length() || (offset = str.FindFirst("%", offset)) < 0))
    {
         __be_int32 curOffset = offset + 1;

        // flags
        bool    do_alternate_form = false;
        bool    left_align = false;
        bool    show_positive = false;
        bool    zero_padded = false;

        // field width
        bool    has_precision_width = false;
        int     field_width = 0;
        int     precision_width = 0;

        // length modifier
        BString length_modifier;

        while (true)
        {
            bool    doBreak = false;

            if (curOffset < 0 || curOffset >= str.Length())
            {
                str.Remove(offset, -1);
                break;
            }

            switch (str[curOffset])
            {
            case '0':
                if (!left_align) zero_padded = true;
                break;

            case ' ':
                break;

            case '+':
                show_positive = true;
                break;

            case '#':
                do_alternate_form = true;
                break;

            case '.':
                if (curOffset >= (str.Length() - 2)) break;
                has_precision_width = true;
                if (str[curOffset + 1] == '0')
                {
                    curOffset++;
                }
                else if (str[curOffset + 1] == '*')
                {
                    precision_width = va_arg(args, int);
                    if (precision_width < 0) precision_width = 0;
                    curOffset++;
                }
                else if (str[curOffset + 1] == '-' && str[curOffset + 2] >= '1' && str[curOffset + 2] <= '9')
                {
                    precision_width = 0;
                    curOffset++;
                    while (true)
                    {
                        curOffset++;
                        if (curOffset < 0 || curOffset >= str.Length()) break;
                        if (str[curOffset] < '0' || str[curOffset] > '9')
                        {
                            curOffset--;
                            break;
                        }
                    }
                }
                else if (str[curOffset + 1] >= '1' && str[curOffset + 1] <= '9')
                {
                    BString strWidth;
                    curOffset++;

                    strWidth.Append(str[curOffset], 1);
                    while (true)
                    {
                        curOffset++;
                        if (curOffset < 0 || curOffset >= str.Length()) break;
                        if (str[curOffset] < '0' || str[curOffset] > '9')
                        {
                            curOffset--;
                            break;
                        }

                        strWidth.Append(str[curOffset], 1);
                    }

                    strWidth.GetInteger((__be_int32 *) &precision_width);
                }
                break;

            case '-':
                if (curOffset >= (str.Length() - 1) || str[curOffset + 1] < '1' || str[curOffset + 1] > '9')
                {
                    left_align = true;
                    zero_padded = false;
                    break;
                }

            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                {
                    BString strWidth;
                    strWidth.Append(str[curOffset], 1);
                    while (true)
                    {
                        curOffset++;
                        if (curOffset < 0 || curOffset >= str.Length()) break;
                        if (str[curOffset] < '0' || str[curOffset] > '9')
                        {
                            curOffset--;
                            break;
                        }

                        strWidth.Append(str[curOffset], 1);
                    }

                    strWidth.GetInteger((__be_int32 *) &field_width);
                }
                break;

            case '*':
                field_width = va_arg(args, int);
                break;

            case 'h':
                if (!(curOffset >= (str.Length() - 1) || str[curOffset + 1] != 'h'))
                {
                    length_modifier = "hh";
                    curOffset++;
                }
                else
                {
                    length_modifier = "h";
                }
                break;

            case 'l':
                if (!(curOffset >= (str.Length() - 1) || str[curOffset + 1] != 'l'))
                {
                    length_modifier = "ll";
                    curOffset++;
                }
                else
                {
                    length_modifier = "l";
                }
                break;

            case 'I':
                if (!(curOffset >= (str.Length() - 2) || str[curOffset + 1] != '6' || str[curOffset + 2] != '4'))
                {
                    length_modifier = "I64";
                    curOffset += 2;
                }
                else if (!(curOffset >= (str.Length() - 2) || str[curOffset + 1] != '3' || str[curOffset + 2] != '2'))
                {
                    length_modifier = "I32";
                    curOffset += 2;
                }
                else if (!(curOffset >= (str.Length() - 2) || str[curOffset + 1] != '1' || str[curOffset + 2] != '6'))
                {
                    length_modifier = "I16";
                    curOffset += 2;
                }
                else if (!(curOffset >= (str.Length() - 1) || str[curOffset + 1] != '8'))
                {
                    length_modifier = "I8";
                    curOffset += 1;
                }
                break;

            case 'q':
                length_modifier = "ll";
                break;

            case 'L':
                length_modifier = "L";
                break;

            case 'p':
                #if SIZEOF_VOID_P == SIZEOF_SHORT
                length_modifier = "h";
                do_alternate_form = true;
                #else
                    #if SIZEOF_VOID_P == SIZEOF_INT
                length_modifier.MakeEmpty();
                do_alternate_form = true;
                    #else
                        #if SIZEOF_VOID_P == SIZEOF_LONG
                length_modifier = "l";
                do_alternate_form = true;
                        #else
                            #if SIZEOF_VOID_P == SIZEOF_LONG_LONG
                length_modifier = "ll";
                do_alternate_form = true;
                            #else
                                #if SIZEOF_VOID_P == SIZEOF___INT64
                length_modifier = "I64";
                do_alternate_form = true;
                                #endif // SIZEOF_VOID_P == SIZEOF___INT64
                            #endif // SIZEOF_VOID_P == SIZEOF_LONG_LONG
                        #endif // SIZEOF_VOID_P == SIZEOF_LONG
                    #endif // SIZEOF_VOID_P == SIZEOF_INT
                #endif // SIZEOF_VOID_P == SIZEOF_SHORT

            case 'd':
            case 'i':
            case 'o':
            case 'u':
            case 'x':
            case 'X':
                {
                    bool    negative = false;
                    bool    has_signed;
                     __be_uint8 base;

                    BString aStr;
                    if (str[curOffset] == 'o')
                    {
                        has_signed = false;
                        base = 8;
                    }
                    else if (str[curOffset] == 'u')
                    {
                        has_signed = false;
                        base = 10;
                    }
                    else if (str[curOffset] == 'x' || str[curOffset] == 'X' || str[curOffset] == 'p')
                    {
                        has_signed = false;
                        base = 16;
                    }
                    else
                    {
                        has_signed = true;
                        base = 10;
                    }

                    if (length_modifier == "hh")
                    {
                        int value = va_arg(args, int);
                        if (has_signed)
                        {
                            negative = (value < 0);
                            bhapi::printf_int
                            (
                                aStr,
                                (char)value,
                                base,
                                (has_precision_width ? max_c(precision_width, 0) : -1),
                                str[curOffset] == 'X'
                            );
                        }
                        else
                        {
                            bhapi::printf_int
                            (
                                aStr,
                                (unsigned char)value,
                                base,
                                (has_precision_width ? max_c(precision_width, 0) : -1),
                                str[curOffset] == 'X'
                            );
                        }
                    }
                    else if (length_modifier == "h")
                    {
                        int value = va_arg(args, int);
                        if (has_signed)
                        {
                            negative = (value < 0);
                            bhapi::printf_int
                            (
                                aStr,
                                (short int)value,
                                base,
                                (has_precision_width ? max_c(precision_width, 0) : -1),
                                str[curOffset] == 'X'
                            );
                        }
                        else
                        {
                            bhapi::printf_int
                            (
                                aStr,
                                (unsigned short int)value,
                                base,
                                (has_precision_width ? max_c(precision_width, 0) : -1),
                                str[curOffset] == 'X'
                            );
                        }
                    }

                    #ifdef HAVE_LONG_LONG
                    else if (length_modifier == "ll")
                    {
                        long long int   value = va_arg(args, long long int);
                        if (has_signed)
                        {
                            negative = (value < (long long int)0);
                            bhapi::printf_int
                            (
                                aStr,
                                value,
                                base,
                                (has_precision_width ? max_c(precision_width, 0) : -1),
                                str[curOffset] == 'X'
                            );
                        }
                        else
                        {
                            bhapi::printf_int
                            (
                                aStr,
                                (unsigned long long int)value,
                                base,
                                (has_precision_width ? max_c(precision_width, 0) : -1),
                                str[curOffset] == 'X'
                            );
                        }
                    }
                    #endif // HAVE_LONG_LONG
                    else if (length_modifier == "I64")
                    {
                         __be_int64 value = va_arg(args,  __be_int64);
                        if (has_signed)
                        {
                            negative = (value < B_INT64_CONSTANT(0));
                            bhapi::printf_int
                            (
                                aStr,
                                value,
                                base,
                                (has_precision_width ? max_c(precision_width, 0) : -1),
                                str[curOffset] == 'X'
                            );
                        }
                        else
                        {
                            bhapi::printf_int
                            (
                                aStr,
                                (__be_uint64) value,
                                base,
                                (has_precision_width ? max_c(precision_width, 0) : -1),
                                str[curOffset] == 'X'
                            );
                        }
                    }
                    else if (length_modifier == "I32")
                    {
                         __be_int32 value = va_arg(args,  __be_int32);
                        if (has_signed)
                        {
                            negative = (value < 0);
                            bhapi::printf_int
                            (
                                aStr,
                                value,
                                base,
                                (has_precision_width ? max_c(precision_width, 0) : -1),
                                str[curOffset] == 'X'
                            );
                        }
                        else
                        {
                            bhapi::printf_int
                            (
                                aStr,
                                (__be_uint32) value,
                                base,
                                (has_precision_width ? max_c(precision_width, 0) : -1),
                                str[curOffset] == 'X'
                            );
                        }
                    }
                    else if (length_modifier == "I16")
                    {
                        int value = va_arg(args, int);
                        if (has_signed)
                        {
                            negative = (value < 0);
                            bhapi::printf_int
                            (
                                aStr,
                                (__be_int16) value,
                                base,
                                (has_precision_width ? max_c(precision_width, 0) : -1),
                                str[curOffset] == 'X'
                            );
                        }
                        else
                        {
                            bhapi::printf_int
                            (
                                aStr,
                                (__be_uint16) value,
                                base,
                                (has_precision_width ? max_c(precision_width, 0) : -1),
                                str[curOffset] == 'X'
                            );
                        }
                    }
                    else if (length_modifier == "I8")
                    {
                        int value = va_arg(args, int);
                        if (has_signed)
                        {
                            negative = (value < 0);
                            bhapi::printf_int
                            (
                                aStr,
                                (__be_int8) value,
                                base,
                                (has_precision_width ? max_c(precision_width, 0) : -1),
                                str[curOffset] == 'X'
                            );
                        }
                        else
                        {
                            bhapi::printf_int
                            (
                                aStr,
                                (__be_uint8) value,
                                base,
                                (has_precision_width ? max_c(precision_width, 0) : -1),
                                str[curOffset] == 'X'
                            );
                        }
                    }
                    else if (length_modifier == "l")
                    {
                        long int    value = va_arg(args, long int);
                        if (has_signed)
                        {
                            negative = (value < (long int)0);
                            bhapi::printf_int
                            (
                                aStr,
                                value,
                                base,
                                (has_precision_width ? max_c(precision_width, 0) : -1),
                                str[curOffset] == 'X'
                            );
                        }
                        else
                        {
                            bhapi::printf_int
                            (
                                aStr,
                                (unsigned long int)value,
                                base,
                                (has_precision_width ? max_c(precision_width, 0) : -1),
                                str[curOffset] == 'X'
                            );
                        }
                    }
                    else
                    {
                        int value = va_arg(args, int);
                        if (has_signed)
                        {
                            negative = (value < 0);
                            bhapi::printf_int
                            (
                                aStr,
                                value,
                                base,
                                (has_precision_width ? max_c(precision_width, 0) : -1),
                                str[curOffset] == 'X'
                            );
                        }
                        else
                        {
                            bhapi::printf_int
                            (
                                aStr,
                                (unsigned int)value,
                                base,
                                (has_precision_width ? max_c(precision_width, 0) : -1),
                                str[curOffset] == 'X'
                            );
                        }
                    }

                    if (do_alternate_form)
                    {
                        if (str[curOffset] == 'o' && aStr.Length() > 0)
                        {
                            if (aStr[0] != 0) aStr.Prepend('0', 1);
                        }
                        else if
                            (
                                (str[curOffset] == 'x' || str[curOffset] == 'X' || str[curOffset] == 'p') &&
                                aStr.Length() > 0
                            )
                        {
                            aStr.Prepend(str[curOffset] == 'X' ? "0X" : "0x");
                        }
                    }

                    if (show_positive)
                        aStr.Prepend(negative ? '-' : '+', 1);
                    else if (negative)
                        aStr.Prepend('-', 1);

                    if (aStr.Length() < field_width)
                    {
                        if (!left_align)
                        {
                            char    cStr = zero_padded && !has_precision_width ? '0' : ' ';
                             __be_int32 cOffset = (cStr == '0' && (show_positive || negative) ? 1 : 0);
                             __be_int32 cCount = field_width - aStr.Length();
                            if (cOffset != 0) cCount--;
                            aStr.Insert(cStr, cCount, cOffset);
                        }
                        else
                        {
                            aStr.Append(' ', field_width - aStr.Length());
                        }
                    }

                    str.Remove(offset, curOffset - offset + 1);
                    str.Insert(aStr, offset);
                    curOffset = offset + max_c(aStr.Length(), 0);
                    doBreak = true;
                }
                break;

            case 'f':
            case 'F':
            case 'e':
            case 'E':
            case 'g':
            case 'G':
                {
                    bool    negative = false;
                     __be_int32 exponent = -1;
                    bool    g_style = (str[curOffset] == 'g' || str[curOffset] == 'G');
                     __be_int32 digits = -1;

                    BString aStr;

                    ;
                    if (length_modifier != "L")
                    {
                        double  value = va_arg(args, double);
                        negative = (value < 0);
                        digits = bhapi::printf_double
                            (
                                aStr,
                                value,
                                (has_precision_width ? precision_width : -1),
                                &exponent,
                                g_style,
                                isupper(str[curOffset]) != 0
                            );
                    }
                    else
                    {
                        long double value = va_arg(args, long double);
                        #ifdef BHAPI_SUPPORT_LONG_DOUBLE
                        negative = (value < 0);
                        digits = bhapi::printf_double
                            (
                                aStr,
                                value,
                                (has_precision_width ? precision_width : -1),
                                &exponent,
                                g_style,
                                isupper(str[curOffset]) != 0
                            );
                        #else
                        BHAPI_DEBUG("[SUPPORT]: BHAPI++ don't support \"long double\".");
                        (void)va_arg(args, long double);
                        str.Remove(offset, curOffset - offset + 1);
                        doBreak = true;
                        break;
                        #endif
                    }

                    if (digits > 0 && str.Length() > 0)
                    {
                        if (exponent < 0 && digits < aStr.Length() && g_style)
                        {
                            for (__be_int32 i = aStr.Length() - 1; i > digits - 1; i--)
                            {
                                if (aStr[i] != '0')
                                {
                                    aStr.Remove(i + 1, -1);
                                    break;
                                }
                            }
                        }

                        if (digits < (exponent < 0 ? aStr.Length() : exponent) || do_alternate_form)
                            aStr.Insert('.', 1, digits);
                    }

                    if (show_positive)
                        aStr.Prepend(negative ? '-' : '+', 1);
                    else if (negative)
                        aStr.Prepend('-', 1);

                    if (aStr.Length() < field_width)
                    {
                        if (!left_align)
                        {
                            char    cStr = zero_padded && !has_precision_width ? '0' : ' ';
                             __be_int32 cOffset = (cStr == '0' && (show_positive || negative) ? 1 : 0);
                             __be_int32 cCount = field_width - aStr.Length();
                            if (cOffset != 0) cCount--;
                            aStr.Insert(cStr, cCount, cOffset);
                        }
                        else
                        {
                            aStr.Append(' ', field_width - aStr.Length());
                        }
                    }

                    str.Remove(offset, curOffset - offset + 1);
                    str.Insert(aStr, offset);
                    curOffset = offset + max_c(aStr.Length(), 0);
                    doBreak = true;
                }
                break;

            case 'c':
            case 's':
                {
                    BString aStr;
                    if (str[curOffset] == 's')
                    {
                        char    *string = va_arg(args, char *);
                        aStr = (string ? string : "(null)");
                    }
                    else
                    {
                        aStr = (char)va_arg(args, int);
                    }

                    if (has_precision_width && aStr.Length() > max_c(precision_width, 0))
                    {
                        aStr.Remove(precision_width, -1);
                    }

                    if (aStr.Length() < field_width)
                    {
                        if (!left_align)
                            aStr.Prepend(' ', field_width - aStr.Length());
                        else
                            aStr.Append(' ', field_width - aStr.Length());
                    }

                    str.Remove(offset, curOffset - offset + 1);
                    str.Insert(aStr, offset);
                    curOffset = offset + max_c(aStr.Length(), 0);
                    doBreak = true;
                }
                break;

            case '%':
                if (curOffset == offset + 1)
                {
                    str.Remove(curOffset, 1);
                    doBreak = true;
                    break;
                }

            default:
                str.Remove(offset, curOffset - offset);
                curOffset = offset;
                doBreak = true;
                break;
            }

            if (doBreak)
                break;
            else if (curOffset >= 0)
                curOffset++;
        }

        offset = curOffset;
    }

    va_end(args);

    return(str.Length() > 0 ? bhapi::strdup(str.String()) : NULL);
}

//-----------------------------------------------------------------------------

BHAPI_EXPORT char *bhapi::strdup_printf(const char *format, ...)
{
    va_list args;
    char    *buffer = NULL;

    if (!format) return NULL;

    va_start(args, format);

    buffer = bhapi::strdup_vprintf(format, args);

    va_end(args);

    return buffer;
}


// define proper names for case-option of _DoReplace()
#define KEEP_CASE false
#define IGNORE_CASE true

// define proper names for count-option of _DoReplace()
#define REPLACE_ALL 0x7FFFFFFF

static const uint32 kPrivateDataOffset = BString::Private::kPrivateDataOffset;
const char* B_EMPTY_STRING = "";

/*!	Detaches this string from an eventually shared fPrivateData, ie. this makes
    this string writable.
*/
status_t BString::_MakeWritable()
{
    if (bhapi::atomic_get(&_ReferenceCount()) > 1) {
        // It might be shared, and this requires special treatment
        char* newData = _Clone(fPrivateData, Length());
        if (bhapi::atomic_add(&_ReferenceCount(), -1) == 1) {
            // someone else left, we were the last owner
            _FreePrivateData();
        }
        if (newData == NULL)
            return B_NO_MEMORY;

        fPrivateData = newData;
    }

    return B_OK;
}


/*!	Makes this string writable, and resizes the buffer to \a length bytes (not
    including the terminating null).

    @param length The length of the new buffer in bytes.
    @param copy If true, the current string will be copied into the new string.
*/
status_t BString::_MakeWritable(int32 length, bool copy)
{
    char* newData = NULL;

    if (bhapi::atomic_get(&_ReferenceCount()) > 1) {
        // we might share our data with someone else
        if (copy)
            newData = _Clone(fPrivateData, length);
        else
            newData = _Allocate(length);

        if (newData == NULL)
            return B_NO_MEMORY;

        if (bhapi::atomic_add(&_ReferenceCount(), -1) == 1) {
            // someone else left, we were the last owner
            _FreePrivateData();
        }
    } else {
        // we don't share our data with someone else
        newData = _Resize(length);

        if (newData == NULL)
            return B_NO_MEMORY;
    }

    fPrivateData = newData;
    return B_OK;
}


/*!	Allocates a new private data buffer with the space to store \a length bytes
    (not including the terminating null).
*/
/*static*/ char* BString::_Allocate(int32 length)
{
    if (length < 0)
        return NULL;

    char* newData = (char*)malloc(length + kPrivateDataOffset + 1);
    if (newData == NULL)
        return NULL;

    newData += kPrivateDataOffset;
    newData[length] = '\0';

    // initialize reference count & length
    Private::DataRefCount(newData) = 1;
    Private::DataLength(newData) = length & 0x7fffffff;

    return newData;
}


/*!	Resizes the private data buffer. You must already have a writable buffer
    when you call this method.
*/
char* BString::_Resize(int32 length)
{
    ASSERT(_ReferenceCount() == 1 || _ReferenceCount() == -1);

    if (length == Length())
        return fPrivateData;

    char* data = fPrivateData ? fPrivateData - kPrivateDataOffset : NULL;
    if (length < 0)
        length = 0;

    data = (char*)realloc(data, length + kPrivateDataOffset + 1);
    if (data == NULL)
        return NULL;

    data += kPrivateDataOffset;

    fPrivateData = data;
    fPrivateData[length] = '\0';

    _SetLength(length);
    _ReferenceCount() = 1;

    return data;
}


void BString::_Init(const char* src, int32 length)
{
    fPrivateData = _Clone(src, length);
    if (fPrivateData == NULL)
        fPrivateData = _Clone(NULL, 0);
}


char* BString::_Clone(const char* data, int32 length)
{
    char* newData = _Allocate(length);
    if (newData == NULL)
        return NULL;

    if (data != NULL && length > 0) {
        // "data" may not span over the whole length
        strncpy(newData, data, length);
    }

    return newData;
}


char* BString::_OpenAtBy(int32 offset, int32 length)
{
    int32 oldLength = Length();

    if (_MakeWritable() != B_OK)
        return NULL;

    memmove(fPrivateData + offset + length, fPrivateData + offset,
        oldLength - offset);
    return _Resize(oldLength + length);
}


char* BString::_ShrinkAtBy(int32 offset, int32 length)
{
    int32 oldLength = Length();
    if (_MakeWritable() != B_OK)
        return NULL;

    memmove(fPrivateData + offset, fPrivateData + offset + length,
        oldLength - offset - length);
    return _Resize(oldLength - length);
}


void BString::_SetLength(int32 length)
{
    Private::DataLength(fPrivateData) = length & 0x7fffffff;
}


void BString::_FreePrivateData()
{
    if (fPrivateData != NULL) {
        free(fPrivateData - kPrivateDataOffset);
        fPrivateData = NULL;
    }
}


bool BString::_DoAppend(const char* string, int32 length)
{
    int32 oldLength = Length();
    if (_MakeWritable(oldLength + length, true) == B_OK) {
        strncpy(fPrivateData + oldLength, string, length);
        return true;
    }
    return false;
}


bool BString::_DoPrepend(const char* string, int32 length)
{
    // TODO: this could be optimized (allocate a new buffer, use memcpy())
    int32 oldLength = Length();
    if (_MakeWritable(oldLength + length, true) == B_OK) {
        memmove(fPrivateData + length, fPrivateData, oldLength);
        if (string && length)
            strncpy(fPrivateData, string, length);
        return true;
    }
    return false;
}


bool BString::_DoInsert(const char* string, int32 offset, int32 length)
{
    int32 oldLength = Length();
    if (_MakeWritable(oldLength + length, true) == B_OK) {
        memmove(fPrivateData + offset + length, fPrivateData + offset,
            oldLength - offset);
        if (string != NULL && length)
            strncpy(fPrivateData + offset, string, length);
        return true;
    }
    return false;
}


int32 BString::_ShortFindAfter(const char* string, int32 len) const
{
    const char* ptr = strstr(String(), string);

    if (ptr != NULL)
        return ptr - String();

    return B_ERROR;
}


int32 BString::_FindAfter(const char* string, int32 offset, int32 length) const
{
    const char* ptr = strstr(String() + offset, string);

    if (ptr != NULL)
        return ptr - String();

    return B_ERROR;
}


int32 BString::_IFindAfter(const char* string, int32 offset, int32 length) const
{
    const char* ptr = bhapi::strcasestr(String() + offset, string);

    if (ptr != NULL)
        return ptr - String();

    return B_ERROR;
}


int32 BString::_FindBefore(const char* string, int32 offset, int32 length) const
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


int32 BString::_IFindBefore(const char* string, int32 offset, int32 length) const
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


BString& BString::_DoCharacterEscape(const char* string, const char* setOfCharsToEscape,
    char escapeChar)
{
    if (_MakeWritable(strlen(string), false) != B_OK)
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
        _Resize(0);
        return *this;
    }

    char* newData = _Allocate(newLength);
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

        _FreePrivateData();
        fPrivateData = newData;
    }
    return *this;
}


BString& BString::_DoCharacterDeescape(const char* string, char escapeChar)
{
    if (_MakeWritable(strlen(string), false) != B_OK)
        return *this;

    memcpy(fPrivateData, string, Length());
    const char escape[2] = { escapeChar, '\0' };
    return _DoReplace(escape, "", REPLACE_ALL, 0, KEEP_CASE);
}


BString& BString::_DoReplace(const char* findThis, const char* replaceWith,
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
        ? &BString::_IFindAfter : &BString::_FindAfter;

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
    _ReplaceAtPositions(&positions, findLen, replaceWith, replaceLen);
    return *this;
}


void BString::_ReplaceAtPositions(const PosVect* positions, int32 searchLength,
    const char* with, int32 withLength)
{
    int32 length = Length();
    uint32 count = positions->CountItems();
    int32 newLength = length + count * (withLength - searchLength);
    if (!newLength) {
        _Resize(0);
        return;
    }

    char* newData = _Allocate(newLength);
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

    _FreePrivateData();
    fPrivateData = newData;
}

}   // extern "C"
