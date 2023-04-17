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
 * File: Button.h
 * Description: BButton --- A view like button for control in window
 * 
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_BUTTON_H
#define BHAPI_BUTTON_H

#ifdef __cplusplus /* Just for C++ */

class BRect;
class BMessage;
class BMessageRunner;
#include <kits/interface/Control.h>
class BHAPI_IMPEXP BButton : public BControl {
public:
    BButton(BRect frame,
		const char *name,
		const char *label,
		BMessage *message,
         uint32_t resizeMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
         uint32_t flags = B_WILL_DRAW | B_NAVIGABLE);
    virtual ~BButton();

	virtual void	SetLabel(const char *label);

    virtual void	Draw(BRect updateRect);
	virtual void	DetachedFromWindow();
    virtual void	MouseDown(BPoint where);
    virtual void	MouseUp(BPoint where);
    virtual void	MouseMoved(BPoint where,  uint32_t code, const BMessage *a_message);
    virtual void	KeyDown(const char *bytes,  int32_t numBytes);
    virtual void	KeyUp(const char *bytes,  int32_t numBytes);
    virtual void	SetFont(const BFont *font,  uint8 mask = B_FONT_ALL);
	virtual void	WindowActivated(bool state);
	virtual void	MakeFocus(bool focusState = true);

	virtual void	GetPreferredSize(float *width, float *height);

protected:
    BRect		ContentFrame() const;
	virtual void	DrawContent();

private:
	bool fInsided;
	bool fMousePushed;
     uint8 fFocusFlash;
	BMessageRunner *fRunner;
};

#endif /* __cplusplus */

#endif /* BHAPI_BUTTON_H */

