/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku source-code, All Rights Reserved
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
 * File: Base64.h
 *
 * --------------------------------------------------------------------------*/
/*
 * Copyright 2011-2013 Haiku, Inc. All rights reserved.
 * Copyright 2001-2003 Dr. Zoidberg Enterprises. All rights reserved.
 */

#include "mail_encoding.h"
#include "../support/SupportDefs.h"
#include <ctype.h>
#include <string.h>


static const char kBase64Alphabet[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '+',
    '/'
};


ssize_t
encode_base64(char *out, const char *in, off_t length, int headerMode)
{
    uint32 concat;
    int i = 0;
    int k = 0;
    int lineLength = 4;
        // Stop before it actually gets too long

    while (i < length) {
        concat = ((in[i] & 0xff) << 16);

        if ((i+1) < length)
            concat |= ((in[i+1] & 0xff) << 8);
        if ((i+2) < length)
            concat |= (in[i+2] & 0xff);

        i += 3;

        out[k++] = kBase64Alphabet[(concat >> 18) & 63];
        out[k++] = kBase64Alphabet[(concat >> 12) & 63];
        out[k++] = kBase64Alphabet[(concat >> 6) & 63];
        out[k++] = kBase64Alphabet[concat & 63];

        if (i >= length) {
            int v;
            for (v = 0; v <= (i - length); v++)
                out[k-v] = '=';
        }

        lineLength += 4;

        // No line breaks in header mode, since the text is part of a Subject:
        // line or some other single header line.  The header code will do word
        // wrapping separately from this encoding stuff.
        if (!headerMode && lineLength > BASE64_LINELENGTH) {
            out[k++] = '\r';
            out[k++] = '\n';

            lineLength = 4;
        }
    }

    return k;
}


ssize_t
decode_base64(char *out, const char *in, off_t length)
{
    uint32 concat, value;
    int lastOutLine = 0;
    int i, j;
    int outIndex = 0;

    for (i = 0; i < length; i += 4) {
        concat = 0;

        for (j = 0; j < 4 && (i + j) < length; j++) {
            value = in[i + j];

            if (value == '\n' || value == '\r') {
                // jump over line breaks
                lastOutLine = outIndex;
                i++;
                j--;
                continue;
            }

            if ((value >= 'A') && (value <= 'Z'))
                value -= 'A';
            else if ((value >= 'a') && (value <= 'z'))
                value = value - 'a' + 26;
            else if ((value >= '0') && (value <= '9'))
                value = value - '0' + 52;
            else if (value == '+')
                value = 62;
            else if (value == '/')
                value = 63;
            else if (value == '=')
                break;
            else {
                // there is an invalid character in this line - we will
                // ignore the whole line and go to the next
                outIndex = lastOutLine;
                while (i < length && in[i] != '\n' && in[i] != '\r')
                    i++;
                concat = 0;
            }

            value = value << ((3-j)*6);

            concat |= value;
        }

        if (j > 1)
            out[outIndex++] = (concat & 0x00ff0000) >> 16;
        if (j > 2)
            out[outIndex++] = (concat & 0x0000ff00) >> 8;
        if (j > 3)
            out[outIndex++] = (concat & 0x000000ff);
    }

    return outIndex;
}


#if __GNUC__ <= 2
    // BeOS-ABI compatible wrappers.
    ssize_t encode_base64(char *out, char *in, off_t length)
    {
        return encode_base64(out, in, length, 0);
    }

    ssize_t	decode_base64(char *out, const char *in, off_t length,
        bool /*replace_cr*/)
    {
        return decode_base64(out, in, length);
    }
#endif	//  __GNUC__ <= 2
