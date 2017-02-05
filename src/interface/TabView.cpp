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
 * File: TabView.cpp
 * 
 * --------------------------------------------------------------------------*/

#include "./../support/String.h>
#include "TabView.h"


ETab::ETab(EView *targetView)
	: fLabel(NULL), fEnabled(true), fFocus(false), fOwner(NULL)
{
	fView = targetView;
}


ETab::~ETab()
{
	if(fOwner != NULL) fOwner->ETabView::RemoveTab(fOwner->TabIndexOf(this));

	if(fLabel) delete[] fLabel;
	if(fView != NULL)
	{
		fView->RemoveSelf();
		delete fView;
	}
}


void
ETab::SetLabel(const char *label)
{
	if(fLabel) delete[] fLabel;
	fLabel = (label == NULL ? NULL : EStrdup(label));
}


const char*
ETab::Label() const
{
	return fLabel;
}


void
ETab::Select()
{
	if(fOwner == NULL) return;

	eint32 index = fOwner->TabIndexOf(this);
	if(index == fOwner->fSelection) return;

	ETab *oldTab = fOwner->TabAt(fOwner->fSelection);
	if(oldTab) oldTab->Deselect();

	fOwner->fSelection = index;

	if(fView) fView->Show();
}


void
ETab::Deselect()
{
	if(IsSelected() == false) return;
	fOwner->fSelection = -1;
	if(fView) fView->Hide();
}


bool
ETab::IsSelected() const
{
	if(fOwner == NULL) return false;
	return(fOwner->fSelection == fOwner->TabIndexOf(this));
}


void
ETab::SetEnabled(bool state)
{
	fEnabled = state;
}


bool
ETab::IsEnabled() const
{
	return fEnabled;
}


void
ETab::MakeFocus(bool state)
{
	fFocus = state;
}


bool
ETab::IsFocus() const
{
	return fFocus;
}


bool
ETab::SetView(EView *targetView, EView **oldTargetView)
{
	if(targetView == NULL ? false : (targetView->Parent() != NULL || targetView->Window() != NULL)) return false;

	if(fView) fView->RemoveSelf();

	if(oldTargetView) *oldTargetView = fView;
	else if(fView) delete fView;

	fView = targetView;

	if(fView == NULL || fOwner == NULL || fOwner->fContainer == NULL) return true;

	if(IsSelected()) fView->Show();
	else fView->Hide();
	fOwner->fContainer->AddChild(fView);

	return true;
}


EView*
ETab::View() const
{
	return fView;
}


ETabView*
ETab::TabView() const
{
	return fOwner;
}


void
ETab::DrawFocusMark(EView* owner, ERect frame)
{
	if(!fFocus || !fEnabled) return;

	// TODO
}


void
ETab::DrawLabel(EView* owner, ERect frame)
{
	if(fLabel == NULL) return;

	e_rgb_color textColor = e_ui_color(E_PANEL_TEXT_COLOR);
	if(!fEnabled) textColor.disable(owner->ViewColor());

	EFont font;
	e_font_height fontHeight;
	owner->GetFont(&font);
	font.GetHeight(&fontHeight);

	owner->SetHighColor(textColor);
	EPoint penLocation = frame.LeftTop();
	penLocation.x += (frame.Width() - font.StringWidth(fLabel)) / 2.f;
	penLocation.y += fontHeight.ascent + 1.f;

	owner->DrawString(fLabel, penLocation);
}


void
ETab::DrawTab(EView* owner, ERect frame, e_tab_position position, bool full)
{
	e_rgb_color shineColor = e_ui_color(E_SHINE_COLOR);
	e_rgb_color shadowColor = e_ui_color(E_SHADOW_COLOR);

	owner->PushState();

	e_rgb_color bgColor = owner->ViewColor();
	if(position == E_TAB_FRONT) bgColor.set_to(235, 220, 30);

	if(!fEnabled)
	{
		shineColor.disable(bgColor);
		shadowColor.disable(bgColor);
	}

	owner->SetPenSize(0);
	owner->SetDrawingMode(E_OP_COPY);

	if(position == E_TAB_FRONT) {owner->SetHighColor(bgColor); owner->FillRect(frame);}

	owner->SetHighColor(shineColor);
	owner->StrokeLine(frame.LeftBottom(), frame.LeftTop());
	owner->StrokeLine(frame.RightTop());
	owner->SetHighColor(shadowColor);
	owner->StrokeLine(frame.RightBottom());

	owner->ConstrainClippingRegion(frame.InsetByCopy(1, 1));

	DrawLabel(owner, frame);
	DrawFocusMark(owner, frame);

	owner->PopState();
}


