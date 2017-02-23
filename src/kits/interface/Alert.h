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
 * File: Alert.h
 * Description: BAlert --- Display a modal window that notifies something
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_ALERT__H
#define BHAPI_ALERT__H

#include "../support/SupportDefs.h"

#include "../interface/InterfaceDefs.h"
#include "../interface/Window.h"

enum b_alert_type {
    B_EMPTY_ALERT = 0,
    B_INFO_ALERT,
    B_IDEA_ALERT,
    B_WARNING_ALERT,
    B_STOP_ALERT
};

#ifdef __cplusplus /* Just for C++ */

class BInvoker;
class BButton;
class BTextView;

class IMPEXP_BHAPI BAlert : public BWindow {
public:
    BAlert(const char *title,
	       const char *text,
	       const char *button1_label,
	       const char *button2_label = NULL,
	       const char *button3_label = NULL,
	       b_button_width width = B_WIDTH_AS_USUAL,
	       b_alert_type type = B_INFO_ALERT);
    virtual ~BAlert();

	// run synchronously then auto-destruct when it return.
	// "could_proxy" must be "true" when it called from looper of BApplication!
    b_int32		Go(bool could_proxy = true);

	// run asynchronously and auto-destruct after message send
	b_status_t	Go(BInvoker *invoker);

    BButton		*ButtonAt(b_int32 index) const;
    BTextView	*TextView() const;

	virtual bool	QuitRequested();

private:
    BButton *fButtons[3];
    BTextView *fTextView;
};

#endif /* __cplusplus */

#endif /* BHAPI_ALERT__H */

