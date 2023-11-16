/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
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
 * File: MenuItem.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_MENU_ITEM_H
#define BHAPI_MENU_ITEM_H

#ifdef __cplusplus /* Just for C++ */
class BMenu;
class BMessage;
#include <kits/support/Archivable.h>
#include <kits/interface/Rect.h>
#include <kits/app/Invoker.h>
#include <Haiku.h>
class BHAPI_IMPEXP BMenuItem : public BArchivable, public BInvoker {
public:
    BMenuItem(const char *label, BMessage *message, char shortcut = 0,  uint32_t modifiers = 0);
    BMenuItem(BMenu *menu, BMessage *message = NULL);
    virtual ~BMenuItem();

    virtual void		SetLabel(const char *label);
    virtual void		SetEnabled(bool state);
    virtual void		SetMarked(bool state);
    virtual void		SetShortcut(char ch,  uint32_t modifiers);

    const char*		Label() const;
    bool			IsEnabled() const;
    bool			IsMarked() const;
    char			Shortcut(uint32_t *modifiers = NULL) const;

    BMenu*			Submenu() const;
    BMenu*			Menu() const;
    BRect			Frame() const;

    virtual status_t	Invoke(const BMessage *msg = NULL);

protected:
    friend class BMenu;

    bool		IsSelected() const;
    virtual bool	SelectChanged();

    virtual void	GetContentSize(float *width, float *height) const;

    virtual void	DrawContent();
    virtual void	Draw();
    virtual void	Highlight(bool on);

private:
    char fShortcut;
     uint32_t fModifiers;

    BRect fFrame;
    bool fMarked;
    bool fEnabled;

    char *fLabel;
    char *fShortcuts;

    BMenu *fSubmenu;
    BMenu *fMenu;

    void ShowSubmenu(bool selectFirstItem = true);
};


class BHAPI_IMPEXP BMenuSeparatorItem : public BMenuItem {
public:
    BMenuSeparatorItem();
    virtual ~BMenuSeparatorItem();

protected:
    virtual void	GetContentSize(float *width, float *height) const;
    virtual void	Draw();
    virtual bool	SelectChanged();
};


#endif /* __cplusplus */

#endif /* BHAPI_MENU_ITEM_H */

