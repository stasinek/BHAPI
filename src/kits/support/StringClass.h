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
 * File: StringMe.h
 * Description: String --- string allocation and manipulation
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_STRING_H
#define BHAPI_STRING_H

#include <Haiku.h>

#ifdef __cplusplus /* Just for C++ */
extern "C" {
#endif /* __cplusplus */
#ifdef __STRICT_ANSI__
#define __SA__
#undef __STRICT_ANSI__
#endif
#ifdef __cplusplus /* Just for C++ */
namespace bhapi {
#endif
int __cdecl strcasecmp(const char *a, const char *b);
int __cdecl strncasecmp(const char *a, const char *b, size_t c);
//#define bhapi::strncasecmp(a,b,c) _strnicmp(a,b,c)
#ifdef __cplusplus /* Just for C++ */
} // namespace
#endif
#ifdef __SA__
#define __STRICT_ANSI__
#undef __SA__
#endif

#ifdef __cplusplus /* Just for C++ */
namespace bhapi {
#endif
/* the result must be free by "free" */
BHAPI_IMPEXP char* strdup(const char *src);
BHAPI_IMPEXP char* strndup(const char *src,  __be_int32 length);
BHAPI_IMPEXP char* strdup_dirty(const char* src,  __be_int32 length = -1);
BHAPI_IMPEXP char* strdup_vprintf(const char *format, va_list ap);
BHAPI_IMPEXP char* strdup_printf(const char *format, ...);
BHAPI_IMPEXP char* strcasestr(const char *haystack, const char *needle);
BHAPI_IMPEXP char* strrcasestr(const char *haystack, const char *needle);
BHAPI_IMPEXP char* strrstr(const char *haystack, const char *needle);
BHAPI_IMPEXP char* strcasechr(const char *s, int c);
BHAPI_IMPEXP char* strrcasechr(const char *s, int c);

#ifdef __cplusplus /* Just for C++ */
} // namespace
#endif
#ifdef __cplusplus /* Just for C++ */
} // extern "C"
#endif
// bhapi::strdup_dirty(): like bhapi::strdup(), but the result must be free by "delete[]"

#ifdef __cplusplus /* Just for C++ */
#ifndef BSTRINGARRAY_DEF
#define BSTRINGARRAY_DEF
class BStringArray;
#endif
class BHAPI_IMPEXP BString {
public:
    BString();
    BString(const char *str);
    BString(const BString &str);
    BString(const char *str,  __be_int32 maxLength);
    ~BString();
    const char *String() const;
     __be_int32		Length() const; // ASCII
     __be_int32		CountChars() const; // UTF-8
     __be_int32		CountBytes(__be_int32 fromCharOffset, __be_int32 charCount) const;
    bool		IsEmpty() const;
    uint32      HashValue() const;
    static	uint32			HashValue(const char* string);

    char		operator[](__be_int32 index) const; // ASCII
    char		ByteAt(__be_int32 index) const; // ASCII
    const char*	CharAt(__be_int32 index,  __be_uint8 *length = NULL) const; // UTF-8

    BString 	&operator=(const BString &str);
    BString 	&operator=(const char *str);
    BString 	&operator=(char c);

    BString		&SetTo(const BString &str);
    BString		&SetTo(const BString &str,  __be_int32 length);
    BString		&SetTo(const char *str);
    BString		&SetTo(const char *str,  __be_int32 length);
    BString		&SetTo(char c,  __be_int32 count);

    BString		&Adopt(BString &from);
    BString		&Adopt(BString &from,  __be_int32 length);

    BString&	SetToChars(const char* string, int32 charCount);
    BString&	SetToChars(const BString& string, int32 charCount);
    BString&	AdoptChars(BString& from, int32 charCount);

/*	BString&		SetToFormat(const char* format, ...)
                            __attribute__((__format__(__printf__, 2, 3)));
    BString&		SetToFormatVarArgs(const char* format,
                        va_list args)
                        __attribute__((__format__(__printf__, 2, 0)));
    int				ScanWithFormat(const char* format, ...)
                    __attribute__((__format__(__scanf__, 2, 3)));
    int				ScanWithFormatVarArgs(const char* format,
                        va_list args)
                    __attribute__((__format__(__scanf__, 2, 0)));
*/
    BString		&CopyInto(BString &into,  __be_int32 fromOffset,  __be_int32 length) const;
    void		CopyInto(char *into, size_t into_size,  __be_int32 fromOffset,  __be_int32 length) const;

    BString		&MoveInto(BString &into,  __be_int32 from,  __be_int32 length);
    void		MoveInto(char *into, size_t into_size,  __be_int32 from,  __be_int32 length);
    bool		Split(const char* separator, bool noEmptyStrings, BStringArray& _list) const;
    void		MakeEmpty();

    BString		&operator+=(const BString &str);
    BString		&operator+=(const char *str);
    BString		&operator+=(char c);
    operator const char*() const;

