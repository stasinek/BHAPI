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
 * File: String.h
 * Description: EString --- string allocation and manipulation
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_STRING_H__
#define __ETK_STRING_H__

#include <stdarg.h>
#include "./../support/SupportDefs.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* the result must be free by "free" */
_IMPEXP_ETK char*		e_strdup(const char *src);
_IMPEXP_ETK char*		e_strndup(const char *src, eint32 length);
_IMPEXP_ETK char*		e_strdup_vprintf(const char *format, va_list ap);
_IMPEXP_ETK char*		e_strdup_printf(const char *format, ...);

_IMPEXP_ETK eunichar*		e_utf8_convert_to_unicode(const char *str, eint32 length);
_IMPEXP_ETK eunichar32*		e_utf8_convert_to_utf32(const char *str, eint32 length);
_IMPEXP_ETK char*		e_unicode_convert_to_utf8(const eunichar *str, eint32 ulength);
_IMPEXP_ETK eunichar32*		e_unicode_convert_to_utf32(const eunichar *str, eint32 ulength);
_IMPEXP_ETK char*		e_utf32_convert_to_utf8(const eunichar32 *str, eint32 ulength);
_IMPEXP_ETK eunichar*		e_utf32_convert_to_unicode(const eunichar32 *str, eint32 ulength);

_IMPEXP_ETK bool		e_utf8_is_token(const char *str);
_IMPEXP_ETK eint32		e_utf8_strlen(const char *str);
_IMPEXP_ETK eint32		e_utf8_strlen_etc(const char *str, eint32 nbytes);
_IMPEXP_ETK eint32		e_utf8_strlen_fast(const char *str, eint32 nbytes); /* none checking */
_IMPEXP_ETK const char*		e_utf8_at(const char *str, eint32 index, euint8 *nbytes);
_IMPEXP_ETK const char*		e_utf8_next(const char *str, euint8 *length);

_IMPEXP_ETK eint32		e_unicode_strlen(const eunichar *ustr);
_IMPEXP_ETK eint32		e_unicode_strlen_etc(const eunichar *ustr, eint32 nchars, bool utf16_style);
_IMPEXP_ETK const eunichar*	e_unicode_at(const eunichar* ustr, eint32 index, bool *utf16);
_IMPEXP_ETK const eunichar*	e_unicode_next(const eunichar* ustr, bool *utf16);

_IMPEXP_ETK eint32		e_utf32_strlen(const eunichar32 *ustr);
_IMPEXP_ETK eint32		e_utf32_strlen_etc(const eunichar32 *ustr, eint32 nchars);
_IMPEXP_ETK const eunichar32*	e_utf32_at(const eunichar32* ustr, eint32 index);
_IMPEXP_ETK const eunichar32*	e_utf32_next(const eunichar32* ustr);

#ifdef __cplusplus /* Just for C++ */
} // extern "C"

// EStrdup(): like e_strdup(), but the result must be free by "delete[]"
_IMPEXP_ETK char *EStrdup(const char* src, eint32 length = -1);


class EStringArray;


class _IMPEXP_ETK EString {
public:
	EString();
	EString(const char *str);
	EString(const EString &str);
	EString(const char *str, eint32 maxLength);
	~EString();

	const char	*String() const;

	eint32		Length() const; // ASCII
	eint32		CountChars() const; // UTF-8

	char		operator[](eint32 index) const; // ASCII
	char		ByteAt(eint32 index) const; // ASCII
	const char*	CharAt(eint32 index, euint8 *length = NULL) const; // UTF-8

	EString 	&operator=(const EString &str);
	EString 	&operator=(const char *str);
	EString 	&operator=(char c);

	EString		&SetTo(const EString &str);
	EString		&SetTo(const EString &str, eint32 length);
	EString		&SetTo(const char *str);
	EString		&SetTo(const char *str, eint32 length);
	EString		&SetTo(char c, eint32 count);

	EString		&Adopt(EString &from);
	EString		&Adopt(EString &from, eint32 length);

	EString		&CopyInto(EString &into, eint32 fromOffset, eint32 length) const;
	void		CopyInto(char *into, size_t into_size, eint32 fromOffset, eint32 length) const;

	EString		&MoveInto(EString &into, eint32 from, eint32 length);
	void		MoveInto(char *into, size_t into_size, eint32 from, eint32 length);

	void		MakeEmpty();

	EString		&operator+=(const EString &str);
	EString		&operator+=(const char *str);
	EString		&operator+=(char c);

	EString		&Append(const EString &str);
	EString		&Append(const EString &str, eint32 length);
	EString		&Append(const char *str);
	EString		&Append(const char *str, eint32 length);
	EString		&Append(char c, eint32 count);
	EString		&AppendFormat(const char *format, ...);

