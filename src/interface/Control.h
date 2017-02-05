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
 * File: Control.h
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_CONTROL_H__
#define __ETK_CONTROL_H__

#include "./../interface/View.h>
#include "./../app/Invoker.h>

#ifdef __cplusplus /* Just for C++ */

enum {
	E_CONTROL_OFF = 0,
	E_CONTROL_ON = 1
};

class _IMPEXP_ETK EControl : public EView, public EInvoker {
public:
	EControl(ERect frame,
		 const char *name,
		 const char *label,
		 EMessage *message,
		 euint32 resizeMode,
		 euint32 flags);
	virtual ~EControl();

	virtual void SetLabel(const char *label);
	const char* Label() const;

	virtual void SetValue(eint32 value);
	eint32 Value() const;

	virtual e_status_t Invoke(const EMessage *msg = NULL);

	virtual void AttachedToWindow();
	virtual void DetachedFromWindow();

	virtual void MakeFocus(bool focusState = true);

protected:
	bool IsFocusChanging() const;
	void SetValueNoUpdate(eint32 value);

private:
	char *fLabel;
	eint32 fValue;
	bool fFocusChanging;
};

#endif /* __cplusplus */

#endif /* __ETK_CONTROL_H__ */