    BString		&Append(const BString &str);
    BString		&Append(const BString &str,  __be_int32 length);
    BString		&Append(const char *str);
    BString		&Append(const char *str,  __be_int32 length);
    BString		&Append(char c,  __be_int32 count);
    BString		&AppendFormat(const char *format, ...);

    BString		&Prepend(const BString &str);
    BString		&Prepend(const BString &str,  __be_int32 length);
    BString		&Prepend(const char *str);
    BString		&Prepend(const char *str,  __be_int32 length);
    BString		&Prepend(char c,  __be_int32 count);
    BString		&PrependFormat(const char *format, ...);

    BString		&Insert(const BString &str,  __be_int32 pos);
    BString		&Insert(const BString &str,  __be_int32 length,  __be_int32 pos);
    BString		&Insert(const BString &str,  __be_int32 fromOffset,  __be_int32 length,  __be_int32 pos);
    BString		&Insert(const char *str,  __be_int32 pos);
    BString		&Insert(const char *str,  __be_int32 length,  __be_int32 pos);
    BString		&Insert(const char *str,  __be_int32 fromOffset,  __be_int32 length,  __be_int32 pos);
    BString		&Insert(char c,  __be_int32 count,  __be_int32 pos);

    BString		&Truncate(__be_int32 newLength);

    BString		&Remove(__be_int32 from,  __be_int32 length);
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
    int		Compare(const BString &str,  __be_int32 n) const;
    int		Compare(const char *str,  __be_int32 n) const;
    int		ICompare(const BString &str) const;
    int		ICompare(const char *str) const;
    int		ICompare(const BString &str,  __be_int32 n) const;
    int		ICompare(const char *str,  __be_int32 n) const;

     __be_int32 	FindFirst(const BString &string) const;
     __be_int32 	FindFirst(const char *string) const;
     __be_int32 	FindFirst(const BString &string,  __be_int32 fromOffset) const;
     __be_int32 	FindFirst(const char *string,  __be_int32 fromOffset) const;
     __be_int32		FindFirst(char c) const;
     __be_int32		FindFirst(char c,  __be_int32 fromOffset) const;
     __be_int32 	FindLast(const BString &string) const;
     __be_int32 	FindLast(const char *string) const;
     __be_int32 	FindLast(const BString &string,  __be_int32 beforeOffset) const;
     __be_int32 	FindLast(const char *string,  __be_int32 beforeOffset) const;
     __be_int32		FindLast(char c) const;
     __be_int32		FindLast(char c,  __be_int32 beforeOffset) const;
     __be_int32 	IFindFirst(const BString &string) const;
     __be_int32 	IFindFirst(const char *string) const;
     __be_int32 	IFindFirst(const BString &string,  __be_int32 fromOffset) const;
     __be_int32 	IFindFirst(const char *string,  __be_int32 fromOffset) const;
     __be_int32		IFindFirst(char c) const;
     __be_int32		IFindFirst(char c,  __be_int32 fromOffset) const;
     __be_int32 	IFindLast(const BString &string) const;
     __be_int32 	IFindLast(const char *string) const;
     __be_int32 	IFindLast(const BString &string,  __be_int32 beforeOffset) const;
     __be_int32 	IFindLast(const char *string,  __be_int32 beforeOffset) const;
     __be_int32		IFindLast(char c) const;
     __be_int32		IFindLast(char c,  __be_int32 beforeOffset) const;
    // Replace
    BString		&ReplaceFirst(char replaceThis, char withThis);
    BString		&ReplaceLast(char replaceThis, char withThis);
    BString		&ReplaceAll(char replaceThis, char withThis,  __be_int32 fromOffset = 0);
    BString		&Replace(char replaceThis, char withThis,  __be_int32 maxReplaceCount,  __be_int32 fromOffset = 0);
    BString 	&ReplaceFirst(const char *replaceThis, const char *withThis);
    BString		&ReplaceLast(const char *replaceThis, const char *withThis);
    BString		&ReplaceAll(const char *replaceThis, const char *withThis);
    BString		&ReplaceAll(const char *replaceThis, const char *withThis,  __be_int32 fromOffset);
    BString		&Replace(const char *replaceThis, const char *withThis,  __be_int32 maxReplaceCount,  __be_int32 fromOffset = 0);
    BString		&ReplaceSet(const char *setOfChars, char with);
    BString		&ReplaceSet(const char *setOfChars, const char *with);
    BString		&IReplaceFirst(char replaceThis, char withThis);
    BString		&IReplaceLast(char replaceThis, char withThis);
    BString		&IReplaceAll(char replaceThis, char withThis,  __be_int32 fromOffset = 0);
    BString		&IReplace(char replaceThis, char withThis,  __be_int32 maxReplaceCount,  __be_int32 fromOffset = 0);
    BString 	&IReplaceFirst(const char *replaceThis, const char *withThis);
    BString		&IReplaceLast(const char *replaceThis, const char *withThis);
    BString		&IReplaceAll(const char *replaceThis, const char *withThis,  __be_int32 fromOffset = 0);
    BString		&IReplace(const char *replaceThis, const char *withThis,  __be_int32 maxReplaceCount,  __be_int32 fromOffset = 0);
    BString		&IReplaceSet(const char *setOfChars, char with);
    BString		&IReplaceSet(const char *setOfChars, const char *with);
    // Converts all to upper-case or lower-case
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
    bool		GetInteger(__be_int8 *value) const;
    bool		GetInteger(__be_uint8 *value) const;
    bool		GetInteger(__be_int16 *value) const;
    bool		GetInteger(__be_uint16 *value) const;
    bool		GetInteger(__be_int32 *value) const;
    bool		GetInteger(__be_uint32 *value) const;
    bool		GetInteger(__be_int64 *value) const;
    bool		GetInteger(__be_uint64 *value) const;

