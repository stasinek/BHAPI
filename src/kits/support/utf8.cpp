#include "UTF8.h"
#include "String.h"

EXPORTBHAPI bool bhapi::IsInsideGlyph(uchar ch)
{
    return (ch & 0xc0) == 0x80;
}

EXPORTBHAPI uint32 bhapi::UTF8NextCharLenUnsafe(const char *text)
{
    const char *ptr = text;

    do {
        ptr++;
    } while (IsInsideGlyph(*ptr));

    return ptr - text;
}

EXPORTBHAPI uint32 bhapi::UTF8NextCharLen(const char *text)
{
    if (text == NULL || *text == 0)
        return 0;

    return UTF8NextCharLenUnsafe(text);
}

EXPORTBHAPI uint32 bhapi::UTF8NextCharLen(const char *bytes, size_t length)
{
    if (bytes == NULL || length == 0 || bytes[0] == 0)
        return 0;

    if ((bytes[0] & 0x80) == 0) {
        // A single ASCII char - or so...
        return 1;
    }

    if (IsInsideGlyph(bytes[0])) {
        // Not a proper multibyte start.
        return 0;
    }

    // We already know that we have the upper two bits set due to the above
    // two checks.
    uint8 mask = 0x20;
    size_t bytesExpected = 2;
    while ((bytes[0] & mask) != 0) {
        if (mask == 0x02) {
            // Seven byte char - invalid.
            return 0;
        }

        bytesExpected++;
        mask >>= 1;
    }

    // There would need to be more bytes to satisfy the char.
    if (bytesExpected > length)
        return 0;

    // We already know the first byte is fine, check the rest.
    for (size_t i = 1; i < bytesExpected; i++) {
        if (!IsInsideGlyph(bytes[i])) {
            // The sequence is incomplete.
            return 0;
        }
    }

    // Puh, everything's fine.
    return bytesExpected;
}

EXPORTBHAPI uint32 bhapi::UTF8PreviousCharLen(const char *text, const char *limit)
{
    const char *ptr = text;

    if (ptr == NULL || limit == NULL)
        return 0;

    do {
        if (ptr == limit)
            break;
        ptr--;
    } while (IsInsideGlyph(*ptr));

    return text - ptr;
}


/*!	UTF8CountBytes gets the length (in bytes) of a UTF8 string. Up to
    numChars characters are read. If numChars is a negative value it is ignored
    and the string is read up to the terminating 0.
*/
EXPORTBHAPI uint32 bhapi::UTF8CountBytes(const char *bytes, int32 numChars)
{
    if (bytes == NULL)
        return 0;

    if (numChars < 0)
        numChars = INT_MAX;

    const char *base = bytes;
    while (bytes[0] != '\0') {
        if ((bytes[0] & 0xc0) != 0x80) {
            if (--numChars < 0)
                break;
        }
        bytes++;
    }

    return bytes - base;
}

/*!	UTF8CountChars gets the length (in characters) of a UTF8 string. Up to
    numBytes bytes are read. If numBytes is a negative value it is ignored
    and the string is read up to the terminating 0.
*/
EXPORTBHAPI uint32 bhapi::UTF8CountChars(const char *bytes, int32 numBytes)
{
    if (bytes == NULL)
        return 0;

    uint32 length = 0;
    const char *last;
    if (numBytes < 0)
        last = (const char *)SIZE_MAX;
    else
        last = bytes + numBytes - 1;

    while (bytes[0] && bytes <= last) {
        if ((bytes++[0] & 0xc0) != 0x80)
            length++;
    }

    return length;
}

