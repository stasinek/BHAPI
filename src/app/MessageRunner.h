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
 * File: MessageRunner.h
 * Description: Sending messages at intervals
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_MESSAGE_RUNNER__H
#define BHAPI_MESSAGE_RUNNER__H
#include "./../support/SupportDefs.h"

#ifdef __cplusplus /* Just for C++ */
class BMessenger;
class IMPEXP_BHAPI BMessageRunner {
public:
	BMessageRunner(const BMessenger &target,
		       const BMessage *msg,
		       b_bigtime_t interval,
               b_int32 count = -1);
	BMessageRunner(const BMessenger &target,
		       const BMessage *msg,
		       b_bigtime_t interval,
               b_int32 count,
		       const BMessenger &replyTo);
	virtual ~BMessageRunner();

	bool		IsValid() const;

	b_status_t	SetTarget(const BMessenger &target);
	b_status_t	SetReplyTo(const BMessenger &replyTo);
	b_status_t	SetMessage(const BMessage *msg);
    b_status_t	SetInterval(b_bigtime_t interval);
    b_status_t	SetCount(b_int32 count);

    b_status_t	GetInfo(b_bigtime_t *interval, b_int32 *count) const;
	b_status_t	GetInfo(BMessenger *target, BMessage *msg,
                b_bigtime_t *interval, b_int32 *count,
				BMessenger *replyTo = NULL) const;

private:
	friend class BApplication;

    b_int32 fToken;

	BMessenger *fTarget;
	BMessenger *fReplyTo;
	BMessage *fMessage;
	b_bigtime_t fInterval;
	b_bigtime_t fPrevSendTime;
    b_int32 fCount;
};

#endif /* __cplusplus */

#endif /* BHAPI_MESSAGE_RUNNER__H */

