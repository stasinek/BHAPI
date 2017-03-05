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
 * File: Control.h
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_CONTROL_H
#define BHAPI_CONTROL_H

#include "../support/SupportDefs.h"

#ifdef __cplusplus /* Just for C++ */
namespace bhapi {
#endif
enum {
    B_CONTROL_OFF = 0,
    B_CONTROL_ON = 1
};
#ifdef __cplusplus /* Just for C++ */
} // namespace
#endif

#ifdef __cplusplus /* Just for C++ */
class BRect;
class BMessage;
#include "../interface/View.h"
#include "../app/Invoker.h"
class IMPEXPBHAPI BControl : public BView, public BInvoker {
public:
	BControl(BRect frame,
		 const char *name,
		 const char *label,
		 BMessage *message,
         b_uint32 resizeMode,
         b_uint32 flags);
	virtual ~BControl();

	virtual void SetLabel(const char *label);
	const char* Label() const;

    virtual void SetValue(b_int32 value);
    b_int32 Value() const;

	virtual b_status_t Invoke(const BMessage *msg = NULL);

	virtual void AttachedToWindow();
	virtual void DetachedFromWindow();

	virtual void MakeFocus(bool focusState = true);

protected:
	bool IsFocusChanging() const;
    void SetValueNoUpdate(b_int32 value);

private:
	char *fLabel;
    b_int32 fValue;
	bool fFocusChanging;
};

#endif /* __cplusplus */
#endif /* BHAPI_CONTROL_H */

