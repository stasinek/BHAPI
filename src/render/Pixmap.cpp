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
 * File: Pixmap.cpp
 *
 * --------------------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>

#include "./../support/StringArray.h"

#include "Pixmap.h"


EPixmap::EPixmap()
	: ERender(), fPtr(NULL), fColorSpace((e_color_space)0), fRows(0), fColumns(0), fRowBytes(0)
{
}


EPixmap::EPixmap(euint32 width, euint32 height, e_color_space space)
	: ERender(), fPtr(NULL), fColorSpace((e_color_space)0), fRows(0), fColumns(0), fRowBytes(0)
{
	ResizeTo(width, height, space);
}


EPixmap::EPixmap(ERect bounds, e_color_space space)
	: ERender(), fPtr(NULL), fColorSpace((e_color_space)0), fRows(0), fColumns(0), fRowBytes(0)
{
	ResizeTo((euint32)(max_c(bounds.IntegerWidth() + 1, 0)), (euint32)(max_c(bounds.IntegerHeight() + 1, 0)), space);
}


EPixmap::~EPixmap()
{
	if(fPtr) FreeData(fPtr);
}


void
EPixmap::MakeEmpty()
{
	if(fPtr) FreeData(fPtr);
	fPtr = NULL;
}


e_status_t
EPixmap::InitCheck() const
{
	return(fPtr != NULL ? E_OK : E_ERROR);
}


void*
EPixmap::Bits() const
{
	return((void*)fPtr);
}


euint32
EPixmap::BitsLength() const
{
	if(!fPtr) return 0;
	return(fRowBytes * fRows);
}


euint32
EPixmap::BytesPerRow() const
{
	if(!fPtr) return 0;
	return fRowBytes;
}


e_color_space
EPixmap::ColorSpace() const
{
	return fColorSpace;
}


ERect
EPixmap::Bounds() const
{
	if(!fPtr) return ERect();
	return ERect(0, 0, (float)(fColumns - 1), (float)(fRows - 1));
}


bool
EPixmap::ResizeTo(euint32 width, euint32 height, e_color_space space)
{
	if(width == 0 || height == 0)
	{
		MakeEmpty();
		return true;
	}

	size_t allocSize;

	switch(space)
	{
		case E_CMAP8:
			allocSize = (size_t)(width * height);
			break;

		case E_RGB24:
		case E_RGB24_BIG:
			allocSize = 3 * (size_t)(width * height);
			break;

		case E_RGB32:
		case E_RGBA32:
			allocSize = 4 * (size_t)(width * height);
			break;

		default:
			allocSize = 0;
	}

	if(allocSize == 0)
	{
		ETK_WARNING("[RENDER]: %s --- color space(%d) not supported!", __PRETTY_FUNCTION__, space);
		return false;
	}

	void *newPtr = AllocData(allocSize);
	if(!newPtr) return false;

	if(fPtr) FreeData(fPtr);
	fPtr = newPtr;
	bzero(fPtr, allocSize);

	fRows = height;
	fColumns = width;
	fColorSpace = space;
	fRowBytes = (euint32)allocSize / fRows;

	return true;
}


bool
EPixmap::ResizeTo(ERect bounds, e_color_space space)
{
	return ResizeTo((euint32)(max_c(bounds.IntegerWidth() + 1, 0)), (euint32)(max_c(bounds.IntegerHeight() + 1, 0)), space);
}


void*
EPixmap::AllocData(size_t size)
{
	return(size > 0 && size <= 16000000 ? malloc(size) : NULL);
}


void
EPixmap::FreeData(void *data)
{
	if(data) free(data);
}


void
EPixmap::GetFrame(eint32 *originX, eint32 *originY, euint32 *width, euint32 *height) const
{
	if(originX) *originX = 0;
	if(originY) *originY = 0;
	if(width) *width = fColumns;
	if(height) *height = fRows;
}


void
EPixmap::GetPixel(eint32 x, eint32 y, e_rgb_color &color) const
{
	if(fPtr == NULL) return;

	if(x < 0 || y < 0 || (euint32)x >= fColumns || (euint32)y >= fRows) return;

	if(fColorSpace == E_RGB32 || fColorSpace == E_RGBA32)
	{
		const euint32 *bits = (const euint32*)fPtr;
		bits += (size_t)(fColumns * (euint32)y + (euint32)x);
#ifndef ETK_BIG_ENDIAN
		/* A-R-G-B */
		color.set_to((*bits >> 16) & 0xff, (*bits >> 8) & 0xff, *bits & 0xff,
			     fColorSpace == E_RGBA32 ? (*bits >> 24) : 0xff);
#else
		/* B-G-R-A */
		color.set_to((*bits >> 8) & 0xff, (*bits >> 16) & 0xff, *bits >> 24,
			     fColorSpace == E_RGBA32 ? (*bits & 0xff) : 0xff);
#endif
	}
	else if(fColorSpace == E_RGB24)
	{
		const euint8 *bits = (const euint8*)fPtr;
		bits += 3 * (size_t)(fColumns * (euint32)y + (euint32)x);
		color.blue = *bits++;
		color.green = *bits++;
		color.red = *bits;
		color.alpha = 0xff;
	}
	else if(fColorSpace == E_RGB24_BIG)
	{
		const euint8 *bits = (const euint8*)fPtr;
		bits += 3 * (size_t)(fColumns * (euint32)y + (euint32)x);
		color.red = *bits++;
		color.green = *bits++;
		color.blue = *bits;
		color.alpha = 0xff;
	}
	else if(fColorSpace == E_CMAP8)
	{
		const euint8 *bits = (const euint8*)fPtr;
		bits += (size_t)(fColumns * (euint32)y + (euint32)x);
		color = etk_find_color_for_index(*bits);
	}
}


