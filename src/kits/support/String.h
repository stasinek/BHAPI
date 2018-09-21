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
//-------------------------------------------------------------------------------------------------
#ifdef __STRICT_ANSI__
#define __SA__
#undef __STRICT_ANSI__
#endif
BHAPI_IMPEXP int __cdecl strcasecmp(const char *a, const char *b);
BHAPI_IMPEXP int __cdecl strncasecmp(const char *a, const char *b, size_t c);
//#define bhapi::strncasecmp(a,b,c) _strnicmp(a,b,c)
#ifdef __SA__
#define __STRICT_ANSI__
#undef __SA__
#endif
/* the result must be free by "free" */
BHAPI_IMPEXP char* strdup(const char *src);
BHAPI_IMPEXP char* strndup(const char *src,  int32_t length);
BHAPI_IMPEXP char* strdup_dirty(const char* src,  int32_t length = -1);
// bhapi::strdup_dirty(): like bhapi::strdup(), but the result must be free by "delete[]"
BHAPI_IMPEXP char* strdup_vprintf(const char *format, va_list ap);
BHAPI_IMPEXP char* strdup_printf(const char *format, ...);
BHAPI_IMPEXP char* strcasestr(const char *haystack, const char *needle);
BHAPI_IMPEXP char* strrcasestr(const char *haystack, const char *needle);
BHAPI_IMPEXP char* strrstr(const char *haystack, const char *needle);
BHAPI_IMPEXP char* strcasechr(const char *s, int c);
BHAPI_IMPEXP char* strrcasechr(const char *s, int c);
//-------------------------------------------------------------------------------------------------
#ifdef __cplusplus /* Just for C++ */
#ifndef BSTRINGLIST_DEF
#define BSTRINGLIST_DEF
class BStringList;
#endif
#ifndef BSTRINGREF_DEF
#define BSTRINGREF_DEF
class BStringRef;
#endif
//-------------------------------------------------------------------------------------------------
class BHAPI_IMPEXP BString {
public:
            BString();
            BString(const char* string);
            BString(const BString& string);
            BString(const char* string, int32_t maxLength);
           ~BString();
    // Access
    const char*		String() const;
    int32_t 			Length() const;
    int32_t			CountChars() const;
    int32_t			CountBytes(int32_t fromCharOffset, int32_t charCount) const;
    bool			IsEmpty() const;

    uint32_t			HashValue() const;
    static	uint32_t	HashValue(const char* string);
    // Assignment
    BString&		operator=(const BString& string);
    BString&		operator=(const char* string);
    BString&		operator=(char c);

    BString&		SetTo(const char* string);
    BString&		SetTo(const char* string, int32_t maxLength);

    BString&		SetTo(const BString& string);
    BString&		Adopt(BString& from);

    BString&		SetTo(const BString& string, int32_t maxLength);
    BString&		Adopt(BString& from, int32_t maxLength);

    BString&		SetTo(char c, int32_t count);

    BString&		SetToChars(const char* string, int32_t charCount);
    BString&		SetToChars(const BString& string, int32_t charCount);
    BString&		AdoptChars(BString& from, int32_t charCount);

    BString&		SetToFormat(const char* format, ...)
                __attribute__((__format__(__printf__, 2, 3)));
    BString&		SetToFormatVarArgs(const char* format, va_list args)
                __attribute__((__format__(__printf__, 2, 0)));

    int				ScanWithFormat(const char* format, ...)
                __attribute__((__format__(__scanf__, 2, 3)));
    int				ScanWithFormatVarArgs(const char* format, va_list args)
                __attribute__((__format__(__scanf__, 2, 0)));
    // Substring copying
    BString&		CopyInto(BString& into, int32_t fromOffset, int32_t length) const;
    void			CopyInto(char* into, int32_t fromOffset, int32_t length) const;

    BString&		CopyCharsInto(BString& into, int32_t fromCharOffset, int32_t charCount) const;
    bool			CopyCharsInto(char* into, int32_t* intoLength, int32_t fromCharOffset, int32_t charCount) const;

