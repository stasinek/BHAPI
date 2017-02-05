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
 * File: StringView.cpp
 *
 * --------------------------------------------------------------------------*/

#include "StringView.h"

#define ETK_STRING_VIEW_LINE_SPACING	0.25f


EStringView::EStringView(ERect frame, const char *name, const char *initial_text, euint32 resizeMode, euint32 flags)
	: EView(frame, name, resizeMode, flags), fTextArray(NULL), fAlignment(E_ALIGN_LEFT), fVerticalAlignment(E_ALIGN_TOP)
{
	if(initial_text)
	{
		fText = initial_text;
		if(fText.Length() > 0) fTextArray = fText.Split('\n');
	}

	SetHighColor(e_ui_color(E_PANEL_TEXT_COLOR));
	SetLowColor(ViewColor());
}


EStringView::~EStringView()
{
	if(fTextArray) delete fTextArray;
}


void
EStringView::SetText(const char *text)
{
	if(fText != text)
	{
		if(fTextArray) delete fTextArray;
		fTextArray = NULL;
		fText = text;
		if(fText.Length() > 0) fTextArray = fText.Split('\n');
		Invalidate();
	}
}


const char*
EStringView::Text() const
{
	return fText.String();
}


void
EStringView::SetAlignment(e_alignment alignment)
{
	if(fAlignment != alignment)
	{
		fAlignment = alignment;
		Invalidate();
	}
}


e_alignment
EStringView::Alignment() const
{
	return fAlignment;
}


void
EStringView::SetVerticalAlignment(e_vertical_alignment alignment)
{
	if(fVerticalAlignment != alignment)
	{
		fVerticalAlignment = alignment;
		Invalidate();
	}
}


e_vertical_alignment
EStringView::VerticalAlignment() const
{
	return fVerticalAlignment;
}


void
EStringView::Draw(ERect updateRect)
{
	if(Window() == NULL || !fTextArray || fTextArray->CountItems() <= 0) return;

	ERegion clipping;
	GetClippingRegion(&clipping);
	if(clipping.CountRects() > 0) clipping &= updateRect;
	else clipping = updateRect;
	if(clipping.CountRects() <= 0) return;

	e_rgb_color fgColor = HighColor();

	if(!IsEnabled())
	{
		e_rgb_color color = ViewColor();
		color.alpha = 127;
		fgColor.mix(color);
	}

	EFont font;
	e_font_height fontHeight;
	GetFont(&font);
	font.GetHeight(&fontHeight);
	float sHeight = fontHeight.ascent + fontHeight.descent;

	float allHeight = (float)(fTextArray->CountItems() - 1) * (float)ceil((double)(sHeight * ETK_STRING_VIEW_LINE_SPACING)) +
			  (float)(fTextArray->CountItems()) * sHeight;
	float lineHeight = sHeight + (float)ceil((double)(sHeight * ETK_STRING_VIEW_LINE_SPACING));

	ERect bounds = Frame().OffsetToSelf(E_ORIGIN);

	float yStart = 0;
	switch(fVerticalAlignment)
	{
		case E_ALIGN_BOTTOM:
			yStart = bounds.bottom - allHeight;
			break;

		case E_ALIGN_MIDDLE:
			yStart = bounds.Center().y - allHeight / 2.f;
			break;

		default:
			break;
	}


	PushState();
	ConstrainClippingRegion(&clipping);
	SetDrawingMode(E_OP_COPY);
	SetHighColor(fgColor);
	SetLowColor(ViewColor());
	for(eint32 i = 0; i < fTextArray->CountItems(); i++)
	{
		const EString *str = fTextArray->ItemAt(i);
		float strWidth = 0;
		if(!(!str || str->Length() <= 0 || (strWidth = font.StringWidth(str->String())) <= 0))
		{
			float xStart = 0;
			switch(fAlignment)
			{
				case E_ALIGN_RIGHT:
					xStart = bounds.right - strWidth;
					break;

				case E_ALIGN_CENTER:
					xStart = bounds.Center().x - strWidth / 2.f;
					break;

				default:
					break;
			}
			DrawString(str->String(), EPoint(xStart, yStart + fontHeight.ascent + 1));
		}
		yStart += lineHeight;
	}
	PopState();
}


void
EStringView::SetFont(const EFont *font, euint8 mask)
{
	EFont fontPrev;
	EFont fontCurr;
	GetFont(&fontPrev);
	EView::SetFont(font, mask);
	GetFont(&fontCurr);

	if(fontPrev != fontCurr) Invalidate();
}


void
EStringView::GetPreferredSize(float *width, float *height)
{
	if(!width && !height) return;

	EFont font;
	GetFont(&font);

	if(width)
	{
		*width = 0;
		if(fTextArray != NULL) for(eint32 i = 0; i < fTextArray->CountItems(); i++)
		{
			const EString *str = fTextArray->ItemAt(i);
			if(str) *width = max_c(*width, (float)ceil((double)font.StringWidth(str->String())));
		}
	}

	if(height)
	{
		e_font_height fontHeight;
		font.GetHeight(&fontHeight);
		float sHeight = fontHeight.ascent + fontHeight.descent;

		if(fTextArray == NULL || fTextArray->CountItems() <= 0)
			*height = 0;
		else
			*height = (float)(fTextArray->CountItems() - 1) * (float)ceil((double)(sHeight * ETK_STRING_VIEW_LINE_SPACING)) +
				  (float)(fTextArray->CountItems()) * sHeight;
	}
}

