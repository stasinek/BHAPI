﻿/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku OS source-code, All Rights Reserved
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
 * File: UTF8.h
 *
 * --------------------------------------------------------------------------*/
/*
 * Copyright 2007 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef BHAPI_UTF8_H
#define BHAPI_UTF8_H

#include "../interface/InterfaceDefs.h"
#include <Haiku.h>

/*  Conversion Flavors */
enum {
    B_ISO1_CONVERSION,				/*  ISO 8859-x */
    B_ISO2_CONVERSION,
    B_ISO3_CONVERSION,
    B_ISO4_CONVERSION,
    B_ISO5_CONVERSION,
    B_ISO6_CONVERSION,
    B_ISO7_CONVERSION,
    B_ISO8_CONVERSION,
    B_ISO9_CONVERSION,
    B_ISO10_CONVERSION,
    B_MAC_ROMAN_CONVERSION,			/*  Macintosh Roman */
    B_SJIS_CONVERSION,				/*  Shift-JIS */
    B_EUC_CONVERSION,				/*  EUC Packed Japanese */
    B_JIS_CONVERSION,				/*  JIS X 0208-1990 */
    B_MS_WINDOWS_CONVERSION,		/*  Windows Latin-1 Codepage 1252 */
    B_UNICODE_CONVERSION,			/*  Unicode 2.0, UCS-2 */
    B_KOI8R_CONVERSION,				/*  KOI8-R */
    B_MS_WINDOWS_1251_CONVERSION,	/*  Windows Cyrillic Codepage 1251 */
    B_MS_DOS_866_CONVERSION,		/*  MS-DOS Codepage 866 */
    B_MS_DOS_CONVERSION,			/*  MS-DOS Codepage 437 */
    B_EUC_KR_CONVERSION,			/*  EUC Korean */
    B_ISO13_CONVERSION,
    B_ISO14_CONVERSION,
    B_ISO15_CONVERSION,
    B_BIG5_CONVERSION,				/*  Chinese Big5 */
    B_GBK_CONVERSION,				/*  Chinese GB18030 */
    B_UTF16_CONVERSION,				/*  Unicode UTF-16 */
    B_MS_WINDOWS_1250_CONVERSION	/*  Windows Central European Codepage */
};

/*  Conversion Functions */

#ifdef __cplusplus
namespace bhapi {
status_t convert_to_utf8(uint32 sourceEncoding, const char* source, int32* sourceLength, char* dest, int32* destLength, int32* state,
    char substitute = B_SUBSTITUTE);
status_t convert_from_utf8(uint32 destEncoding, const char* source,
    int32* sourceLength, char* dest, int32* destLength, int32* state,
    char substitute = B_SUBSTITUTE);
BHAPI_IMPEXP unichar16*		utf8_convert_to_unicode(const char *str,  int32 length);
BHAPI_IMPEXP unichar32*		utf8_convert_to_utf32(const char *str,  int32 length);
BHAPI_IMPEXP bool                utf8_is_token(const char *str);
BHAPI_IMPEXP  int32             utf8_strlen(const char *str);
BHAPI_IMPEXP  int32             utf8_strlen_etc(const char *str,  int32 nbytes);
BHAPI_IMPEXP  int32             utf8_strlen_fast(const char *str,  int32 nbytes); /* none checking */
BHAPI_IMPEXP const char*         utf8_at(const char *str,  int32 index,  uint8 *nbytes);
BHAPI_IMPEXP const char*         utf8_next(const char *str,  uint8 *length);

BHAPI_IMPEXP char*               utf32_convert_to_utf8(const unichar32 *str,  int32 ulength);
BHAPI_IMPEXP unichar16*		utf32_convert_to_unicode(const unichar32 *str,  int32 ulength);
BHAPI_IMPEXP  int32             utf32_strlen(const unichar32 *ustr);
BHAPI_IMPEXP  int32             utf32_strlen_etc(const unichar32 *ustr,  int32 nchars);
BHAPI_IMPEXP const unichar32*	utf32_at(const unichar32* ustr,  int32 index);
BHAPI_IMPEXP const unichar32*	utf32_next(const unichar32* ustr);

BHAPI_IMPEXP char*               unicode_convert_to_utf8(const unichar16*str,  int32 ulength);
BHAPI_IMPEXP unichar32*		unicode_convert_to_utf32(const unichar16*str,  int32 ulength);
BHAPI_IMPEXP  int32             unicode_strlen(const unichar16*ustr);
BHAPI_IMPEXP  int32             unicode_strlen_etc(const unichar16*ustr,  int32 nchars, bool utf16_style);
BHAPI_IMPEXP const unichar16*	unicode_at(const unichar16* ustr,  int32 index, bool *utf16);
BHAPI_IMPEXP const unichar16*	unicode_next(const unichar16* ustr, bool *utf16);

BHAPI_IMPEXP bool IsInsideGlyph(uchar ch);
BHAPI_IMPEXP uint32 UTF8NextCharLenUnsafe(const char *text);
BHAPI_IMPEXP uint32 UTF8NextCharLen(const char *text);
BHAPI_IMPEXP uint32 UTF8NextCharLen(const char *bytes, size_t length);
BHAPI_IMPEXP uint32 UTF8PreviousCharLen(const char *text, const char *limit);

/*!	UTF8CountBytes gets the length (in bytes) of a UTF8 string. Up to
    numChars characters are read. If numChars is a negative value it is ignored
    and the string is read up to the terminating 0.
*/
BHAPI_IMPEXP uint32 UTF8CountBytes(const char *bytes, int32 numChars);
/*!	UTF8CountChars gets the length (in characters) of a UTF8 string. Up to
    numBytes bytes are read. If numBytes is a negative value it is ignored
    and the string is read up to the terminating 0.
*/
BHAPI_IMPEXP uint32 UTF8CountChars(const char *bytes, int32 numBytes);

/*!	UTF8ToCharCode converts the input that includes potential multibyte chars
    to UTF-32 char codes that can be used by FreeType. The string pointer is
    then advanced to the next character in the string. In case the terminating
    0 is reached, the string pointer is not advanced anymore and nulls are
    returned. This makes it safe to overruns and enables streamed processing
    of UTF8 strings.
*/
BHAPI_IMPEXP uint32 UTF8ToCharCode(const char **bytes);
}
#endif	/* __cplusplus */
#endif	/* BHAPI_UTF8_H */