    bool			Split(const char* separator, bool noEmptyStrings,  BStringList& _list) const;
    // Appending
    BString&		operator+=(const BString& string);
    BString&		operator+=(const char* string);
    BString&		operator+=(char c);

    BString&		Append(const BString& string);
    BString&		Append(const char* string);

    BString&		Append(const BString& string, int32_t length);
    BString&		Append(const char* string, int32_t length);
    BString&		Append(char c, int32_t count);

    BString&		AppendChars(const BString& string, int32_t charCount);
    BString&		AppendChars(const char* string, int32_t charCount);
    // Prepending
    BString&		Prepend(const char* string);
    BString&		Prepend(const BString& string);
    BString&		Prepend(const char* string, int32_t length);
    BString&		Prepend(const BString& string, int32_t length);
    BString&		Prepend(char c, int32_t count);

    BString&		PrependChars(const char* string, int32_t charCount);
    BString&		PrependChars(const BString& string, int32_t charCount);
    // Inserting
    BString&		Insert(const char* string, int32_t position);
    BString&		Insert(const char* string, int32_t length, int32_t position);
    BString&		Insert(const char* string, int32_t fromOffset, int32_t length, int32_t position);
    BString&		Insert(const BString& string, int32_t position);
    BString&		Insert(const BString& string, int32_t length, int32_t position);
    BString&		Insert(const BString& string, int32_t fromOffset, int32_t length, int32_t position);
    BString&		Insert(char c, int32_t count, int32_t position);

    BString&		InsertChars(const char* string, int32_t charPosition);
    BString&		InsertChars(const char* string, int32_t charCount, int32_t charPosition);
    BString&		InsertChars(const char* string, int32_t fromCharOffset, int32_t charCount, int32_t charPosition);
    BString&		InsertChars(const BString& string, int32_t charPosition);
    BString&		InsertChars(const BString& string, int32_t charCount, int32_t charPosition);
    BString&		InsertChars(const BString& string, int32_t fromCharOffset, int32_t charCount, int32_t charPosition);
    // Removing
    BString&		Truncate(int32_t newLength, bool lazy = true);
    BString&		TruncateChars(int32_t newCharCount, bool lazy = true);

    BString&		Remove(int32_t from, int32_t length);
    BString&		RemoveChars(int32_t fromCharOffset, int32_t charCount);

    BString&		RemoveFirst(const BString& string);
    BString&		RemoveLast(const BString& string);
    BString&		RemoveAll(const BString& string);

    BString&		RemoveFirst(const char* string);
    BString&		RemoveLast(const char* string);
    BString&		RemoveAll(const char* string);

    BString&		RemoveSet(const char* setOfBytesToRemove);
    BString&		RemoveCharsSet(const char* setOfCharsToRemove);

    BString&		MoveInto(BString& into, int32_t from, int32_t length);
    void			MoveInto(char* into, int32_t from, int32_t length);

    BString&		MoveCharsInto(BString& into, int32_t fromCharOffset,  int32_t charCount);
    bool			MoveCharsInto(char* into, int32_t* intoLength, int32_t fromCharOffset, int32_t charCount);
    // Compare functions
    bool			operator<(const BString& string) const;
    bool			operator<=(const BString& string) const;
    bool			operator==(const BString& string) const;
    bool			operator>=(const BString& string) const;
    bool			operator>(const BString& string) const;
    bool			operator!=(const BString& string) const;

    bool			operator<(const char* string) const;
    bool			operator<=(const char* string) const;
    bool			operator==(const char* string) const;
    bool			operator>=(const char* string) const;
    bool			operator>(const char* string) const;
    bool			operator!=(const char* string) const;

            operator const char*() const;
    // strcmp()-style compare functions
    int				Compare(const BString& string) const;
    int				Compare(const char* string) const;
    int				Compare(const BString& string, int32_t length) const;
    int				Compare(const char* string, int32_t length) const;

    int				CompareChars(const BString& string, int32_t charCount) const;
    int				CompareChars(const char* string, int32_t charCount) const;

