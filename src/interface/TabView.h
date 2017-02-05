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
 * File: TabView.h
 * 
 * --------------------------------------------------------------------------*/

#ifndef __ETK_TAB_VIEW_H__
#define __ETK_TAB_VIEW_H__

#include "./../support/List.h>
#include "./../interface/View.h>

typedef enum {
	E_TAB_FIRST = 0,
	E_TAB_FRONT,
	E_TAB_ANY
} e_tab_position;

#ifdef __cplusplus /* Just for C++ */

class ETabView;


class _IMPEXP_ETK ETab : public EArchivable {
public:
	ETab(EView *targetView = NULL);
	virtual ~ETab();

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
	virtual bool		SetView(EView *targetView, EView **oldTargetView = NULL);
	EView			*View() const;

	ETabView		*TabView() const;

	virtual void		DrawFocusMark(EView* owner, ERect frame);
	virtual void		DrawLabel(EView* owner, ERect frame);
	virtual void		DrawTab(EView* owner, ERect frame, e_tab_position position, bool full = true);

private:
	friend class ETabView;

	char *fLabel;
	bool fEnabled;
	bool fFocus;

	EView *fView;
	ETabView *fOwner;
};


class _IMPEXP_ETK ETabView : public EView {
public:
	ETabView(ERect frame, const char *name,
		 e_button_width tabWidth = E_WIDTH_AS_USUAL,
		 euint32 resizeMode = E_FOLLOW_ALL,
		 euint32 flags = E_WILL_DRAW | E_NAVIGABLE_JUMP | E_FRAME_EVENTS | E_NAVIGABLE);
	virtual ~ETabView();

	virtual void		Select(eint32 tabIndex);
	eint32			Selection() const;

	virtual bool		AddTab(EView *tabTargetView, ETab *tab = NULL);
	virtual ETab		*RemoveTab(eint32 tabIndex);

	eint32			CountTabs() const;
	ETab			*TabAt(eint32 tabIndex) const;
	eint32			TabIndexOf(const ETab *tab) const;
	EView			*ViewForTab(eint32 tabIndex) const;
	EView			*ContainerView() const;

	virtual ERect		TabFrame(eint32 tabIndex) const;
	virtual ERect		DrawTabs();
	virtual void		DrawBox(ERect selTabRect);

	virtual void		SetTabWidth(e_button_width tabWidth);
	e_button_width		TabWidth() const;

	virtual void		SetTabHeight(float tabHeight);
	float			TabHeight() const;

	virtual void		Draw(ERect updateRect);
	virtual void		MouseDown(EPoint where);

protected:
	virtual void		ChildRemoving(EView *child);

private:
	friend class ETab;

	EList fTabs;
	e_button_width fTabWidth;
	float fTabHeight;
	eint32 fSelection;

	EView *fContainer;
};

#endif /* __cplusplus */

#endif /* __ETK_TAB_VIEW_H__ */

