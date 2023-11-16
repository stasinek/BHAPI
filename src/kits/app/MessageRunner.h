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
 * File: MessageRunner.h
 * Description: Sending messages at intervals
 *
 * --------------------------------------------------------------------------*/
#ifndef BHAPI_MESSAGE_RUNNER_H
#define BHAPI_MESSAGE_RUNNER_H
//-----------------------------------------------------------------------------
#ifdef __cplusplus /* Just for C++ */
//-----------------------------------------------------------------------------
#ifndef BMESSENGER_DEF
class BMessenger;
#define BMESSENGER_DEF
#endif
//-----------------------------------------------------------------------------
#ifndef BMESSAGE_DEF
class BMessage;
#define BMESSAGE_DEF
#endif
//-----------------------------------------------------------------------------
#include <kits/kernel/OS.h>
//-----------------------------------------------------------------------------
class BHAPI_IMPEXP BMessageRunner {
public:
    BMessageRunner(const BMessenger &target,
               const BMessage *msg,
               bigtime_t interval,
                int32_t count = -1);
    BMessageRunner(const BMessenger &target,
               const BMessage *msg,
               bigtime_t interval,
                int32_t count,
               const BMessenger &replyTo);
    virtual ~BMessageRunner();

    bool		IsValid() const;

    status_t	SetTarget(const BMessenger &target);
    status_t	SetReplyTo(const BMessenger &replyTo);
    status_t	SetMessage(const BMessage *msg);
    status_t	SetInterval(bigtime_t interval);
    status_t	SetCount(int32_t count);

    status_t	GetInfo(bigtime_t *interval,  int32_t *count) const;
    status_t	GetInfo(BMessenger *target, BMessage *msg,
                bigtime_t *interval,  int32_t *count,
                BMessenger *replyTo = NULL) const;

private:
    friend class BApplication;

     int32_t fToken;

    BMessenger *fTarget;
    BMessenger *fReplyTo;
    BMessage *fMessage;
    bigtime_t fInterval;
    bigtime_t fPrevSendTime;
     int32_t fCount;
};
//-----------------------------------------------------------------------------
#endif /* __cplusplus */
//-----------------------------------------------------------------------------
#define BMESSAGERUNNER_DEF
#endif /* BHAPI_MESSAGE_RUNNER_H */
//-----------------------------------------------------------------------------