/*!	UTF8ToCharCode converts the input that includes potential multibyte chars
    to UTF-32 char codes that can be used by FreeType. The string pointer is
    then advanced to the next character in the string. In case the terminating
    0 is reached, the string pointer is not advanced anymore and nulls are
    returned. This makes it safe to overruns and enables streamed processing
    of UTF8 strings.
*/
EXPORTBHAPI uint32 bhapi::UTF8ToCharCode(const char **bytes)
{
    #define UTF8_SUBSTITUTE_CHARACTER	0xfffd

    uint32 result;
    if (((*bytes)[0] & 0x80) == 0) {
        // a single byte character
        result = (*bytes)[0];
        if (result != '\0') {
            // do not advance beyond the terminating '\0'
            (*bytes)++;
        }

        return result;
    }

    if (((*bytes)[0] & 0xc0) == 0x80) {
        // not a proper multibyte start
        (*bytes)++;
        return UTF8_SUBSTITUTE_CHARACTER;
    }

    // start of a multibyte character
    uint8 mask = 0x80;
    result = (uint32)((*bytes)[0] & 0xff);
    (*bytes)++;

    while (result & mask) {
        if (mask == 0x02) {
            // seven byte char - invalid
            return UTF8_SUBSTITUTE_CHARACTER;
        }

        result &= ~mask;
        mask >>= 1;
    }

    while (((*bytes)[0] & 0xc0) == 0x80) {
        result <<= 6;
        result += (*bytes)[0] & 0x3f;
        (*bytes)++;

        mask <<= 1;
        if (mask == 0x40)
            return result;
    }

    if (mask == 0x40)
        return result;

    if ((*bytes)[0] == '\0') {
        // string terminated within multibyte char
        return 0x00;
    }

    // not enough bytes in multibyte char
    return UTF8_SUBSTITUTE_CHARACTER;

    #undef UTF8_SUBSTITUTE_CHARACTER
}
//-----------------------------------------------------------------------------

EXPORTBHAPI bool bhapi::utf8_is_token(const char *str)
{
    const unsigned char *p = (const unsigned char *)str;
    if (str == NULL) return true;

    return(*p < 0x80 || !(*p < 0xc0 || *p >= 0xfe));
}
//-----------------------------------------------------------------------------

EXPORTBHAPI b_int32 bhapi::utf8_strlen(const char *str)
{
    return bhapi::utf8_strlen_etc(str, -1);
}
//-----------------------------------------------------------------------------

LOCALBHAPI b_int32 _e_utf8_strlen_etc(const char *str, b_int32 nbytes, bool check)
{
    const unsigned char *p = (const unsigned char *)str;
    b_int32             uLen = 0;

    b_int32 strLen = (b_int32) strlen(str);
    if (str == NULL || *str == 0 || nbytes == 0) return 0;
    if (nbytes < 0 || nbytes > strLen) nbytes = strLen;

    while (*p)
    {
        b_int8  len = 0;

        if (*p < 0x80) len = 1; // 0xxxxxxx : ASCII
        else if (*p < 0xc0 || *p >= 0xfe)
            len = 0;            // 10xxxxxx or 1111111x : invalid UTF8
        else if (*p < 0xe0)
            len = 2;            // 110xxxxx : 2 bytes
        else if (*p < 0xf0)
            len = 3;            // 1110xxxx : 3 bytes
        else if (*p < 0xf8)
            len = 4;            // 11110xxx : 4 bytes
        else if (*p < 0xfc)
            len = 5;            // 111110xx : 5 bytes, it's invalid UTF8 util this wrote
        else if (*p < 0xfe)
            len = 6;            // 1111110x : 6 bytes, it's invalid UTF8 util this wrote
        if (!check)
        {
            p += max_c(len, 1);
        }
        else
        {
            for (b_int8 i = len; i >= 0; i--)
            {
                p++;
                if (i <= 1) break;
                if (*p < 0x80 || *p >= 0xc0) i = len = 0;   // 0xxxxxxx or 11xxxxxx : invalid UTF8
            }
        }

        if ((b_int32) (p - (const unsigned char *)str) > nbytes) break;
        if (len > 0 && len <= 4) uLen++;
    }

    return uLen;
}
//-----------------------------------------------------------------------------

EXPORTBHAPI b_int32 bhapi::utf8_strlen_etc(const char *str, b_int32 nbytes)
{
    return _e_utf8_strlen_etc(str, nbytes, true);
}
//-----------------------------------------------------------------------------

EXPORTBHAPI b_int32 bhapi::utf8_strlen_fast(const char *str, b_int32 nbytes)
{
    return _e_utf8_strlen_etc(str, nbytes, false);
}
//-----------------------------------------------------------------------------

