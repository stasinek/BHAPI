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
 * File: BStringView.h
 * Description: BStringView --- A view just for display a string
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_STRING_VIEW_H
#define BHAPI_STRING_VIEW_H

#ifdef __cplusplus /* Just for C++ */
#include <kits/interface/View.h>
#include <kits/support/StringArray.h>
#include <kits/support/String.h>
#include <Haiku.h>
class BHAPI_IMPEXP BStringView : public BView {
public:
    BStringView(BRect frame,
            const char *name,
            const char *initial_text,
             uint32_t resizeMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
             uint32_t flags = B_WILL_DRAW);
    virtual ~BStringView();

    virtual void		SetText(const char *text);
    void			SetText(const BString &text);
    const char*		Text() const;

    virtual void		SetAlignment(bhapi::alignment alignment);
    bhapi::alignment		Alignment() const;

    virtual void		SetVerticalAlignment(bhapi::vertical_alignment alignment);
    bhapi::vertical_alignment	VerticalAlignment() const;

    virtual void		Draw(BRect updateRect);
    virtual void		SetFont(const BFont *font,  uint8 mask = B_FONT_ALL);
    virtual void		GetPreferredSize(float *width, float *height);

private:
    BString fText;
    BStringArray *fTextArray;
    bhapi::alignment fAlignment;
    bhapi::vertical_alignment fVerticalAlignment;
};

#ifdef BHAPI_BUILD_LIBRARY

inline void BStringView::SetText(const BString &text)
{
    SetText(text.String());
}
#endif // BHAPI_BUILD_LIBRARY
#endif /* __cplusplus */
#define BSTRINGVIEW_DEF
#endif /* BHAPI_STRING_VIEW_H */

