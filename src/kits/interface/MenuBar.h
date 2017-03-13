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
 * File: MenuBar.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_MENU_BAR_H
#define BHAPI_MENU_BAR_H

#ifdef __cplusplus /* Just for C++ */
namespace bhapi {
#endif
typedef enum menu_bar_border {
    B_BORDER_NONE = 0,
    B_BORDER_FRAME,
    B_BORDER_CONTENTS,
    B_BORDER_EACH_ITEM
} menu_bar_border;
#ifdef __cplusplus /* Just for C++ */
} // namespace
#endif

#ifdef __cplusplus /* Just for C++ */
#include "../interface/Menu.h"
class BHAPI_IMPEXP BMenuBar : public BMenu {
public:
    BMenuBar(BRect frame, const char *title,
          __be_uint32 resizeMode = B_FOLLOW_LEFT_RIGHT | B_FOLLOW_TOP,
         bhapi::menu_layout layout = B_ITEMS_IN_ROW,
         bool resizeToFit = true);
    virtual ~BMenuBar();

    virtual void		SetBorder(bhapi::menu_bar_border border);
    bhapi::menu_bar_border	Border() const;

    virtual void		Draw(BRect updateRect);

    virtual void		MessageReceived(BMessage *msg);
    virtual void		MouseDown(BPoint where);
    virtual void		MouseUp(BPoint where);
    virtual void		MouseMoved(BPoint where,  __be_uint32 code, const BMessage *a_message);
    virtual void		KeyDown(const char *bytes,  __be_int32 numBytes);
    virtual void		KeyUp(const char *bytes,  __be_int32 numBytes);

    virtual void		MakeFocus(bool focusState = true);

protected:
    virtual void		ItemInvoked(BMenuItem *item);

private:
    bhapi::menu_bar_border fBorder;
};

#endif /* __cplusplus */

#endif /* BHAPI_MENU_BAR_H */

