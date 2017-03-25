/* --------------------------------------------------------------------------
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
#ifndef BHAPI_UTF8_FUNCTIONS_H
#define BHAPI_UTF8_FUNCTIONS_H
#include <Haiku.h>

#ifdef __cplusplus
namespace bhapi {
#endif	/* __cplusplus */
status_t convert_to_utf8(uint32 sourceEncoding, const char* source, int32* sourceLength, char* dest, int32* destLength, int32* state, char substitute = B_SUBSTITUTE);
status_t convert_from_utf8(uint32 destEncoding, const char* source, int32* sourceLength, char* dest, int32* destLength, int32* state, char substitute = B_SUBSTITUTE);
BHAPI_IMPEXP unichar16*	utf8_convert_to_unicode(const char *str,  int32 length);
BHAPI_IMPEXP unichar32* utf8_convert_to_utf32(const char *str,  int32 length);
BHAPI_IMPEXP bool utf8_is_token(const char *str);
BHAPI_IMPEXP  int32 utf8_strlen(const char *str);
BHAPI_IMPEXP  int32 utf8_strlen_etc(const char *str,  int32 nbytes);
BHAPI_IMPEXP  int32  utf8_strlen_fast(const char *str,  int32 nbytes); /* none checking */
BHAPI_IMPEXP const char* utf8_at(const char *str,  int32 index,  uint8 *nbytes);
BHAPI_IMPEXP const char* utf8_next(const char *str,  uint8 *length);

BHAPI_IMPEXP char* utf32_convert_to_utf8(const unichar32 *str,  int32 ulength);
BHAPI_IMPEXP unichar16* utf32_convert_to_unicode(const unichar32 *str,  int32 ulength);
BHAPI_IMPEXP  int32 utf32_strlen(const unichar32 *ustr);
BHAPI_IMPEXP  int32 utf32_strlen_etc(const unichar32 *ustr,  int32 nchars);
BHAPI_IMPEXP const unichar32* utf32_at(const unichar32* ustr,  int32 index);
BHAPI_IMPEXP const unichar32* utf32_next(const unichar32* ustr);

BHAPI_IMPEXP char* unicode_convert_to_utf8(const unichar16*str,  int32 ulength);
BHAPI_IMPEXP unichar32* unicode_convert_to_utf32(const unichar16*str,  int32 ulength);
BHAPI_IMPEXP  int32 unicode_strlen(const unichar16*ustr);
BHAPI_IMPEXP  int32 unicode_strlen_etc(const unichar16*ustr,  int32 nchars, bool utf16_style);
BHAPI_IMPEXP const unichar16* unicode_at(const unichar16* ustr,  int32 index, bool *utf16);
BHAPI_IMPEXP const unichar16* unicode_next(const unichar16* ustr, bool *utf16);
#ifdef __cplusplus
}
#endif	/* __cplusplus */
#endif	/* BHAPI_UTF8_H */
