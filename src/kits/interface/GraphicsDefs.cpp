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
 * File: GraphicsDefs.cpp
 *
 * --------------------------------------------------------------------------*/

#include "GraphicsDefs.h"
#include "Point.h"

extern EXPORTBHAPI const BPoint B_ORIGIN(0, 0);

extern "C" {
extern EXPORTBHAPI const bhapi::pattern B_SOLID_HIGH = bhapi::make_pattern(0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff);
extern EXPORTBHAPI const bhapi::pattern B_MIXED_COLORS = bhapi::make_pattern(0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55);
extern EXPORTBHAPI const bhapi::pattern B_SOLID_LOW = bhapi::make_pattern(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

extern EXPORTBHAPI const bhapi::rgb_color B_TRANSPARENT_COLOR = bhapi::make_rgb_color(233, 233, 233, 255);

#ifdef BHAPI_BIG_ENDIAN
extern EXPORTBHAPI const b_uint32 B_TRANSPARENT_MAGIC_RGBA32 = 0xE9E9E9FF;
#else
extern EXPORTBHAPI const b_uint32 B_TRANSPARENT_MAGIC_RGBA32 = 0xFFE9E9E9;
#endif

EXPORTBHAPI b_uint8 bhapi::find_index_for_color(b_uint8 r, b_uint8 g, b_uint8 b)
{
	// RGB: 3-3-2
	return((r & 0xe0) | ((g >> 3) & 0x1c) | (b >> 6));
}


EXPORTBHAPI bhapi::rgb_color b_find_color_for_index(b_uint8 index)
{
	// RGB: 3-3-2
	b_uint8 r = index & 0xe0;
	b_uint8 g = (index & 0x1c) << 3;
	b_uint8 b = (index & 0x03) << 6;
    return bhapi::make_rgb_color(r | (r >> 3) | (r >> 6),
	       			g | (g >> 3) | (g >> 6),
				b | (b >> 2) | (b >> 4) | (b >> 6),
				0xff);
}


} // extern "C"


bhapi::rgb_color& bhapi::rgb_color::mix(b_uint8 r, b_uint8 g, b_uint8 b, b_uint8 a)
{
	if(a == 0xff)
	{
		red = r; green = g; blue = b;
	}
	else if(a != 0)
	{
		red = (b_uint8)(((b_uint16)red * ((b_uint16)0xff - (b_uint16)a) + (b_uint16)r * (b_uint16)a) / (b_uint16)0xff);
		green = (b_uint8)(((b_uint16)green * ((b_uint16)0xff - (b_uint16)a) + (b_uint16)g * (b_uint16)a) / (b_uint16)0xff);
		blue = (b_uint8)(((b_uint16)blue * ((b_uint16)0xff - (b_uint16)a) + (b_uint16)b * (b_uint16)a) / (b_uint16)0xff);
	}

	return *this;
}


bhapi::rgb_color& bhapi::rgb_color::mix(const bhapi::rgb_color &o)
{
	return mix(o.red, o.green, o.blue, o.alpha);
}


bhapi::rgb_color& bhapi::rgb_color::mix_copy(b_uint8 r, b_uint8 g, b_uint8 b, b_uint8 a) const
{
	bhapi::rgb_color color = *this;
	return color.mix(r, g, b, a);
}


bhapi::rgb_color& bhapi::rgb_color::mix_copy(const bhapi::rgb_color &o) const
{
	bhapi::rgb_color color = *this;
	return color.mix(o.red, o.green, o.blue, o.alpha);
}


bhapi::rgb_color& bhapi::rgb_color::disable(b_uint8 r, b_uint8 g, b_uint8 b, b_uint8 a)
{
	bhapi::rgb_color color;
	color.set_to(r, g, b, a);
	return disable(color);
}


bhapi::rgb_color& bhapi::rgb_color::disable(const bhapi::rgb_color &background)
{
	*this = background.mix_copy(red, green, blue, 150);
	return mix(0, 0, 0, 20);
}


bhapi::rgb_color& bhapi::rgb_color::disable_copy(b_uint8 r, b_uint8 g, b_uint8 b, b_uint8 a) const
{
	bhapi::rgb_color color = *this;
	return color.disable(r, g, b, a);
}


bhapi::rgb_color& bhapi::rgb_color::disable_copy(const bhapi::rgb_color &background) const
{
	bhapi::rgb_color color = *this;
	return color.disable(background.red, background.green, background.blue, background.alpha);
}

