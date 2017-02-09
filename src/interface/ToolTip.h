/* --------------------------------------------------------------------------
 *
 * BHAPI++ previously named ETK++, The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
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
 * File: ToolTip.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_TOOL_TIP__H
#define BHAPI_TOOL_TIP__H

#include "./../app/Messenger.h"
#include "./../interface/GraphicsDefs.h"
#include "./../interface/Point.h"
#include "./../interface/Font.h"


#if 0
class IMPEXP_BHAPI BToolTipInfo {
public:
    BToolTipInfo();
    virtual ~BToolTipInfo();

	void			SetText(const char *text);
	const char		*Text() const;

	void			SetFont(const BFont *font);
	const BFont*		Font() const;

    void			SetBackgroundColor(b_rgb_color color);
	b_rgb_color		BackgroundColor() const;

    void			SetTextColor(b_rgb_color color);
	b_rgb_color		TextColor() const;

	void			SetInline(bool state);
	bool			IsInline() const;

	void			SetTextOrigin(BPoint origin);
	BPoint			TextOrigin() const;

private:
	char *fText;
	BFont fFont;
	b_rgb_color fColor[2];
	bool fIsInline;
	BPoint fTextOrigin;
};


class IMPEXP_BHAPI BToolTip {
public:
    BToolTip();
    virtual ~BToolTip();

	virtual b_status_t	ShowTip(const BMessenger &msgr);
	virtual b_status_t	HideTip(const BMessenger &msgr);
	virtual b_status_t	KillTip(const BMessenger &msgr);
	virtual b_status_t	SetToolTipInfo(const BMessenger &msgr,
					       BRect region,
                           BToolTipInfo *info);

	virtual b_status_t	CursorMoved(const BMessenger &msgr,
					    BPoint where,
					    BPoint delta);

    static BToolTip		*Default();

private:
	void *fData;
};
#endif


#endif /* BHAPI_TOOL_TIP__H */