EXPORTBHAPI const char *bhapi::utf8_at(const char *str, b_int32 index, b_uint8 *length)
{
    const unsigned char *p = (const unsigned char *)str;
    b_int32             uLen = 0;
    b_int8              len = 0;

    if (length) *length = 0;
    if (index < 0 || str == NULL || strlen(str) <= (size_t) index) return NULL;

    while (*p && index >= uLen)
    {
        if (*p < 0x80) len = 1; // 0xxxxxxx : ASCII
        else if (*p < 0xc0 || *p >= 0xfe)
            len = 0;            // 10xxxxxx or 1111111x : invalid UTF8
        else if (*p < 0xe0)
            len = 2;            // 110xxxxx : 2 bytes
        else if (*p < 0xf0)
            len = 3;            // 1110xxxx : 3 bytes
        else if (*p < 0xf8)
            len = 4;            // 11110xxx : 4 bytes
        else if (*p < 0xfc)
            len = 5;            // 111110xx : 5 bytes, it's invalid UTF8 util this wrote
        else if (*p < 0xfe)
            len = 6;            // 1111110x : 6 bytes, it's invalid UTF8 util this wrote
        for (b_int8 i = len; i >= 0; i--)
        {
            p++;
            if (i <= 1) break;
            if (*p < 0x80 || *p >= 0xc0) i = len = -(len - i + 1);  // 0xxxxxxx or 11xxxxxx : invalid UTF8
        }

        if (len > 0 && len <= 4) uLen++;
    }

    if (index == uLen - 1)
    {
        if (len < 0)
        {
            p += len;
            len = 0;
        }
        else
            p -= len;
        if (length) *length = (b_uint8) len;
        return (const char *)p;
    }

    return NULL;
}
//-----------------------------------------------------------------------------

EXPORTBHAPI const char *bhapi::utf8_next(const char *str, b_uint8 *length)
{
    const unsigned char *p = (const unsigned char *)str;
    bool                first = true;

    if (length) *length = 0;
    if (str == NULL || *str == 0) return NULL;

    while (*p)
    {
        b_int8  len = 0;

        if (*p < 0x80) len = 1; // 0xxxxxxx : ASCII
        else if (*p < 0xc0 || *p >= 0xfe)
            len = 0;            // 10xxxxxx or 1111111x : invalid UTF8
        else if (*p < 0xe0)
            len = 2;            // 110xxxxx : 2 bytes
        else if (*p < 0xf0)
            len = 3;            // 1110xxxx : 3 bytes
        else if (*p < 0xf8)
            len = 4;            // 11110xxx : 4 bytes
        else if (*p < 0xfc)
            len = 5;            // 111110xx : 5 bytes, it's invalid UTF8 util this wrote
        else if (*p < 0xfe)
            len = 6;            // 1111110x : 6 bytes, it's invalid UTF8 util this wrote
        for (b_int8 i = len; i >= 0; i--)
        {
            p++;
            if (i <= 1) break;
            if (*p < 0x80 || *p >= 0xc0) i = len = -(len - i + 1);  // 0xxxxxxx or 11xxxxxx : invalid UTF8
        }

        if (!first && len > 0 && len <= 4)
        {
            if (length) *length = len;
            p -= len;
            return (const char *)p;
        }
        else
        {
            first = false;
        }
    }

    return NULL;
}
//-----------------------------------------------------------------------------