	EString		&Prepend(const EString &str);
	EString		&Prepend(const EString &str, eint32 length);
	EString		&Prepend(const char *str);
	EString		&Prepend(const char *str, eint32 length);
	EString		&Prepend(char c, eint32 count);
	EString		&PrependFormat(const char *format, ...);

	EString		&Insert(const EString &str, eint32 pos);
	EString		&Insert(const EString &str, eint32 length, eint32 pos);
	EString		&Insert(const EString &str, eint32 fromOffset, eint32 length, eint32 pos);
	EString		&Insert(const char *str, eint32 pos);
	EString		&Insert(const char *str, eint32 length, eint32 pos);
	EString		&Insert(const char *str, eint32 fromOffset, eint32 length, eint32 pos);
	EString		&Insert(char c, eint32 count, eint32 pos);

	EString		&Truncate(eint32 newLength);

	EString		&Remove(eint32 from, eint32 length);

	EString		&RemoveFirst(const EString &str);
	EString		&RemoveLast(const EString &str);
	EString		&RemoveAll(const EString &str);
	EString		&RemoveFirst(const char *str);
	EString		&RemoveLast(const char *str);
	EString		&RemoveAll(const char *str);
	EString		&RemoveSet(const char *setOfCharsToRemove);

	EString		&IRemoveFirst(const EString &str);
	EString		&IRemoveLast(const EString &str);
	EString		&IRemoveAll(const EString &str);
	EString		&IRemoveFirst(const char *str);
	EString		&IRemoveLast(const char *str);
	EString		&IRemoveAll(const char *str);
	EString		&IRemoveSet(const char *setOfCharsToRemove);

	bool		operator<(const EString &str) const;
	bool		operator<=(const EString &str) const;
	bool		operator==(const EString &str) const;
	bool		operator>=(const EString &str) const;
	bool		operator>(const EString &str) const;
	bool		operator!=(const EString &str) const;

	bool		operator<(const char *str) const;
	bool		operator<=(const char *str) const;
	bool		operator==(const char *str) const;
	bool		operator>=(const char *str) const;
	bool		operator>(const char *str) const;
	bool		operator!=(const char *str) const;

	int		Compare(const EString &str) const;
	int		Compare(const char *str) const;
	int		Compare(const EString &str, eint32 n) const;
	int		Compare(const char *str, eint32 n) const;
	int		ICompare(const EString &str) const;
	int		ICompare(const char *str) const;
	int		ICompare(const EString &str, eint32 n) const;
	int		ICompare(const char *str, eint32 n) const;

	eint32 		FindFirst(const EString &string) const;
	eint32 		FindFirst(const char *string) const;
	eint32 		FindFirst(const EString &string, eint32 fromOffset) const;
	eint32 		FindFirst(const char *string, eint32 fromOffset) const;
	eint32		FindFirst(char c) const;
	eint32		FindFirst(char c, eint32 fromOffset) const;

	eint32 		FindLast(const EString &string) const;
	eint32 		FindLast(const char *string) const;
	eint32 		FindLast(const EString &string, eint32 beforeOffset) const;
	eint32 		FindLast(const char *string, eint32 beforeOffset) const;
	eint32		FindLast(char c) const;
	eint32		FindLast(char c, eint32 beforeOffset) const;

	eint32 		IFindFirst(const EString &string) const;
	eint32 		IFindFirst(const char *string) const;
	eint32 		IFindFirst(const EString &string, eint32 fromOffset) const;
	eint32 		IFindFirst(const char *string, eint32 fromOffset) const;
	eint32		IFindFirst(char c) const;
	eint32		IFindFirst(char c, eint32 fromOffset) const;

	eint32 		IFindLast(const EString &string) const;
	eint32 		IFindLast(const char *string) const;
	eint32 		IFindLast(const EString &string, eint32 beforeOffset) const;
	eint32 		IFindLast(const char *string, eint32 beforeOffset) const;
	eint32		IFindLast(char c) const;
	eint32		IFindLast(char c, eint32 beforeOffset) const;

	EString		&ReplaceFirst(char replaceThis, char withThis);
	EString		&ReplaceLast(char replaceThis, char withThis);
	EString		&ReplaceAll(char replaceThis, char withThis, eint32 fromOffset = 0);
	EString		&Replace(char replaceThis, char withThis, eint32 maxReplaceCount, eint32 fromOffset = 0);
	EString 	&ReplaceFirst(const char *replaceThis, const char *withThis);
	EString		&ReplaceLast(const char *replaceThis, const char *withThis);
	EString		&ReplaceAll(const char *replaceThis, const char *withThis, eint32 fromOffset = 0);
	EString		&Replace(const char *replaceThis, const char *withThis, eint32 maxReplaceCount, eint32 fromOffset = 0);
	EString		&ReplaceSet(const char *setOfChars, char with);
	EString		&ReplaceSet(const char *setOfChars, const char *with);

