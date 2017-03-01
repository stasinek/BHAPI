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
 * File: StringMe.h
 * Description: String --- string allocation and manipulation
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_STRINGME__H
#define BHAPI_STRINGME__H
#include "../support/SupportDefs.h"
#ifdef __cplusplus /* Just for C++ */
extern "C" {
#endif /* __cplusplus */
#ifdef __STRICT_ANSI__
#define __SA__
#undef __STRICT_ANSI__
#endif
namespace bhapi {
int __cdecl __MINGW_NOTHROW	strcasecmp(const char *a, const char *b);
int __cdecl __MINGW_NOTHROW	strncasecmp(const char *a, const char *b, size_t c);
//#define bhapi::strncasecmp(a,b,c) _strnicmp(a,b,c)
}
#ifdef __SA__
#define __STRICT_ANSI__
#undef __SA__
#endif

namespace bhapi {
/* the result must be free by "free" */
IMPEXP_BHAPI char*		strdup(const char *src);
IMPEXP_BHAPI char*		strndup(const char *src, b_int32 length);
IMPEXP_BHAPI char*		strdup_vprintf(const char *format, va_list ap);
IMPEXP_BHAPI char*		strdup_printf(const char *format, ...);

IMPEXP_BHAPI b_unichar16*		utf8_convert_to_unicode(const char *str, b_int32 length);
IMPEXP_BHAPI b_unichar32*		utf8_convert_to_utf32(const char *str, b_int32 length);
IMPEXP_BHAPI char*		unicode_convert_to_utf8(const b_unichar16*str, b_int32 ulength);
IMPEXP_BHAPI b_unichar32*		unicode_convert_to_utf32(const b_unichar16*str, b_int32 ulength);
IMPEXP_BHAPI char*		utf32_convert_to_utf8(const b_unichar32 *str, b_int32 ulength);
IMPEXP_BHAPI b_unichar16*		utf32_convert_to_unicode(const b_unichar32 *str, b_int32 ulength);

IMPEXP_BHAPI bool		utf8_is_token(const char *str);
IMPEXP_BHAPI b_int32		utf8_strlen(const char *str);
IMPEXP_BHAPI b_int32		utf8_strlen_etc(const char *str, b_int32 nbytes);
IMPEXP_BHAPI b_int32		utf8_strlen_fast(const char *str, b_int32 nbytes); /* none checking */
IMPEXP_BHAPI const char*		utf8_at(const char *str, b_int32 index, b_uint8 *nbytes);
IMPEXP_BHAPI const char*		utf8_next(const char *str, b_uint8 *length);

IMPEXP_BHAPI b_int32		unicode_strlen(const b_unichar16*ustr);
IMPEXP_BHAPI b_int32		unicode_strlen_etc(const b_unichar16*ustr, b_int32 nchars, bool utf16_style);
IMPEXP_BHAPI const b_unichar16*	unicode_at(const b_unichar16* ustr, b_int32 index, bool *utf16);
IMPEXP_BHAPI const b_unichar16*	unicode_next(const b_unichar16* ustr, bool *utf16);

IMPEXP_BHAPI b_int32		utf32_strlen(const b_unichar32 *ustr);
IMPEXP_BHAPI b_int32		utf32_strlen_etc(const b_unichar32 *ustr, b_int32 nchars);
IMPEXP_BHAPI const b_unichar32*	utf32_at(const b_unichar32* ustr, b_int32 index);
IMPEXP_BHAPI const b_unichar32*	utf32_next(const b_unichar32* ustr);
}

#ifdef __cplusplus /* Just for C++ */
} // extern "C"
// bhapi::strdup_dirty(): like bhapi::strdup(), but the result must be free by "delete[]"
namespace bhapi {
IMPEXP_BHAPI char *strdup_dirty(const char* src, b_int32 length = -1);
}
class BStringArray;
class IMPEXP_BHAPI BString {
public:
    BString();
    BString(const char *str);
    BString(const BString &str);
    BString(const char *str, b_int32 maxLength);
    ~BString();

    const char	*String() const;

    b_int32		Length() const; // ASCII
    b_int32		CountChars() const; // UTF-8

    char		operator[](b_int32 index) const; // ASCII
    char		ByteAt(b_int32 index) const; // ASCII
    const char*	CharAt(b_int32 index, b_uint8 *length = NULL) const; // UTF-8

    BString 	&operator=(const BString &str);
    BString 	&operator=(const char *str);
    BString 	&operator=(char c);

    BString		&SetTo(const BString &str);
    BString		&SetTo(const BString &str, b_int32 length);
    BString		&SetTo(const char *str);
    BString		&SetTo(const char *str, b_int32 length);
    BString		&SetTo(char c, b_int32 count);

    BString		&Adopt(BString &from);
    BString		&Adopt(BString &from, b_int32 length);

    BString		&CopyInto(BString &into, b_int32 fromOffset, b_int32 length) const;
    void		CopyInto(char *into, size_t into_size, b_int32 fromOffset, b_int32 length) const;