EXPORTBHAPI b_unichar16 *bhapi::utf8_convert_to_unicode(const char *str, b_int32 length)
{
    b_unichar16 *unicode = (b_unichar16 *)malloc(sizeof(b_unichar16) * (size_t) (length + 1));
    const unsigned char *p = (const unsigned char *)str;
    b_unichar16         *tmp = unicode;
    b_int32 strLen = (b_int32) strlen(str);

    if (str == NULL || *str == 0 || length == 0) return NULL;
    if (length < 0 || length > strLen) length = strLen;
    if (!unicode) return NULL;

    while (*p)
    {
        b_int8  len = 0;

        if (*p < 0x80) len = 1; // 0xxxxxxx : ASCII
        else if (*p < 0xc0 || *p >= 0xfe)
            len = 0;            // 10xxxxxx or 1111111x : invalid UTF8
        else if (*p < 0xe0)
            len = 2;            // 110xxxxx : 2 bytes
        else if (*p < 0xf0)
            len = 3;            // 1110xxxx : 3 bytes
        else if (*p < 0xf8)
            len = 4;            // 11110xxx : 4 bytes
        else if (*p < 0xfc)
            len = 5;            // 111110xx : 5 bytes, it's invalid UTF8 util this wrote
        else if (*p < 0xfe)
            len = 6;            // 1111110x : 6 bytes, it's invalid UTF8 util this wrote
        for (b_int8 i = len; i >= 0; i--)
        {
            p++;
            if (i <= 1) break;
            if (*p < 0x80 || *p >= 0xc0) i = len = 0;   // 0xxxxxxx or 11xxxxxx : invalid UTF8
        }

        if ((b_int32) ((const char *)p - str) > length) break;

        if (len > 0)
        {
            p -= len;
            if (len == 1)
            {
                *tmp = (b_unichar16) (*p++);
            }
            else if (len == 2)
            {
                *tmp = ((b_unichar16) (*p++) & 0x1f) << 6;
                *tmp |= (b_unichar16) (*p++) & 0x3f;
            }
            else if (len == 3)
            {
                *tmp = ((b_unichar16) (*p++) & 0x0f) << 12;
                *tmp |= ((b_unichar16) (*p++) & 0x3f) << 6;
                *tmp |= (b_unichar16) (*p++) & 0x3f;
            }
            else if (len == 4)
            {
                b_unichar32 tmp32 = ((b_unichar32) (*p++) & 0x07) << 18;
                tmp32 |= ((b_unichar32) (*p++) & 0x3f) << 12;
                tmp32 |= ((b_unichar32) (*p++) & 0x3f) << 6;
                tmp32 |= (b_unichar32) (*p++) & 0x3f;

                if (tmp32 > 0xffff)
                {
                    *tmp++ = 0xd800 | ((tmp32 - 0x10000) >> 10);
                    *tmp = 0xdc00 | ((tmp32 - 0x10000) & 0x03ff);
                }
                else
                {
                    *tmp = tmp32;
                }
            }
            else
            {
                p += len;
                continue;
            }   // don't support the 5 or 6 bytes UTF-8

            tmp++;
        }
    }

    *tmp = 0;

    return unicode;
}
//-----------------------------------------------------------------------------

EXPORTBHAPI b_unichar32 *bhapi::utf8_convert_to_utf32(const char *str, b_int32 length)
{
    const unsigned char *p = (const unsigned char *)str;
    b_unichar32 *unicode = (b_unichar32 *)malloc(sizeof(b_unichar32) * (size_t) (length + 1));
    b_unichar32         *tmp = unicode;
    b_int32 strLen = (b_int32) strlen(str);

    if (str == NULL || *str == 0 || length == 0) return NULL;
    if (length < 0 || length > strLen) length = strLen;
    if (unicode == NULL) return NULL;

    while (*p)
    {
        b_int8  len = 0;

        if (*p < 0x80) len = 1; // 0xxxxxxx : ASCII
        else if (*p < 0xc0 || *p >= 0xfe)
            len = 0;            // 10xxxxxx or 1111111x : invalid UTF8
        else if (*p < 0xe0)
            len = 2;            // 110xxxxx : 2 bytes
        else if (*p < 0xf0)
            len = 3;            // 1110xxxx : 3 bytes
        else if (*p < 0xf8)
            len = 4;            // 11110xxx : 4 bytes
        else if (*p < 0xfc)
            len = 5;            // 111110xx : 5 bytes, it's invalid UTF8 util this wrote
        else if (*p < 0xfe)
            len = 6;            // 1111110x : 6 bytes, it's invalid UTF8 util this wrote
        for (b_int8 i = len; i >= 0; i--)
        {
            p++;
            if (i <= 1) break;
            if (*p < 0x80 || *p >= 0xc0) i = len = 0;   // 0xxxxxxx or 11xxxxxx : invalid UTF8
        }

        if ((b_int32) ((const char *)p - str) > length) break;

        if (len > 0)
        {
            p -= len;
            if (len == 1)
            {
                *tmp = (b_unichar32) (*p++);
            }
            else if (len == 2)
            {
                *tmp = ((b_unichar32) (*p++) & 0x1f) << 6;
                *tmp |= (b_unichar32) (*p++) & 0x3f;
            }
            else if (len == 3)
            {
                *tmp = ((b_unichar32) (*p++) & 0x0f) << 12;
                *tmp |= ((b_unichar32) (*p++) & 0x3f) << 6;
                *tmp |= (b_unichar32) (*p++) & 0x3f;
            }
            else if (len == 4)
            {
                *tmp = ((b_unichar32) (*p++) & 0x07) << 18;
                *tmp |= ((b_unichar32) (*p++) & 0x3f) << 12;
                *tmp |= ((b_unichar32) (*p++) & 0x3f) << 6;
                *tmp |= (b_unichar32) (*p++) & 0x3f;
            }
            else
            {
                p += len;
                continue;
            }   // don't support the 5 or 6 bytes UTF-8

            tmp++;
        }
    }

    *tmp = 0;

    return unicode;
}
//-----------------------------------------------------------------------------