    int				ICompare(const BString& string) const;
    int				ICompare(const char* string) const;
    int				ICompare(const BString& string, int32_t length) const;
    int				ICompare(const char* string, int32_t length) const;
    // Searching
    int32_t			FindFirst(const BString& string) const;
    int32_t			FindFirst(const char* string) const;
    int32_t			FindFirst(const BString& string, int32_t fromOffset) const;
    int32_t			FindFirst(const char* string, int32_t fromOffset) const;
    int32_t			FindFirst(char c) const;
    int32_t			FindFirst(char c, int32_t fromOffset) const;

    int32_t			FindFirstChars(const BString& string, int32_t fromCharOffset) const;
    int32_t			FindFirstChars(const char* string, int32_t fromCharOffset) const;

    int32_t			FindLast(const BString& string) const;
    int32_t			FindLast(const char* string) const;
    int32_t			FindLast(const BString& string, int32_t beforeOffset) const;
    int32_t			FindLast(const char* string, int32_t beforeOffset) const;
    int32_t			FindLast(char c) const;
    int32_t			FindLast(char c, int32_t beforeOffset) const;

    int32_t			FindLastChars(const BString& string, int32_t beforeCharOffset) const;
    int32_t			FindLastChars(const char* string, int32_t beforeCharOffset) const;

    int32_t			IFindFirst(const BString& string) const;
    int32_t			IFindFirst(const char* string) const;
    int32_t			IFindFirst(const BString& string, int32_t fromOffset) const;
    int32_t			IFindFirst(const char* string, int32_t fromOffset) const;

    int32_t			IFindLast(const BString& string) const;
    int32_t			IFindLast(const char* string) const;
    int32_t			IFindLast(const BString& string, int32_t beforeOffset) const;
    int32_t			IFindLast(const char* string, int32_t beforeOffset) const;

    bool			StartsWith(const BString& string) const;
    bool			StartsWith(const char* string) const;
    bool			StartsWith(const char* string, int32_t length) const;

    bool			EndsWith(const BString& string) const;
    bool			EndsWith(const char* string) const;
    bool			EndsWith(const char* string, int32_t length) const;
    // Replacing
    BString&		ReplaceFirst(char replaceThis, char withThis);
    BString&		ReplaceLast(char replaceThis, char withThis);
    BString&		ReplaceAll(char replaceThis, char withThis, int32_t fromOffset = 0);
    BString&		Replace(char replaceThis, char withThis, int32_t maxReplaceCount, int32_t fromOffset = 0);
    BString&		ReplaceFirst(const char* replaceThis, const char* withThis);
    BString&		ReplaceLast(const char* replaceThis, const char* withThis);
    BString&		ReplaceAll(const char* replaceThis, const char* withThis, int32_t fromOffset = 0);
    BString&		Replace(const char* replaceThis, const char* withThis, int32_t maxReplaceCount, int32_t fromOffset = 0);

    BString&		ReplaceAllChars(const char* replaceThis, const char* withThis, int32_t fromCharOffset);
    BString&		ReplaceChars(const char* replaceThis, const char* withThis, int32_t maxReplaceCount, int32_t fromCharOffset);

    BString&		IReplaceFirst(char replaceThis, char withThis);
    BString&		IReplaceLast(char replaceThis, char withThis);
    BString&		IReplaceAll(char replaceThis, char withThis, int32_t fromOffset = 0);
    BString&		IReplace(char replaceThis, char withThis, int32_t maxReplaceCount, int32_t fromOffset = 0);
    BString&		IReplaceFirst(const char* replaceThis, const char* withThis);
    BString&		IReplaceLast(const char* replaceThis, const char* withThis);
    BString&		IReplaceAll(const char* replaceThis, const char* withThis, int32_t fromOffset = 0);
    BString&		IReplace(const char* replaceThis, const char* withThis, int32_t maxReplaceCount, int32_t fromOffset = 0);

    BString&		ReplaceSet(const char* setOfBytes, char with);
    BString&		ReplaceSet(const char* setOfBytes, const char* with);

