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
 * File: TextControl.cpp
 * Description: ETextControl --- display a labeled field could deliver a message as modifying
 * 
 * --------------------------------------------------------------------------*/

#include "./../app/Application.h"
#include "./../app/Clipboard.h"

#include "Window.h"
#include "TextControl.h"


ETextControl::ETextControl(ERect frame, const char *name,
			   const char *label, const char *text, EMessage *message,
			   euint32 resizeMode, euint32 flags)
	: ETextEditable(frame, name, text, message, resizeMode, flags),
	  fLabelAlignment(E_ALIGN_LEFT), fDivider(-1), fModificationMessage(NULL)
{
	SetTextAlignment(E_ALIGN_LEFT);
	SetLabel(label);
}


ETextControl::~ETextControl()
{
	if(fModificationMessage) delete fModificationMessage;
}


void
ETextControl::SetDivider(float divider)
{
	if(fDivider < 0 && divider < 0) return;

	if(fDivider != divider)
	{
		fDivider = divider;

		EFont font;
		GetFont(&font);
		e_font_height fontHeight;
		font.GetHeight(&fontHeight);
		float sHeight = fontHeight.ascent + fontHeight.descent;

		ERect margins(1, 1, 1, 1);
		margins.left += (fDivider >= 0 ? fDivider : max_c(font.StringWidth(Label()), 0)) + UnitsPerPixel();
		if(sHeight + 2.f * UnitsPerPixel() > Frame().Height())
			margins.top = margins.bottom = (sHeight - Frame().Height()) / 2.f + UnitsPerPixel();
		SetMargins(margins.left, margins.top, margins.right, margins.bottom);
	}
}


float
ETextControl::Divider() const
{
	return fDivider;
}


void
ETextControl::SetModificationMessage(EMessage *msg)
{
	if(msg == fModificationMessage) return;
	if(fModificationMessage) delete fModificationMessage;
	fModificationMessage = msg;
}


EMessage*
ETextControl::ModificationMessage() const
{
	return fModificationMessage;
}


void
ETextControl::SetText(const char *text)
{
	ETextEditable::SetText(text);
	if(fModificationMessage != NULL) Invoke(fModificationMessage);
}


void
ETextControl::SetLabel(const char *label)
{
	ERect margins(1, 1, 1, 1);

	EControl::SetLabel(label);

	EFont font;
	GetFont(&font);
	e_font_height fontHeight;
	font.GetHeight(&fontHeight);
	float sHeight = fontHeight.ascent + fontHeight.descent;
	margins.left += (fDivider >= 0 ? fDivider : max_c(font.StringWidth(Label()), 0)) + UnitsPerPixel();
	if(sHeight + 2.f * UnitsPerPixel() > Frame().Height())
		margins.top = margins.bottom = (sHeight - Frame().Height()) / 2.f + UnitsPerPixel();

	SetMargins(margins.left, margins.top, margins.right, margins.bottom);
}


void
ETextControl::Draw(ERect updateRect)
{
	if(Label() != NULL && fDivider != 0 && !IsFocusChanging())
	{
		EFont font;
		GetFont(&font);
		e_font_height fontHeight;
		font.GetHeight(&fontHeight);
		float sHeight = fontHeight.ascent + fontHeight.descent;

		ERect rect = Frame().OffsetToSelf(E_ORIGIN);
		rect.right = (fDivider >= 0 ? fDivider : max_c(font.StringWidth(Label()), 0));

		EPoint penLocation;

		if(fLabelAlignment == E_ALIGN_LEFT || fDivider < 0)
			penLocation.x = 0;
		else if(fLabelAlignment == E_ALIGN_RIGHT)
			penLocation.x = rect.right - max_c(font.StringWidth(Label()), 0);
		else
			penLocation.x = (rect.right - max_c(font.StringWidth(Label()), 0)) / 2;

		penLocation.y = rect.Center().y - sHeight / 2.f;
		penLocation.y += fontHeight.ascent + 1;

		PushState();
		ConstrainClippingRegion(rect);
		SetHighColor(IsEnabled() ? e_ui_color(E_PANEL_TEXT_COLOR) : e_ui_color(E_SHINE_COLOR).disable(ViewColor()));
		SetLowColor(ViewColor());
		DrawString(Label(), penLocation);
		if(!IsEnabled())
		{
			SetHighColor(e_ui_color(E_SHADOW_COLOR).disable(ViewColor()));
			DrawString(Label(), penLocation - EPoint(1, 1));
		}
		PopState();
	}

	ETextEditable::Draw(updateRect);

	if(IsFocusChanging()) return;

	e_rgb_color shineColor = e_ui_color(E_SHINE_COLOR);
	e_rgb_color shadowColor = e_ui_color(E_SHADOW_COLOR);

	if(!IsEnabled())
	{
		shineColor.disable(ViewColor());
		shadowColor.disable(ViewColor());
	}

	float l, t, r, b;
	GetMargins(&l, &t, &r, &b);
	ERect rect = Frame().OffsetToSelf(E_ORIGIN);
	rect.left += l;
	rect.top += t;
	rect.right -= r;
	rect.bottom -= b;
	rect.InsetBy(-1, -1);

	SetHighColor(shineColor);
	StrokeRect(rect);
	SetHighColor(shadowColor);
	StrokeLine(rect.LeftBottom(), rect.LeftTop());
	StrokeLine(rect.RightTop());
}


