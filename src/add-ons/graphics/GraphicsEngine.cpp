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
 * File: GraphicsEngine.cpp
 *
 * --------------------------------------------------------------------------*/

#include "GraphicsEngine.h"
#include "./../../support/Errors.h"
#include "./../../interface/Window.h"
#include "./../../interface/View.h"

BGraphicsContext::BGraphicsContext()
{
	fDrawingMode = B_OP_COPY;
	fHighColor.set_to(0, 0, 0, 255);
	fLowColor.set_to(255, 255, 255, 255);
	fPattern = B_SOLID_HIGH;
	fPenSize = 0;
	fSquarePoint = false;
}


BGraphicsContext::~BGraphicsContext()
{
}


b_status_t
BGraphicsContext::SetDrawingMode(b_drawing_mode mode)
{
	fDrawingMode = mode;
	return B_OK;
}


b_status_t
BGraphicsContext::SetClipping(const BRegion &clipping)
{
	fClipping = clipping;
	return B_OK;
}


b_status_t
BGraphicsContext::SetHighColor(b_rgb_color highColor)
{
	fHighColor.set_to(highColor);
	return B_OK;
}


b_status_t
BGraphicsContext::SetHighColor(b_uint8 r, b_uint8 g, b_uint8 b, b_uint8 a)
{
	b_rgb_color color;
	color.set_to(r, g, b, a);
	return SetHighColor(color);
}


b_status_t
BGraphicsContext::SetLowColor(b_rgb_color lowColor)
{
	fLowColor.set_to(lowColor);
	return B_OK;
}


b_status_t
BGraphicsContext::SetLowColor(b_uint8 r, b_uint8 g, b_uint8 b, b_uint8 a)
{
	b_rgb_color color;
	color.set_to(r, g, b, a);
	return SetLowColor(color);
}


b_status_t
BGraphicsContext::SetPattern(b_pattern pattern)
{
	fPattern = pattern;
	return B_OK;
}


b_status_t
BGraphicsContext::SetPenSize(b_uint32 penSize)
{
	fPenSize = penSize;
	return B_OK;
}


b_status_t
BGraphicsContext::SetSquarePointStyle(bool state)
{
	fSquarePoint = state;
	return B_OK;
}


b_drawing_mode
BGraphicsContext::DrawingMode() const
{
	return fDrawingMode;
}


const BRegion*
BGraphicsContext::Clipping() const
{
	return &fClipping;
}


b_rgb_color
BGraphicsContext::HighColor() const
{
	return fHighColor;
}


b_rgb_color
BGraphicsContext::LowColor() const
{
	return fLowColor;
}


b_pattern
BGraphicsContext::Pattern() const
{
	return fPattern;
}


b_uint32
BGraphicsContext::PenSize() const
{
	return fPenSize;
}


bool
BGraphicsContext::IsSquarePointStyle() const
{
	return fSquarePoint;
}


BGraphicsDrawable::BGraphicsDrawable()
{
	fBkColor.set_to(255, 255, 255, 255);
}


BGraphicsDrawable::~BGraphicsDrawable()
{
}


b_status_t
BGraphicsDrawable::SetBackgroundColor(b_rgb_color bkColor)
{
	fBkColor.set_to(bkColor);
	return B_OK;
}


b_status_t
BGraphicsDrawable::SetBackgroundColor(b_uint8 r, b_uint8 g, b_uint8 b, b_uint8 a)
{
	b_rgb_color color;
	color.set_to(r, g, b, a);
	return SetBackgroundColor(color);
}


b_rgb_color
BGraphicsDrawable::BackgroundColor() const
{
	return fBkColor;
}


BGraphicsWindow::BGraphicsWindow()
	: BGraphicsDrawable()
{
}


BGraphicsWindow::~BGraphicsWindow()
{
}


BGraphicsEngine::BGraphicsEngine()
{
}


BGraphicsEngine::~BGraphicsEngine()
{
}


BGraphicsWindow*
BGraphicsEngine::GetWindow(BWindow *win)
{
	return(win == NULL ? NULL : win->fWindow);
}


BGraphicsDrawable*
BGraphicsEngine::GetPixmap(BWindow *win)
{
	return(win == NULL ? NULL : win->fPixmap);
}


BGraphicsContext*
BGraphicsEngine::GetContext(BView *view)
{
	return(view == NULL ? NULL : view->fDC);
}