EXPORTBHAPI b_int32 bhapi::unicode_strlen(const b_unichar16 *ustr)
{
    return bhapi::unicode_strlen_etc(ustr, -1, true);
}
//-----------------------------------------------------------------------------

EXPORTBHAPI b_int32 bhapi::unicode_strlen_etc(const b_unichar16 *ustr, b_int32 nchars, bool utf16_style)
{
    b_int32             len = 0;
    const b_unichar16   *p = ustr;

    if (ustr == NULL || nchars == 0) return 0;

    while (*p)
    {
        if (utf16_style ? (*p >= 0xd800 && *p <= 0xdfff) : false)
        {
            p++;
            if (*p >= 0xdc00 && *p <= 0xdfff)
            {
                if (nchars > 0)
                    if ((b_int32) (p - ustr) > nchars - 1) break;
                p++;
                len++;
            }
        }
        else
        {
            if (nchars > 0)
                if ((b_int32) (p - ustr) > nchars - 1) break;
            p++;
            len++;
        }
    }

    return len;
}
//-----------------------------------------------------------------------------

EXPORTBHAPI const b_unichar16 *bhapi::unicode_at(const b_unichar16 *ustr, b_int32 index, bool *utf16)
{
    b_int32             len = 0;
    const b_unichar16   *p = ustr;

    if (utf16) *utf16 = false;
    if (ustr == NULL || index < 0) return NULL;

    while (*p)
    {
        bool    _utf16_ = false;

        if (*p >= 0xd800 && *p <= 0xdfff)
        {
            p++;
            if (*p >= 0xdc00 && *p <= 0xdfff)
            {
                p++;
                len++;
                _utf16_ = true;
            }
        }
        else
        {
            p++;
            len++;
        }

        if (index == len - 1)
        {
            if (utf16) *utf16 = _utf16_;
            p--;
            if (_utf16_) p--;
            return p;
        }
    }

    return NULL;
}
//-----------------------------------------------------------------------------

EXPORTBHAPI const b_unichar16 *bhapi::unicode_next(const b_unichar16 *ustr, bool *utf16)
{
    const b_unichar16   *p = ustr;
    bool                first = true;

    if (utf16) *utf16 = false;
    if (ustr == NULL) return NULL;

    while (*p)
    {
        if (*p >= 0xd800 && *p <= 0xdfff)
        {
            p++;
            if (*p >= 0xdc00 && *p <= 0xdfff)
            {
                if (!first)
                {
                    if (utf16) *utf16 = true;
                    p--;
                    return p;
                }
                else
                {
                    first = false;
                    p++;
                }
            }
        }
        else
        {
            if (!first)
            {
                if (utf16) *utf16 = false;
                return p;
            }
            else
            {
                first = false;
                p++;
            }
        }
    }

    return NULL;
}
//-----------------------------------------------------------------------------