ETabView::ETabView(ERect frame, const char *name,
		   e_button_width tabWidth, euint32 resizeMode, euint32 flags)
	: EView(frame, name, resizeMode, flags), fSelection(-1)
{
	fTabWidth = tabWidth;

	e_font_height fontHeight;
	GetFontHeight(&fontHeight);
	fTabHeight = fontHeight.ascent + fontHeight.descent + 2.f;

	ERect rect = frame;
	rect.OffsetTo(EPoint(0, 0));
	rect.top += fTabHeight;
	rect.InsetBy(2.f, 2.f);
	fContainer = new EView(rect, NULL, E_FOLLOW_NONE, 0);
	AddChild(fContainer);
}


ETabView::~ETabView()
{
	ETab *tab;
	while((tab = (ETab*)fTabs.RemoveItem((eint32)0)) != NULL)
	{
		tab->fOwner = NULL;
		delete tab;
	}
}


void
ETabView::Select(eint32 tabIndex)
{
	if((tabIndex >= 0 ? tabIndex == fSelection : fSelection < 0) || tabIndex >= fTabs.CountItems()) return;

	if(tabIndex >= 0)
	{
		ETab *tab = (ETab*)fTabs.ItemAt(tabIndex);
		tab->Select();
	}
	else if(fSelection >= 0)
	{
		ETab *tab = (ETab*)fTabs.ItemAt(fSelection);
		tab->Deselect();
	}
}


eint32
ETabView::Selection() const
{
	return fSelection;
}


bool
ETabView::AddTab(EView *tabTargetView, ETab *tab)
{
	if(tabTargetView == NULL && tab == NULL) return false;

	if(tabTargetView == NULL ? tab->fOwner != NULL :
				   (tabTargetView->Parent() != NULL || tabTargetView->Window() != NULL)) return false;

	EView *oldTargetView = NULL;
	ETab *newTab = NULL;

	if(tab == NULL)
	{
		newTab = (tab = new ETab(tabTargetView));
		tab->SetLabel(tabTargetView->Name());
	}
	else if(tabTargetView != NULL)
	{
		if(tab->SetView(tabTargetView, &oldTargetView) == false) return false;
	}

	if(fTabs.AddItem(tab) == false)
	{
		if(newTab != NULL) delete newTab;
		if(tabTargetView != NULL && tab != NULL) tab->SetView(oldTargetView, &oldTargetView);
		return false;
	}

	tab->fOwner = this;
	if(tab->fView != NULL && fContainer != NULL)
	{
		tab->fView->Hide();
		fContainer->AddChild(tab->fView);
		if(fSelection < 0) tab->Select();
	}

	return true;
}


ETab*
ETabView::RemoveTab(eint32 tabIndex)
{
	ETab *tab = (ETab*)fTabs.RemoveItem(tabIndex);
	if(tab == NULL) return NULL;

	tab->fOwner = NULL;
	if(tab->fView != NULL) tab->fView->RemoveSelf();
	if(tabIndex == fSelection) fSelection = -1;

	return tab;
}


eint32
ETabView::CountTabs() const
{
	return fTabs.CountItems();
}


ETab*
ETabView::TabAt(eint32 tabIndex) const
{
	return (ETab*)fTabs.ItemAt(tabIndex);
}


eint32
ETabView::TabIndexOf(const ETab *tab) const
{
	if(tab == NULL || tab->fOwner != this) return -1;
	return fTabs.IndexOf((void*)tab);
}


EView*
ETabView::ViewForTab(eint32 tabIndex) const
{
	ETab *tab = (ETab*)fTabs.ItemAt(tabIndex);
	return(tab == NULL ? NULL : tab->View());
}


EView*
ETabView::ContainerView() const
{
	return fContainer;
}


void
ETabView::SetTabWidth(e_button_width tabWidth)
{
	if(fTabWidth != tabWidth)
	{
		fTabWidth = tabWidth;

		ERect r = Frame().OffsetToSelf(E_ORIGIN);
		r.bottom = r.top + fTabHeight;
		Invalidate(r);
	}
}


e_button_width
ETabView::TabWidth() const
{
	return fTabWidth;
}


void
ETabView::SetTabHeight(float tabHeight)
{
	if(tabHeight > 0 && tabHeight != fTabHeight)
	{
		fTabHeight = tabHeight;

		if(fContainer != NULL)
		{
			ERect frame = Frame().OffsetToSelf(E_ORIGIN);
			frame.top += fTabHeight;
			frame.InsetBy(2, 2);

			fContainer->ResizeTo(frame.Width(), frame.Height());
			fContainer->MoveTo(frame.LeftTop());
		}
		Invalidate();
	}
}