void
EPixmap::PutPixel(eint32 x, eint32 y, e_rgb_color color)
{
	if(fPtr == NULL) return;

	if(x < 0 || y < 0 || (euint32)x >= fColumns || (euint32)y >= fRows) return;

	if(fColorSpace == E_RGB32 || fColorSpace == E_RGBA32)
	{
		euint32 *bits = (euint32*)fPtr;
		bits += (size_t)(fColumns * (euint32)y + (euint32)x);
#ifndef ETK_BIG_ENDIAN
		/* A-R-G-B */
		*bits = ((fColorSpace == E_RGBA32 ? (euint32)color.alpha : 0x000000ff) << 24) |
			((euint32)color.red << 16) | ((euint32)color.green << 8) | (euint32)color.blue;
#else
		/* B-G-R-A */
		*bits = (fColorSpace == E_RGBA32 ? (euint32)color.alpha : 0x000000ff) |
			((euint32)color.red << 8) | ((euint32)color.green << 16) | ((euint32)color.blue << 24);
#endif
	}
	else if(fColorSpace == E_RGB24)
	{
		euint8 *bits = (euint8*)fPtr;
		bits += 3 * (size_t)(fColumns * (euint32)y + (euint32)x);

		*bits++ = color.blue;
		*bits++ = color.green;
		*bits = color.red;
	}
	else if(fColorSpace == E_RGB24_BIG)
	{
		euint8 *bits = (euint8*)fPtr;
		bits += 3 * (size_t)(fColumns * (euint32)y + (euint32)x);

		*bits++ = color.red;
		*bits++ = color.green;
		*bits = color.blue;
	}
	else if(fColorSpace == E_CMAP8)
	{
		euint8 *bits = (euint8*)fPtr;
		bits += (size_t)(fColumns * (euint32)y + (euint32)x);

		*bits = etk_find_index_for_color(color.red, color.green, color.blue);
	}
}


void
EPixmap::PutRect(eint32 x, eint32 y, euint32 width, euint32 height, e_rgb_color color)
{
	if(fPtr == NULL) return;

	if(x < 0 || y < 0 || (euint32)x >= fColumns || (euint32)y >= fRows) return;

	width = min_c((euint32)(fColumns - x), width);
	height = min_c((euint32)(fRows - y), height);

	if(width == 0 || height == 0) return;

	if((fColorSpace == E_RGB32 || fColorSpace == E_RGBA32) && DrawingMode() == E_OP_COPY)
	{
		euint32 val;
#ifndef ETK_BIG_ENDIAN
		/* A-R-G-B */
		val = ((fColorSpace == E_RGBA32 ? (euint32)color.alpha : 0x000000ff) << 24) |
		       ((euint32)color.red << 16) | ((euint32)color.green << 8) | (euint32)color.blue;
#else
		/* B-G-R-A */
		val = (fColorSpace == E_RGBA32 ? (euint32)color.alpha : 0x000000ff) |
		       ((euint32)color.red << 8) | ((euint32)color.green << 16) | ((euint32)color.blue << 24);
#endif
		for(euint32 k = 0; k < height; k++)
		{
			euint32 *bits = (euint32*)fPtr + (size_t)(fColumns * (euint32)(y + k) + (euint32)x);
			for(euint32 i = 0; i < width; i++) memcpy(bits++, &val, sizeof(val));
		}
	}
	else if(fColorSpace == E_CMAP8 && DrawingMode() == E_OP_COPY)
	{
		euint8 val = etk_find_index_for_color(color.red, color.green, color.blue);
		for(euint32 k = 0; k < height; k++)
		{
			euint8 *bits = (euint8*)fPtr + (size_t)(fColumns * (euint32)(y + k) + (euint32)x);
			for(euint32 i = 0; i < width; i++) memcpy(bits++, &val, sizeof(val));
		}
	}
	else
	{
		for(euint32 k = 0; k < height; k++)
			for(euint32 i = 0; i < width; i++) PutPixel(x + i, y + k, color);
	}
}


void
EPixmap::SetBits(const void *data, eint32 length, eint32 offset, e_color_space space)
{
	if(data == NULL || length <= 0 || offset < 0 || !IsValid()) return;
	if(BitsLength() - (euint32)length < (euint32)offset) length = BitsLength() - (euint32)offset;
	if(length <= 0) return;

	if(fColorSpace == space)
	{
		memcpy((euint8*)fPtr + offset, data, (size_t)length);
	}
	else
	{
		// TODO
		ETK_WARNING("[RENDER]: %s --- color space must same as the pixmap.", __PRETTY_FUNCTION__);
	}
}


