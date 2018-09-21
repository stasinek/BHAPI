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
 * File: GraphicsDefs.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_GRAPHICS_DEFS_H
#define BHAPI_GRAPHICS_DEFS_H

#include <Haiku.h>

#ifdef __cplusplus
namespace bhapi {
#endif /* __cplusplus */
// Pattern
typedef struct b_pattern {
    uint8_t data[8];
} pattern;
#ifdef __cplusplus
} // namespace
#endif /* __cplusplus */

#ifdef __cplusplus
inline bool operator==(const bhapi::pattern& a, const bhapi::pattern& b)
{
    uint64_t* pa = (uint64_t*)a.data;
    uint64_t* pb = (uint64_t*)b.data;
    return (*pa == *pb);
}

inline bool operator!=(const bhapi::pattern& a, const bhapi::pattern& b)
{
    return !(a == b);
}
#endif // __cplusplus

#ifdef __cplusplus
namespace bhapi {
#endif /* __cplusplus */
// rgb_color
typedef struct rgb_color {
    uint8_t		red;
    uint8_t		green;
    uint8_t		blue;
    uint8_t		alpha;
#ifdef __cplusplus
// some convenient additions
inline rgb_color& set_to(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
{
    red = r;
    green = g;
    blue = b;
    alpha = a;
    return *this;
}
inline rgb_color& set_to(const rgb_color& other)
{
    return set_to(other.red, other.green, other.blue, other.alpha);
}

inline bool operator==(const rgb_color& other) const
{
    return *(const uint32_t *)this == *(const uint32_t *)&other;
}

inline bool operator!=(const rgb_color& other) const
{
    return *(const uint32_t *)this != *(const uint32_t *)&other;
}

inline rgb_color& operator=(const rgb_color& other)
{
    return set_to(other.red, other.green, other.blue, other.alpha);
}
rgb_color& mix(uint8_t r,  uint8_t g,  uint8_t b,  uint8_t a);
rgb_color& mix(const rgb_color &o);
rgb_color& mix_copy(uint8_t r,  uint8_t g,  uint8_t b,  uint8_t a) const;
rgb_color& mix_copy(const rgb_color &o) const;

rgb_color& disable(uint8_t r,  uint8_t g,  uint8_t b,  uint8_t a);
rgb_color& disable(const rgb_color &background);
rgb_color& disable_copy(uint8_t r,  uint8_t g,  uint8_t b,  uint8_t a) const;
rgb_color& disable_copy(const rgb_color &background) const;
#endif
} rgb_color;

#ifdef __cplusplus
inline rgb_color make_color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
{
    rgb_color color = {red, green, blue, alpha};
    return color;
}
#endif
// color map
typedef struct color_map {
    int32_t				id;
    rgb_color			color_list[256];
    uint8_t				inversion_map[256];
    uint8_t				index_map[32768];
} color_map;
// overlay
typedef struct overlay_rect_limits {
    uint16_t				horizontal_alignment;
    uint16_t				vertical_alignment;
    uint16_t				width_alignment;
    uint16_t				height_alignment;
    uint16_t				min_width;
    uint16_t				max_width;
    uint16_t				min_height;
    uint16_t				max_height;
    uint32_t				reserved[8];
} overlay_rect_limits;
// overlay restrictions
typedef struct overlay_restrictions {
    overlay_rect_limits	source;
    overlay_rect_limits	destination;
    float				min_width_scale;
    float				max_width_scale;
    float				min_height_scale;
    float				max_height_scale;
    uint32_t				reserved[8];
} overlay_restrictions;
// Screen ID
typedef struct screen_id { int32_t id; } screen_id;
// Color spaces
typedef enum {
    B_NO_COLOR_SPACE	= 0x0000,
    // linear color space (little endian)
    B_RGB32				= 0x0008,	// BGR-		-RGB 8:8:8:8
    B_RGBA32			= 0x2008,	// BGRA		ARGB 8:8:8:8
    B_RGB24				= 0x0003,	// BGR		 RGB 8:8:8
    B_RGB16				= 0x0005,	// BGR		 RGB 5:6:5
    B_RGB15				= 0x0010,	// BGR-		-RGB 1:5:5:5
    B_RGBA15			= 0x2010,	// BGRA		ARGB 1:5:5:5
    B_CMAP8				= 0x0004,	// 256 color index table
    B_GRAY8				= 0x0002,	// 256 greyscale table
    B_GRAY1				= 0x0001,	// Each bit represents a single pixel
    // linear color space (big endian)
    B_RGB32_BIG			= 0x1008,	// -RGB		BGR- 8:8:8:8
    B_RGBA32_BIG		= 0x3008,	// ARGB		BGRA 8:8:8:8
    B_RGB24_BIG			= 0x1003,	//  RGB		BGR  8:8:8
    B_RGB16_BIG			= 0x1005,	//  RGB		BGR  5:6:5
    B_RGB15_BIG			= 0x1010,	// -RGB		BGR- 5:5:5:1
    B_RGBA15_BIG		= 0x3010,	// ARGB		BGRA 5:5:5:1
    // linear color space (little endian, for completeness)
    B_RGB32_LITTLE		= B_RGB32,
    B_RGBA32_LITTLE		= B_RGBA32,
    B_RGB24_LITTLE		= B_RGB24,
    B_RGB16_LITTLE		= B_RGB16,
    B_RGB15_LITTLE		= B_RGB15,
    B_RGBA15_LITTLE		= B_RGBA15,
    // non linear color space -- incidently, all with 8 bits per value
    // Note, BBitmap and BView do not support all of these!
    // Loss / saturation points:
    //  Y		16 - 235 (absolute)
    //  Cb/Cr	16 - 240 (center 128)

    B_YCbCr422			= 0x4000,	// Y0  Cb0 Y1  Cr0
                                    // Y2  Cb2 Y3  Cr4
    B_YCbCr411			= 0x4001,	// Cb0 Y0  Cr0 Y1
                                    // Cb4 Y2  Cr4 Y3
                                    // Y4  Y5  Y6  Y7
    B_YCbCr444			= 0x4003,	// Y   Cb  Cr
    B_YCbCr420			= 0x4004,	// Non-interlaced only
        // on even scan lines: Cb0  Y0  Y1  Cb2 Y2  Y3
        // on odd scan lines:  Cr0  Y0  Y1  Cr2 Y2  Y3

    // Extrema points are:
    //  Y 0 - 207 (absolute)
    //  U -91 - 91 (offset 128)
    //  V -127 - 127 (offset 128)

    // Note that YUV byte order is different from YCbCr; use YCbCr, not YUV,
    // when that's what you mean!
    B_YUV422			= 0x4020,	// U0  Y0  V0  Y1
                                    // U2  Y2  V2  Y3
    B_YUV411			= 0x4021,	// U0  Y0  Y1  V0  Y2  Y3
                                    // U4  Y4  Y5  V4  Y6  Y7
    B_YUV444			= 0x4023,	// U0  Y0  V0  U1  Y1  V1
    B_YUV420			= 0x4024,	// Non-interlaced only
        // on even scan lines: U0  Y0  Y1  U2 Y2  Y3
        // on odd scan lines:  V0  Y0  Y1  V2 Y2  Y3
    B_YUV9				= 0x402C,
    B_YUV12				= 0x402D,
    B_UVL24				= 0x4030,	// UVL
    B_UVL32				= 0x4031,	// UVL-
    B_UVLA32			= 0x6031,	// UVLA
    // L lightness, a/b color-opponent dimensions
    B_LAB24				= 0x4032,	// Lab
    B_LAB32				= 0x4033,	// Lab-
    B_LABA32			= 0x6033,	// LabA
    // Red is at hue 0
    B_HSI24				= 0x4040,	// HSI
    B_HSI32				= 0x4041,	// HSI-
    B_HSIA32			= 0x6041,	// HSIA
    B_HSV24				= 0x4042,	// HSV
    B_HSV32				= 0x4043,	// HSV-
    B_HSVA32			= 0x6043,	// HSVA
    B_HLS24				= 0x4044,	// HLS
    B_HLS32				= 0x4045,	// HLS-
    B_HLSA32			= 0x6045,	// HLSA
    B_CMY24				= 0xC001,	// CMY
    B_CMY32				= 0xC002,	// CMY-
    B_CMYA32			= 0xE002,	// CMYA
    B_CMYK32			= 0xC003,	// CMYK
    // Compatibility declarations
    B_MONOCHROME_1_BIT	= B_GRAY1,
    B_GRAYSCALE_8_BIT	= B_GRAY8,
    B_COLOR_8_BIT		= B_CMAP8,
    B_RGB_32_BIT		= B_RGB32,
    B_RGB_16_BIT		= B_RGB15,
    B_BIG_RGB_32_BIT	= B_RGB32_BIG,
    B_BIG_RGB_16_BIT	= B_RGB15_BIG
} color_space;
// Bitmap Support Flags
enum {
    B_VIEWS_SUPPORT_DRAW_BITMAP			= 0x1,
    B_BITMAPS_SUPPORT_ATTACHED_VIEWS	= 0x2,
    B_BITMAPS_SUPPORT_OVERLAY			= 0x4
};
// buffer orientation
enum buffer_orientation {
    B_BUFFER_TOP_TO_BOTTOM,
    B_BUFFER_BOTTOM_TO_TOP
};
// buffer layout
enum buffer_layout {
    B_BUFFER_NONINTERLEAVED = 1
};
// Drawing Modes
enum drawing_mode {
    B_OP_COPY,
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
    B_OP_XOR
};
enum source_alpha {
    B_PIXEL_ALPHA = 0,
    B_CONSTANT_ALPHA
};
enum alpha_function {
    B_ALPHA_OVERLAY = 0,
    B_ALPHA_COMPOSITE
};
// Fixed Screen Modes
enum {
    B_8_BIT_640x480		= 0x00000001,
    B_8_BIT_800x600		= 0x00000002,
    B_8_BIT_1024x768	= 0x00000004,
    B_8_BIT_1280x1024	= 0x00000008,
    B_8_BIT_1600x1200	= 0x00000010,
    B_16_BIT_640x480	= 0x00000020,
    B_16_BIT_800x600	= 0x00000040,
    B_16_BIT_1024x768	= 0x00000080,
    B_16_BIT_1280x1024	= 0x00000100,
    B_16_BIT_1600x1200	= 0x00000200,
    B_32_BIT_640x480	= 0x00000400,
    B_32_BIT_800x600	= 0x00000800,
    B_32_BIT_1024x768	= 0x00001000,
    B_32_BIT_1280x1024	= 0x00002000,
    B_32_BIT_1600x1200	= 0x00004000,
    B_8_BIT_1152x900	= 0x00008000,
    B_16_BIT_1152x900	= 0x00010000,
    B_32_BIT_1152x900	= 0x00020000,
    B_15_BIT_640x480	= 0x00040000,
    B_15_BIT_800x600	= 0x00080000,
    B_15_BIT_1024x768	= 0x00100000,
    B_15_BIT_1280x1024	= 0x00200000,
    B_15_BIT_1600x1200	= 0x00400000,
    B_15_BIT_1152x900	= 0x00800000,
    B_8_BIT_640x400		= 0x80000000
};
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
extern const bhapi::pattern B_SOLID_HIGH;
extern const bhapi::pattern B_MIXED_COLORS;
extern const bhapi::pattern B_SOLID_LOW;
extern const bhapi::rgb_color 	B_TRANSPARENT_COLOR;
extern const uint8_t		B_TRANSPARENT_MAGIC_CMAP8;
extern const uint16_t		B_TRANSPARENT_MAGIC_RGBA15;
extern const uint16_t		B_TRANSPARENT_MAGIC_RGBA15_BIG;
extern const uint32_t		B_TRANSPARENT_MAGIC_RGBA32;
extern const uint32_t		B_TRANSPARENT_MAGIC_RGBA32_BIG;
extern const uint8_t 		B_TRANSPARENT_8_BIT;
extern const bhapi::rgb_color	B_TRANSPARENT_32_BIT;
extern const screen_id B_MAIN_SCREEN_ID;
bool bitmaps_support_space(color_space space, uint32_t* _supportFlags);
status_t get_pixel_size_for(color_space space, size_t* _pixelChunk, size_t* _rowAlignment, size_t* _pixelsPerChunk);
#ifdef __cplusplus // just for C++
inline pattern make_pattern(uint8_t d1,  uint8_t d2,  uint8_t d3,  uint8_t d4,  uint8_t d5,  uint8_t d6,  uint8_t d7,  uint8_t d8)
{
    bhapi::pattern p;
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
inline rgb_color make_rgb_color(uint8_t r,  uint8_t g,  uint8_t b,  uint8_t a = 0xff)
{
    rgb_color c;
    c.set_to(r, g, b, a);
    return c;
}
#endif /* __cplusplus */
BHAPI_IMPEXP  uint8_t find_index_for_color(uint8_t r,  uint8_t g,  uint8_t b);
BHAPI_IMPEXP bhapi::rgb_color find_color_for_index(uint8_t index);
#ifdef __cplusplus
} // extern "C"
#endif /* __cplusplus */
#ifdef __cplusplus
} // namespace
#endif /* __cplusplus */
#endif /* BHAPI_GRAPHICS_DEFS_H */