float
ETabView::TabHeight() const
{
	return fTabHeight;
}


void
ETabView::ChildRemoving(EView *child)
{
	if(child == fContainer)
	{
		if(fSelection >= 0)
		{
			EView *tabView = ViewForTab(fSelection);
			if(tabView != NULL) tabView->RemoveSelf();
			fSelection = -1;
		}
		fContainer = NULL;
	}
}


ERect
ETabView::TabFrame(eint32 tabIndex) const
{
	if(tabIndex < 0 || tabIndex >= fTabs.CountItems()) return ERect();

	EFont font;
	GetFont(&font);

	ERect r = Frame().OffsetToSelf(E_ORIGIN);
	r.bottom = r.top + fTabHeight;
	r.right = r.left;

	for(eint32 i = 0; i < fTabs.CountItems(); i++)
	{
		ETab *tab = (ETab*)fTabs.ItemAt(i);
		if(fTabWidth == E_WIDTH_FROM_LABEL)
		{
			if(i > 0) r.left = r.right + 5.f;
			r.right = r.left + max_c(font.StringWidth(tab->Label()) + 2.f, 10.f);
			if(i == tabIndex) break;
		}
		else /* fTabWidth == E_WIDTH_AS_USUAL */
		{
			r.right = r.left + max_c(r.Width(), max_c(font.StringWidth(tab->Label()) + 2.f, 10.f));
		}
	}

	if(fTabWidth == E_WIDTH_AS_USUAL)
	{
		float maxWidth = r.Width();
		r.left += (maxWidth + 5.f) * (float)tabIndex;
		r.right = r.left + maxWidth;
	}

	return r;
}


ERect
ETabView::DrawTabs()
{
	ERect selTabRect;

	for(eint32 i = 0; i < fTabs.CountItems(); i++)
	{
		if(i == fSelection) continue;

		ETab *tab = (ETab*)fTabs.ItemAt(i);
		ERect tabRect = TabFrame(i);
		tab->DrawTab(this, tabRect, (i == 0 ? E_TAB_FIRST : E_TAB_ANY), true);
	}

	if(fSelection >= 0)
	{
		ETab *tab = (ETab*)fTabs.ItemAt(fSelection);
		selTabRect = TabFrame(fSelection);
		tab->DrawTab(this, selTabRect, E_TAB_FRONT, true);
	}

	return selTabRect;
}


void
ETabView::DrawBox(ERect selTabRect)
{
	ERect rect = Frame().OffsetToSelf(E_ORIGIN);
	rect.top += fTabHeight;

	e_rgb_color shineColor = e_ui_color(E_SHINE_COLOR);
	e_rgb_color shadowColor = e_ui_color(E_SHADOW_COLOR);

	if(!IsEnabled())
	{
		shineColor.disable(ViewColor());
		shadowColor.disable(ViewColor());
	}

	PushState();

	ERegion clipping(rect);
	clipping.Exclude(selTabRect.InsetByCopy(0, -1));
	ConstrainClippingRegion(&clipping);

	SetPenSize(0);
	SetDrawingMode(E_OP_COPY);
	SetHighColor(shineColor);
	StrokeRect(rect);
	SetHighColor(shadowColor);
	StrokeLine(rect.LeftBottom(), rect.RightBottom());
	StrokeLine(rect.RightTop());

	PopState();
}


void
ETabView::Draw(ERect updateRect)
{
	ERect selTabRect = DrawTabs();
	DrawBox(selTabRect);
}


void
ETabView::MouseDown(EPoint where)
{
	eint32 btnClicks = 1;
	if(where.y > fTabHeight + 1.f || !IsEnabled() ||
	   !QueryCurrentMouse(true, E_PRIMARY_MOUSE_BUTTON, true, &btnClicks) || btnClicks > 1) return;

	// TODO
	eint32 index = -1;
	for(eint32 i = 0; i < fTabs.CountItems(); i++) {if(TabFrame(i).Contains(where)) index = i;}

	if(index < 0 || fSelection == index) return;

	ETab *tab = (ETab*)fTabs.ItemAt(index);
	if(tab->IsEnabled() == false) return;
	tab->Select();

	ERect r = Frame().OffsetToSelf(E_ORIGIN);
	r.bottom = r.top + fTabHeight;
	Invalidate(r);
}