    BString		&MoveInto(BString &into, b_int32 from, b_int32 length);
    void		MoveInto(char *into, size_t into_size, b_int32 from, b_int32 length);

	void		MakeEmpty();

    BString		&operator+=(const BString &str);
    BString		&operator+=(const char *str);
    BString		&operator+=(char c);

    BString		&Append(const BString &str);
    BString		&Append(const BString &str, b_int32 length);
    BString		&Append(const char *str);
    BString		&Append(const char *str, b_int32 length);
    BString		&Append(char c, b_int32 count);
    BString		&AppendFormat(const char *format, ...);

    BString		&Prepend(const BString &str);
    BString		&Prepend(const BString &str, b_int32 length);
    BString		&Prepend(const char *str);
    BString		&Prepend(const char *str, b_int32 length);
    BString		&Prepend(char c, b_int32 count);
    BString		&PrependFormat(const char *format, ...);

    BString		&Insert(const BString &str, b_int32 pos);
    BString		&Insert(const BString &str, b_int32 length, b_int32 pos);
    BString		&Insert(const BString &str, b_int32 fromOffset, b_int32 length, b_int32 pos);
    BString		&Insert(const char *str, b_int32 pos);
    BString		&Insert(const char *str, b_int32 length, b_int32 pos);
    BString		&Insert(const char *str, b_int32 fromOffset, b_int32 length, b_int32 pos);
    BString		&Insert(char c, b_int32 count, b_int32 pos);

    BString		&Truncate(b_int32 newLength);

    BString		&Remove(b_int32 from, b_int32 length);

    BString		&RemoveFirst(const BString &str);
    BString		&RemoveLast(const BString &str);
    BString		&RemoveAll(const BString &str);
    BString		&RemoveFirst(const char *str);
    BString		&RemoveLast(const char *str);
    BString		&RemoveAll(const char *str);
    BString		&RemoveSet(const char *setOfCharsToRemove);

    BString		&IRemoveFirst(const BString &str);
    BString		&IRemoveLast(const BString &str);
    BString		&IRemoveAll(const BString &str);
    BString		&IRemoveFirst(const char *str);
    BString		&IRemoveLast(const char *str);
    BString		&IRemoveAll(const char *str);
    BString		&IRemoveSet(const char *setOfCharsToRemove);

    bool		operator<(const BString &str) const;
    bool		operator<=(const BString &str) const;
    bool		operator==(const BString &str) const;
    bool		operator>=(const BString &str) const;
    bool		operator>(const BString &str) const;
    bool		operator!=(const BString &str) const;

	bool		operator<(const char *str) const;
	bool		operator<=(const char *str) const;
	bool		operator==(const char *str) const;
	bool		operator>=(const char *str) const;
	bool		operator>(const char *str) const;
	bool		operator!=(const char *str) const;

    int		Compare(const BString &str) const;
	int		Compare(const char *str) const;
    int		Compare(const BString &str, b_int32 n) const;
    int		Compare(const char *str, b_int32 n) const;
    int		ICompare(const BString &str) const;
	int		ICompare(const char *str) const;
    int		ICompare(const BString &str, b_int32 n) const;
    int		ICompare(const char *str, b_int32 n) const;

    b_int32 		FindFirst(const BString &string) const;
    b_int32 		FindFirst(const char *string) const;
    b_int32 		FindFirst(const BString &string, b_int32 fromOffset) const;
    b_int32 		FindFirst(const char *string, b_int32 fromOffset) const;
    b_int32		FindFirst(char c) const;
    b_int32		FindFirst(char c, b_int32 fromOffset) const;

    b_int32 		FindLast(const BString &string) const;
    b_int32 		FindLast(const char *string) const;
    b_int32 		FindLast(const BString &string, b_int32 beforeOffset) const;
    b_int32 		FindLast(const char *string, b_int32 beforeOffset) const;
    b_int32		FindLast(char c) const;
    b_int32		FindLast(char c, b_int32 beforeOffset) const;

    b_int32 		IFindFirst(const BString &string) const;
    b_int32 		IFindFirst(const char *string) const;
    b_int32 		IFindFirst(const BString &string, b_int32 fromOffset) const;
    b_int32 		IFindFirst(const char *string, b_int32 fromOffset) const;
    b_int32		IFindFirst(char c) const;
    b_int32		IFindFirst(char c, b_int32 fromOffset) const;

    b_int32 		IFindLast(const BString &string) const;
    b_int32 		IFindLast(const char *string) const;
    b_int32 		IFindLast(const BString &string, b_int32 beforeOffset) const;
    b_int32 		IFindLast(const char *string, b_int32 beforeOffset) const;
    b_int32		IFindLast(char c) const;
    b_int32		IFindLast(char c, b_int32 beforeOffset) const;

