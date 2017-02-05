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
 * File: ScrollView.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_SCROLL_VIEW_H__
#define __ETK_SCROLL_VIEW_H__

#include "./../interface/ScrollBar.h>

#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_ETK EScrollView : public EView {
public:
	EScrollView(ERect frame, const char *name, EView *target,
		    euint32 resizingMode = E_FOLLOW_LEFT | E_FOLLOW_TOP,
		    euint32 flags = 0,
		    bool alwaysShowHorizontal = false,
		    bool alwaysShowVertical = false,
		    e_border_style border = E_FANCY_BORDER);
	virtual ~EScrollView();

	// SetTarget: When it return E_OK, the oldTarget was removed and destroy automatically.
	//            If you want to keep the oldTarget, try oldTarget->RemoveSelf() before.
	e_status_t	SetTarget(EView *newTarget);
	EView		*Target() const;
	ERect		TargetFrame() const;

	virtual void	SetBorder(e_border_style border);
	e_border_style	Border() const;

	void		SetScrollBarAutoState(bool alwaysShowHorizontal, bool alwaysShowVertical);
	void		GetScrollBarAutoState(bool *alwaysShowHorizontal, bool *alwaysShowVertical) const;

	EScrollBar	*ScrollBar(e_orientation direction) const;

	virtual void	SetFlags(euint32 flags); // auto-setting E_WILL_DRAW and E_FRAME_EVENTS
	virtual void	Draw(ERect updateRect);
	virtual void	FrameResized(float new_width, float new_height);

protected:
	virtual void	ChildRemoving(EView *child);

private:
	friend class EView;

	e_border_style fBorder;
	bool fAlwaysShowHorizontal;
	bool fAlwaysShowVertical;

	EScrollBar *fHSB;
	EScrollBar *fVSB;
	EView *fTarget;

	ERect TargetValidFrame(bool ignore_scrollbar = false) const;
};


#endif /* __cplusplus */

#endif /* __ETK_SCROLL_VIEW_H__ */

