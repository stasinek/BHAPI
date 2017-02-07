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
 * File: RadioButton.h
 * Description: BRadioButton --- Radio buttons within their parent only one be on
 * 
 * --------------------------------------------------------------------------*/

#ifndef __BHAPI_RADIO_BUTTON_H__
#define __BHAPI_RADIO_BUTTON_H__

#include "./../interface/Control.h"

#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_BHAPI BRadioButton : public BControl {
public:
    BRadioButton(BRect frame,
		     const char *name,
		     const char *label,
		     BMessage *message,
             b_uint32 resizeMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
             b_uint32 flags = B_WILL_DRAW | B_NAVIGABLE);
    virtual ~BRadioButton();

	virtual void	SetLabel(const char *label);

	virtual void	Draw(BRect updateRect);
	virtual void	MouseDown(BPoint where);
    virtual void	KeyDown(const char *bytes, b_int32 numBytes);
    virtual void	SetFont(const BFont *font, b_uint8 mask = B_FONT_ALL);
	virtual void	WindowActivated(bool state);

    virtual void	SetValue(b_int32 value);
	virtual void	GetPreferredSize(float *width, float *height);
};

#endif /* __cplusplus */

#endif /* __BHAPI_RADIO_BUTTON_H__ */