    BString		&ReplaceFirst(char replaceThis, char withThis);
    BString		&ReplaceLast(char replaceThis, char withThis);
    BString		&ReplaceAll(char replaceThis, char withThis, b_int32 fromOffset = 0);
    BString		&Replace(char replaceThis, char withThis, b_int32 maxReplaceCount, b_int32 fromOffset = 0);
    BString 	&ReplaceFirst(const char *replaceThis, const char *withThis);
    BString		&ReplaceLast(const char *replaceThis, const char *withThis);
    BString		&ReplaceAll(const char *replaceThis, const char *withThis);
    BString		&ReplaceAll(const char *replaceThis, const char *withThis, b_int32 fromOffset);
    BString		&Replace(const char *replaceThis, const char *withThis, b_int32 maxReplaceCount, b_int32 fromOffset = 0);
    BString		&ReplaceSet(const char *setOfChars, char with);
    BString		&ReplaceSet(const char *setOfChars, const char *with);

    BString		&IReplaceFirst(char replaceThis, char withThis);
    BString		&IReplaceLast(char replaceThis, char withThis);
    BString		&IReplaceAll(char replaceThis, char withThis, b_int32 fromOffset = 0);
    BString		&IReplace(char replaceThis, char withThis, b_int32 maxReplaceCount, b_int32 fromOffset = 0);
    BString 	&IReplaceFirst(const char *replaceThis, const char *withThis);
    BString		&IReplaceLast(const char *replaceThis, const char *withThis);
    BString		&IReplaceAll(const char *replaceThis, const char *withThis, b_int32 fromOffset = 0);
    BString		&IReplace(const char *replaceThis, const char *withThis, b_int32 maxReplaceCount, b_int32 fromOffset = 0);
    BString		&IReplaceSet(const char *setOfChars, char with);
    BString		&IReplaceSet(const char *setOfChars, const char *with);

    BString		&ToLower();
    BString		&ToUpper();

	// Converts first character to upper-case, rest to lower-case
    BString		&Capitalize();

	// Converts first character in each white-space-separated word to upper-case, rest to lower-case
    BString		&CapitalizeEachWord();

	// copies original into <this>, escaping characters specified in <setOfCharsToEscape> by prepending them with <escapeWith>
    BString		&CharacterEscape(const char *original, const char *setOfCharsToEscape, char escapeWith);

	// escapes characters specified in <setOfCharsToEscape> by prepending them with <escapeWith>
    BString		&CharacterEscape(const char *setOfCharsToEscape, char escapeWith);

	// copy <original> into the string removing the escaping characters <escapeChar>
    BString		&CharacterDeescape(const char *original, char escapeChar);

	// remove the escaping characters <escapeChar> from the string
    BString		&CharacterDeescape(char escapeChar);

	// IsNumber: whether it is decimalism number like +1.23,-12,12.21,-.12,+.98,3.14e+04,0xffff
	bool		IsNumber() const;

	// IsInteger: whether it is integer in decimalism like +1.000,-2.0,1,2,+1,-2,0xffff don't support x.xxxe+xx style
	bool		IsInteger() const;

	// IsDecimal: whether it is decimal in decimalism like +1.2343,-0.23,12.43,-.23,+.23,3.14e+02
	bool		IsDecimal() const;

	// if IsNumber() is "true", it convert the string to double then return "true", else do nothing and return "false"
	bool		GetDecimal(float *value) const;
	bool		GetDecimal(double *value) const;
    bool		GetInteger(b_int8 *value) const;
    bool		GetInteger(b_uint8 *value) const;
    bool		GetInteger(b_int16 *value) const;
    bool		GetInteger(b_uint16 *value) const;
    bool		GetInteger(b_int32 *value) const;
    bool		GetInteger(b_uint32 *value) const;
    bool		GetInteger(b_int64 *value) const;
    bool		GetInteger(b_uint64 *value) const;

    BString 	&operator<<(const char *str);
    BString 	&operator<<(const BString &str);
    BString 	&operator<<(b_int8 value);
    BString 	&operator<<(b_uint8 value);
    BString 	&operator<<(b_int16 value);
    BString 	&operator<<(b_uint16 value);
    BString 	&operator<<(b_int32 value);
    BString 	&operator<<(b_uint32 value);
    BString 	&operator<<(b_int64 value);
    BString 	&operator<<(b_uint64 value);
    BString 	&operator<<(float value);
    BString 	&operator<<(double value);

	// Split: splits a string into a maximum of max_tokens pieces, using the given delimiter.
	//        If max_tokens is reached, the remainder of string is appended to the last token
	// Returns : a newly-allocated array of strings
    BStringArray	*Split(const char *delimiter, b_uint32 max_tokens = B_MAXUINT32 - 1) const;
    BStringArray	*Split(const char delimiter, b_uint32 max_tokens = B_MAXUINT32 - 1) const;

	// SetMinimumBufferSize: It's NOT to be absolute minimum buffer size even it return "true",
	//                       just for speed up sometimes. The "length" include the null character
    bool		SetMinimumBufferSize(b_int32 length);
    b_int32		MinimumBufferSize() const;

private:
    b_int32 fLen;
    b_int32 fLenReal;
    b_int32 fMinBufferSize;
	char *fBuffer;

    bool _Resize(b_int32 length);
};
#endif /* __cplusplus */

#endif /* BHAPI_STRING__H */

