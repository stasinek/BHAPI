//-----------------------------------------------------------------------------
#include <utf8_functions.h>
#include <StringClass.h>
//-----------------------------------------------------------------------------

BHAPI_EXPORT bool bhapi::utf8_is_token(const char *str)
{
    const unsigned char *p = (const unsigned char *)str;
    if (str == NULL) return true;

    return(*p < 0x80 || !(*p < 0xc0 || *p >= 0xfe));
}
//-----------------------------------------------------------------------------

BHAPI_EXPORT  int32 bhapi::utf8_strlen(const char *str)
{
    return bhapi::utf8_strlen_etc(str, -1);
}
//-----------------------------------------------------------------------------

BHAPI_LOCAL  int32 _e_utf8_strlen_etc(const char *str,  int32 nbytes, bool check)
{
    const unsigned char *p = (const unsigned char *)str;
     int32             uLen = 0;

     int32 strLen = (int32) strlen(str);
    if (str == NULL || *str == 0 || nbytes == 0) return 0;
    if (nbytes < 0 || nbytes > strLen) nbytes = strLen;

    while (*p)
    {
         int8  len = 0;

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
            for (int8 i = len; i >= 0; i--)
            {
                p++;
                if (i <= 1) break;
                if (*p < 0x80 || *p >= 0xc0) i = len = 0;   // 0xxxxxxx or 11xxxxxx : invalid UTF8
            }
        }

        if ((int32) (p - (const unsigned char *)str) > nbytes) break;
        if (len > 0 && len <= 4) uLen++;
    }

    return uLen;
}
//-----------------------------------------------------------------------------

BHAPI_EXPORT  int32 bhapi::utf8_strlen_etc(const char *str,  int32 nbytes)
{
    return _e_utf8_strlen_etc(str, nbytes, true);
}
//-----------------------------------------------------------------------------

BHAPI_EXPORT  int32 bhapi::utf8_strlen_fast(const char *str,  int32 nbytes)
{
    return _e_utf8_strlen_etc(str, nbytes, false);
}
//-----------------------------------------------------------------------------

BHAPI_EXPORT const char *bhapi::utf8_at(const char *str,  int32 index,  uint8 *length)
{
    const unsigned char *p = (const unsigned char *)str;
     int32             uLen = 0;
     int8              len = 0;

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
        for (int8 i = len; i >= 0; i--)
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
        if (length) *length = (uint8) len;
        return (const char *)p;
    }

    return NULL;
}
//-----------------------------------------------------------------------------

