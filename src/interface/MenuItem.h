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
 * File: MenuItem.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_MENU_ITEM_H__
#define __ETK_MENU_ITEM_H__

#include "./../support/Archivable.h>
#include "./../app/Invoker.h>

#ifdef __cplusplus /* Just for C++ */

class EMenu;

class _IMPEXP_ETK EMenuItem : public EArchivable, public EInvoker {
public:
	EMenuItem(const char *label, EMessage *message, char shortcut = 0, euint32 modifiers = 0);
	EMenuItem(EMenu *menu, EMessage *message = NULL);
	virtual ~EMenuItem();

	virtual void		SetLabel(const char *label);
	virtual void		SetEnabled(bool state);
	virtual void		SetMarked(bool state);
	virtual void		SetShortcut(char ch, euint32 modifiers);

	const char*		Label() const;
	bool			IsEnabled() const;
	bool			IsMarked() const;
	char			Shortcut(euint32 *modifiers = NULL) const;

	EMenu*			Submenu() const;
	EMenu*			Menu() const;
	ERect			Frame() const;

	virtual e_status_t	Invoke(const EMessage *msg = NULL);

protected:
	friend class EMenu;

	bool		IsSelected() const;
	virtual bool	SelectChanged();

	virtual void	GetContentSize(float *width, float *height) const;

	virtual void	DrawContent();
	virtual void	Draw();
	virtual void	Highlight(bool on);

private:
	char fShortcut;
	euint32 fModifiers;

	ERect fFrame;
	bool fMarked;
	bool fEnabled;

	char *fLabel;
	char *fShortcuts;

	EMenu *fSubmenu;
	EMenu *fMenu;

	void ShowSubmenu(bool selectFirstItem = true);
};


class _IMPEXP_ETK EMenuSeparatorItem : public EMenuItem {
public:
	EMenuSeparatorItem();
	virtual ~EMenuSeparatorItem();

protected:
	virtual void	GetContentSize(float *width, float *height) const;
	virtual void	Draw();
	virtual bool	SelectChanged();
};


#endif /* __cplusplus */

#endif /* __ETK_MENU_ITEM_H__ */

