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
 * File: String.cpp
 * Description: EString --- string allocation and manipulation
 *
 * --------------------------------------------------------------------------*/

#include <string.h"
#include <stdlib.h>
#include <stdio.h"
#include <ctype.h"
#include <math.h>
#include <stdarg.h>

#include "./../config.h"

#include "String.h"
#include "StringArray.h"

#ifndef HAVE_VA_COPY
	#ifdef HAVE___VA_COPY
		#define va_copy(a, b)		__va_copy(a, b)
	#else
		#if defined(_MSC_VER) || defined(__MWERKS__)
			#define va_copy(a, b)	((a) = (b))
		#endif
	#endif // HAVE___VA_COPY
#endif // HAVE_VA_COPY

#ifndef HAVE_ISFINITE
	#ifdef HAVE_FPCLASSIFY
		#define isfinite(a)	((fpclassify(a) != FP_NAN && fpclassify(a) != FP_INFINITE) ? 1 : 0)
		#define HAVE_ISFINITE	1
	#else
		#ifdef _MSC_VER
			#define isfinite(a)	_finite(a)
			#define HAVE_ISFINITE	1 
		#endif // _MSC_VER
	#endif
#endif

#ifndef HAVE_ISNAN
	#ifdef HAVE_FPCLASSIFY
		#define isnan(a)	((fpclassify(a) == FP_NAN) ? 1 : 0)
		#define HAVE_ISNAN	1
	#else
		#ifdef _MSC_VER
			#define isnan(a)	_isnan(a)
			#define HAVE_ISNAN	1
		#endif // _MSC_VER
	#endif
#endif

#ifndef HAVE_STRCASECMP
	#ifdef _MSC_VER
	#define e_strcasecmp(a, b)	_stricmp(a, b)
	#endif
#else
	#define e_strcasecmp(a, b)	strcasecmp(a, b)
#endif // HAVE_STRCASECMP

#ifndef HAVE_STRNCASECMP
	#ifdef _MSC_VER
	#define e_strncasecmp(a, b, c)	_strnicmp(a, b, c)
	#endif
#else
	#define e_strncasecmp(a, b, c)	strncasecmp(a, b, c)
#endif // HAVE_STRNCASECMP


// long double
#undef ETK_SUPPORT_LONG_DOUBLE
#ifdef ETK_SUPPORT_LONG_DOUBLE

#ifndef HAVE_STRTOLD
	#ifdef _MSC_VER
		#ifdef HAVE__STRTOLD
		#define strtold(a, b)	_strtold(a, b)
		#else
		#define strtold(a, b)	strtod(a, b)
		#endif // HAVE__STRTOLD
	#endif
#endif

#ifdef HAVE_FMODL
	#define e_fmodl(a, b)		fmodl(a, b)
#endif

#ifdef HAVE_MODFL
	#define e_modfl(a, b)		modfl(a, b)
#endif

#if !defined(e_fmodl) && defined(HAVE_FLOORL)
static long double e_fmodl(long double x, long double y)
{
	if(x == y) return 0;

	long double remain = x / y;
	return(remain - floorl(remain));
}
#endif

#if !defined(e_modfl) && defined(HAVE_FLOORL)
static long double e_modfl(long double value, long double *iptr)
{
	long double iValue = floorl(value);
	long double fValue = value - iValue;

	if(iptr != NULL) *iptr = iValue;
	return fValue;
}
#endif

#endif // ETK_SUPPORT_LONG_DOUBLE


_IMPEXP_ETK char* EStrdup(const char* src, eint32 length)
{
	if(src == NULL || *src == 0 || length == 0) return NULL;

	char* dest;

	eint32 len = 0;

	if(length < 0)
		len = (eint32)strlen(src);
	else
		len = min_c(length, (eint32)strlen(src));

	dest = new char[len + 1];

	if(!dest) return NULL;

	if(len != 0) memcpy(dest, src, len);
	dest[len] = 0;

	return dest;
}


#ifndef HAVE_STRCASESTR
static char* e_strcasestr(const char *haystack, const char *needle)
{
	if(!haystack || !needle) return NULL;

	size_t needle_len = strlen(needle);
	size_t haystack_len = strlen(haystack);

	if(needle_len == 0) return (char*)haystack;
	if(haystack_len < needle_len) return NULL;

	const char *tmp = haystack;
	const char *end = haystack + haystack_len - needle_len;

	while(*tmp && tmp <= end)
	{
		if(e_strncasecmp(tmp, needle, needle_len) == 0) return (char*)tmp;
		tmp++;
	}

	return NULL;
}
#else
#define e_strcasestr(a, b)	strcasestr(a, b)
#endif


#ifndef HAVE_STRRSTR
static char* e_strrstr(const char *haystack, const char *needle)
{
	if(!haystack || !needle) return NULL;

	size_t needle_len = strlen(needle);
	size_t haystack_len = strlen(haystack);

	if(needle_len == 0) return (char*)haystack;
	if(haystack_len < needle_len) return NULL;

	const char *tmp = haystack + haystack_len - needle_len;

	while(*tmp && tmp >= haystack)
	{
		if(strncmp(tmp, needle, needle_len) == 0) return (char*)tmp;
		tmp--;
	}

	return NULL;
}
#else
#define e_strrstr(a, b)		strrstr(a, b)
#endif


#ifndef HAVE_STRRCASESTR
static char* e_strrcasestr(const char *haystack, const char *needle)
{
	if(!haystack || !needle) return NULL;

	size_t needle_len = strlen(needle);
	size_t haystack_len = strlen(haystack);

	if(needle_len == 0) return (char*)haystack;
	if(haystack_len < needle_len) return NULL;

	const char *tmp = haystack + haystack_len - needle_len;

	while(*tmp && tmp >= haystack)
	{
		if(e_strncasecmp(tmp, needle, needle_len) == 0) return (char*)tmp;
		tmp--;
	}

	return NULL;
}
#else
#define e_strrcasestr(a, b)	strrcasestr(a, b)
#endif


#ifndef HAVE_STRCASECHR
static char* e_strcasechr(const char *s, int c)
{
	if(s == NULL || *s == 0 || c == 0) return NULL;

	const char *tmp = strchr(s, tolower(c));
	if(tmp == NULL) tmp = strchr(s, toupper(c));

	return (char*)tmp;
}
#else
#define e_strcasechr(a, b)	strcasechr(a, b)
#endif


#ifndef HAVE_STRRCASECHR
static char* e_strrcasechr(const char *s, int c)
{
	if(s == NULL || *s == 0 || c == 0) return NULL;

	const char *tmp = strrchr(s, tolower(c));
	if(tmp == NULL) tmp = strrchr(s, toupper(c));

	return (char*)tmp;
}
#else
#define e_strrcasechr(a, b)	strrcasechr(a, b)
#endif


#define MAX_STRING_LENGTH	(E_MAXINT32 - 1)


bool
EString::SetMinimumBufferSize(eint32 length)
{
	if(length > MAX_STRING_LENGTH + 1) return false;

	if(length <= 0 || length <= fLenReal)
	{
		fMinBufferSize = length;
		return true;
	}

	char *newData = (char*)realloc(fBuffer, length);
	if(!newData) return false;

	fMinBufferSize = length;
	fBuffer = newData;
	fBuffer[max_c(fLen, 0)] = 0;
	fLenReal = length;

	return true;
}


eint32
EString::MinimumBufferSize() const
{
	return fMinBufferSize;
}