EXPORTBHAPI char *bhapi::unicode_convert_to_utf8(const b_unichar16 *str, b_int32 ulength)
{
    BString             utf8;

    b_int32             ulen = 0;
    const b_unichar16   *p = str;

    if (str == NULL || *str == 0 || ulength == 0) return NULL;

    while (*p != 0 && (ulength > 0 ? ulength > ulen : true))
    {
        if (*p >= 0xd800 && *p <= 0xdfff)
        {
            p++;
            if (*p >= 0xdc00 && *p <= 0xdfff)
            {
                b_uint8 ustr[5];
                b_uint32 tmp = (b_uint32) (*p++ &0x03ff) << 10;
                p--;

                // convert UTF-16 to UCS4
                tmp |= (b_uint32) (*p++ &0x03ff);
                tmp += 0x10000;

                // convert UCS4 to UTF-8
                bzero(ustr, sizeof(ustr));
                ustr[3] = 0x80 | (b_uint8) (tmp & 0x3f);
                ustr[2] = 0x80 | (b_uint8) ((tmp >> 6) & 0x3f);
                ustr[1] = 0x80 | (b_uint8) ((tmp >> 12) & 0x3f);
                ustr[0] = 0xf0 | (b_uint8) ((tmp >> 18) & 0x07);
                utf8.Append((const char *)ustr);

                ulen++;
            }
        }
        else
        {
            b_uint8     len;
            b_uint16    tmp = *p++;

            b_uint8     ustr[4];
            if (*p < 0x80)
                len = 1;
            else if (*p < 0x0800)
                len = 2;
            else
                len = 3;

            bzero(ustr, sizeof(ustr));

            if (len == 1)
            {
                ustr[0] = (b_uint8) tmp;
            }
            else if (len == 2)
            {
                ustr[1] = 0x80 | (b_uint8) (tmp & 0x3f);
                ustr[0] = 0xc0 | (b_uint8) ((tmp >> 6) & 0x1f);
            }
            else    // len == 3
            {
                ustr[2] = 0x80 | (b_uint8) (tmp & 0x3f);
                ustr[1] = 0x80 | (b_uint8) ((tmp >> 6) & 0x3f);
                ustr[0] = 0xe0 | (b_uint8) ((tmp >> 12) & 0x0f);
            }

            utf8.Append((const char *)ustr);

            ulen++;
        }
    }

    return bhapi::strndup(utf8.String(), -1);
}
//-----------------------------------------------------------------------------

EXPORTBHAPI b_unichar32 *bhapi::unicode_convert_to_utf32(const b_unichar16 *str, b_int32 ulength)
{
    const b_unichar16   *p = str;
    b_int32             ulen = 0;
    b_int32 unicode_length = (ulength < 0 ? bhapi::unicode_strlen(str) : ulength);
    b_unichar32 *utf32 = (b_unichar32 *)malloc(sizeof(b_unichar32) * (size_t) (unicode_length + 1));

    if (str == NULL || *str == 0 || ulength == 0) return NULL;
    if (unicode_length <= 0) return NULL;
    if (!utf32) return NULL;

    while (*p != 0 && (ulength > 0 ? ulength > ulen : true))
    {
        if (*p >= 0xd800 && *p <= 0xdfff)
        {
            p++;
            if (*p >= 0xdc00 && *p <= 0xdfff)
            {
                b_uint32    tmp = ((b_uint32) (*p++ &0x03ff) << 10);
                p--;

                // convert UTF-16 to UCS4
                tmp |= (b_uint32) (*p++ &0x03ff);
                tmp += 0x10000;

                utf32[ulen++] = tmp;

                ulen++;
            }
        }
        else
        {
            utf32[ulen++] = (b_unichar32) (*p++);
        }
    }

    utf32[ulen] = 0;

    return utf32;
}
//-----------------------------------------------------------------------------

EXPORTBHAPI b_int32 bhapi::utf32_strlen(const b_unichar32 *ustr)
{
    return bhapi::utf32_strlen_etc(ustr, -1);
}
//-----------------------------------------------------------------------------

EXPORTBHAPI b_int32 bhapi::utf32_strlen_etc(const b_unichar32 *ustr, b_int32 nchars)
{
    b_int32             len = 0;
    const b_unichar32   *p = ustr;

    if (ustr == NULL || nchars == 0) return 0;

    while (*p)
    {
        if (nchars > 0)
            if ((b_int32) (p - ustr) > nchars - 1) break;
        if (*p <= 0x10ffff) len++;
        p++;
    }

    return len;
}
//-----------------------------------------------------------------------------

