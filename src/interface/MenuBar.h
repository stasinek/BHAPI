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
 * File: MenuBar.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_MENU_BAR_H__
#define __ETK_MENU_BAR_H__

#include "./../interface/Menu.h>

#ifdef __cplusplus /* Just for C++ */

typedef enum e_menu_bar_border {
	E_BORDER_NONE = 0,
	E_BORDER_FRAME,
	E_BORDER_CONTENTS,
	E_BORDER_EACH_ITEM
} e_menu_bar_border;

class _IMPEXP_ETK EMenuBar : public EMenu {
public:
	EMenuBar(ERect frame, const char *title,
		 euint32 resizeMode = E_FOLLOW_LEFT_RIGHT | E_FOLLOW_TOP,
		 e_menu_layout layout = E_ITEMS_IN_ROW,
		 bool resizeToFit = true);
	virtual ~EMenuBar();

	virtual void		SetBorder(e_menu_bar_border border);
	e_menu_bar_border	Border() const;

	virtual void		Draw(ERect updateRect);

	virtual void		MessageReceived(EMessage *msg);
	virtual void		MouseDown(EPoint where);
	virtual void		MouseUp(EPoint where);
	virtual void		MouseMoved(EPoint where, euint32 code, const EMessage *a_message);
	virtual void		KeyDown(const char *bytes, eint32 numBytes);
	virtual void		KeyUp(const char *bytes, eint32 numBytes);

	virtual void		MakeFocus(bool focusState = true);

protected:
	virtual void		ItemInvoked(EMenuItem *item);

private:
	e_menu_bar_border fBorder;
};

#endif /* __cplusplus */

#endif /* __ETK_MENU_BAR_H__ */

