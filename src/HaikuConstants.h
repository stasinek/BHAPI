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
 * File: HaikuConstants.h
 *
 * --------------------------------------------------------------------------*/
/*
 * Copyright 2005-2010 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Author:
 *		Erik Jaesler (erik@cgsoftware.com)
 */
#ifndef BHAPI_HAIKU)CONSTANTS_H
#define BHAPI_HAIKU_CONSTANTS_H

#include "HaikuConfig.h"

enum {
    B_AFFINE_TRANSFORM_TYPE			= 'AMTX',
    B_ALIGNMENT_TYPE				= 'ALGN',
    B_ANY_TYPE						= 'ANYT',
    B_ATOM_TYPE						= 'ATOM',
    B_ATOMREF_TYPE					= 'ATMR',
    B_BOOL_TYPE						= 'BOOL',
    B_CHAR_TYPE						= 'CHAR',
    B_COLOR_8_BIT_TYPE				= 'CLRB',
    B_DOUBLE_TYPE					= 'DBLE',
    B_FLOAT_TYPE					= 'FLOT',
    B_GRAYSCALE_8_BIT_TYPE			= 'GRYB',
    B_INT16_TYPE					= 'SHRT',
    B_INT32_TYPE					= 'LONG',
    B_INT64_TYPE					= 'LLNG',
    B_INT8_TYPE						= 'BYTE',
    B_LARGE_ICON_TYPE				= 'ICON',
    B_MEDIA_PARAMETER_GROUP_TYPE	= 'BMCG',
    B_MEDIA_PARAMETER_TYPE			= 'BMCT',
    B_MEDIA_PARAMETER_WEB_TYPE		= 'BMCW',
    B_MESSAGE_TYPE					= 'MSGG',
    B_MESSENGER_TYPE				= 'MSNG',
    B_MIME_TYPE						= 'MIME',
    B_MINI_ICON_TYPE				= 'MICN',
    B_MONOCHROME_1_BIT_TYPE			= 'MNOB',
    B_OBJECT_TYPE					= 'OPTR',
    B_OFF_T_TYPE					= 'OFFT',
    B_PATTERN_TYPE					= 'PATN',
    B_POINTER_TYPE					= 'PNTR',
    B_POINT_TYPE					= 'BPNT',
    B_PROPERTY_INFO_TYPE			= 'SCTD',
    B_RAW_TYPE						= 'RAWT',
    B_RECT_TYPE						= 'RECT',
    B_REF_TYPE						= 'RREF',
    B_RGB_32_BIT_TYPE				= 'RGBB',
    B_RGB_COLOR_TYPE				= 'RGBC',
    B_SIZE_TYPE						= 'SIZE',
    B_SIZE_T_TYPE					= 'SIZT',
    B_SSIZE_T_TYPE					= 'SSZT',
    B_STRING_TYPE					= 'CSTR',
    B_STRING_LIST_TYPE				= 'STRL',
    B_TIME_TYPE						= 'TIME',
    B_UINT16_TYPE					= 'USHT',
    B_UINT32_TYPE					= 'ULNG',
    B_UINT64_TYPE					= 'ULLG',
    B_UINT8_TYPE					= 'UBYT',
    B_VECTOR_ICON_TYPE				= 'VICN',
    B_XATTR_TYPE					= 'XATR',
    B_NETWORK_ADDRESS_TYPE			= 'NWAD',
    B_MIME_STRING_TYPE				= 'MIMS',

    /* deprecated, do not use */
    B_ASCII_TYPE					= 'TEXT'
        /* use B_STRING_TYPE instead */
};

/* System-wide MIME types for handling URL's */

extern const char* B_URL_HTTP;
    /* application/x-vnd.Be.URL.http */
extern const char* B_URL_HTTPS;
    /* application/x-vnd.Be.URL.https */
extern const char* B_URL_FTP;
    /*  application/x-vnd.Be.URL.ftp */
extern const char* B_URL_GOPHER;
    /*  application/x-vnd.Be.URL.gopher */
extern const char* B_URL_MAILTO;
    /*  application/x-vnd.Be.URL.mailto */
extern const char* B_URL_NEWS;
    /*  application/x-vnd.Be.URL.news */
extern const char* B_URL_NNTP;
    /*  application/x-vnd.Be.URL.nntp */
extern const char* B_URL_TELNET;
    /*  application/x-vnd.Be.URL.telnet */
extern const char* B_URL_RLOGIN;
    /*  application/x-vnd.Be.URL.rlogin */
extern const char* B_URL_TN3270;
    /*  application/x-vnd.Be.URL.tn3270 */
extern const char* B_URL_WAIS;
    /*  application/x-vnd.Be.URL.wais */
extern const char* B_URL_FILE;
    /*  application/x-vnd.Be.URL.file */

#endif	/* _TYPE_CONSTANTS_H */