    BString 	&operator<<(const char *str);
    BString 	&operator<<(const BString &str);
    BString 	&operator<<(__be_int8 value);
    BString 	&operator<<(__be_uint8 value);
    BString 	&operator<<(__be_int16 value);
    BString 	&operator<<(__be_uint16 value);
    BString 	&operator<<(__be_int32 value);
    BString 	&operator<<(__be_uint32 value);
    BString 	&operator<<(__be_int64 value);
    BString 	&operator<<(__be_uint64 value);
    BString 	&operator<<(float value);
    BString 	&operator<<(double value);
    // Split: splits a string into a maximum of max_tokens pieces, using the given delimiter.
    //        If max_tokens is reached, the remainder of string is appended to the last token
    // Returns : a newly-allocated array of strings
    BStringArray	*Split(const char *delimiter,  __be_uint32 max_tokens = B_MAXUINT32 - 1) const;
    BStringArray	*Split(const char delimiter,  __be_uint32 max_tokens = B_MAXUINT32 - 1) const;
    // SetMinimumBufferSize: It's NOT to be absolute minimum buffer size even it return "true",
    //                       just for speed up sometimes. The "length" include the null character
    bool		SetMinimumBufferSize(__be_int32 length);
     __be_int32		MinimumBufferSize() const;

    class Private;
    friend class Private;

private:
   class PosVect;
   friend class BStringRef;

   enum PrivateDataTag {
                PRIVATE_DATA
        };

private:
    BString(char* privateData, PrivateDataTag tag);
private:
     __be_int32 fLen;
     __be_int32 fLenReal;
     __be_int32 fMinBufferSize;
#define fPrivateData fBuffer
    char *fBuffer;

    bool _ResizeBool(__be_int32 length);
    status_t		_MakeWritable();
    status_t		_MakeWritable(int32 length, bool copy);
static	char*			_Allocate(int32 length);
    char*			_Resize(int32 length);
    void			_Init(const char* src, int32 length);
    char*			_Clone(const char* data, int32 length);
    char*			_OpenAtBy(int32 offset, int32 length);
    char*			_ShrinkAtBy(int32 offset, int32 length);

    // Data
    void			_SetLength(int32 length);
    bool			_DoAppend(const char* string, int32 length);
    bool			_DoPrepend(const char* string, int32 length);
    bool			_DoInsert(const char* string, int32 offset, int32 length);

    // Search
    int32			_ShortFindAfter(const char* string, int32 length) const;
    int32			_FindAfter(const char* string, int32 offset, int32 length) const;
    int32			_IFindAfter(const char* string, int32 offset, int32 length) const;
    int32			_FindBefore(const char* string, int32 offset, int32 length) const;
    int32			_IFindBefore(const char* string, int32 offset, int32 length) const;
    // Escape
    BString&		_DoCharacterEscape(const char* string, const char* setOfCharsToEscape, char escapeChar);
    BString&		_DoCharacterDeescape(const char* string, char escapeChar);
    // Replace
    BString&		_DoReplace(const char* findThis,const char* replaceWith, int32 maxReplaceCount,int32 fromOffset, bool ignoreCase);
    void			_ReplaceAtPositions(const PosVect* positions, int32 searchLength, const char* with, int32 withLength);

private:
    int32& 			_ReferenceCount();
    const int32& 	_ReferenceCount() const;
    bool			_IsShareable() const;
    void			_FreePrivateData();
};
//	#pragma mark - BStringRef
class BStringRef {
public:
    BStringRef(BString& string, int32 position);
    ~BStringRef() {}

    operator char() const;

    char* operator&();
    const char* operator&() const;

    BStringRef& operator=(char c);
    BStringRef& operator=(const BStringRef& rc);

private:
    BString&	fString;
    int32		fPosition;
};
namespace bhapi {
template<class TYPE_INT>
BHAPI_IMPEXP void printf_int(BString &str, TYPE_INT value,  __be_uint8 _base, int precision_width, bool upper_style);
BHAPI_IMPEXP  __be_int32 printf_double(BString &str, double value, int precision_width,  __be_int32 *exponent, bool g_style, bool upper_style);
}
#endif /* __cplusplus */
#define BSTRING_DEF
#endif /* BHAPI_STRING_H */