void
ETextControl::GetPreferredSize(float *width, float *height)
{
	if(width) *width = 0;
	if(height) *height = 0;

	ETextEditable::GetPreferredSize(width, height);

	if(width != NULL)
	{
		EFont font;
		GetFont(&font);
		*width += (fDivider >= 0 ? fDivider : max_c(font.StringWidth(Label()), 0)) + 20;
	}

	if(height != NULL)
	{
		e_font_height fontHeight;
		GetFontHeight(&fontHeight);
		float sHeight = fontHeight.ascent + fontHeight.descent;
		if(sHeight + 10 > *height) *height = sHeight + 10;
	}
}


void
ETextControl::SetAlignment(e_alignment forLabel, e_alignment forText)
{
	if(forLabel == fLabelAlignment && TextAlignment() == forText) return;

	fLabelAlignment = forLabel;
	SetTextAlignment(forText);

	Invalidate();
}


void
ETextControl::GetAlignment(e_alignment *forLabel, e_alignment *forText) const
{
	if(forLabel) *forLabel = fLabelAlignment;
	if(forText) *forText = TextAlignment();
}


void
ETextControl::KeyDown(const char *bytes, eint32 numBytes)
{
	EMessage *msg = (Window() == NULL ? NULL : Window()->CurrentMessage());

	eint32 modifiers = 0;
	if(msg != NULL) msg->FindInt32("modifiers", &modifiers);

	if(!(numBytes != 1 || msg == NULL || msg->what != E_KEY_DOWN || !IsEnabled() || !IsEditable() || !(modifiers & E_CONTROL_KEY)))
	{
		if(*bytes == 'c' || *bytes == 'C' || *bytes == 'x' || *bytes == 'X')
		{
			eint32 startPos, endPos;
			char *selText = NULL;
			EMessage *clipMsg = NULL;

			if(GetSelection(&startPos, &endPos) == false || etk_clipboard.Lock() == false) return;
			if((selText = DuplicateText(startPos, endPos)) != NULL && (clipMsg = etk_clipboard.Data()) != NULL)
			{
				const char *text = NULL;
				ssize_t textLen = 0;
				if(clipMsg->FindData("text/plain", E_MIME_TYPE, (const void**)&text, &textLen) == false ||
				   text == NULL || textLen != (ssize_t)strlen(selText) || strncmp(text, selText, (size_t)textLen) != 0)
				{
					etk_clipboard.Clear();
					clipMsg->AddData("text/plain", E_MIME_TYPE, selText, strlen(selText));
					etk_clipboard.Commit();
				}
			}
			if(selText) free(selText);
			etk_clipboard.Unlock();

			if(*bytes == 'x' || *bytes == 'X')
			{
				RemoveText(startPos, endPos);
				SetPosition(startPos);
			}

			return;
		}
		else if(*bytes == 'v' || *bytes == 'V')
		{
			EString str;
			EMessage *clipMsg = NULL;

			if(etk_clipboard.Lock() == false) return;
			if((clipMsg = etk_clipboard.Data()) != NULL)
			{
				const char *text = NULL;
				ssize_t len = 0;
				if(clipMsg->FindData("text/plain", E_MIME_TYPE, (const void**)&text, &len)) str.SetTo(text, (eint32)len);
			}
			etk_clipboard.Unlock();

			if(str.Length() <= 0) return;

			eint32 curPos = Position();
			eint32 startPos, endPos;
			if(GetSelection(&startPos, &endPos)) {RemoveText(startPos, endPos); curPos = startPos;}
			InsertText(str.String(), -1, curPos);
			SetPosition(curPos + str.CountChars());

			return;
		}
	}

	ETextEditable::KeyDown(bytes, numBytes);
}

