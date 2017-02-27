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
 * File: GraphicsDefs.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_GRAPHICS_DEFS__H
#define BHAPI_GRAPHICS_DEFS__H
#include "../support/SupportDefs.h"

typedef struct b_pattern {
    b_uint8		data[8];
#ifdef __cplusplus // just for C++
	inline bool operator==(const b_pattern& o) const
	{
        return (*((const b_uint64*)this)) == (*((const b_uint64*)&o));
	}
	inline bool operator!=(const b_pattern& o) const
	{
        return (*((const b_uint64*)this)) != (*((const b_uint64*)&o));
	}
#endif /* __cplusplus */
} b_pattern;

#ifdef __cplusplus // just for C++
inline b_pattern b_makb_pattern(b_uint8 d1, b_uint8 d2, b_uint8 d3, b_uint8 d4, b_uint8 d5, b_uint8 d6, b_uint8 d7, b_uint8 d8)
{
	b_pattern p;
	p.data[0] = d1;
	p.data[1] = d2;
	p.data[2] = d3;
	p.data[3] = d4;
	p.data[4] = d5;
	p.data[5] = d6;
	p.data[6] = d7;
	p.data[7] = d8;
	return p;
}
#endif /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern IMPEXP_BHAPI const b_pattern B_SOLID_HIGH;
extern IMPEXP_BHAPI const b_pattern B_MIXED_COLORS;
extern IMPEXP_BHAPI const b_pattern B_SOLID_LOW;

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

typedef struct b_rgb_color {
    b_uint8		red;
    b_uint8		green;
    b_uint8		blue;
    b_uint8		alpha;

#ifdef __cplusplus // just for C++
    inline b_rgb_color& set_to(b_uint8 r, b_uint8 g, b_uint8 b, b_uint8 a = 0xff)
	{
		red = r; green = g; blue = b; alpha = a;
		return *this;
	}
	inline b_rgb_color& set_to(const b_rgb_color& o)
	{
		return set_to(o.red, o.green, o.blue, o.alpha);
	}
	inline bool operator==(const b_rgb_color& o) const
	{
        return(*((const b_uint32*)this)) == (*((const b_uint32*)&o));
	}
	inline bool operator!=(const b_rgb_color& o) const
	{
        return(*((const b_uint32*)this)) != (*((const b_uint32*)&o));
	}
    b_rgb_color& mix(b_uint8 r, b_uint8 g, b_uint8 b, b_uint8 a);
	b_rgb_color& mix(const b_rgb_color &o);
    b_rgb_color& mix_copy(b_uint8 r, b_uint8 g, b_uint8 b, b_uint8 a) const;
	b_rgb_color& mix_copy(const b_rgb_color &o) const;

    b_rgb_color& disable(b_uint8 r, b_uint8 g, b_uint8 b, b_uint8 a);
	b_rgb_color& disable(const b_rgb_color &background);
    b_rgb_color& disable_copy(b_uint8 r, b_uint8 g, b_uint8 b, b_uint8 a) const;
	b_rgb_color& disable_copy(const b_rgb_color &background) const;
#endif /* __cplusplus */
} b_rgb_color;

#ifdef __cplusplus // just for C++
inline b_rgb_color b_makb_rgb_color(b_uint8 r, b_uint8 g, b_uint8 b, b_uint8 a = 0xff)
{
	b_rgb_color c;
	c.set_to(r, g, b, a);
	return c;
}
#endif /* __cplusplus */

typedef enum b_drawing_mode {
    B_OP_COPY,
    B_OP_XOR,

    B_OP_OVER,
    B_OP_ERASE,
    B_OP_INVERT,
    B_OP_ADD,
    B_OP_SUBTRACT,
    B_OP_BLEND,
    B_OP_MIN,
    B_OP_MAX,
    B_OP_SELECT,
    B_OP_ALPHA,
} b_drawing_mode;

typedef enum b_color_space {
    B_CMAP8 = 0,		/* D(8) */
    B_RGB32 = 1,		/* BGRx(8:8:8:8) */
    B_RGBA32 = 2,		/* BGRA(8:8:8:8) */
    B_RGB24 = 3,		/* BGR(8:8:8) */
    B_RGB24_BIG = 4,	/* RGB(8:8:8) */
} b_color_space;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern IMPEXP_BHAPI const b_rgb_color    B_TRANSPARENT_COLOR;
extern IMPEXP_BHAPI const b_uint32    B_TRANSPARENT_MAGIC_RGBA32;

namespace bhapi {
IMPEXP_BHAPI b_uint8 find_index_for_color(b_uint8 r, b_uint8 g, b_uint8 b);
IMPEXP_BHAPI b_rgb_color find_color_for_index(b_uint8 index);
}
#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */
#endif /* BHAPI_GRAPHICS_DEFS__H */