void
EPixmap::SetPixel(eint32 x, eint32 y, e_rgb_color color)
{
	PutPixel(x, y, color);
}


e_rgb_color
EPixmap::GetPixel(eint32 x, eint32 y) const
{
	e_rgb_color color = e_make_rgb_color(0, 0, 0, 0);
	GetPixel(x, y, color);
	return color;
}


void
EPixmap::DrawXPM(const char **xpm_data, eint32 destX, eint32 destY, eint32 srcX, eint32 srcY, eint32 srcW, eint32 srcH, euint8 alpha)
{
	if(xpm_data == NULL || *xpm_data == NULL ||
	   destX >= (eint32)fColumns || destY >= (eint32)fRows || srcX < 0 || srcY < 0 || srcW == 0 || srcH == 0) return;

	EString str, tmp;
	EStringArray colors;
	eint32 xpmWidth = 0, xpmHeight = 0, numColor = 0, bytesColor = 0, offset;

	str.SetTo(*xpm_data++);

	offset = 0;
	for(eint32 i = 0; i < 4; i++)
	{
		eint32 oldOffset = offset;
		offset = (i == 3 ? str.Length() : str.FindFirst(" ", offset));
		if(offset < 0) break;
		tmp.SetTo(str.String() + oldOffset, (offset++) - oldOffset);

		if(i == 0)
			tmp.GetInteger(&xpmWidth);
		else if(i == 1)
			tmp.GetInteger(&xpmHeight);
		else if(i == 2)
			tmp.GetInteger(&numColor);
		else if(i == 3)
			tmp.GetInteger(&bytesColor);
	}

//	ETK_DEBUG("[RENDER]: %s --- xpmWidth: %d, xpmHeight: %d, numColor: %d, bytesColor: %d", __PRETTY_FUNCTION__,
//		  xpmWidth, xpmHeight, numColor, bytesColor);

	if(xpmWidth <= 0 || xpmHeight <= 0 || numColor <= 0 || bytesColor <= 0) return;
	if(srcX >= xpmWidth || srcY >= xpmHeight) return;
	if(srcW < 0) srcW = xpmWidth;
	if(srcH < 0) srcH = xpmHeight;

	euint32 *colors_array = (euint32*)malloc(sizeof(euint32) * (size_t)numColor);
	if(colors_array == NULL) return;

	for(eint32 i = 0; i < numColor && *xpm_data != NULL; i++, xpm_data++)
	{
		str.SetTo(*xpm_data);

		if((offset = str.FindLast(" ")) < 0) offset = str.FindLast("\t");
		if(offset < bytesColor + 2 || offset == str.Length() - 1) break;

		tmp.SetTo(str.String() + offset + 1, -1);
//		ETK_DEBUG("[RENDER]: %s --- color(%d) %s", __PRETTY_FUNCTION__, i, tmp.String());

		// TODO: other colors
		if(tmp.ICompare("None") == 0)
		{
			*(colors_array + i) = 0xE9E9E9;
		}
		else
		{
			if(tmp.ByteAt(0) != '#' || tmp.Length() != 7) break;
			tmp.ReplaceFirst("#", "0x");
			if(tmp.GetInteger(colors_array + i) == false) break;
			if(*(colors_array + i) == 0xE9E9E9) *(colors_array + i) = 0xEAEAEA;
		}

		tmp.SetTo(str, bytesColor);
		if(colors.AddItem(tmp, colors_array + i) == false) break;
	}

	if(colors.CountItems() != numColor) {free(colors_array); return;}

	for(eint32 j = 0; j < xpmHeight && *xpm_data != NULL; j++, xpm_data++)
	{
		eint32 Y = destY + (j - srcY);

		if(Y < 0 || j < srcY) continue;
		if(Y >= (eint32)fRows || j >= srcY + (eint32)srcH) break;

		offset = 0;
		str.SetTo(*xpm_data);

		for(eint32 i = 0; i < xpmWidth && offset <= str.Length() - bytesColor; i++, offset += bytesColor)
		{
			eint32 X = destX + (i - srcX);

			if(X < 0 || i < srcX) continue;
			if(X >= (eint32)fColumns || i >= srcX + (eint32)srcW) break;

			tmp.SetTo(str.String() + offset, bytesColor);
			eint32 *color = NULL;
			eint32 found = colors.FindString(tmp);
			if(found < 0) break;
			if(colors.ItemAt(found, (void**)&color) == NULL) break;
			if(color == NULL || *color == 0xE9E9E9) continue;

			e_rgb_color c;
			if(alpha == 255)
			{
				c.set_to((*color >> 16) & 0xff, (*color >> 8) & 0xff, *color & 0xff);
			}
			else
			{
				c = GetPixel(X, Y);
				c.mix((*color >> 16) & 0xff, (*color >> 8) & 0xff, *color & 0xff, alpha);
			}

			SetPixel(X, Y, c);
		}
	}

	free(colors_array);
}

