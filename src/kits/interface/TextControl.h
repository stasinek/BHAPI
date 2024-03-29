﻿/* --------------------------------------------------------------------------
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
 * File: TextControl.h
 * Description: BTextControl --- display a labeled field could deliver a message as modifying
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_TEXT_CONTROL_H
#define BHAPI_TEXT_CONTROL_H

#ifdef __cplusplus /* Just for C++ */
class BMessage;
#include <kits/interface/TextEditable.h>
#include <kits/interface/Rect.h>
#include <Haiku.h>
class BHAPI_IMPEXP BTextControl : public BTextEditable {
public:
    BTextControl(BRect frame,
             const char *name,
             const char *label,
             const char *text,
             BMessage *message,
              uint32_t resizeMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
              uint32_t flags = B_WILL_DRAW | B_FRAME_EVENTS | B_NAVIGABLE);
    virtual ~BTextControl();

    virtual void	SetText(const char *text);

    virtual void	SetDivider(float divider);
    float		Divider() const;

    virtual void	SetAlignment(bhapi::alignment forLabel, bhapi::alignment forText);
    void		GetAlignment(bhapi::alignment *forLabel, bhapi::alignment *forText) const;

    virtual void	SetModificationMessage(BMessage *msg);
    BMessage	*ModificationMessage() const;

    virtual void	SetLabel(const char *label);
    virtual void	Draw(BRect updateRect);
    virtual void	GetPreferredSize(float *width, float *height);
    virtual void	KeyDown(const char *bytes,  int32_t numBytes);

private:
    bhapi::alignment fLabelAlignment;
    float fDivider;
    BMessage *fModificationMessage;
};

#endif /* __cplusplus */

#endif /* BHAPI_TEXT_CONTROL_H */