	EString		&IReplaceFirst(char replaceThis, char withThis);
	EString		&IReplaceLast(char replaceThis, char withThis);
	EString		&IReplaceAll(char replaceThis, char withThis, eint32 fromOffset = 0);
	EString		&IReplace(char replaceThis, char withThis, eint32 maxReplaceCount, eint32 fromOffset = 0);
	EString 	&IReplaceFirst(const char *replaceThis, const char *withThis);
	EString		&IReplaceLast(const char *replaceThis, const char *withThis);
	EString		&IReplaceAll(const char *replaceThis, const char *withThis, eint32 fromOffset = 0);
	EString		&IReplace(const char *replaceThis, const char *withThis, eint32 maxReplaceCount, eint32 fromOffset = 0);
	EString		&IReplaceSet(const char *setOfChars, char with);
	EString		&IReplaceSet(const char *setOfChars, const char *with);

	EString		&ToLower();
	EString		&ToUpper();

	// Converts first character to upper-case, rest to lower-case
	EString		&Capitalize();

	// Converts first character in each white-space-separated word to upper-case, rest to lower-case
	EString		&CapitalizeEachWord();

	// copies original into <this>, escaping characters specified in <setOfCharsToEscape> by prepending them with <escapeWith>
	EString		&CharacterEscape(const char *original, const char *setOfCharsToEscape, char escapeWith);

	// escapes characters specified in <setOfCharsToEscape> by prepending them with <escapeWith>
	EString		&CharacterEscape(const char *setOfCharsToEscape, char escapeWith);

	// copy <original> into the string removing the escaping characters <escapeChar>
	EString		&CharacterDeescape(const char *original, char escapeChar);

	// remove the escaping characters <escapeChar> from the string
	EString		&CharacterDeescape(char escapeChar);

	// IsNumber: whether it is decimalism number like +1.23,-12,12.21,-.12,+.98,3.14e+04,0xffff
	bool		IsNumber() const;

	// IsInteger: whether it is integer in decimalism like +1.000,-2.0,1,2,+1,-2,0xffff don't support x.xxxe+xx style
	bool		IsInteger() const;

	// IsDecimal: whether it is decimal in decimalism like +1.2343,-0.23,12.43,-.23,+.23,3.14e+02
	bool		IsDecimal() const;

	// if IsNumber() is "true", it convert the string to double then return "true", else do nothing and return "false"
	bool		GetDecimal(float *value) const;
	bool		GetDecimal(double *value) const;
	bool		GetInteger(eint8 *value) const;
	bool		GetInteger(euint8 *value) const;
	bool		GetInteger(eint16 *value) const;
	bool		GetInteger(euint16 *value) const;
	bool		GetInteger(eint32 *value) const;
	bool		GetInteger(euint32 *value) const;
	bool		GetInteger(eint64 *value) const;
	bool		GetInteger(euint64 *value) const;

	EString 	&operator<<(const char *str);
	EString 	&operator<<(const EString &str);
	EString 	&operator<<(char c);
	EString 	&operator<<(eint8 value);
	EString 	&operator<<(euint8 value);
	EString 	&operator<<(eint16 value);
	EString 	&operator<<(euint16 value);
	EString 	&operator<<(eint32 value);
	EString 	&operator<<(euint32 value);
	EString 	&operator<<(eint64 value);
	EString 	&operator<<(euint64 value);
	EString 	&operator<<(float value);
	EString 	&operator<<(double value);

	// Split: splits a string into a maximum of max_tokens pieces, using the given delimiter.
	//        If max_tokens is reached, the remainder of string is appended to the last token
	// Returns : a newly-allocated array of strings
	EStringArray	*Split(const char *delimiter, euint32 max_tokens = E_MAXUINT32 - 1) const;
	EStringArray	*Split(const char delimiter, euint32 max_tokens = E_MAXUINT32 - 1) const;

	// SetMinimumBufferSize: It's NOT to be absolute minimum buffer size even it return "true",
	//                       just for speed up sometimes. The "length" include the null character
	bool		SetMinimumBufferSize(eint32 length);
	eint32		MinimumBufferSize() const;

private:
	eint32 fLen;
	eint32 fLenReal;
	eint32 fMinBufferSize;
	char *fBuffer;

	bool _Resize(eint32 length);
};


#endif /* __cplusplus */

#endif /* __ETK_STRING_H__ */

