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
 * File: ColorControl.h
 * Description: EColorControl --- Displays a palette of selectable colors
 * 
 * --------------------------------------------------------------------------*/

#ifndef __ETK_COLOR_CONTROL_H__
#define __ETK_COLOR_CONTROL_H__

#include "./../interface/Control.h"

#ifdef __cplusplus /* Just for C++ */

class EBitmap;

class _IMPEXP_ETK EColorControl : public EControl {
public:
	EColorControl(EPoint leftTop, const char *name, EMessage *message = NULL, bool bufferedDrawing = false);
	virtual ~EColorControl();

	virtual void	SetValue(eint32 color);
	void		SetValue(e_rgb_color color);
	e_rgb_color	ValueAsColor();

	virtual void	Draw(ERect updateRect);
	virtual void	MouseDown(EPoint where);
	virtual void	GetPreferredSize(float *width, float *height);

private:
	EBitmap *fBitmap;

	ERect _MarkFrame(ERect colorFrame, euint8 channel);
	ERect _ColorsFrame();
	ERect _DescriptionFrame();

	void _DrawColors(ERect updateRect);
	void _DrawDescription(ERect updateRect);
};


#endif /* __cplusplus */

#endif /* __ETK_COLOR_CONTROL_H__ */

