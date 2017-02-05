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
 * File: GraphicsEngine.cpp
 *
 * --------------------------------------------------------------------------*/

#include "GraphicsEngine.h"

EGraphicsContext::EGraphicsContext()
{
	fDrawingMode = E_OP_COPY;
	fHighColor.set_to(0, 0, 0, 255);
	fLowColor.set_to(255, 255, 255, 255);
	fPattern = E_SOLID_HIGH;
	fPenSize = 0;
	fSquarePoint = false;
}


EGraphicsContext::~EGraphicsContext()
{
}


e_status_t
EGraphicsContext::SetDrawingMode(e_drawing_mode mode)
{
	fDrawingMode = mode;
	return E_OK;
}


e_status_t
EGraphicsContext::SetClipping(const ERegion &clipping)
{
	fClipping = clipping;
	return E_OK;
}


e_status_t
EGraphicsContext::SetHighColor(e_rgb_color highColor)
{
	fHighColor.set_to(highColor);
	return E_OK;
}


e_status_t
EGraphicsContext::SetHighColor(euint8 r, euint8 g, euint8 b, euint8 a)
{
	e_rgb_color color;
	color.set_to(r, g, b, a);
	return SetHighColor(color);
}


e_status_t
EGraphicsContext::SetLowColor(e_rgb_color lowColor)
{
	fLowColor.set_to(lowColor);
	return E_OK;
}


e_status_t
EGraphicsContext::SetLowColor(euint8 r, euint8 g, euint8 b, euint8 a)
{
	e_rgb_color color;
	color.set_to(r, g, b, a);
	return SetLowColor(color);
}


e_status_t
EGraphicsContext::SetPattern(e_pattern pattern)
{
	fPattern = pattern;
	return E_OK;
}


e_status_t
EGraphicsContext::SetPenSize(euint32 penSize)
{
	fPenSize = penSize;
	return E_OK;
}


e_status_t
EGraphicsContext::SetSquarePointStyle(bool state)
{
	fSquarePoint = state;
	return E_OK;
}


e_drawing_mode
EGraphicsContext::DrawingMode() const
{
	return fDrawingMode;
}


const ERegion*
EGraphicsContext::Clipping() const
{
	return &fClipping;
}


e_rgb_color
EGraphicsContext::HighColor() const
{
	return fHighColor;
}


e_rgb_color
EGraphicsContext::LowColor() const
{
	return fLowColor;
}


e_pattern
EGraphicsContext::Pattern() const
{
	return fPattern;
}


euint32
EGraphicsContext::PenSize() const
{
	return fPenSize;
}


bool
EGraphicsContext::IsSquarePointStyle() const
{
	return fSquarePoint;
}


EGraphicsDrawable::EGraphicsDrawable()
{
	fBkColor.set_to(255, 255, 255, 255);
}


EGraphicsDrawable::~EGraphicsDrawable()
{
}


e_status_t
EGraphicsDrawable::SetBackgroundColor(e_rgb_color bkColor)
{
	fBkColor.set_to(bkColor);
	return E_OK;
}


e_status_t
EGraphicsDrawable::SetBackgroundColor(euint8 r, euint8 g, euint8 b, euint8 a)
{
	e_rgb_color color;
	color.set_to(r, g, b, a);
	return SetBackgroundColor(color);
}


e_rgb_color
EGraphicsDrawable::BackgroundColor() const
{
	return fBkColor;
}


EGraphicsWindow::EGraphicsWindow()
	: EGraphicsDrawable()
{
}


EGraphicsWindow::~EGraphicsWindow()
{
}


EGraphicsEngine::EGraphicsEngine()
{
}


EGraphicsEngine::~EGraphicsEngine()
{
}


EGraphicsWindow*
EGraphicsEngine::GetWindow(EWindow *win)
{
	return(win == NULL ? NULL : win->fWindow);
}


EGraphicsDrawable*
EGraphicsEngine::GetPixmap(EWindow *win)
{
	return(win == NULL ? NULL : win->fPixmap);
}


EGraphicsContext*
EGraphicsEngine::GetContext(EView *view)
{
	return(view == NULL ? NULL : view->fDC);
}

