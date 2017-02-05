/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2007, Anthony Lee, All Rights Reserved
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
 * File: ToolTip.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_TOOL_TIP_H__
#define __ETK_TOOL_TIP_H__

#include "./../app/Messenger.h>
#include "./../interface/GraphicsDefs.h>
#include "./../interface/Point.h"
#include "./../interface/Font.h>


#if 0
class _IMPEXP_ETK EToolTipInfo {
public:
	EToolTipInfo();
	virtual ~EToolTipInfo();

	void			SetText(const char *text);
	const char		*Text() const;

	void			SetFont(const EFont *font);
	const EFont*		Font() const;

	void			SetBackgroundColor(e_rgb_color color);
	e_rgb_color		BackgroundColor() const;

	void			SetTextColor(e_rgb_color color);
	e_rgb_color		TextColor() const;

	void			SetInline(bool state);
	bool			IsInline() const;

	void			SetTextOrigin(EPoint origin);
	EPoint			TextOrigin() const;

private:
	char *fText;
	EFont fFont;
	e_rgb_color fColor[2];
	bool fIsInline;
	EPoint fTextOrigin;
};


class _IMPEXP_ETK EToolTip {
public:
	EToolTip();
	virtual ~EToolTip();

	virtual e_status_t	ShowTip(const EMessenger &msgr);
	virtual e_status_t	HideTip(const EMessenger &msgr);
	virtual e_status_t	KillTip(const EMessenger &msgr);
	virtual e_status_t	SetToolTipInfo(const EMessenger &msgr,
					       ERect region,
					       EToolTipInfo *info);

	virtual e_status_t	CursorMoved(const EMessenger &msgr,
					    EPoint where,
					    EPoint delta);

	static EToolTip		*Default();

private:
	void *fData;
};
#endif


#endif /* __ETK_TOOL_TIP_H__ */

