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
 * File: TabView.h
 * 
 * --------------------------------------------------------------------------*/

#ifndef __BHAPI_TAB_VIEW_H__
#define __BHAPI_TAB_VIEW_H__

#include "./../support/List.h"
#include "./../interface/View.h"

typedef enum {
    B_TAB_FIRST = 0,
    B_TAB_FRONT,
    B_TAB_ANY
} b_tab_position;

#ifdef __cplusplus /* Just for C++ */

class BTabView;


class _IMPEXP_BHAPI BTab : public BArchivable {
public:
    BTab(BView *targetView = NULL);
    virtual ~BTab();

	virtual void		SetLabel(const char *label);
	const char		*Label() const;

	virtual void		Select();
	virtual void		Deselect();
	bool			IsSelected() const;

	virtual void		SetEnabled(bool state);
	bool			IsEnabled() const;

	virtual void		MakeFocus(bool state = true);
	bool			IsFocus() const;

	// SetView: the old targetView will be destructed automatically if you assigned "oldTargetView" to be NULL.
	virtual bool		SetView(BView *targetView, BView **oldTargetView = NULL);
	BView			*View() const;

    BTabView		*TabView() const;

	virtual void		DrawFocusMark(BView* owner, BRect frame);
	virtual void		DrawLabel(BView* owner, BRect frame);
	virtual void		DrawTab(BView* owner, BRect frame, b_tab_position position, bool full = true);

private:
    friend class BTabView;

	char *fLabel;
	bool fEnabled;
	bool fFocus;

	BView *fView;
    BTabView *fOwner;
};


class _IMPEXP_BHAPI BTabView : public BView {
public:
    BTabView(BRect frame, const char *name,
		 b_button_width tabWidth = B_WIDTH_AS_USUAL,
         b_uint32 resizeMode = B_FOLLOW_ALL,
         b_uint32 flags = B_WILL_DRAW | B_NAVIGABLE_JUMP | B_FRAME_EVENTS | B_NAVIGABLE);
    virtual ~BTabView();

    virtual void		Select(b_int32 tabIndex);
    b_int32			Selection() const;

    virtual bool		AddTab(BView *tabTargetView, BTab *tab = NULL);
    virtual BTab		*RemoveTab(b_int32 tabIndex);

    b_int32			CountTabs() const;
    BTab			*TabAt(b_int32 tabIndex) const;
    b_int32			TabIndexOf(const BTab *tab) const;
    BView			*ViewForTab(b_int32 tabIndex) const;
	BView			*ContainerView() const;

    virtual BRect		TabFrame(b_int32 tabIndex) const;
	virtual BRect		DrawTabs();
	virtual void		DrawBox(BRect selTabRect);

    virtual void		SetTabWidth(b_button_width tabWidth);
	b_button_width		TabWidth() const;

	virtual void		SetTabHeight(float tabHeight);
	float			TabHeight() const;

	virtual void		Draw(BRect updateRect);
	virtual void		MouseDown(BPoint where);

protected:
	virtual void		ChildRemoving(BView *child);

private:
    friend class BTab;

	BList fTabs;
	b_button_width fTabWidth;
	float fTabHeight;
    b_int32 fSelection;

	BView *fContainer;
};

#endif /* __cplusplus */

#endif /* __BHAPI_TAB_VIEW_H__ */

