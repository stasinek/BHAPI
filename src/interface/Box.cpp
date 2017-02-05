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
 * File: Box.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../support/ClassInfo.h>
#include "./../interface/StringView.h>
#include "./../add-ons/theme/ThemeEngine.h>

#include "Box.h"

EBox::EBox(ERect frame, const char *name, euint32 resizingMode, euint32 flags, e_border_style border)
	: EView(frame, name, resizingMode, flags), fLabelView(NULL), fBorder(E_NO_BORDER), fAlignment(E_ALIGN_LEFT)
{
	fBorder = border;
}


EBox::~EBox()
{
}


void
EBox::SetBorder(e_border_style border)
{
	if(fBorder != border)
	{
		fBorder = border;
		Invalidate();
	}
}


e_border_style
EBox::Border() const
{
	return fBorder;
}


void
EBox::SetLabelAlignment(e_alignment labelAlignment)
{
	if(fAlignment != labelAlignment)
	{
		fAlignment = labelAlignment;
		ReAdjustLabel();
	}
}


e_alignment
EBox::LabelAlignment() const
{
	return fAlignment;
}


void
EBox::SetLabel(const char *label)
{
	if(!(label == NULL || *label == 0))
	{
		EStringView *strView = e_cast_as(fLabelView, EStringView);
		if(strView != NULL)
		{
			strView->SetText(label);
			strView->ResizeToPreferred();
			ReAdjustLabel();
			return;
		}

		if((strView = new EStringView(ERect(0, 0, 1, 1), NULL, label, E_FOLLOW_NONE)) == NULL) return;
		strView->SetFont(etk_bold_font);
		strView->ResizeToPreferred();
		if(SetLabel(strView) != E_OK) delete strView;
	}
	else if(fLabelView != NULL)
	{
		EView *view = fLabelView;
		fLabelView = NULL;

		view->RemoveSelf();
		delete view;
	}
}


e_status_t
EBox::SetLabel(EView *viewLabel)
{
	if(viewLabel != NULL)
	{
		if(viewLabel == this || viewLabel->Window() != NULL || viewLabel->Parent() != NULL) return E_ERROR;
		AddChild(viewLabel, ChildAt(0));
		if(viewLabel->Parent() != this) return E_ERROR;
		viewLabel->SetResizingMode(E_FOLLOW_NONE);
	}
	else if(fLabelView == NULL)
	{
		return E_OK;
	}

	if(fLabelView != NULL)
	{
		EView *view = fLabelView;
		fLabelView = NULL;

		view->RemoveSelf();
		delete view;
	}

	fLabelView = viewLabel;
	ReAdjustLabel();

	return E_OK;
}


const char*
EBox::Label() const
{
	if(fLabelView == NULL) return NULL;

	EStringView *strView = e_cast_as(fLabelView, EStringView);
	if(strView == NULL) return NULL;

	return strView->Text();
}


EView*
EBox::LabelView() const
{
	return fLabelView;
}


ERect
EBox::ContentBounds() const
{
	e_theme_engine *theme = etk_get_current_theme_engine();

	float l = 0, t = 0, r = 0, b = 0;
	if(!(theme == NULL || theme->get_border_margins == NULL))
		theme->get_border_margins(theme, this, &l, &t, &r, &b, fBorder, PenSize());

	float labelHeight = ((fLabelView == NULL || fLabelView->Frame().Width() <= 0) ? 0.f : fLabelView->Frame().Height());

	ERect bounds = Frame().OffsetToSelf(E_ORIGIN);
	bounds.left += l;
	bounds.top += max_c(t, labelHeight);
	bounds.right -= r;
	bounds.bottom -= b;

	return bounds;
}


void
EBox::Draw(ERect updateRect)
{
	if(!IsVisible() || fBorder == E_NO_BORDER) return;

	e_theme_engine *theme = etk_get_current_theme_engine();
	if(theme == NULL || theme->get_border_margins == NULL || theme->draw_border == NULL) return;

	float l = 0, t = 0, r = 0, b = 0;
	theme->get_border_margins(theme, this, &l, &t, &r, &b, fBorder, PenSize());

	ERect rect = Frame().OffsetToSelf(E_ORIGIN);
	if(!(fLabelView == NULL || fLabelView->Frame().Width() <= 0 || fLabelView->Frame().Height() < t))
		rect.top += (fLabelView->Frame().Height() - t) / 2.f;

	PushState();

	ERegion clipping(updateRect);
	if(!(fLabelView == NULL || fLabelView->Frame().IsValid() == false)) clipping.Exclude(fLabelView->Frame());
	ConstrainClippingRegion(&clipping);

	if(clipping.CountRects() > 0) theme->draw_border(theme, this, rect, fBorder, PenSize());

	PopState();
}


void
EBox::FrameResized(float new_width, float new_height)
{
	ReAdjustLabel();
}


void
EBox::ResizeToPreferred()
{
	if(fLabelView) fLabelView->ResizeToPreferred();
	EView::ResizeToPreferred();
}


void
EBox::GetPreferredSize(float *width, float *height)
{
	if(!width && !height) return;

	float w = 0, h = 0;
	if(fLabelView) fLabelView->GetPreferredSize(&w, &h);

	e_theme_engine *theme = etk_get_current_theme_engine();

	float l = 0, t = 0, r = 0, b = 0;
	if(!(theme == NULL || theme->get_border_margins == NULL))
		theme->get_border_margins(theme, this, &l, &t, &r, &b, fBorder, PenSize());

	w += (l + r) + 2.f;
	if(h < t) h = t;
	h += b + 2.f;

	if(width) *width = w;
	if(height) *height = h;
}


void
EBox::ReAdjustLabel()
{
	if(fLabelView == NULL) return;

	switch(fAlignment)
	{
		case E_ALIGN_RIGHT:
			fLabelView->MoveTo(Frame().Width() - fLabelView->Frame().Width() - 5.f, 0);
			break;
		case E_ALIGN_CENTER:
			fLabelView->MoveTo((Frame().Width() - fLabelView->Frame().Width()) / 2.f, 0);
			break;
		default:
			fLabelView->MoveTo(5, 0);
	}
}


void
EBox::ChildRemoving(EView *child)
{
	if(fLabelView == child) fLabelView = NULL;
}

