/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
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
 * File: ViewPrivate.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_VIEW_PRIVATE_H__
#define __ETK_VIEW_PRIVATE_H__

#include "layout/Layout.h"
#include "View.h"


class _LOCAL EViewLayout : public ELayoutItem {
public:
	EViewLayout(EView *view, ERect frame, euint32 resizingMode);
	virtual ~EViewLayout();

	void		SetEnabled(bool state);
	bool		IsEnabled() const;

	virtual void	GetPreferredSize(float *width, float *height);
	virtual void	ResizeToPreferred();
	virtual void	MoveTo(EPoint where);
	virtual void	ResizeTo(float width, float height);

	virtual void	Invalidate(ERect rect);
	virtual void	UpdateVisibleRegion();

	void		_GetVisibleRegion(ERegion **region);

private:
	bool fEnabled;
};


inline
EViewLayout::EViewLayout(EView *view, ERect frame, euint32 resizingMode)
	: ELayoutItem(frame, resizingMode), fEnabled(true)
{
	SetPrivateData(view);
}


inline
EViewLayout::~EViewLayout()
{
}


inline void
EViewLayout::SetEnabled(bool state)
{
	fEnabled = state;
}


inline bool
EViewLayout::IsEnabled() const
{
	return fEnabled;
}


inline void
EViewLayout::GetPreferredSize(float *width, float *height)
{
	((EView*)PrivateData())->GetPreferredSize(width, height);
}


inline void
EViewLayout::ResizeToPreferred()
{
	((EView*)PrivateData())->ResizeToPreferred();
}


inline void
EViewLayout::MoveTo(EPoint where)
{
	ERect oldFrame = Frame();
	ELayoutItem::MoveTo(where);
	ERect newFrame = Frame();
	((EView*)PrivateData())->_FrameChanged(oldFrame, newFrame);
}


inline void
EViewLayout::ResizeTo(float width, float height)
{
	ERect oldFrame = Frame();
	ELayoutItem::ResizeTo(width, height);
	ERect newFrame = Frame();
	((EView*)PrivateData())->_FrameChanged(oldFrame, newFrame);
}


inline void
EViewLayout::Invalidate(ERect rect)
{
	if(((EView*)PrivateData())->Window() == NULL) return;
	((EView*)PrivateData())->ConvertToWindow(&rect);
	((EView*)PrivateData())->Window()->Invalidate(rect);
}


inline void
EViewLayout::UpdateVisibleRegion()
{
	ELayoutItem::UpdateVisibleRegion();
	((EView*)PrivateData())->_UpdateVisibleRegion();
}


inline void
EViewLayout::_GetVisibleRegion(ERegion **region)
{
	GetVisibleRegion(region);
}

#endif /* __ETK_VIEW_PRIVATE_H__ */