bool
EString::_Resize(eint32 length)
{
	if(length <= 0)
	{
		if(fMinBufferSize > 0)
		{
			if(fLenReal != fMinBufferSize)
			{
				char *newData = (char*)realloc(fBuffer, fMinBufferSize);
				if(newData)
				{
					fBuffer = newData;
					fLenReal = fMinBufferSize;
				}
			}

			fLen = 0;
			if(fBuffer) fBuffer[0] = 0;
			else fLenReal = 0;
		}
		else
		{
			if(fBuffer) free(fBuffer);
			fBuffer = NULL;
			fLen = 0;
			fLenReal = 0;
		}
		return true;
	}
	else if(length == fLenReal - 1)
	{
		fLen = length;
		return true;
	}
	else if(length > MAX_STRING_LENGTH)
	{
		return false;
	}
	else if(length < fMinBufferSize && fMinBufferSize == fLenReal)
	{
		fLen = length;
		fBuffer[fLen] = 0;
		return true;
	}

	eint32 length_to_alloc = max_c(length + 1, fMinBufferSize);
	char *newData = (char*)realloc(fBuffer, length_to_alloc);
	if(!newData && length + 1 < fMinBufferSize)
	{
		length_to_alloc = length + 1;
		newData = (char*)realloc(fBuffer, length_to_alloc);
	}

	if(!newData)
	{
		if(length < fLenReal)
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


EString::EString()
	: fLen(0), fLenReal(0), fMinBufferSize(0), fBuffer(NULL)
{
}


EString::EString(const char *str)
	: fLen(0), fLenReal(0), fMinBufferSize(0), fBuffer(NULL)
{
	Append(str);
}


EString::EString(const EString &str)
	: fLen(0), fLenReal(0), fMinBufferSize(0), fBuffer(NULL)
{
	Append(str);
}


EString::EString(const char *str, eint32 maxLength)
	: fLen(0), fLenReal(0), fMinBufferSize(0), fBuffer(NULL)
{
	Append(str, maxLength);
}


EString::~EString()
{
	if(fBuffer) free(fBuffer);
}


const char*
EString::String() const
{
	return fBuffer;
}


eint32
EString::Length() const
{
	return fLen;
}


EString&
EString::operator=(const EString &str)
{
	this->SetTo(str);
	return *this;
}


EString&
EString::operator=(const char *str)
{
	this->SetTo(str);
	return *this;
}


EString&
EString::operator=(char c)
{
	this->SetTo(c, 1);
	return *this;
}


EString&
EString::SetTo(const EString &str)
{
	MakeEmpty();
	return Append(str);
}


EString&
EString::SetTo(const EString &str, eint32 length)
{
	MakeEmpty();
	return Append(str, length);
}


EString&
EString::SetTo(const char *str)
{
	MakeEmpty();
	return Append(str);
}


EString&
EString::SetTo(const char *str, eint32 length)
{
	MakeEmpty();
	return Append(str, length);
}


EString&
EString::SetTo(char c, eint32 count)
{
	MakeEmpty();
	return Append(c, count);
}


EString&
EString::Adopt(EString &from)
{
	Append(from);
	from.MakeEmpty();

	return *this;
}


EString&
EString::Adopt(EString &from, eint32 length)
{
	Append(from, length);
	from.MakeEmpty();

	return *this;
}


EString&
EString::CopyInto(EString &into, eint32 fromOffset, eint32 length) const
{
	if(this == &into)
	{
		EString str = *this;
		if(str == *this) str.CopyInto(into, fromOffset, length);
	}
	else if(fromOffset >= 0 && fromOffset < fLen)
	{
		into.MakeEmpty();
		into.Append(fBuffer + fromOffset, length);
	}

	return into;
}


void
EString::CopyInto(char *into, size_t into_size, eint32 fromOffset, eint32 length) const
{
	if(!into || into_size <= 0) return;
	if(fromOffset < 0 || fromOffset >= fLen) return;
	if(length < 0 || length > fLen - fromOffset) length = fLen - fromOffset;

	if(fBuffer && fromOffset >= 0 && fromOffset < fLen && length > 0)
		strncpy(into, fBuffer + fromOffset, min_c(into_size, (size_t)length));
	else
		into[0] = '\0';
}


EString&
EString::MoveInto(EString &into, eint32 from, eint32 length)
{
	if(this == &into)
	{
		EString str = *this;
		if(str == *this) str.MoveInto(into, from, length);
	}
	else if(from >= 0 && from < fLen)
	{
		into.MakeEmpty();
		into.Append(fBuffer + from, length);

		Remove(from, length);
	}

	return into;
}


void
EString::MoveInto(char *into, size_t into_size, eint32 from, eint32 length)
{
	if(!into || into_size <= 0) return;
	if(from < 0 || from >= fLen) return;
	if(length < 0 || length > fLen - from) length = fLen - from;

	if(fBuffer && from >= 0 && from < fLen)
	{
		strncpy(into, fBuffer + from, min_c(into_size, (size_t)length));
		Remove(from, length);
	}
	else
	{
		into[0] = '\0';
	}
}


void
EString::MakeEmpty()
{
	Remove(0, -1);
}


EString&
EString::operator+=(const EString &str)
{
	return Append(str);
}


EString&
EString::operator+=(const char *str)
{
	return Append(str);
}


EString&
EString::operator+=(char c)
{
	return Append(c, 1);
}


EString&
EString::Append(const EString &str)
{
	return Append(str, str.Length());
}


EString&
EString::Append(const EString &str, eint32 length)
{
	if(str.Length() < 1 || length == 0) return *this;
	if(length < 0) length = str.Length();

	return Append(str.String(), length);
}


EString&
EString::Append(const char *str)
{
	if(str) Append(str, -1);

	return *this;
}


EString&
EString::Append(const char *str, eint32 length)
{
	if(str == NULL || *str == 0 || length == 0) return *this;

	eint32 strLen = (eint32)strlen(str);

	if(length < 0 || length > strLen) length = strLen;
	if(MAX_STRING_LENGTH - fLen < length) return *this;

	if(_Resize(fLen + length))
	{
		if(memcpy(fBuffer + fLen - length, str, length) == NULL)
		{
			fLen -= length;
		}
	}

	return *this;
}


EString&
EString::Append(char c, eint32 count)
{
	if(c == 0 || MAX_STRING_LENGTH - fLen < count || count <= 0) return *this;

	if(_Resize(fLen + count))
	{
		if(memset(fBuffer + fLen - count, (int)c, count) == NULL)
		{
			fLen -= count;
		}
	}

	return *this;
}


EString&
EString::Prepend(const EString &str)
{
	return Prepend(str, str.Length());
}


EString&
EString::Prepend(const EString &str, eint32 length)
{
	if(str.Length() < 1 || length == 0) return *this;
	if(length < 0) length = str.Length();

	return Prepend(str.String(), length);
}


EString&
EString::Prepend(const char *str)
{
	if(str) Prepend(str, -1);

	return *this;
}


EString&
EString::Prepend(const char *str, eint32 length)
{
	return Insert(str, length, 0);
}


EString&
EString::Prepend(char c, eint32 count)
{
	return Insert(c, count, 0);
}


EString&
EString::Insert(const EString &str, eint32 pos)
{
	return Insert(str, str.Length(), pos);
}


EString&
EString::Insert(const EString &str, eint32 length, eint32 pos)
{
	return Insert(str, 0, length, pos);
}


EString&
EString::Insert(const EString &str, eint32 fromOffset, eint32 length, eint32 pos)
{
	return Insert(str.String(), fromOffset, length, pos);
}


EString&
EString::Insert(const char *str, eint32 pos)
{
	if(str) Insert(str, -1, pos);
	return *this;
}


EString&
EString::Insert(const char *str, eint32 length, eint32 pos)
{
	return Insert(str, 0, length, pos);
}


EString&
EString::Insert(const char *str, eint32 fromOffset, eint32 length, eint32 pos)
{
	if(str == NULL || *str == 0 || length == 0 || fromOffset < 0) return *this;

	eint32 strLen = (eint32)strlen(str);
	if(fromOffset >= strLen) return *this;

	if((pos == 0 && fLen <= 0) || (pos > 0 && pos == fLen)) return Append(str + fromOffset, length);
	if(pos < 0 || pos >= fLen) return *this;

	if(length < 0 || length > strLen - fromOffset) length = strLen - fromOffset;

	if(MAX_STRING_LENGTH - fLen < length) return *this;

	if(!_Resize(fLen + length)) return *this;

	if(memmove(fBuffer + pos + length, fBuffer + pos, fLen - length - pos) == NULL)
	{
		fLen -= length;
		return *this;
	}
	else if(memcpy(fBuffer + pos, str + fromOffset, length) == NULL)
	{
		fLen -= length;
	}

	return *this;
}


EString&
EString::Insert(char c, eint32 count, eint32 pos)
{
	if((pos == 0 && fLen <= 0) || (pos > 0 && pos == fLen)) return Append(c, count);

	if(pos < 0 || pos >= fLen) return *this;
	if(c == 0 || MAX_STRING_LENGTH - fLen < count || count <= 0) return *this;

	if(!_Resize(fLen + count)) return *this;

	if(memmove(fBuffer + pos + count, fBuffer + pos, fLen - count - pos) == NULL)
	{
		fLen -= count;
		return *this;
	}
	else if(memset(fBuffer + pos, (int)c, count) == NULL)
	{
		fLen -= count;
	}

	return *this;
}


EString&
EString::Truncate(eint32 newLength)
{
	if(!(newLength < 0 || newLength >= fLen)) _Resize(newLength);

	return *this;
}


EString&
EString::Remove(eint32 from, eint32 length)
{
	if(from < 0 || from >= fLen) return *this;

	if(length == 0) return *this;
	else if(length < 0) length = fLen - from;
	else length = min_c(fLen - from, length);

	if(from < (fLen - 1) && length != (fLen - from))
	{
		if(memmove(fBuffer + from, fBuffer + from + length, fLen - from - length) == NULL) return *this;
	}

	_Resize(fLen - length);

	return *this;
}


EString&
EString::RemoveFirst(const EString &str)
{
	return RemoveFirst(str.String());
}


EString&
EString::RemoveLast(const EString &str)
{
	return RemoveLast(str.String());
}


EString&
EString::RemoveAll(const EString &str)
{
	return RemoveAll(str.String());
}


EString&
EString::RemoveFirst(const char *str)
{
	if(str == NULL || *str == 0) return *this;

	const char *s = String();
	if(s)
	{
		const char *tmp = strstr(s, str);
		if(tmp)
		{
			eint32 start = (eint32)(tmp - s);
			Remove(start, (eint32)strlen(str));
		}
	}

	return *this;
}


EString&
EString::RemoveLast(const char *str)
{
	if(str == NULL || *str == 0) return *this;

	const char *s = String();
	if(s)
	{
		const char *tmp = e_strrstr(s, str);
		if(tmp)
		{
			eint32 start = (eint32)(tmp - s);
			Remove(start, (eint32)strlen(str));
		}
	}

	return *this;
}


EString&
EString::RemoveAll(const char *str)
{
	if(str == NULL || *str == 0) return *this;

	const char *s = String();
	if(s)
	{
		const char *tmp;
		while((tmp = strstr(s, str)) != NULL)
		{
			eint32 start = (eint32)(tmp - s);
			Remove(start, (eint32)strlen(str));
			s = String();

			if(!s) break;
		}
	}

	return *this;
}


EString&
EString::RemoveSet(const char *setOfCharsToRemove)
{
	if(setOfCharsToRemove == NULL || *setOfCharsToRemove == 0) return *this;

	eint32 len = (eint32)strlen(setOfCharsToRemove);

	char str[2];
	str[1] = '\0';

	for(eint32 i = 0; i < len; i++)
	{
		str[0] = setOfCharsToRemove[i];
		RemoveAll(str);

		if(Length() == 0) break;
	}

	return *this;
}


EString&
EString::IRemoveFirst(const EString &str)
{
	return IRemoveFirst(str.String());
}


EString&
EString::IRemoveLast(const EString &str)
{
	return IRemoveLast(str.String());
}


EString&
EString::IRemoveAll(const EString &str)
{
	return IRemoveAll(str.String());
}


EString&
EString::IRemoveFirst(const char *str)
{
	if(str == NULL || *str == 0) return *this;

	const char *s = String();
	if(s)
	{
		const char *tmp = e_strcasestr(s, str);
		if(tmp)
		{
			eint32 start = (eint32)(tmp - s);
			Remove(start, (eint32)strlen(str));
		}
	}

	return *this;
}


EString&
EString::IRemoveLast(const char *str)
{
	if(str == NULL || *str == 0) return *this;

	const char *s = String();
	if(s)
	{
		const char *tmp = e_strrcasestr(s, str);
		if(tmp)
		{
			eint32 start = (eint32)(tmp - s);
			Remove(start, (eint32)strlen(str));
		}
	}

	return *this;
}


EString&
EString::IRemoveAll(const char *str)
{
	if(str == NULL || *str == 0) return *this;

	const char *s = String();
	if(s)
	{
		const char *tmp;
		while((tmp = e_strcasestr(s, str)) != NULL)
		{
			eint32 start = (eint32)(tmp - s);
			Remove(start, (eint32)strlen(str));
			s = String();

			if(!s) break;
		}
	}

	return *this;
}


EString&
EString::IRemoveSet(const char *setOfCharsToRemove)
{
	if(setOfCharsToRemove == NULL || *setOfCharsToRemove == 0) return *this;

	eint32 len = (eint32)strlen(setOfCharsToRemove);

	char str[2];
	str[1] = 0;

	for(eint32 i = 0; i < len; i++)
	{
		str[0] = setOfCharsToRemove[i];
		IRemoveAll(str);

		if(Length() == 0) break;
	}

	return *this;
}


bool
EString::operator<(const EString &str) const
{
	return(Compare(str) < 0);
}


bool
EString::operator<=(const EString &str) const
{
	return(Compare(str) <= 0);
}


bool
EString::operator==(const EString &str) const
{
	const char *a = String();
	const char *b = str.String();

	if(a == NULL && b == NULL) return true;
	if(a == NULL || b == NULL) return false;

	return(!(strlen(a) != strlen(b) || strcmp(a, b) != 0));
}


bool
EString::operator>=(const EString &str) const
{
	return(Compare(str) >= 0);
}


bool
EString::operator>(const EString &str) const
{
	return(Compare(str) > 0);
}


bool
EString::operator!=(const EString &str) const
{
	return !operator==(str);
}


bool
EString::operator<(const char *str) const
{
	return(Compare(str) < 0);
}


bool
EString::operator<=(const char *str) const
{
	return(Compare(str) <= 0);
}


bool
EString::operator==(const char *str) const
{
	const char *a = String();
	const char *b = str;

	if(a == NULL && b == NULL) return true;
	if(a == NULL || b == NULL) return false;

	return(!(strlen(a) != strlen(b) || strcmp(a, b) != 0));
}


bool
EString::operator>=(const char *str) const
{
	return(Compare(str) >= 0);
}


bool
EString::operator>(const char *str) const
{
	return(Compare(str) > 0);
}


bool
EString::operator!=(const char *str) const
{
	return !operator==(str);
}


int
EString::Compare(const EString &str) const
{
	const char *a = "";
	const char *b = "";

	if(String()) a = String();
	if(str.String()) b = str.String();

	return strcmp(a, b);
}


int
EString::Compare(const char *str) const
{
	const char *a = "";
	const char *b = "";

	if(String()) a = String();
	if(str) b = str;

	return strcmp(a, b);
}


int
EString::Compare(const EString &str, eint32 n) const
{
	const char *a = "";
	const char *b = "";

	if(String()) a = String();
	if(str.String()) b = str.String();

	return strncmp(a, b, (size_t)n);
}


int
EString::Compare(const char *str, eint32 n) const
{
	const char *a = "";
	const char *b = "";

	if(String()) a = String();
	if(str) b = str;

	return strncmp(a, b, (size_t)n);
}


int
EString::ICompare(const EString &str) const
{
	const char *a = "";
	const char *b = "";

	if(String()) a = String();
	if(str.String()) b = str.String();

	return e_strcasecmp(a, b);
}


int
EString::ICompare(const char *str) const
{
	const char *a = "";
	const char *b = "";

	if(String()) a = String();
	if(str) b = str;

	return e_strcasecmp(a, b);
}


int
EString::ICompare(const EString &str, eint32 n) const
{
	const char *a = "";
	const char *b = "";

	if(String()) a = String();
	if(str.String()) b = str.String();

	return e_strncasecmp(a, b, (size_t)n);
}


int
EString::ICompare(const char *str, eint32 n) const
{
	const char *a = "";
	const char *b = "";

	if(String()) a = String();
	if(str) b = str;

	return e_strncasecmp(a, b, (size_t)n);
}


eint32
EString::FindFirst(const EString &string) const
{
	if(String() == NULL || string.String() == NULL) return -1;

	const char *tmp = strstr(String(), string.String());

	if(tmp == NULL) return -1;

	return((eint32)(tmp - String()));
}


eint32
EString::FindFirst(const char *string) const
{
	if(String() == NULL || string == NULL || *string == 0) return -1;

	const char *tmp = strstr(String(), string);

	if(tmp == NULL) return -1;

	return((eint32)(tmp - String()));
}


eint32
EString::FindFirst(const EString &string, eint32 fromOffset) const
{
	if(String() == NULL || string.String() == NULL || fromOffset < 0 || fromOffset >= Length()) return -1;

	const char *string_find = String() + fromOffset;
	const char *tmp = strstr(string_find, string.String());

	if(tmp == NULL) return -1;

	return((eint32)(tmp - String()));
}


eint32
EString::FindFirst(const char *string, eint32 fromOffset) const
{
	if(String() == NULL || string == NULL || *string == 0 || fromOffset < 0 || fromOffset >= Length()) return -1;

	const char *string_find = String() + fromOffset;
	const char *tmp = strstr(string_find, string);

	if(tmp == NULL) return -1;

	return((eint32)(tmp - String()));
}


eint32
EString::FindFirst(char c) const
{
	if(String() == NULL || c == 0) return -1;

	const char *tmp = strchr(String(), c);

	if(tmp == NULL) return -1;

	return((eint32)(tmp - String()));
}


eint32
EString::FindFirst(char c, eint32 fromOffset) const
{
	if(String() == NULL || c == 0 || fromOffset < 0 || fromOffset >= Length()) return -1;

	const char *string_find = String() + fromOffset;
	const char *tmp = strchr(string_find, c);

	if(tmp == NULL) return -1;

	return((eint32)(tmp - String()));
}


eint32
EString::FindLast(const EString &string) const
{
	if(String() == NULL || string.String() == NULL) return -1;

	const char *tmp = e_strrstr(String(), string.String());

	if(tmp == NULL) return -1;

	return((eint32)(tmp - String()));
}


eint32
EString::FindLast(const char *string) const
{
	if(String() == NULL || string == NULL || *string == 0) return -1;

	const char *tmp = e_strrstr(String(), string);

	if(tmp == NULL) return -1;

	return((eint32)(tmp - String()));
}


eint32
EString::FindLast(const EString &string, eint32 beforeOffset) const
{
	if(String() == NULL || string.String() == NULL || beforeOffset < 0 || beforeOffset >= Length()) return -1;

	char *string_find = e_strndup(String(), beforeOffset + 1);
	if(string_find == NULL) return -1;
	
	const char *tmp = e_strrstr(string_find, string.String());

	eint32 ret = -1;
	if(tmp != NULL) ret = (eint32)(tmp - string_find);

	free(string_find);

	return ret;
}


eint32
EString::FindLast(const char *string, eint32 beforeOffset) const
{
	if(String() == NULL || string == NULL || *string == 0 || beforeOffset < 0 || beforeOffset >= Length()) return -1;

	char *string_find = e_strndup(String(), beforeOffset + 1);
	if(string_find == NULL) return -1;

	const char *tmp = e_strrstr(string_find, string);

	eint32 ret = -1;
	if(tmp != NULL) ret = (eint32)(tmp - string_find);

	free(string_find);

	return ret;
}


eint32
EString::FindLast(char c) const
{
	if(String() == 0 || c == 0) return -1;

	const char *tmp = strrchr(String(), c);

	if(tmp == NULL) return -1;

	return((eint32)(tmp - String()));
}


eint32
EString::FindLast(char c, eint32 beforeOffset) const
{
	if(String() == NULL || c == 0 || beforeOffset < 0 || beforeOffset >= Length()) return -1;

	char *string_find = e_strndup(String(), beforeOffset + 1);
	if(string_find == NULL) return -1;
	
	const char *tmp = strrchr(string_find, c);

	eint32 ret = -1;
	if(tmp != NULL) ret = (eint32)(tmp - string_find);

	free(string_find);

	return ret;
}


eint32
EString::IFindFirst(const EString &string) const
{
	if(String() == NULL || string.String() == NULL) return -1;

	const char *tmp = e_strcasestr(String(), string.String());

	if(tmp == NULL) return -1;

	return((eint32)(tmp - String()));
}


eint32
EString::IFindFirst(const char *string) const
{
	if(String() == NULL || string == NULL || *string == 0) return -1;

	const char *tmp = e_strcasestr(String(), string);

	if(tmp == NULL) return -1;

	return((eint32)(tmp - String()));
}


eint32
EString::IFindFirst(const EString &string, eint32 fromOffset) const
{
	if(String() == NULL || string.String() == NULL || fromOffset < 0 || fromOffset >= Length()) return -1;

	const char *string_find = String() + fromOffset;
	const char *tmp = e_strcasestr(string_find, string.String());

	if(tmp == NULL) return -1;

	return((eint32)(tmp - String()));
}


eint32
EString::IFindFirst(const char *string, eint32 fromOffset) const
{
	if(String() == NULL || string == NULL || *string == 0 || fromOffset < 0 || fromOffset >= Length()) return -1;

	const char *string_find = String() + fromOffset;
	const char *tmp = e_strcasestr(string_find, string);

	if(tmp == NULL) return -1;

	return((eint32)(tmp - String()));
}


eint32
EString::IFindFirst(char c) const
{
	if(String() == NULL || c == 0) return -1;

	const char *tmp = e_strcasechr(String(), c);

	if(tmp == NULL) return -1;

	return((eint32)(tmp - String()));
}


eint32
EString::IFindFirst(char c, eint32 fromOffset) const
{
	if(String() == NULL || c == 0 || fromOffset < 0 || fromOffset >= Length()) return -1;

	const char *string_find = String() + fromOffset;
	const char *tmp = e_strcasechr(string_find, c);

	if(tmp == NULL) return -1;

	return((eint32)(tmp - String()));
}


eint32
EString::IFindLast(const EString &string) const
{
	if(String() == NULL || string.String() == NULL) return -1;

	const char *tmp = e_strrcasestr(String(), string.String());

	if(tmp == NULL) return -1;

	return((eint32)(tmp - String()));
}


eint32
EString::IFindLast(const char *string) const
{
	if(String() == NULL || string == NULL || *string == 0) return -1;

	const char *tmp = e_strrcasestr(String(), string);

	if(tmp == NULL) return -1;

	return((eint32)(tmp - String()));
}


eint32
EString::IFindLast(const EString &string, eint32 beforeOffset) const
{
	if(String() == NULL || string.String() == NULL || beforeOffset < 0 || beforeOffset >= Length()) return -1;

	char *string_find = e_strndup(String(), beforeOffset + 1);
	if(string_find == NULL) return -1;

	const char *tmp = e_strrcasestr(string_find, string.String());

	eint32 ret = -1;
	if(tmp != NULL) ret = (eint32)(tmp - string_find);

	free(string_find);

	return ret;
}


eint32
EString::IFindLast(const char *string, eint32 beforeOffset) const
{
	if(String() == NULL || string == NULL || *string == 0 || beforeOffset < 0 || beforeOffset >= Length()) return -1;

	char *string_find = e_strndup(String(), beforeOffset + 1);
	if(string_find == NULL) return -1;

	const char *tmp = e_strrcasestr(string_find, string);

	eint32 ret = -1;
	if(tmp != NULL) ret = (eint32)(tmp - string_find);

	free(string_find);

	return ret;
}


eint32
EString::IFindLast(char c) const
{
	if(String() == NULL || c == 0) return -1;

	const char *tmp = e_strrcasechr(String(), c);

	if(tmp == NULL) return -1;

	return((eint32)(tmp - String()));
}


eint32
EString::IFindLast(char c, eint32 beforeOffset) const
{
	if(String() == NULL || c == 0 || beforeOffset < 0 || beforeOffset >= Length()) return -1;

	char *string_find = e_strndup(String(), beforeOffset + 1);
	if(string_find == NULL) return -1;
	
	const char *tmp = e_strrcasechr(string_find, c);

	eint32 ret = -1;
	if(tmp != NULL) ret = (eint32)(tmp - string_find);

	free(string_find);

	return ret;
}


EString&
EString::ReplaceFirst(char replaceThis, char withThis)
{
	eint32 index = FindFirst(replaceThis);

	if(index >= 0) fBuffer[index] = withThis;

	return *this;
}


EString&
EString::ReplaceLast(char replaceThis, char withThis)
{
	eint32 index = FindLast(replaceThis);

	if(index >= 0) fBuffer[index] = withThis;

	return *this;
}


EString&
EString::ReplaceAll(char replaceThis, char withThis, eint32 fromOffset)
{
	if(fromOffset < 0 || fromOffset >= fLen) return *this;

	while(fromOffset < fLen)
	{
		eint32 index = FindFirst(replaceThis, fromOffset);

		if(index >= 0)
		{
			fBuffer[index] = withThis;
			fromOffset = index + 1;
		}
		else break;
	}

	return *this;
}


EString&
EString::ReplaceFirst(const char *replaceThis, const char *withThis)
{
	if(replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;

	eint32 index = FindFirst(replaceThis);

	if(index >= 0)
	{
		Remove(index, (eint32)strlen(replaceThis));
		Insert(withThis, index);
	}

	return *this;
}


EString&
EString::ReplaceLast(const char *replaceThis, const char *withThis)
{
	if(replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;

	eint32 index = FindLast(replaceThis);

	if(index >= 0)
	{
		Remove(index, (eint32)strlen(replaceThis));
		Insert(withThis, index);
	}

	return *this;
}


EString&
EString::ReplaceAll(const char *replaceThis, const char *withThis, eint32 fromOffset)
{
	if(fromOffset < 0 || fromOffset >= fLen) return *this;
	if(replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;

	eint32 strLenReplace = (eint32)strlen(replaceThis);
	eint32 strLenWith = (eint32)strlen(withThis);

	while(fromOffset < fLen)
	{
		eint32 index = FindFirst(replaceThis, fromOffset);

		if(index >= 0)
		{
			Remove(index, strLenReplace);
			Insert(withThis, index);
			fromOffset = index + strLenWith;
		}
		else break;
	}

	return *this;
}


EString&
EString::Replace(const char *replaceThis, const char *withThis, eint32 maxReplaceCount, eint32 fromOffset)
{
	if(fromOffset < 0 || fromOffset >= fLen) return *this;
	if(replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;

	if(maxReplaceCount == 0) return *this;
	if(maxReplaceCount < 0) return ReplaceAll(replaceThis, withThis, fromOffset);

	int count = 0;
	eint32 strLenReplace = (eint32)strlen(replaceThis);
	eint32 strLenWith = (eint32)strlen(withThis);

	while(true)
	{
		eint32 index = FindFirst(replaceThis, fromOffset);

		if(index >= 0)
		{
			Remove(index, strLenReplace);
			Insert(withThis, index);
			fromOffset = index + strLenWith;
			count++;
		}
		else break;

		if(count >= maxReplaceCount) break;
	}

	return *this;
}


EString&
EString::IReplaceFirst(char replaceThis, char withThis)
{
	eint32 index = IFindFirst(replaceThis);

	if(index >= 0) fBuffer[index] = withThis;

	return *this;
}


EString&
EString::IReplaceLast(char replaceThis, char withThis)
{
	eint32 index = IFindLast(replaceThis);

	if(index >= 0) fBuffer[index] = withThis;

	return *this;
}


EString&
EString::IReplaceAll(char replaceThis, char withThis, eint32 fromOffset)
{
	if(fromOffset < 0 || fromOffset >= fLen) return *this;

	while(fromOffset < fLen)
	{
		eint32 index = IFindFirst(replaceThis, fromOffset);

		if(index >= 0)
		{
			fBuffer[index] = withThis;
			fromOffset = index + 1;
		}
		else break;
	}

	return *this;
}


EString&
EString::IReplaceFirst(const char *replaceThis, const char *withThis)
{
	if(replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;

	eint32 index = IFindFirst(replaceThis);

	if(index >= 0)
	{
		Remove(index, (eint32)strlen(replaceThis));
		Insert(withThis, index);
	}

	return *this;
}


EString&
EString::IReplaceLast(const char *replaceThis, const char *withThis)
{
	if(replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;

	eint32 index = IFindLast(replaceThis);

	if(index >= 0)
	{
		Remove(index, (eint32)strlen(replaceThis));
		Insert(withThis, index);
	}

	return *this;
}


EString&
EString::IReplaceAll(const char *replaceThis, const char *withThis, eint32 fromOffset)
{
	if(fromOffset < 0 || fromOffset >= fLen) return *this;
	if(replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;

	eint32 strLenReplace = (eint32)strlen(replaceThis);
	eint32 strLenWith = (eint32)strlen(withThis);

	while(fromOffset < fLen)
	{
		eint32 index = IFindFirst(replaceThis, fromOffset);

		if(index >= 0)
		{
			Remove(index, strLenReplace);
			Insert(withThis, index);
			fromOffset = index + strLenWith;
		}
		else break;
	}

	return *this;
}


EString&
EString::IReplace(const char *replaceThis, const char *withThis, eint32 maxReplaceCount, eint32 fromOffset)
{
	if(fromOffset < 0 || fromOffset >= fLen) return *this;
	if(replaceThis == NULL || *replaceThis == 0 || withThis == NULL || *withThis == 0) return *this;

	if(maxReplaceCount == 0) return *this;
	if(maxReplaceCount < 0) return IReplaceAll(replaceThis, withThis, fromOffset);

	int count = 0;
	eint32 strLenReplace = (eint32)strlen(replaceThis);
	eint32 strLenWith = (eint32)strlen(withThis);

	while(true)
	{
		eint32 index = IFindFirst(replaceThis, fromOffset);

		if(index >= 0)
		{
			Remove(index, strLenReplace);
			Insert(withThis, index);
			fromOffset = index + strLenWith;
			count++;
		}
		else break;

		if(count >= maxReplaceCount) break;
	}

	return *this;
}


EString&
EString::ReplaceSet(const char *set_to_replace, char with)
{
	if(set_to_replace == NULL || *set_to_replace == 0 || with == 0) return *this;

	eint32 length = (eint32)strlen(set_to_replace);
	eint32 offset = 0;

	while(true)
	{
		eint32 index = -1;
		eint32 i;

		for(i = 0; i < length; i++)
		{
			index = FindFirst(set_to_replace[i], offset);
			if(index >= 0) break;
		}

		if(index >= 0)
		{
			Remove(index, 1);
			Insert(with, 1, index);
			offset = index + 1;
		}
		else break;
	}

	return *this;
}


EString&
EString::ReplaceSet(const char *set_to_replace, const char *with)
{
	if(set_to_replace == NULL || *set_to_replace == 0 || with == NULL || *with == 0) return *this;

	eint32 length = (eint32)strlen(set_to_replace);
	eint32 strLenWith = (eint32)strlen(with);
	eint32 offset = 0;

	while(true)
	{
		eint32 index = -1;
		eint32 i;

		for(i = 0; i < length; i++)
		{
			index = FindFirst(set_to_replace[i], offset);
			if(index >= 0) break;
		}

		if(index >= 0)
		{
			Remove(index, 1);
			Insert(with, index);
			offset = index + strLenWith;
		}
		else break;
	}

	return *this;
}



EString&
EString::IReplaceSet(const char *set_to_replace, char with)
{
	if(set_to_replace == NULL || *set_to_replace == 0 || with == 0) return *this;

	eint32 length = (eint32)strlen(set_to_replace);
	eint32 offset = 0;

	while(true)
	{
		eint32 index = -1;
		eint32 i;

		for(i = 0; i < length; i++)
		{
			index = IFindFirst(set_to_replace[i], offset);
			if(index >= 0) break;
		}

		if(index >= 0)
		{
			Remove(index, 1);
			Insert(with, 1, index);
			offset = index + 1;
		}
		else break;
	}

	return *this;
}


EString&
EString::IReplaceSet(const char *set_to_replace, const char *with)
{
	if(set_to_replace == NULL || *set_to_replace == 0 || with == NULL || *with == 0) return *this;

	eint32 length = (eint32)strlen(set_to_replace);
	eint32 strLenWith = (eint32)strlen(with);
	eint32 offset = 0;

	while(true)
	{
		eint32 index = -1;
		eint32 i;

		for(i = 0; i < length; i++)
		{
			index = IFindFirst(set_to_replace[i], offset);
			if(index >= 0) break;
		}

		if(index >= 0)
		{
			Remove(index, 1);
			Insert(with, index);
			offset = index + strLenWith;
		}
		else break;
	}

	return *this;
}


char
EString::operator[](eint32 index) const
{
	return fBuffer[index];
}


char
EString::ByteAt(eint32 index) const
{
	if(!fBuffer || index < 0 || index >= Length()) return 0;
	return fBuffer[index];
}


EString&
EString::ToLower()
{
	for(eint32 i = 0; i < fLen; i++) fBuffer[i] = tolower(fBuffer[i]);

	return *this;
}


EString&
EString::ToUpper()
{
	for(eint32 i = 0; i < fLen; i++) fBuffer[i] = toupper(fBuffer[i]);

	return *this;
}


EString&
EString::CharacterEscape(const char *original, const char *setOfCharsToEscape, char escapeWith)
{
	SetTo(original);
	return CharacterEscape(setOfCharsToEscape, escapeWith);
}


EString&
EString::CharacterEscape(const char *setOfCharsToEscape, char escapeWith)
{
	if(Length() <= 0 || setOfCharsToEscape == NULL || *setOfCharsToEscape == 0 || escapeWith == 0) return *this;

	eint32 length = (eint32)strlen(setOfCharsToEscape);
	eint32 offset = 0;

	while(true)
	{
		eint32 index = -1;
		eint32 i;

		for(i = 0; i < length; i++)
		{
			index = FindFirst(setOfCharsToEscape[i], offset);
			if(index >= 0) break;
		}

		if(index >= 0)
		{
			Insert(escapeWith, 1, index);
			offset = index + 2;
		}
		else break;
	}

	return *this;
}


EString&
EString::CharacterDeescape(const char *original, char escapeChar)
{
	SetTo(original);
	return CharacterDeescape(escapeChar);
}


EString&
EString::CharacterDeescape(char escapeChar)
{
	if(Length() <= 0 || escapeChar == 0) return *this;

	eint32 offset = 0;

	while(true)
	{
		eint32 index = -1;

		index = FindFirst(escapeChar, offset);

		if(index >= 0)
		{
			Remove(index, 1);
			offset = index + 1;
		}
		else break;
	}

	return *this;
}


EString&
EString::Capitalize()
{
	ToLower();

	if(Length() > 0) fBuffer[0] = toupper(fBuffer[0]);

	return *this;
}


EString&
EString::CapitalizeEachWord()
{
	ToLower();

	eint32 length = Length();
	if(length > 0)
	{
		if(fBuffer[0] != ' ') fBuffer[0] = toupper(fBuffer[0]);

		char *src = fBuffer;
		
		while(true)
		{
			char *tmp = strchr(src, ' ');

			if(!tmp) break;
			if(tmp - fBuffer >= length - 1) break;

			tmp++;
			if(*tmp != ' ')
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


bool
EString::IsNumber() const
{
	const char *s = String();
	eint32 length = Length();

	if(s == NULL) return false;
	if(length < 1) return false;

	if(!(length < 2 || s[0] != '0' || !(s[1] == 'x' || s[1] == 'X')))
	{
		eint32 i;
		for(i = 2; i < length; i++)
		{
			if(s[i] >= '0' && s[i] <= '9') continue;
			if(s[i] >= 'a' && s[i] <= 'f') continue;
			if(s[i] >= 'A' && s[i] <= 'F') continue;
			break;
		}

		return(i != 2 && i >= length);
	}

	if(s[0] == '-' || s[0] == '+')
	{
		if(length < 2) return false;
		if(strchr(&s[1], '-') != NULL || strchr(&s[1], '+') != NULL) return false; // double '-' or '+'
	}

	const char *tmp = strchr(s, '.');
	if(tmp != NULL)
	{
		tmp++;
		if(strchr(tmp, '.') != NULL) return false; // double '.'
	}

	for(eint32 i = 0; i < length; i++)
	{
		if(!(isdigit(s[i]) || s[i] == '-' || s[i] == '.' || s[i] == '+'))
		{
			if(s[i] != 'e' || s[i] != 'E' || i != length - 4) return false;
			if(s[i + 1] != '+' || s[i + 1] != '-') return false;
			if(!isdigit(s[i + 2]) || !isdigit(s[i + 3])) return false;
			break;
		}
	}

	return true;
}


bool
EString::IsInteger() const
{
	if(!IsNumber()) return false;

	const char *s = strchr(String(), '.');

	if(s == NULL) return true;

	if(strchr(s, '.') != NULL)
	{
		s++;
		eint32 length = (eint32)strlen(s);
		if(length > 0)
		{
			for(eint32 i = 0; i < length; i++)
			{
				if(s[i] != '0') return false;
			}
		}
	}

	return true;
}


bool
EString::IsDecimal() const
{
	if(!IsNumber()) return false;

	return(!IsInteger());
}


template<class TYPE_INT>
bool e_get_hex(const EString &str, TYPE_INT *value)
{
	if(value == NULL || str.Length() < 2 || str[0] != '0' || !(str[1] == 'x' || str[1] == 'X')) return false;

	*value = (TYPE_INT)0;

	eint32 maxLen = 2 + (eint32)sizeof(TYPE_INT) * 2;
	euint32 bitOffset = 0;

	for(eint32 i = min_c(str.Length(), maxLen) - 1; i >= 2; i--, bitOffset += 4)
	{
		int v = 0;
		if(str[i] >= '0' && str[i] <= '9') v = str[i] - '0';
		else if(str[i] >= 'a' && str[i] <= 'f') v = str[i] - 'a' + 10;
		else v = str[i] - 'A' + 10;
		*value |= (((TYPE_INT)v) << bitOffset);
	}

	return true;
}


bool
EString::GetDecimal(float *value) const
{
#if !(defined(_MSC_VER) && _MSC_VER <= 0x4b0)

	if(!value || !IsNumber()) return false;

	euint64 tmp;
	if(e_get_hex(*this, &tmp)) {*value = (float)tmp; return true;}

#ifdef HAVE_STRTOF
	*value = strtof(String(), NULL);
#else
	*value = (float)strtod(String(), NULL);
#endif

	return true;

#else
	// TODO
	return false;
#endif
}


bool
EString::GetDecimal(double *value) const
{
#if !(defined(_MSC_VER) && _MSC_VER <= 0x4b0)

	if(!value || !IsNumber()) return false;

	euint64 tmp;
	if(e_get_hex(*this, &tmp)) {*value = (double)tmp; return true;}

	*value = strtod(String(), NULL);

	return true;

#else
	// TODO
	return false;
#endif
}


#ifdef ETK_SUPPORT_LONG_DOUBLE
bool
EString::GetDecimal(long double *value) const
{
	if(!value || !IsNumber()) return false;

	euint64 tmp;
	if(e_get_hex(*this, &tmp)) {*value = (long double)tmp; return true;}

	*value = strtold(String(), NULL);

	return true;
}
#endif // ETK_SUPPORT_LONG_DOUBLE


bool
EString::GetInteger(eint8 *value) const
{
	if(!value || !IsNumber()) return false;

	*value = (eint8)strtol(String(), NULL, 10);

	return true;
}


bool
EString::GetInteger(euint8 *value) const
{
	if(!value || !IsNumber()) return false;

	if(e_get_hex(*this, value)) return true;

	*value = (euint8)strtoul(String(), NULL, 10);

	return true;
}


bool
EString::GetInteger(eint16 *value) const
{
	if(!value || !IsNumber()) return false;

	*value = (eint16)strtol(String(), NULL, 10);

	return true;
}


bool
EString::GetInteger(euint16 *value) const
{
	if(!value || !IsNumber()) return false;

	if(e_get_hex(*this, value)) return true;

	*value = (euint16)strtoul(String(), NULL, 10);

	return true;
}


bool
EString::GetInteger(eint32 *value) const
{
	if(!value || !IsNumber()) return false;

	if(e_get_hex(*this, value)) return true;

	*value = (eint32)strtol(String(), NULL, 10);

	return true;
}


bool
EString::GetInteger(euint32 *value) const
{
	if(!value || !IsNumber()) return false;

	if(e_get_hex(*this, value)) return true;

	*value = (euint32)strtoul(String(), NULL, 10);

	return true;
}


bool
EString::GetInteger(eint64 *value) const
{
	if(!value || !IsNumber()) return false;

	if(e_get_hex(*this, value)) return true;

#if SIZEOF_INT == 8 || SIZEOF_LONG == 8
	*value = (eint64)strtol(String(), NULL, 10);
	return true;
#else
#if SIZEOF_LONG_LONG == 8 && !defined(_MSC_VER)
	*value = (eint64)strtoll(String(), NULL, 10);
	return true;
#else
#if SIZEOF___INT64 == 8
#if _MSC_VER <= 0x4b0
	*value = (eint64)_atoi64(String());
#else
	*value = (eint64)_strtoi64(String(), NULL, 10);
#endif
	return true;
#else
	return false;
#endif
#endif
#endif
}


bool
EString::GetInteger(euint64 *value) const
{
#if !(defined(_MSC_VER) && _MSC_VER <= 0x4b0)
	if(!value || !IsNumber()) return false;

	if(e_get_hex(*this, value)) return true;

#if SIZEOF_INT == 8 || SIZEOF_LONG == 8
	*value = (euint64)strtoul(String(), NULL, 10);
	return true;
#else
#if SIZEOF_LONG_LONG == 8 && !defined(_MSC_VER)
	*value = (euint64)strtoull(String(), NULL, 10);
	return true;
#else
#if SIZEOF___INT64 == 8
	*value = (euint64)_strtoui64(String(), NULL, 10);
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


EString&
EString::operator<<(const char *str)
{
	return Append(str);
}


EString&
EString::operator<<(const EString &str)
{
	return Append(str);
}


EString&
EString::operator<<(char c)
{
	return Append(c, 1);
}


EString&
EString::operator<<(eint8 value)
{
	return AppendFormat("%I8i", value);
}


EString&
EString::operator<<(euint8 value)
{
	return AppendFormat("%I8u", value);
}


EString&
EString::operator<<(eint16 value)
{
	return AppendFormat("%I16i", value);
}


EString&
EString::operator<<(euint16 value)
{
	return AppendFormat("%I16u", value);
}


EString&
EString::operator<<(eint32 value)
{
	return AppendFormat("%I32i", value);
}


EString&
EString::operator<<(euint32 value)
{
	return AppendFormat("%I32u", value);
}


EString&
EString::operator<<(eint64 value)
{
	return AppendFormat("%I64i", value);
}


EString&
EString::operator<<(euint64 value)
{
	return AppendFormat("%I64u", value);
}


EString&
EString::operator<<(float value)
{
	return AppendFormat("%g", value);
}


EString&
EString::operator<<(double value)
{
	return AppendFormat("%g", value);
}


#ifdef ETK_SUPPORT_LONG_DOUBLE
EString&
EString::operator<<(long double value)
{
	return AppendFormat("%Lg", value);
}
#endif // ETK_SUPPORT_LONG_DOUBLE


EString&
EString::AppendFormat(const char *format, ...)
{
	if(!format) return *this;

	va_list args;

	char *buffer = NULL;

	va_start(args, format);

	buffer = e_strdup_vprintf(format, args);
	if(buffer)
	{
		Append(buffer);
		free(buffer);
	}

	va_end(args);

	return *this;
}


EString&
EString::PrependFormat(const char *format, ...)
{
	if(!format) return *this;

	va_list args;

	char *buffer = NULL;

	va_start(args, format);

	buffer = e_strdup_vprintf(format, args);
	if(buffer)
	{
		Prepend(buffer);
		free(buffer);
	}

	va_end(args);

	return *this;
}


EStringArray*
EString::Split(const char *delimiter, euint32 max_tokens) const
{
	if(delimiter == NULL || *delimiter == 0) return NULL;
	if(max_tokens == E_MAXUINT32) return NULL;

	eint32 delimiter_len = (eint32)strlen(delimiter);

	EStringArray *array = new EStringArray();
	if(!array) return NULL;

	eint32 offset = 0;
	eint32 found = 0;
	euint32 i = 0;

	while(true)
	{
		if(offset >= Length() || i >= max_tokens) break;

		if((found = FindFirst(delimiter, offset)) < 0) found = Length();
		i++;

		EString line;

		eint32 length = found - offset;

		if(length > 0) line.SetTo(String() + offset, length);

		array->AddItem(line);

		offset = found + delimiter_len;
	}

	if(array->CountItems() <= 0)
	{
		delete array;
		array = NULL;
	}

	return array;
}


EStringArray*
EString::Split(const char delimiter, euint32 max_tokens) const
{
	char string[2];
	bzero(string, 2);
	string[0] = delimiter;
	return Split(string, max_tokens);
}


eint32
EString::CountChars() const
{
	if(fLen <= 0) return fLen;
	return e_utf8_strlen(fBuffer);
}


const char*
EString::CharAt(eint32 index, euint8 *length) const
{
	if(length) *length = 0;
	if(fLen <= index) return NULL;
	return e_utf8_at(fBuffer, index, length);
}


template<class TYPE_INT>
void e_printf_int(EString &str, TYPE_INT value, euint8 _base, int precision_width, bool upper_style)
{
	if(str.Length() > 0 || !(_base == 8 || _base == 10 || _base == 16)) return;

	if(value == (TYPE_INT)0)
	{
		if(precision_width != 0)
			str.Append('0', max_c(precision_width, 1));
		return;
	}

	TYPE_INT base = (TYPE_INT)_base;

	while(value != (TYPE_INT)0)
	{
		int nIndex = (int)(value % base);
		if(nIndex < 0) nIndex = -nIndex;
		if(nIndex >= (int)_base) break;
		str.Prepend((char)(nIndex <= 9 ? '0' + nIndex : (upper_style ? 'A' : 'a') + nIndex - 10), 1);
		value /= base;
	}

	if(!(str.Length() < 0 || precision_width < 0 || str.Length() >= precision_width)) str.Prepend('0', precision_width - str.Length());
}


eint32 e_printf_double(EString &str, double value, int precision_width, eint32 *exponent, bool g_style, bool upper_style)
{
	if(exponent) *exponent = -1;
	if(str.Length() > 0 || (!g_style && exponent == NULL)) return -1;

#if defined(HAVE_ISFINITE) && defined(HAVE_ISNAN)
	if(isfinite(value) == 0)
	{
		if(upper_style)
			str = (isnan(value) ? "NAN" : "INF");
		else
			str = (isnan(value) ? "nan" : "inf");
		return -1;
	}
#endif

	if(precision_width < 0 && !g_style) precision_width = 6;
	else if(precision_width <= 0 && g_style) precision_width = (precision_width < 0 ? 6 : 1);

	eint32 digits = 0;
	double iValue = 0;
	double fValue = modf(value, &iValue);
	bool iValueZero = (iValue == 0);

	if(iValueZero)
	{
		str.Append("0");
		digits++;
	}
	else while(iValue != 0 && precision_width > 0)
	{
		double n = fmod(iValue, 10);
		int nIndex = (int)n;
		if(nIndex < 0) nIndex = -nIndex;
		if(nIndex >= 10) {str = (upper_style ? "NAN" : "nan"); return -1;}
		str.Prepend((char)('0' + nIndex), 1);
		modf(iValue / 10, &iValue);
		digits++;
	}

	eint32 exp = -1;
#if 0
	if(exponent != NULL) // 'g' or 'G' or 'e' or 'E' style
#else
	if(!g_style && exponent != NULL) // 'e' or 'E' style
#endif
	{
		exp = digits - 1;
		if(exp >= precision_width)
		{
			*exponent = precision_width + (g_style ? 0 : 1);
			str.Remove(*exponent, -1);

			str.Append(upper_style ? "E+" : "e+");
			EString nStr;
			e_printf_int(nStr, exp, 10, 2, false);
			str.Append(nStr);

			return 1;
		}

		precision_width -= (g_style ? digits : exp);
		if(!g_style) digits = 1;
	}

	while(fValue != 0 && precision_width > 0)
	{
		fValue *= 10;
		double n = 0;
		fValue = modf(fValue, &n);
		int nIndex = (int)n;
		if(nIndex < 0) nIndex = -nIndex;
		if(nIndex >= 10) {str = (upper_style ? "NAN" : "nan"); return -1;}
		else if(g_style && iValueZero && exp <= 0)
		{
			if(nIndex == 0)
			{
				exp--;
				continue;
			}
			else
			{
				exp--;
				if(exp < -4)
				{
					str = (char)('0' + nIndex);
					iValueZero = false;
					continue;
				}
				else
				{
					if(exp < -1)
					{
						str.Append('0', min_c(-exp - 1, precision_width));
						precision_width -= (-exp - 1);
						if(precision_width <= 0) break;
					}
					iValueZero = false;
				}
			}
		}
		str.Append((char)('0' + nIndex), 1);
		precision_width--;
	}

	if((g_style && exp < -4) || (!g_style && exponent != NULL))
	{
		*exponent = str.Length();

		str.Append(upper_style ? "E" : "e");
		str.Append(exp > 0 ? "+" : "-");
		EString nStr;
		e_printf_int(nStr, exp, 10, 2, false);
		str.Append(nStr);
	}

	return digits;
}


#ifdef ETK_SUPPORT_LONG_DOUBLE
eint32 e_printf_double(EString &str, long double value, int precision_width, eint32 *exponent, bool g_style, bool upper_style)
{
	if(exponent) *exponent = -1;
	if(str.Length() > 0 || (!g_style && exponent == NULL)) return -1;

#if defined(HAVE_ISFINITE) && defined(HAVE_ISNAN)
	if(isfinite(value) == 0)
	{
		if(upper_style)
			str = (isnan(value) ? "NAN" : "INF");
		else
			str = (isnan(value) ? "nan" : "inf");
		return -1;
	}
#endif

	if(precision_width < 0 && !g_style) precision_width = 6;
	else if(precision_width <= 0 && g_style) precision_width = (precision_width < 0 ? 6 : 1);

	eint32 digits = 0;
	long double iValue = 0;
	long double fValue = e_modfl(value, &iValue);
	bool iValueZero = (iValue == 0);

	if(iValueZero)
	{
		str.Append("0");
		digits++;
	}
	else while(iValue != 0 && precision_width > 0)
	{
		long double n = e_fmodl(iValue, 10);
		int nIndex = (int)n;
		if(nIndex < 0) nIndex = -nIndex;
		if(nIndex >= 10) {str = (upper_style ? "NAN" : "nan"); return -1;}
		str.Prepend((char)('0' + nIndex), 1);
		e_modfl(iValue / 10, &iValue);
		digits++;
	}

	eint32 exp = -1;
#if 0
	if(exponent != NULL) // 'g' or 'G' or 'e' or 'E' style
#else
	if(!g_style && exponent != NULL) // 'e' or 'E' style
#endif
	{
		exp = digits - 1;
		if(exp >= precision_width)
		{
			*exponent = precision_width + (g_style ? 0 : 1);
			str.Remove(*exponent, -1);

			str.Append(upper_style ? "E+" : "e+");
			EString nStr;
			e_printf_int(nStr, exp, 10, 2, false);
			str.Append(nStr);

			return 1;
		}

		precision_width -= (g_style ? digits : exp);
		if(!g_style) digits = 1;
	}

	while(fValue != 0 && precision_width > 0)
	{
		fValue *= 10;
		long double n = 0;
		fValue = e_modfl(fValue, &n);
		int nIndex = (int)n;
		if(nIndex < 0) nIndex = -nIndex;
		if(nIndex >= 10) {str = (upper_style ? "NAN" : "nan"); return -1;}
		else if(g_style && iValueZero && exp <= 0)
		{
			if(nIndex == 0)
			{
				exp--;
				continue;
			}
			else
			{
				exp--;
				if(exp < -4)
				{
					str = (char)('0' + nIndex);
					iValueZero = false;
					continue;
				}
				else
				{
					if(exp < -1)
					{
						str.Append('0', min_c(-exp - 1, precision_width));
						precision_width -= (-exp - 1);
						if(precision_width <= 0) break;
					}
					iValueZero = false;
				}
			}
		}
		str.Append((char)('0' + nIndex), 1);
		precision_width--;
	}

	if((g_style && exp < -4) || (!g_style && exponent != NULL))
	{
		*exponent = str.Length();

		str.Append(upper_style ? "E" : "e");
		str.Append(exp > 0 ? "+" : "-");
		EString nStr;
		e_printf_int(nStr, exp, 10, 2, false);
		str.Append(nStr);
	}

	return digits;
}
#endif // ETK_SUPPORT_LONG_DOUBLE


extern "C" {

_IMPEXP_ETK char* e_strndup(const char* src, eint32 length)
{
	char *dest;
	eint32 len = 0;

	if(src == NULL || *src == 0 || length == 0) return NULL;

	if(length < 0)
		len = (eint32)strlen(src);
	else
		len = min_c(length, (eint32)strlen(src));

	dest = (char*)malloc((size_t)(len + 1));

	if(!dest) return NULL;

	if(len != 0) memcpy(dest, src, (size_t)len);
	dest[len] = 0;

	return dest;
}


_IMPEXP_ETK char* e_strdup(const char* src)
{
	return e_strndup(src, -1);
}


_IMPEXP_ETK char* e_strdup_vprintf(const char *format, va_list ap)
{
	va_list args;
	EString str(format);

	if(str.Length() <= 0) return NULL;
	str.SetMinimumBufferSize(1024);

	va_copy(args, ap);

	eint32 offset = 0;
	while(!(offset < 0 || offset >= str.Length() || (offset = str.FindFirst("%", offset)) < 0))
	{
		eint32 curOffset = offset + 1;

		// flags
		bool do_alternate_form = false;
		bool left_align = false;
		bool show_positive = false;
		bool zero_padded = false;

		// field width
		bool has_precision_width = false;
		int field_width = 0;
		int precision_width = 0;

		// length modifier
		EString length_modifier;

		while(true)
		{
			bool doBreak = false;

			if(curOffset < 0 || curOffset >= str.Length())
			{
				str.Remove(offset, -1);
				break;
			}

			switch(str[curOffset])
			{
				case '0':
					if(!left_align) zero_padded = true;
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
					if(curOffset >= (str.Length() - 2)) break;
					has_precision_width = true;
					if(str[curOffset + 1] == '0')
					{
						curOffset++;
					}
					else if(str[curOffset + 1] == '*')
					{
						precision_width = va_arg(args, int);
						if(precision_width < 0) precision_width = 0;
						curOffset++;
					}
					else if(str[curOffset + 1] == '-' && str[curOffset + 2] >= '1' && str[curOffset + 2] <= '9')
					{
						precision_width = 0;
						curOffset++;
						while(true)
						{
							curOffset++;
							if(curOffset < 0 || curOffset >= str.Length()) break;
							if(str[curOffset] < '0' || str[curOffset] > '9')
							{
								curOffset--;
								break;
							}
						}
					}
					else if(str[curOffset + 1] >= '1' && str[curOffset + 1] <= '9')
					{
						curOffset++;

						EString strWidth;
						strWidth.Append(str[curOffset], 1);
						while(true)
						{
							curOffset++;
							if(curOffset < 0 || curOffset >= str.Length()) break;
							if(str[curOffset] < '0' || str[curOffset] > '9')
							{
								curOffset--;
								break;
							}

							strWidth.Append(str[curOffset], 1);
						}
						strWidth.GetInteger(&precision_width);
					}
					break;

				case '-':
					if(curOffset >= (str.Length() - 1) || str[curOffset + 1] < '1' || str[curOffset + 1] > '9')
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
						EString strWidth;
						strWidth.Append(str[curOffset], 1);
						while(true)
						{
							curOffset++;
							if(curOffset < 0 || curOffset >= str.Length()) break;
							if(str[curOffset] < '0' || str[curOffset] > '9')
							{
								curOffset--;
								break;
							}

							strWidth.Append(str[curOffset], 1);
						}
						strWidth.GetInteger(&field_width);
					}
					break;

				case '*':
					field_width = va_arg(args, int);
					break;

				case 'h':
					if(!(curOffset >= (str.Length() - 1) || str[curOffset + 1] != 'h'))
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
					if(!(curOffset >= (str.Length() - 1) || str[curOffset + 1] != 'l'))
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
					if(!(curOffset >= (str.Length() - 2) || str[curOffset + 1] != '6' || str[curOffset + 2] != '4'))
					{
						length_modifier = "I64";
						curOffset += 2;
					}
					else if(!(curOffset >= (str.Length() - 2) || str[curOffset + 1] != '3' || str[curOffset + 2] != '2'))
					{
						length_modifier = "I32";
						curOffset += 2;
					}
					else if(!(curOffset >= (str.Length() - 2) || str[curOffset + 1] != '1' || str[curOffset + 2] != '6'))
					{
						length_modifier = "I16";
						curOffset += 2;
					}
					else if(!(curOffset >= (str.Length() - 1) || str[curOffset + 1] != '8'))
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
						bool negative = false;
						bool has_signed;
						euint8 base;

						if(str[curOffset] == 'o') {has_signed = false; base = 8;}
						else if(str[curOffset] == 'u') {has_signed = false; base = 10;}
						else if(str[curOffset] == 'x' || str[curOffset] == 'X' || str[curOffset] == 'p') {has_signed = false; base = 16;}
						else {has_signed = true; base = 10;}

						EString aStr;
						if(length_modifier == "hh")
						{
							int value = va_arg(args, int);
							if(has_signed)
							{
								negative = (value < 0);
								e_printf_int(aStr, (char)value, base,
									     (has_precision_width ? max_c(precision_width, 0) : -1),
									     str[curOffset] == 'X');
							}
							else
							{
								e_printf_int(aStr, (unsigned char)value, base,
									     (has_precision_width ? max_c(precision_width, 0) : -1),
									     str[curOffset] == 'X');
							}
						}
						else if(length_modifier == "h")
						{
							int value = va_arg(args, int);
							if(has_signed)
							{
								negative = (value < 0);
								e_printf_int(aStr, (short int)value, base,
									     (has_precision_width ? max_c(precision_width, 0) : -1),
									     str[curOffset] == 'X');
							}
							else
							{
								e_printf_int(aStr, (unsigned short int)value, base,
									     (has_precision_width ? max_c(precision_width, 0) : -1),
									     str[curOffset] == 'X');
							}
						}
#ifdef HAVE_LONG_LONG
						else if(length_modifier == "ll")
						{
							long long int value = va_arg(args, long long int);
							if(has_signed)
							{
								negative = (value < (long long int)0);
								e_printf_int(aStr, value, base,
									     (has_precision_width ? max_c(precision_width, 0) : -1),
									     str[curOffset] == 'X');
							}
							else
							{
								e_printf_int(aStr, (unsigned long long int)value, base,
									     (has_precision_width ? max_c(precision_width, 0) : -1),
									     str[curOffset] == 'X');
							}
						}
#endif // HAVE_LONG_LONG
						else if(length_modifier == "I64")
						{
							eint64 value = va_arg(args, eint64);
							if(has_signed)
							{
								negative = (value < E_INT64_CONSTANT(0));
								e_printf_int(aStr, value, base,
									     (has_precision_width ? max_c(precision_width, 0) : -1),
									     str[curOffset] == 'X');
							}
							else
							{
								e_printf_int(aStr, (euint64)value, base,
									     (has_precision_width ? max_c(precision_width, 0) : -1),
									     str[curOffset] == 'X');
							}
						}
						else if(length_modifier == "I32")
						{
							eint32 value = va_arg(args, eint32);
							if(has_signed)
							{
								negative = (value < 0);
								e_printf_int(aStr, value, base,
									     (has_precision_width ? max_c(precision_width, 0) : -1),
									     str[curOffset] == 'X');
							}
							else
							{
								e_printf_int(aStr, (euint32)value, base,
									     (has_precision_width ? max_c(precision_width, 0) : -1),
									     str[curOffset] == 'X');
							}
						}
						else if(length_modifier == "I16")
						{
							int value = va_arg(args, int);
							if(has_signed)
							{
								negative = (value < 0);
								e_printf_int(aStr, (eint16)value, base,
									     (has_precision_width ? max_c(precision_width, 0) : -1),
									     str[curOffset] == 'X');
							}
							else
							{
								e_printf_int(aStr, (euint16)value, base,
									     (has_precision_width ? max_c(precision_width, 0) : -1),
									     str[curOffset] == 'X');
							}
						}
						else if(length_modifier == "I8")
						{
							int value = va_arg(args, int);
							if(has_signed)
							{
								negative = (value < 0);
								e_printf_int(aStr, (eint8)value, base,
									     (has_precision_width ? max_c(precision_width, 0) : -1),
									     str[curOffset] == 'X');
							}
							else
							{
								e_printf_int(aStr, (euint8)value, base,
									     (has_precision_width ? max_c(precision_width, 0) : -1),
									     str[curOffset] == 'X');
							}
						}
						else if(length_modifier == "l")
						{
							long int value = va_arg(args, long int);
							if(has_signed)
							{
								negative = (value < (long int)0);
								e_printf_int(aStr, value, base,
									     (has_precision_width ? max_c(precision_width, 0) : -1),
									     str[curOffset] == 'X');
							}
							else
							{
								e_printf_int(aStr, (unsigned long int)value, base,
									     (has_precision_width ? max_c(precision_width, 0) : -1),
									     str[curOffset] == 'X');
							}
						}
						else
						{
							int value = va_arg(args, int);
							if(has_signed)
							{
								negative = (value < 0);
								e_printf_int(aStr, value, base,
									     (has_precision_width ? max_c(precision_width, 0) : -1),
									     str[curOffset] == 'X');
							}
							else
							{
								e_printf_int(aStr, (unsigned int)value, base,
									     (has_precision_width ? max_c(precision_width, 0) : -1),
									     str[curOffset] == 'X');
							}
						}

						if(do_alternate_form)
						{
							if(str[curOffset] == 'o' && aStr.Length() > 0)
							{
								if(aStr[0] != 0) aStr.Prepend('0', 1);
							}
							else if((str[curOffset] == 'x' || str[curOffset] == 'X' || str[curOffset] == 'p') && aStr.Length() > 0)
							{
								aStr.Prepend(str[curOffset] == 'X' ? "0X" : "0x");
							}
						}

						if(show_positive) aStr.Prepend(negative ? '-' : '+', 1);
						else if(negative) aStr.Prepend('-', 1);

						if(aStr.Length() < field_width)
						{
							if(!left_align)
							{
								char cStr = zero_padded && !has_precision_width ? '0' : ' ';
								eint32 cOffset = (cStr == '0' && (show_positive || negative) ? 1 : 0);
								eint32 cCount = field_width - aStr.Length();
								if(cOffset != 0) cCount--;
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
						bool negative = false;;
						eint32 exponent = -1;
						bool g_style = (str[curOffset] == 'g' || str[curOffset] == 'G');
						eint32 digits = -1;

						EString aStr;

						if(length_modifier != "L")
						{
							double value = va_arg(args, double);
							negative = (value < 0);
							digits = e_printf_double(aStr, value, (has_precision_width ? precision_width : -1),
										 &exponent, g_style, isupper(str[curOffset]) != 0);
						}
						else
						{
#ifdef ETK_SUPPORT_LONG_DOUBLE
							long double value = va_arg(args, long double);
							negative = (value < 0);
							digits = e_printf_double(aStr, value, (has_precision_width ? precision_width : -1),
										 &exponent, g_style, isupper(str[curOffset]) != 0);
#else
							ETK_DEBUG("[SUPPORT]: ETK++ don't support \"long double\".");
							(void)va_arg(args, long double);
							str.Remove(offset, curOffset - offset + 1);
							doBreak = true;
							break;
#endif
						}

						if(digits > 0 && str.Length() > 0)
						{
							if(exponent < 0 && digits < aStr.Length() && g_style)
							{
								for(eint32 i = aStr.Length() - 1; i > digits - 1; i--)
								{
									if(aStr[i] != '0')
									{
										aStr.Remove(i + 1, -1);
										break;
									}
								}
							}

							if(digits < (exponent < 0 ? aStr.Length() : exponent) || do_alternate_form)
								aStr.Insert('.', 1, digits);
						}

						if(show_positive) aStr.Prepend(negative ? '-' : '+', 1);
						else if(negative) aStr.Prepend('-', 1);

						if(aStr.Length() < field_width)
						{
							if(!left_align)
							{
								char cStr = zero_padded && !has_precision_width ? '0' : ' ';
								eint32 cOffset = (cStr == '0' && (show_positive || negative) ? 1 : 0);
								eint32 cCount = field_width - aStr.Length();
								if(cOffset != 0) cCount--;
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
						EString aStr;
						if(str[curOffset] == 's')
						{
							char *string = va_arg(args, char*);
							aStr = (string ? string : "(null)");
						}
						else
						{
							aStr = (char)va_arg(args, int);
						}

						if(has_precision_width && aStr.Length() > max_c(precision_width, 0))
						{
							aStr.Remove(precision_width, -1);
						}
						if(aStr.Length() < field_width)
						{
							if(!left_align)
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
					if(curOffset == offset + 1)
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

			if(doBreak) break;
			else if(curOffset >= 0) curOffset++;
		}

		offset = curOffset;
	}

	va_end(args);

	return (str.Length() > 0 ? e_strdup(str.String()) : NULL);
}


_IMPEXP_ETK char* e_strdup_printf(const char *format, ...)
{
	va_list args;
	char *buffer = NULL;

	if(!format) return NULL;

	va_start(args, format);

	buffer = e_strdup_vprintf(format, args);

	va_end(args);

	return buffer;
}


_IMPEXP_ETK bool e_utf8_is_token(const char *str)
{
	if(str == NULL) return true;

	const unsigned char *p = (const unsigned char*)str;
	return(*p < 0x80 || !(*p < 0xc0 || *p >= 0xfe));
}



_IMPEXP_ETK eint32 e_utf8_strlen(const char *str)
{
	return e_utf8_strlen_etc(str, -1);
}


_LOCAL eint32 _e_utf8_strlen_etc(const char *str, eint32 nbytes, bool check)
{
	if(str == NULL || *str == 0 || nbytes == 0) return 0;

	eint32 strLen = (eint32)strlen(str);
	if(nbytes < 0 || nbytes > strLen) nbytes = strLen;

	const unsigned char *p = (const unsigned char*)str;
	eint32 uLen = 0;

	while(*p)
	{
		eint8 len = 0;

		if(*p < 0x80) len = 1; // 0xxxxxxx : ASCII
		else if(*p < 0xc0 || *p >= 0xfe) len = 0; // 10xxxxxx or 1111111x : invalid UTF8
		else if(*p < 0xe0) len = 2; // 110xxxxx : 2 bytes
		else if(*p < 0xf0) len = 3; // 1110xxxx : 3 bytes
		else if(*p < 0xf8) len = 4; // 11110xxx : 4 bytes
		else if(*p < 0xfc) len = 5; // 111110xx : 5 bytes, it's invalid UTF8 util this wrote
		else if(*p < 0xfe) len = 6; // 1111110x : 6 bytes, it's invalid UTF8 util this wrote

		if(!check)
		{
			p += max_c(len, 1);
		}
		else
		{
			for(eint8 i = len; i >= 0; i--)
			{
				p++;
				if(i <= 1) break;
				if(*p < 0x80 || *p >= 0xc0) i = len = 0; // 0xxxxxxx or 11xxxxxx : invalid UTF8
			}
		}

		if((eint32)(p - (const unsigned char*)str) > nbytes) break;

		if(len > 0 && len <= 4) uLen++;
	}

	return uLen;
}


_IMPEXP_ETK eint32 e_utf8_strlen_etc(const char *str, eint32 nbytes)
{
	return _e_utf8_strlen_etc(str, nbytes, true);
}


_IMPEXP_ETK eint32 e_utf8_strlen_fast(const char *str, eint32 nbytes)
{
	return _e_utf8_strlen_etc(str, nbytes, false);
}


_IMPEXP_ETK const char* e_utf8_at(const char *str, eint32 index, euint8 *length)
{
	if(length) *length = 0;
	if(index < 0 || str == NULL || strlen(str) <= (size_t)index) return NULL;

	const unsigned char *p = (const unsigned char*)str;
	eint32 uLen = 0;
	eint8 len = 0;

	while(*p && index >= uLen)
	{
		if(*p < 0x80) len = 1; // 0xxxxxxx : ASCII
		else if(*p < 0xc0 || *p >= 0xfe) len = 0; // 10xxxxxx or 1111111x : invalid UTF8
		else if(*p < 0xe0) len = 2; // 110xxxxx : 2 bytes
		else if(*p < 0xf0) len = 3; // 1110xxxx : 3 bytes
		else if(*p < 0xf8) len = 4; // 11110xxx : 4 bytes
		else if(*p < 0xfc) len = 5; // 111110xx : 5 bytes, it's invalid UTF8 util this wrote
		else if(*p < 0xfe) len = 6; // 1111110x : 6 bytes, it's invalid UTF8 util this wrote

		for(eint8 i = len; i >= 0; i--)
		{
			p++;
			if(i <= 1) break;
			if(*p < 0x80 || *p >= 0xc0) i = len = -(len - i + 1); // 0xxxxxxx or 11xxxxxx : invalid UTF8
		}

		if(len > 0 && len <= 4) uLen++;
	}

	if(index == uLen - 1)
	{
		if(len < 0) {p += len; len = 0;}
		else p -= len;
		if(length) *length = (euint8)len;
		return (const char*)p;
	}

	return NULL;
}


_IMPEXP_ETK const char* e_utf8_next(const char *str, euint8 *length)
{
	if(length) *length = 0;
	if(str == NULL || *str == 0) return NULL;

	const unsigned char *p = (const unsigned char*)str;
	bool first = true;

	while(*p)
	{
		eint8 len = 0;

		if(*p < 0x80) len = 1; // 0xxxxxxx : ASCII
		else if(*p < 0xc0 || *p >= 0xfe) len = 0; // 10xxxxxx or 1111111x : invalid UTF8
		else if(*p < 0xe0) len = 2; // 110xxxxx : 2 bytes
		else if(*p < 0xf0) len = 3; // 1110xxxx : 3 bytes
		else if(*p < 0xf8) len = 4; // 11110xxx : 4 bytes
		else if(*p < 0xfc) len = 5; // 111110xx : 5 bytes, it's invalid UTF8 util this wrote
		else if(*p < 0xfe) len = 6; // 1111110x : 6 bytes, it's invalid UTF8 util this wrote

		for(eint8 i = len; i >= 0; i--)
		{
			p++;
			if(i <= 1) break;
			if(*p < 0x80 || *p >= 0xc0) i = len = -(len - i + 1); // 0xxxxxxx or 11xxxxxx : invalid UTF8
		}

		if(!first && len > 0 && len <= 4)
		{
			if(length) *length = len;
			p -= len;
			return (const char*)p;
		}
		else
		{
			first = false;
		}
	}

	return NULL;
}


_IMPEXP_ETK eunichar* e_utf8_convert_to_unicode(const char *str, eint32 length)
{
	if(str == NULL || *str == 0 || length == 0) return NULL;

	eint32 strLen = (eint32)strlen(str);
	if(length < 0 || length > strLen) length = strLen;

	eunichar* unicode = (eunichar*)malloc(sizeof(eunichar) * (size_t)(length + 1));
	if(!unicode) return NULL;

	const unsigned char *p = (const unsigned char*)str;
	eunichar *tmp = unicode;

	while(*p)
	{
		eint8 len = 0;

		if(*p < 0x80) len = 1; // 0xxxxxxx : ASCII
		else if(*p < 0xc0 || *p >= 0xfe) len = 0; // 10xxxxxx or 1111111x : invalid UTF8
		else if(*p < 0xe0) len = 2; // 110xxxxx : 2 bytes
		else if(*p < 0xf0) len = 3; // 1110xxxx : 3 bytes
		else if(*p < 0xf8) len = 4; // 11110xxx : 4 bytes
		else if(*p < 0xfc) len = 5; // 111110xx : 5 bytes, it's invalid UTF8 util this wrote
		else if(*p < 0xfe) len = 6; // 1111110x : 6 bytes, it's invalid UTF8 util this wrote

		for(eint8 i = len; i >= 0; i--)
		{
			p++;
			if(i <= 1) break;
			if(*p < 0x80 || *p >= 0xc0) i = len = 0; // 0xxxxxxx or 11xxxxxx : invalid UTF8
		}

		if((eint32)((const char*)p - str) > length) break;

		if(len > 0)
		{
			p -= len;
			if(len == 1)
			{
				*tmp = (eunichar)(*p++);
			}
			else if(len == 2)
			{
				*tmp = ((eunichar)(*p++) & 0x1f) << 6;
				*tmp |= (eunichar)(*p++) & 0x3f;
			}
			else if(len == 3)
			{
				*tmp = ((eunichar)(*p++) & 0x0f) << 12;
				*tmp |= ((eunichar)(*p++) & 0x3f) << 6;
				*tmp |= (eunichar)(*p++) & 0x3f;
			}
			else if(len == 4)
			{
				eunichar32 tmp32 = ((eunichar32)(*p++) & 0x07) << 18;
				tmp32 |= ((eunichar32)(*p++) & 0x3f) << 12;
				tmp32 |= ((eunichar32)(*p++) & 0x3f) << 6;
				tmp32 |= (eunichar32)(*p++) & 0x3f;

				if(tmp32 > 0xffff)
				{
					*tmp++ = 0xd800 | ((tmp32 - 0x10000) >> 10);
					*tmp = 0xdc00 | ((tmp32 - 0x10000) & 0x03ff);
				}
				else
				{
					*tmp = tmp32;
				}
			}
			else {p += len; continue;} // don't support the 5 or 6 bytes UTF-8

			tmp++;
		}
	}
	*tmp = 0;

	return unicode;
}


_IMPEXP_ETK eunichar32* e_utf8_convert_to_utf32(const char *str, eint32 length)
{
	if(str == NULL || *str == 0 || length == 0) return NULL;

	eint32 strLen = (eint32)strlen(str);
	if(length < 0 || length > strLen) length = strLen;

	eunichar32* unicode = (eunichar32*)malloc(sizeof(eunichar32) * (size_t)(length + 1));
	if(unicode == NULL) return NULL;

	const unsigned char *p = (const unsigned char*)str;
	eunichar32 *tmp = unicode;

	while(*p)
	{
		eint8 len = 0;

		if(*p < 0x80) len = 1; // 0xxxxxxx : ASCII
		else if(*p < 0xc0 || *p >= 0xfe) len = 0; // 10xxxxxx or 1111111x : invalid UTF8
		else if(*p < 0xe0) len = 2; // 110xxxxx : 2 bytes
		else if(*p < 0xf0) len = 3; // 1110xxxx : 3 bytes
		else if(*p < 0xf8) len = 4; // 11110xxx : 4 bytes
		else if(*p < 0xfc) len = 5; // 111110xx : 5 bytes, it's invalid UTF8 util this wrote
		else if(*p < 0xfe) len = 6; // 1111110x : 6 bytes, it's invalid UTF8 util this wrote

		for(eint8 i = len; i >= 0; i--)
		{
			p++;
			if(i <= 1) break;
			if(*p < 0x80 || *p >= 0xc0) i = len = 0; // 0xxxxxxx or 11xxxxxx : invalid UTF8
		}

		if((eint32)((const char*)p - str) > length) break;

		if(len > 0)
		{
			p -= len;
			if(len == 1)
			{
				*tmp = (eunichar32)(*p++);
			}
			else if(len == 2)
			{
				*tmp = ((eunichar32)(*p++) & 0x1f) << 6;
				*tmp |= (eunichar32)(*p++) & 0x3f;
			}
			else if(len == 3)
			{
				*tmp = ((eunichar32)(*p++) & 0x0f) << 12;
				*tmp |= ((eunichar32)(*p++) & 0x3f) << 6;
				*tmp |= (eunichar32)(*p++) & 0x3f;
			}
			else if(len == 4)
			{
				*tmp = ((eunichar32)(*p++) & 0x07) << 18;
				*tmp |= ((eunichar32)(*p++) & 0x3f) << 12;
				*tmp |= ((eunichar32)(*p++) & 0x3f) << 6;
				*tmp |= (eunichar32)(*p++) & 0x3f;
			}
			else {p += len; continue;} // don't support the 5 or 6 bytes UTF-8

			tmp++;
		}
	}
	*tmp = 0;

	return unicode;
}


_IMPEXP_ETK eint32 e_unicode_strlen(const eunichar *ustr)
{
	return e_unicode_strlen_etc(ustr, -1, true);
}


_IMPEXP_ETK eint32 e_unicode_strlen_etc(const eunichar *ustr, eint32 nchars, bool utf16_style)
{
	if(ustr == NULL || nchars == 0) return 0;

	eint32 len = 0;
	const eunichar *p = ustr;

	while(*p)
	{
		if(utf16_style ? (*p >= 0xd800 && *p <= 0xdfff) : false)
		{
			p++;
			if(*p >= 0xdc00 && *p <= 0xdfff)
			{
				if(nchars > 0) if((eint32)(p - ustr) > nchars - 1) break;
				p++;
				len++;
			}
		}
		else
		{
			if(nchars > 0) if((eint32)(p - ustr) > nchars - 1) break;
			p++;
			len++;
		}
	}

	return len;
}


_IMPEXP_ETK const eunichar* e_unicode_at(const eunichar* ustr, eint32 index, bool *utf16)
{
	if(utf16) *utf16 = false;
	if(ustr == NULL || index < 0) return NULL;

	eint32 len = 0;
	const eunichar *p = ustr;

	while(*p)
	{
		bool _utf16_ = false;

		if(*p >= 0xd800 && *p <= 0xdfff)
		{
			p++;
			if(*p >= 0xdc00 && *p <= 0xdfff) {p++; len++; _utf16_ = true;}
		}
		else
		{
			p++;
			len++;
		}

		if(index == len - 1)
		{
			if(utf16) *utf16 = _utf16_;
			p--; if(_utf16_) p--;
			return p;
		}
	}

	return NULL;
}


_IMPEXP_ETK const eunichar* e_unicode_next(const eunichar* ustr, bool *utf16)
{
	if(utf16) *utf16 = false;
	if(ustr == NULL) return NULL;

	const eunichar *p = ustr;
	bool first = true;

	while(*p)
	{
		if(*p >= 0xd800 && *p <= 0xdfff)
		{
			p++;
			if(*p >= 0xdc00 && *p <= 0xdfff) {if(!first) {if(utf16) *utf16 = true; p--; return p;} else {first = false; p++;}}
		}
		else
		{
			if(!first) {if(utf16) *utf16 = false; return p;} else {first = false; p++;}
		}
	}

	return NULL;
}


_IMPEXP_ETK char* e_unicode_convert_to_utf8(const eunichar *str, eint32 ulength)
{
	if(str == NULL || *str == 0 || ulength == 0) return NULL;

	EString utf8;

	eint32 ulen = 0;
	const eunichar *p = str;

	while(*p != 0 && (ulength > 0 ? ulength > ulen : true))
	{
		if(*p >= 0xd800 && *p <= 0xdfff)
		{
			p++;
			if(*p >= 0xdc00 && *p <= 0xdfff)
			{
				p--;

				// convert UTF-16 to UCS4
				euint32 tmp = (euint32)(*p++ & 0x03ff) << 10;
				tmp |= (euint32)(*p++ & 0x03ff);
				tmp += 0x10000;

				// convert UCS4 to UTF-8
				euint8 ustr[5];
				bzero(ustr, sizeof(ustr));
				ustr[3] = 0x80 | (euint8)(tmp & 0x3f);
				ustr[2] = 0x80 | (euint8)((tmp >> 6) & 0x3f);
				ustr[1] = 0x80 | (euint8)((tmp >> 12) & 0x3f);
				ustr[0] = 0xf0 | (euint8)((tmp >> 18) & 0x07);
				utf8.Append((const char*)ustr);

				ulen++;
			}
		}
		else
		{
			euint8 len;
			if(*p < 0x80) len = 1;
			else if(*p < 0x0800) len = 2;
			else len = 3;

			euint16 tmp = *p++;

			euint8 ustr[4];
			bzero(ustr, sizeof(ustr));

			if(len == 1)
			{
				ustr[0] = (euint8)tmp;
			}
			else if(len == 2)
			{
				ustr[1] = 0x80 | (euint8)(tmp & 0x3f);
				ustr[0] = 0xc0 | (euint8)((tmp >> 6) & 0x1f);
			}
			else // len == 3
			{
				ustr[2] = 0x80 | (euint8)(tmp & 0x3f);
				ustr[1] = 0x80 | (euint8)((tmp >> 6) & 0x3f);
				ustr[0] = 0xe0 | (euint8)((tmp >> 12) & 0x0f);
			}

			utf8.Append((const char*)ustr);

			ulen++;
		}
	}

	return e_strndup(utf8.String(), -1);
}


_IMPEXP_ETK eunichar32* e_unicode_convert_to_utf32(const eunichar *str, eint32 ulength)
{
	if(str == NULL || *str == 0 || ulength == 0) return NULL;

	eint32 unicode_length = (ulength < 0 ? e_unicode_strlen(str) : ulength);
	if(unicode_length <= 0) return NULL;

	eunichar32* utf32 = (eunichar32*)malloc(sizeof(eunichar32) * (size_t)(unicode_length + 1));
	if(!utf32) return NULL;

	eint32 ulen = 0;
	const eunichar *p = str;

	while(*p != 0 && (ulength > 0 ? ulength > ulen : true))
	{
		if(*p >= 0xd800 && *p <= 0xdfff)
		{
			p++;
			if(*p >= 0xdc00 && *p <= 0xdfff)
			{
				p--;

				// convert UTF-16 to UCS4
				euint32 tmp = ((euint32)(*p++ & 0x03ff) << 10);
				tmp |= (euint32)(*p++ & 0x03ff);
				tmp += 0x10000;

				utf32[ulen++] = tmp;

				ulen++;
			}
		}
		else
		{
			utf32[ulen++] = (eunichar32)(*p++);
		}
	}

	utf32[ulen] = 0;

	return utf32;
}


_IMPEXP_ETK eint32 e_utf32_strlen(const eunichar32 *ustr)
{
	return e_utf32_strlen_etc(ustr, -1);
}


_IMPEXP_ETK eint32 e_utf32_strlen_etc(const eunichar32 *ustr, eint32 nchars)
{
	if(ustr == NULL || nchars == 0) return 0;

	eint32 len = 0;
	const eunichar32 *p = ustr;

	while(*p)
	{
		if(nchars > 0) if((eint32)(p - ustr) > nchars - 1) break;
		if(*p <= 0x10ffff) len++;
		p++;
	}

	return len;
}


_IMPEXP_ETK const eunichar32* e_utf32_at(const eunichar32* ustr, eint32 index)
{
	if(ustr == NULL || index < 0) return NULL;

	eint32 len = 0;
	const eunichar32 *p = ustr;

	while(*p)
	{
		if(*p <= 0x10ffff)
		{
			if(index == len) return p;
			else len++;
		}
		p++;
	}

	return NULL;
}


_IMPEXP_ETK const eunichar32* e_utf32_next(const eunichar32* ustr)
{
	if(ustr == NULL) return NULL;

	const eunichar32 *p = ustr;

	while(*p)
	{
		p++;
		if(*p > 0 && *p <= 0x10ffff) return p;
	}

	return NULL;
}


_IMPEXP_ETK char* e_utf32_convert_to_utf8(const eunichar32 *str, eint32 ulength)
{
	if(str == NULL || *str == 0 || ulength == 0) return NULL;

	EString utf8;

	eint32 ulen = 0;
	const eunichar32 *p = str;

	while(*p != 0 && (ulength > 0 ? ulength > ulen : true))
	{
		if(*p > 0xffff && *p <= 0x10ffff)
		{
			euint32 tmp = *p;

			// convert UCS4 to UTF-8
			euint8 ustr[5];
			bzero(ustr, sizeof(ustr));
			ustr[3] = 0x80 | (euint8)(tmp & 0x3f);
			ustr[2] = 0x80 | (euint8)((tmp >> 6) & 0x3f);
			ustr[1] = 0x80 | (euint8)((tmp >> 12) & 0x3f);
			ustr[0] = 0xf0 | (euint8)((tmp >> 18) & 0x07);
			utf8.Append((const char*)ustr);

			ulen++;
		}
		else if(*p <= 0xffff)
		{
			euint8 len;
			if(*p < 0x80) len = 1;
			else if(*p < 0x0800) len = 2;
			else len = 3;

			euint32 tmp = *p;

			euint8 ustr[4];
			bzero(ustr, sizeof(ustr));

			if(len == 1)
			{
				ustr[0] = (euint8)tmp;
			}
			else if(len == 2)
			{
				ustr[1] = 0x80 | (euint8)(tmp & 0x3f);
				ustr[0] = 0xc0 | (euint8)((tmp >> 6) & 0x1f);
			}
			else // len == 3
			{
				ustr[2] = 0x80 | (euint8)(tmp & 0x3f);
				ustr[1] = 0x80 | (euint8)((tmp >> 6) & 0x3f);
				ustr[0] = 0xe0 | (euint8)((tmp >> 12) & 0x0f);
			}

			utf8.Append((const char*)ustr);

			ulen++;
		}

		p++;
	}

	return e_strndup(utf8.String(), -1);
}


_IMPEXP_ETK eunichar* e_utf32_convert_to_unicode(const eunichar32 *str, eint32 ulength)
{
	if(str == NULL || *str == 0 || ulength == 0) return NULL;

	eint32 utf32_length = (ulength < 0 ? e_utf32_strlen(str) : ulength);
	if(utf32_length <= 0) return NULL;

	eunichar* unicode = (eunichar*)malloc(sizeof(eunichar) * (size_t)(2 * utf32_length + 1));
	if(!unicode) return NULL;

	eint32 ulen = 0;
	const eunichar32 *p = str;
	eunichar *tmp = unicode;

	while(*p != 0 && (ulength > 0 ? ulength > ulen : true))
	{
		if(*p > 0xffff && *p <= 0x10ffff)
		{
			*tmp++ = 0xd800 | ((*p - 0x10000) >> 10);
			*tmp++ = 0xdc00 | ((*p - 0x10000) & 0x03ff);
			ulen++;
		}
		else if(*p <= 0xffff)
		{
			*tmp++ = (eunichar)(*p);
			ulen++;
		}

		p++;
	}

	*tmp = 0;

	return unicode;
}

} // extern "C"

