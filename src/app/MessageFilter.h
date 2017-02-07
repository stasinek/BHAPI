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
 * File: MessageFilter.h
 * Description: Filter message before BLooper::DispatchMessage
 * Warning: ignore _QUIT_
 *
 * --------------------------------------------------------------------------*/

#ifndef __BHAPI_MESSAGE_FILTER_H__
#define __BHAPI_MESSAGE_FILTER_H__

#include "./../app/Handler.h"

typedef enum b_filter_result {
    B_SKIP_MESSAGE,
    B_DISPATCH_MESSAGE
} b_filter_result;

typedef enum b_message_delivery {
    B_DROPPED_DELIVERY	=	0x1,
    B_PROGRAMMED_DELIVERY	=	0x2,
    B_ANY_DELIVERY		=	0xff
} b_message_delivery;

typedef enum b_message_source {
    B_LOCAL_SOURCE		=	0x1,
    B_REMOTE_SOURCE		=	0x2,
    B_ANY_SOURCE		=	0xff
} b_message_source;

#ifdef __cplusplus /* Just for C++ */

class BMessage;
class BMessageFilter;

typedef b_filter_result (*b_filter_hook)(BMessage *message, BHandler **target, BMessageFilter *filter);


class _IMPEXP_BHAPI BMessageFilter {
public:
    BMessageFilter(b_message_delivery delivery, b_message_source source,
               b_uint32 command, b_filter_hook filter = NULL);
    BMessageFilter(b_message_delivery delivery, b_message_source source,
		       b_filter_hook filter = NULL);
    BMessageFilter(b_uint32 command, b_filter_hook filter = NULL);
	BMessageFilter(const BMessageFilter &filter);
	BMessageFilter(const BMessageFilter *filter);
	virtual ~BMessageFilter();

	BMessageFilter &operator=(const BMessageFilter &from);

	virtual b_filter_result		Filter(BMessage *message, BHandler **target);

	b_message_delivery		MessageDelivery() const;
	b_message_source		MessageSource() const;
    b_uint32				Command() const;
	bool				FiltersAnyCommand() const;
	BLooper				*Looper() const;

private:
	friend class BLooper;
	friend class BHandler;

    b_uint32 fCommand;
	bool fFiltersAny;
	b_message_delivery fDelivery;
	b_message_source fSource;
	b_filter_hook fFilterHook;

	BHandler *fHandler;

	b_filter_result doFilter(BMessage *message, BHandler **target);
};

#endif /* __cplusplus */

#endif /* __BHAPI_MESSAGE_FILTER_H__ */