EXPORTBHAPI const b_unichar32 *bhapi::utf32_at(const b_unichar32 *ustr, b_int32 index)
{
    b_int32             len = 0;
    const b_unichar32   *p = ustr;

    if (ustr == NULL || index < 0) return NULL;

    while (*p)
    {
        if (*p <= 0x10ffff)
        {
            if (index == len)
                return p;
            else
                len++;
        }

        p++;
    }

    return NULL;
}
//-----------------------------------------------------------------------------

EXPORTBHAPI const b_unichar32 *bhapi::utf32_next(const b_unichar32 *ustr)
{
    const b_unichar32   *p = ustr;

    if (ustr == NULL) return NULL;

    while (*p)
    {
        p++;
        if (*p > 0 && *p <= 0x10ffff) return p;
    }

    return NULL;
}
//-----------------------------------------------------------------------------

EXPORTBHAPI char *bhapi::utf32_convert_to_utf8(const b_unichar32 *str, b_int32 ulength)
{
    BString             utf8;
    b_int32             ulen = 0;
    const b_unichar32   *p = str;

    if (str == NULL || *str == 0 || ulength == 0) return NULL;

    while (*p != 0 && (ulength > 0 ? ulength > ulen : true))
    {
        if (*p > 0xffff && *p <= 0x10ffff)
        {
            b_uint32    tmp = *p;

            // convert UCS4 to UTF-8
            b_uint8     ustr[5];
            bzero(ustr, sizeof(ustr));
            ustr[3] = 0x80 | (b_uint8) (tmp & 0x3f);
            ustr[2] = 0x80 | (b_uint8) ((tmp >> 6) & 0x3f);
            ustr[1] = 0x80 | (b_uint8) ((tmp >> 12) & 0x3f);
            ustr[0] = 0xf0 | (b_uint8) ((tmp >> 18) & 0x07);
            utf8.Append((const char *)ustr);

            ulen++;
        }
        else if (*p <= 0xffff)
        {
            b_uint8     len;
            b_uint32    tmp = *p;

            b_uint8     ustr[4];
            if (*p < 0x80)
                len = 1;
            else if (*p < 0x0800)
                len = 2;
            else
                len = 3;

            bzero(ustr, sizeof(ustr));

            if (len == 1)
            {
                ustr[0] = (b_uint8) tmp;
            }
            else if (len == 2)
            {
                ustr[1] = 0x80 | (b_uint8) (tmp & 0x3f);
                ustr[0] = 0xc0 | (b_uint8) ((tmp >> 6) & 0x1f);
            }
            else    // len == 3
            {
                ustr[2] = 0x80 | (b_uint8) (tmp & 0x3f);
                ustr[1] = 0x80 | (b_uint8) ((tmp >> 6) & 0x3f);
                ustr[0] = 0xe0 | (b_uint8) ((tmp >> 12) & 0x0f);
            }

            utf8.Append((const char *)ustr);

            ulen++;
        }

        p++;
    }

    return bhapi::strndup(utf8.String(), -1);
}
//-----------------------------------------------------------------------------

EXPORTBHAPI b_unichar16 *bhapi::utf32_convert_to_unicode(const b_unichar32 *str, b_int32 ulength)
{
    b_int32 utf32_length = (ulength < 0 ? bhapi::utf32_strlen(str) : ulength);
    b_unichar16 *unicode = (b_unichar16 *)malloc(sizeof(b_unichar16) * (size_t) (2 * utf32_length + 1));
    b_int32             ulen = 0;
    const b_unichar32   *p = str;
    b_unichar16         *tmp = unicode;

    if (str == NULL || *str == 0 || ulength == 0) return NULL;
    if (utf32_length <= 0) return NULL;
    if (!unicode) return NULL;

    while (*p != 0 && (ulength > 0 ? ulength > ulen : true))
    {
        if (*p > 0xffff && *p <= 0x10ffff)
        {
            *tmp++ = 0xd800 | ((*p - 0x10000) >> 10);
            *tmp++ = 0xdc00 | ((*p - 0x10000) & 0x03ff);
            ulen++;
        }
        else if (*p <= 0xffff)
        {
            *tmp++ = (b_unichar16) (*p);
            ulen++;
        }

        p++;
    }

    *tmp = 0;

    return unicode;
}
