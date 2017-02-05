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
 * File: Menu.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_MENU_H__
#define __ETK_MENU_H__

#include "./../support/List.h>
#include "./../interface/View.h>
#include "./../interface/MenuItem.h>

#ifdef __cplusplus /* Just for C++ */

typedef enum e_menu_layout {
	E_ITEMS_IN_ROW = 0,
	E_ITEMS_IN_COLUMN,
	E_ITEMS_IN_MATRIX
} e_menu_layout;

class ESubmenuWindow;
class EPopUpMenu;

class _IMPEXP_ETK EMenu : public EView {
public:
	EMenu(const char *title, e_menu_layout layout = E_ITEMS_IN_COLUMN);
	EMenu(const char *title, float width, float height);
	virtual ~EMenu();

	e_menu_layout		Layout() const;

	bool			AddItem(EMenuItem *item);
	bool			AddItem(EMenuItem *item, eint32 index);
	bool			AddItem(EMenuItem *item, ERect frame);
	bool			AddItem(EMenu *menu);
	bool			AddItem(EMenu *menu, eint32 index);
	bool			AddItem(EMenu *menu, ERect frame);
	bool			AddSeparatorItem();
	bool			RemoveItem(EMenuItem *item);
	EMenuItem*		RemoveItem(eint32 index);
	bool			RemoveItem(EMenu *menu);

	EMenuItem*		ItemAt(eint32 index) const;
	EMenu*			SubmenuAt(eint32 index) const;
	eint32			CountItems() const;
	eint32			IndexOf(const EMenuItem *item) const;
	eint32			IndexOf(const EMenu *menu) const;

	EMenuItem*		FindItem(euint32 command) const;
	EMenuItem*		FindItem(const char *name) const;
	EMenuItem*		FindMarked(eint32 *index = NULL) const;

	EMenu*			Supermenu() const;
	EMenuItem*		Superitem() const;

	virtual e_status_t	SetTargetForItems(EHandler *target);
	virtual e_status_t	SetTargetForItems(EMessenger messenger);

	virtual void		SetEnabled(bool state);
	bool			IsEnabled() const;

	virtual void		SetRadioMode(bool state);
	bool			IsRadioMode() const;

	virtual void		SetLabelFromMarked(bool state);
	bool			IsLabelFromMarked() const;

	virtual void		AttachedToWindow();
	virtual void		DetachedFromWindow();

	virtual void		MessageReceived(EMessage *msg);
	virtual void		Draw(ERect updateRect);
	virtual void		MouseDown(EPoint where);
	virtual void		MouseUp(EPoint where);
	virtual void		MouseMoved(EPoint where, euint32 code, const EMessage *a_message);
	virtual void		KeyDown(const char *bytes, eint32 numBytes);
	virtual void		KeyUp(const char *bytes, eint32 numBytes);
	virtual void		Hide();

	virtual void		GetPreferredSize(float *width, float *height);

	EMenuItem*		CurrentSelection() const;
	void			SelectItem(EMenuItem *item, bool showSubmenu = false, bool selectFirstItem = false);

protected:
	EMenu(ERect frame, const char *title, euint32 resizeMode, euint32 flags, e_menu_layout layout, bool resizeToFit);

	void			SetItemMargins(float left, float top, float right, float bottom);
	void			GetItemMargins(float *left, float *top, float *right, float *bottom) const;

	eint32			FindItem(EPoint where);
	ERect			ItemFrame(eint32 index) const;

	virtual void		ItemInvoked(EMenuItem *item);

private:
	friend class ESubmenuWindow;
	friend class EMenuItem;
	friend class EPopUpMenu;

	ERect fMargins;
	bool fResizeToFit;
	e_menu_layout fLayout;
	EList fMenuItems;

	EMenuItem *fSuperitem;

	bool fRadioMode;
	bool fLabelFromMarked;
	eint32 fSelectedIndex;
	eint32 fTrackingIndex;
	eint32 fMarkedIndex;
	bool fShowSubmenuByKeyDown;

	void Refresh();
	void SetLayout(e_menu_layout layout, float width, float height, bool resizeToFit);

	bool GetPopUpWhere(EPoint *where);
	bool PopUp(EPoint where, bool selectFirstItem);
	void ClosePopUp();
};

#endif /* __cplusplus */

#endif /* __ETK_MENU_H__ */

