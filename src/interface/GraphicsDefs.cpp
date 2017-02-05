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
 * File: GraphicsDefs.cpp
 *
 * --------------------------------------------------------------------------*/

#include "GraphicsDefs.h"
#include "Point.h"

extern _IMPEXP_ETK const EPoint E_ORIGIN(0, 0);

extern "C" {

extern _IMPEXP_ETK const e_pattern E_SOLID_HIGH = e_make_pattern(0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
extern _IMPEXP_ETK const e_pattern E_MIXED_COLORS = e_make_pattern(0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55);
extern _IMPEXP_ETK const e_pattern E_SOLID_LOW = e_make_pattern(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

extern _IMPEXP_ETK const e_rgb_color E_TRANSPARENT_COLOR = e_make_rgb_color(233, 233, 233, 255);

#ifdef ETK_BIG_ENDIAN
extern _IMPEXP_ETK const euint32 E_TRANSPARENT_MAGIC_RGBA32 = 0xE9E9E9FF;
#else
extern _IMPEXP_ETK const euint32 E_TRANSPARENT_MAGIC_RGBA32 = 0xFFE9E9E9;
#endif


_IMPEXP_ETK euint8 etk_find_index_for_color(euint8 r, euint8 g, euint8 b)
{
	// RGB: 3-3-2
	return((r & 0xe0) | ((g >> 3) & 0x1c) | (b >> 6));
}


_IMPEXP_ETK e_rgb_color etk_find_color_for_index(euint8 index)
{
	// RGB: 3-3-2
	euint8 r = index & 0xe0;
	euint8 g = (index & 0x1c) << 3;
	euint8 b = (index & 0x03) << 6;
	return e_make_rgb_color(r | (r >> 3) | (r >> 6),
	       			g | (g >> 3) | (g >> 6),
				b | (b >> 2) | (b >> 4) | (b >> 6),
				0xff);
}


} // extern "C"


e_rgb_color&
e_rgb_color::mix(euint8 r, euint8 g, euint8 b, euint8 a)
{
	if(a == 0xff)
	{
		red = r; green = g; blue = b;
	}
	else if(a != 0)
	{
		red = (euint8)(((euint16)red * ((euint16)0xff - (euint16)a) + (euint16)r * (euint16)a) / (euint16)0xff);
		green = (euint8)(((euint16)green * ((euint16)0xff - (euint16)a) + (euint16)g * (euint16)a) / (euint16)0xff);
		blue = (euint8)(((euint16)blue * ((euint16)0xff - (euint16)a) + (euint16)b * (euint16)a) / (euint16)0xff);
	}

	return *this;
}


e_rgb_color&
e_rgb_color::mix(const e_rgb_color &o)
{
	return mix(o.red, o.green, o.blue, o.alpha);
}


e_rgb_color&
e_rgb_color::mix_copy(euint8 r, euint8 g, euint8 b, euint8 a) const
{
	e_rgb_color color = *this;
	return color.mix(r, g, b, a);
}


e_rgb_color&
e_rgb_color::mix_copy(const e_rgb_color &o) const
{
	e_rgb_color color = *this;
	return color.mix(o.red, o.green, o.blue, o.alpha);
}


e_rgb_color&
e_rgb_color::disable(euint8 r, euint8 g, euint8 b, euint8 a)
{
	e_rgb_color color;
	color.set_to(r, g, b, a);
	return disable(color);
}


e_rgb_color&
e_rgb_color::disable(const e_rgb_color &background)
{
	*this = background.mix_copy(red, green, blue, 150);
	return mix(0, 0, 0, 20);
}


e_rgb_color&
e_rgb_color::disable_copy(euint8 r, euint8 g, euint8 b, euint8 a) const
{
	e_rgb_color color = *this;
	return color.disable(r, g, b, a);
}


e_rgb_color&
e_rgb_color::disable_copy(const e_rgb_color &background) const
{
	e_rgb_color color = *this;
	return color.disable(background.red, background.green, background.blue, background.alpha);
}