BHAPI_EXPORT const char *bhapi::utf8_next(const char *str,  uint8 *length)
{
    const unsigned char *p = (const unsigned char *)str;
    bool                first = true;

    if (length) *length = 0;
    if (str == NULL || *str == 0) return NULL;

    while (*p)
    {
         int8  len = 0;

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
        for (int8 i = len; i >= 0; i--)
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

BHAPI_EXPORT unichar16 *bhapi::utf8_convert_to_unicode(const char *str,  int32 length)
{
    unichar16 *unicode = (unichar16 *)malloc(sizeof(unichar16) * (size_t) (length + 1));
    const unsigned char *p = (const unsigned char *)str;
    unichar16         *tmp = unicode;
     int32 strLen = (int32) strlen(str);

    if (str == NULL || *str == 0 || length == 0) return NULL;
    if (length < 0 || length > strLen) length = strLen;
    if (!unicode) return NULL;

    while (*p)
    {
         int8  len = 0;

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
        for (int8 i = len; i >= 0; i--)
        {
            p++;
            if (i <= 1) break;
            if (*p < 0x80 || *p >= 0xc0) i = len = 0;   // 0xxxxxxx or 11xxxxxx : invalid UTF8
        }

        if ((int32) ((const char *)p - str) > length) break;

        if (len > 0)
        {
            p -= len;
            if (len == 1)
            {
                *tmp = (unichar16) (*p++);
            }
            else if (len == 2)
            {
                *tmp = ((unichar16) (*p++) & 0x1f) << 6;
                *tmp |= (unichar16) (*p++) & 0x3f;
            }
            else if (len == 3)
            {
                *tmp = ((unichar16) (*p++) & 0x0f) << 12;
                *tmp |= ((unichar16) (*p++) & 0x3f) << 6;
                *tmp |= (unichar16) (*p++) & 0x3f;
            }
            else if (len == 4)
            {
                unichar32 tmp32 = ((unichar32) (*p++) & 0x07) << 18;
                tmp32 |= ((unichar32) (*p++) & 0x3f) << 12;
                tmp32 |= ((unichar32) (*p++) & 0x3f) << 6;
                tmp32 |= (unichar32) (*p++) & 0x3f;

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

BHAPI_EXPORT unichar32 *bhapi::utf8_convert_to_utf32(const char *str,  int32 length)
{
    const unsigned char *p = (const unsigned char *)str;
    unichar32 *unicode = (unichar32 *)malloc(sizeof(unichar32) * (size_t) (length + 1));
    unichar32         *tmp = unicode;
     int32 strLen = (int32) strlen(str);

    if (str == NULL || *str == 0 || length == 0) return NULL;
    if (length < 0 || length > strLen) length = strLen;
    if (unicode == NULL) return NULL;

    while (*p)
    {
         int8  len = 0;

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
        for (int8 i = len; i >= 0; i--)
        {
            p++;
            if (i <= 1) break;
            if (*p < 0x80 || *p >= 0xc0) i = len = 0;   // 0xxxxxxx or 11xxxxxx : invalid UTF8
        }

        if ((int32) ((const char *)p - str) > length) break;

        if (len > 0)
        {
            p -= len;
            if (len == 1)
            {
                *tmp = (unichar32) (*p++);
            }
            else if (len == 2)
            {
                *tmp = ((unichar32) (*p++) & 0x1f) << 6;
                *tmp |= (unichar32) (*p++) & 0x3f;
            }
            else if (len == 3)
            {
                *tmp = ((unichar32) (*p++) & 0x0f) << 12;
                *tmp |= ((unichar32) (*p++) & 0x3f) << 6;
                *tmp |= (unichar32) (*p++) & 0x3f;
            }
            else if (len == 4)
            {
                *tmp = ((unichar32) (*p++) & 0x07) << 18;
                *tmp |= ((unichar32) (*p++) & 0x3f) << 12;
                *tmp |= ((unichar32) (*p++) & 0x3f) << 6;
                *tmp |= (unichar32) (*p++) & 0x3f;
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

BHAPI_EXPORT  int32 bhapi::unicode_strlen(const unichar16 *ustr)
{
    return bhapi::unicode_strlen_etc(ustr, -1, true);
}
//-----------------------------------------------------------------------------

BHAPI_EXPORT  int32 bhapi::unicode_strlen_etc(const unichar16 *ustr,  int32 nchars, bool utf16_style)
{
     int32             len = 0;
    const unichar16   *p = ustr;

    if (ustr == NULL || nchars == 0) return 0;

    while (*p)
    {
        if (utf16_style ? (*p >= 0xd800 && *p <= 0xdfff) : false)
        {
            p++;
            if (*p >= 0xdc00 && *p <= 0xdfff)
            {
                if (nchars > 0)
                    if ((int32) (p - ustr) > nchars - 1) break;
                p++;
                len++;
            }
        }
        else
        {
            if (nchars > 0)
                if ((int32) (p - ustr) > nchars - 1) break;
            p++;
            len++;
        }
    }

    return len;
}
//-----------------------------------------------------------------------------

BHAPI_EXPORT const unichar16 *bhapi::unicode_at(const unichar16 *ustr,  int32 index, bool *utf16)
{
     int32             len = 0;
    const unichar16   *p = ustr;

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

BHAPI_EXPORT const unichar16 *bhapi::unicode_next(const unichar16 *ustr, bool *utf16)
{
    const unichar16   *p = ustr;
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

BHAPI_EXPORT char *bhapi::unicode_convert_to_utf8(const unichar16 *str,  int32 ulength)
{
    BString             utf8;

     int32             ulen = 0;
    const unichar16   *p = str;

    if (str == NULL || *str == 0 || ulength == 0) return NULL;

    while (*p != 0 && (ulength > 0 ? ulength > ulen : true))
    {
        if (*p >= 0xd800 && *p <= 0xdfff)
        {
            p++;
            if (*p >= 0xdc00 && *p <= 0xdfff)
            {
                 uint8 ustr[5];
                 uint32 tmp = (uint32) (*p++ &0x03ff) << 10;
                p--;

                // convert UTF-16 to UCS4
                tmp |= (uint32) (*p++ &0x03ff);
                tmp += 0x10000;

                // convert UCS4 to UTF-8
                bzero(ustr, sizeof(ustr));
                ustr[3] = 0x80 | (uint8) (tmp & 0x3f);
                ustr[2] = 0x80 | (uint8) ((tmp >> 6) & 0x3f);
                ustr[1] = 0x80 | (uint8) ((tmp >> 12) & 0x3f);
                ustr[0] = 0xf0 | (uint8) ((tmp >> 18) & 0x07);
                utf8.Append((const char *)ustr);

                ulen++;
            }
        }
        else
        {
             uint8     len;
             uint16    tmp = *p++;

             uint8     ustr[4];
            if (*p < 0x80)
                len = 1;
            else if (*p < 0x0800)
                len = 2;
            else
                len = 3;

            bzero(ustr, sizeof(ustr));

            if (len == 1)
            {
                ustr[0] = (uint8) tmp;
            }
            else if (len == 2)
            {
                ustr[1] = 0x80 | (uint8) (tmp & 0x3f);
                ustr[0] = 0xc0 | (uint8) ((tmp >> 6) & 0x1f);
            }
            else    // len == 3
            {
                ustr[2] = 0x80 | (uint8) (tmp & 0x3f);
                ustr[1] = 0x80 | (uint8) ((tmp >> 6) & 0x3f);
                ustr[0] = 0xe0 | (uint8) ((tmp >> 12) & 0x0f);
            }

            utf8.Append((const char *)ustr);

            ulen++;
        }
    }

    return strndup(utf8.String(), -1);
}
//-----------------------------------------------------------------------------

BHAPI_EXPORT unichar32 *bhapi::unicode_convert_to_utf32(const unichar16 *str,  int32 ulength)
{
    const unichar16   *p = str;
     int32             ulen = 0;
     int32 unicode_length = (ulength < 0 ? bhapi::unicode_strlen(str) : ulength);
    unichar32 *utf32 = (unichar32 *)malloc(sizeof(unichar32) * (size_t) (unicode_length + 1));

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
                 uint32    tmp = ((uint32) (*p++ &0x03ff) << 10);
                p--;

                // convert UTF-16 to UCS4
                tmp |= (uint32) (*p++ &0x03ff);
                tmp += 0x10000;

                utf32[ulen++] = tmp;

                ulen++;
            }
        }
        else
        {
            utf32[ulen++] = (unichar32) (*p++);
        }
    }

    utf32[ulen] = 0;

    return utf32;
}
//-----------------------------------------------------------------------------

BHAPI_EXPORT  int32 bhapi::utf32_strlen(const unichar32 *ustr)
{
    return bhapi::utf32_strlen_etc(ustr, -1);
}
//-----------------------------------------------------------------------------

BHAPI_EXPORT  int32 bhapi::utf32_strlen_etc(const unichar32 *ustr,  int32 nchars)
{
     int32             len = 0;
    const unichar32   *p = ustr;

    if (ustr == NULL || nchars == 0) return 0;

    while (*p)
    {
        if (nchars > 0)
            if ((int32) (p - ustr) > nchars - 1) break;
        if (*p <= 0x10ffff) len++;
        p++;
    }

    return len;
}
//-----------------------------------------------------------------------------

BHAPI_EXPORT const unichar32 *bhapi::utf32_at(const unichar32 *ustr,  int32 index)
{
     int32             len = 0;
    const unichar32   *p = ustr;

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

BHAPI_EXPORT const unichar32 *bhapi::utf32_next(const unichar32 *ustr)
{
    const unichar32   *p = ustr;

    if (ustr == NULL) return NULL;

    while (*p)
    {
        p++;
        if (*p > 0 && *p <= 0x10ffff) return p;
    }

    return NULL;
}
//-----------------------------------------------------------------------------

BHAPI_EXPORT char *bhapi::utf32_convert_to_utf8(const unichar32 *str,  int32 ulength)
{
    BString             utf8;
     int32             ulen = 0;
    const unichar32   *p = str;

    if (str == NULL || *str == 0 || ulength == 0) return NULL;

    while (*p != 0 && (ulength > 0 ? ulength > ulen : true))
    {
        if (*p > 0xffff && *p <= 0x10ffff)
        {
             uint32    tmp = *p;

            // convert UCS4 to UTF-8
             uint8     ustr[5];
            bzero(ustr, sizeof(ustr));
            ustr[3] = 0x80 | (uint8) (tmp & 0x3f);
            ustr[2] = 0x80 | (uint8) ((tmp >> 6) & 0x3f);
            ustr[1] = 0x80 | (uint8) ((tmp >> 12) & 0x3f);
            ustr[0] = 0xf0 | (uint8) ((tmp >> 18) & 0x07);
            utf8.Append((const char *)ustr);

            ulen++;
        }
        else if (*p <= 0xffff)
        {
             uint8     len;
             uint32    tmp = *p;

             uint8     ustr[4];
            if (*p < 0x80)
                len = 1;
            else if (*p < 0x0800)
                len = 2;
            else
                len = 3;

            bzero(ustr, sizeof(ustr));

            if (len == 1)
            {
                ustr[0] = (uint8) tmp;
            }
            else if (len == 2)
            {
                ustr[1] = 0x80 | (uint8) (tmp & 0x3f);
                ustr[0] = 0xc0 | (uint8) ((tmp >> 6) & 0x1f);
            }
            else    // len == 3
            {
                ustr[2] = 0x80 | (uint8) (tmp & 0x3f);
                ustr[1] = 0x80 | (uint8) ((tmp >> 6) & 0x3f);
                ustr[0] = 0xe0 | (uint8) ((tmp >> 12) & 0x0f);
            }

            utf8.Append((const char *)ustr);

            ulen++;
        }

        p++;
    }

    return strndup(utf8.String(), -1);
}
//-----------------------------------------------------------------------------

BHAPI_EXPORT unichar16 *bhapi::utf32_convert_to_unicode(const unichar32 *str,  int32 ulength)
{
     int32 utf32_length = (ulength < 0 ? bhapi::utf32_strlen(str) : ulength);
    unichar16 *unicode = (unichar16 *)malloc(sizeof(unichar16) * (size_t) (2 * utf32_length + 1));
     int32             ulen = 0;
    const unichar32   *p = str;
    unichar16         *tmp = unicode;

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
            *tmp++ = (unichar16) (*p);
            ulen++;
        }

        p++;
    }

    *tmp = 0;

    return unicode;
}
