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
 * File: GraphicsDefs.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_GRAPHICS_DEFS_H__
#define __ETK_GRAPHICS_DEFS_H__

#include "./../support/SupportDefs.h"


typedef struct e_pattern {
	euint8		data[8];
#ifdef __cplusplus // just for C++
	inline bool operator==(const e_pattern& o) const
	{
		return (*((const euint64*)this)) == (*((const euint64*)&o));
	}

	inline bool operator!=(const e_pattern& o) const
	{
		return (*((const euint64*)this)) != (*((const euint64*)&o));
	}
#endif /* __cplusplus */
} e_pattern;


#ifdef __cplusplus // just for C++
inline e_pattern e_make_pattern(euint8 d1, euint8 d2, euint8 d3, euint8 d4, euint8 d5, euint8 d6, euint8 d7, euint8 d8)
{
	e_pattern p;
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

extern _IMPEXP_ETK const e_pattern E_SOLID_HIGH;
extern _IMPEXP_ETK const e_pattern E_MIXED_COLORS;
extern _IMPEXP_ETK const e_pattern E_SOLID_LOW;

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */


typedef struct e_rgb_color {
	euint8		red;
	euint8		green;
	euint8		blue;
	euint8		alpha;

#ifdef __cplusplus // just for C++
	inline e_rgb_color& set_to(euint8 r, euint8 g, euint8 b, euint8 a = 0xff)
	{
		red = r; green = g; blue = b; alpha = a;
		return *this;
	}

	inline e_rgb_color& set_to(const e_rgb_color& o)
	{
		return set_to(o.red, o.green, o.blue, o.alpha);
	}

	inline bool operator==(const e_rgb_color& o) const
	{
		return(*((const euint32*)this)) == (*((const euint32*)&o));
	}
	
	inline bool operator!=(const e_rgb_color& o) const
	{
		return(*((const euint32*)this)) != (*((const euint32*)&o));
	}

	e_rgb_color& mix(euint8 r, euint8 g, euint8 b, euint8 a);
	e_rgb_color& mix(const e_rgb_color &o);
	e_rgb_color& mix_copy(euint8 r, euint8 g, euint8 b, euint8 a) const;
	e_rgb_color& mix_copy(const e_rgb_color &o) const;

	e_rgb_color& disable(euint8 r, euint8 g, euint8 b, euint8 a);
	e_rgb_color& disable(const e_rgb_color &background);
	e_rgb_color& disable_copy(euint8 r, euint8 g, euint8 b, euint8 a) const;
	e_rgb_color& disable_copy(const e_rgb_color &background) const;
#endif /* __cplusplus */
} e_rgb_color;


#ifdef __cplusplus // just for C++
inline e_rgb_color e_make_rgb_color(euint8 r, euint8 g, euint8 b, euint8 a = 0xff)
{
	e_rgb_color c;
	c.set_to(r, g, b, a);
	return c;
}
#endif /* __cplusplus */


typedef enum e_drawing_mode {
	E_OP_COPY,
	E_OP_XOR,

	E_OP_OVER,
	E_OP_ERASE,
	E_OP_INVERT,
	E_OP_ADD,
	E_OP_SUBTRACT,
	E_OP_BLEND,
	E_OP_MIN,
	E_OP_MAX,
	E_OP_SELECT,
	E_OP_ALPHA,
} e_drawing_mode;


typedef enum e_color_space {
	E_CMAP8 = 0,		/* D(8) */
	E_RGB32 = 1,		/* BGRx(8:8:8:8) */
	E_RGBA32 = 2,		/* BGRA(8:8:8:8) */
	E_RGB24 = 3,		/* BGR(8:8:8) */
	E_RGB24_BIG = 4,	/* RGB(8:8:8) */
} e_color_space;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern _IMPEXP_ETK const e_rgb_color	E_TRANSPARENT_COLOR;
extern _IMPEXP_ETK const euint32	E_TRANSPARENT_MAGIC_RGBA32;

_IMPEXP_ETK euint8 etk_find_index_for_color(euint8 r, euint8 g, euint8 b);
_IMPEXP_ETK e_rgb_color etk_find_color_for_index(euint8 index);

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */


#endif /* __ETK_GRAPHICS_DEFS_H__ */