    BString&		ReplaceCharsSet(const char* setOfChars, const char* with);
    // Unchecked char access
    char			operator[](int32_t index) const;
#if __GNUC__ > 3
    BStringRef		operator[](int32_t index);
#else
    char&			operator[](int32_t index);
#endif
    // Checked char access
    char			ByteAt(int32_t index) const;
    const char*		CharAt(int32_t charIndex, int32_t* bytes = NULL) const;
    bool			CharAt(int32_t charIndex, char* buffer, int32_t* bytes) const;
    // Fast low-level manipulation
    char*			LockBuffer(int32_t maxLength);
    BString&		UnlockBuffer(int32_t length = -1);
    // Upercase <-> Lowercase
    BString&		ToLower();
    BString&		ToUpper();

    BString&		Capitalize();
    BString&		CapitalizeEachWord();
    // Escaping and De-escaping
    BString&		CharacterEscape(const char* original, const char* setOfCharsToEscape, char escapeWith);
    BString&		CharacterEscape(const char* setOfCharsToEscape, char escapeWith);
    BString&		CharacterDeescape(const char* original, char escapeChar);
    BString&		CharacterDeescape(char escapeChar);
    // Trimming
    BString&		Trim();
    // Insert
    BString&		operator<<(const char* string);
    BString&		operator<<(const BString& string);
    BString&		operator<<(char c);
    BString&		operator<<(bool value);
    BString&		operator<<(int value);
    BString&		operator<<(unsigned int value);
    BString&		operator<<(unsigned long value);
    BString&		operator<<(long value);
    BString&		operator<<(unsigned long long value);
    BString&		operator<<(long long value);
    // float/double output hardcodes %.2f style formatting
    BString&		operator<<(float value);
    BString&		operator<<(double value);

public:
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

    // Management
    status_t		__MakeWritable();
    status_t		__MakeWritable(int32_t length, bool copy);
    static	char*			__Allocate(int32_t length);
    char*			__Resize(int32_t length);
    void			__Init(const char* src, int32_t length);
    char*			__Clone(const char* data, int32_t length);
    char*			__OpenAtBy(int32_t offset, int32_t length);
    char*			__ShrinkAtBy(int32_t offset, int32_t length);

    // Data
    void			__SetLength(int32_t length);
    bool			__DoAppend(const char* string, int32_t length);
    bool			__DoPrepend(const char* string, int32_t length);
    bool			__DoInsert(const char* string, int32_t offset, int32_t length);

    // Search
    int32_t			__ShortFindAfter(const char* string, int32_t length) const;
    int32_t			__FindAfter(const char* string, int32_t offset, int32_t length) const;
    int32_t			__IFindAfter(const char* string, int32_t offset, int32_t length) const;
    int32_t			__FindBefore(const char* string, int32_t offset, int32_t length) const;
    int32_t			__IFindBefore(const char* string, int32_t offset, int32_t length) const;

    // Escape
    BString&		__DoCharacterEscape(const char* string, const char* setOfCharsToEscape, char escapeChar);
    BString&		__DoCharacterDeescape(const char* string, char escapeChar);

    // Replace
    BString&		__DoReplace(const char* findThis, const char* replaceWith, int32_t maxReplaceCount, int32_t fromOffset, bool ignoreCase);
    void			__ReplaceAtPositions(const PosVect* positions, int32_t searchLength, const char* with, int32_t withLength);

private:
    int32_t& 			__ReferenceCount();
    const int32_t& 	__ReferenceCount() const;
    bool			__IsShareable() const;
    void			__FreePrivateData();

    char*			fPrivateData;

};
//	#pragma mark - BStringRef
class BHAPI_IMPEXP BStringRef {
public:
     BStringRef(BString& string, int32_t position);
    ~BStringRef() {}

    operator char() const;

    char* operator&();
    const char* operator&() const;

    BStringRef& operator=(char c);
    BStringRef& operator=(const BStringRef& rc);

private:
    BString&	fString;
    int32_t		fPosition;
};
namespace bhapi {
template<class TYPE_INT>
BHAPI_IMPEXP void printf_int(BString &str, TYPE_INT value, uint8_t _base, int precision_width, bool upper_style);
BHAPI_IMPEXP  int32_t printf_double(BString &str, double value, int precision_width,  int32_t *exponent, bool g_style, bool upper_style);
}
#endif /* __cplusplus */
#define BSTRING_DEF
#endif /* BHAPI_STRING_H */
