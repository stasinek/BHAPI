/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
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
 * File: MessageQueue.h
 * Description: The first-in/first-out queue of messages for Looper
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_MESSAGE_QUEUE_H
#define BHAPI_MESSAGE_QUEUE_H
//-----------------------------------------------------------------------------
#ifdef __cplusplus /* Just for C++ */
//-----------------------------------------------------------------------------
#ifndef BMESSAGE_DEF
#define BMESSAGE_DEF
class BMessage;
#endif
//-----------------------------------------------------------------------------
#include "../support/List.h"
#include <Haiku.h>
//-----------------------------------------------------------------------------
class BHAPI_IMPEXP BMessageQueue {
public:
    BMessageQueue();
    virtual ~BMessageQueue();

    // add "an_event" to the queue and delete it automatically when FAILED
    bool		AddMessage(BMessage *an_event);

    // remove "an_event" from the queue and delete it automatically when FOUNDED
    bool		RemoveMessage(BMessage *an_event);

    // return the FIRST message and detach from the queue, you should "delete" by yourself
    BMessage	*NextMessage();

    BMessage	*FindMessage(__be_int32 index) const;
    BMessage	*FindMessage(__be_uint32 what,  __be_int32 fromIndex = 0) const;
    BMessage	*FindMessage(__be_uint32 what,  __be_int32 fromIndex,  __be_int32 count) const;
     __be_int32		IndexOfMessage(BMessage *an_event) const;

     __be_int32		CountMessages() const;
    bool		IsEmpty() const;

    bool		Lock();
    void		Unlock();
    status_t	LockWithTimeout(bigtime_t microseconds_timeout);

private:
    BList fMessagesList;
    void *fLocker;
};
//-----------------------------------------------------------------------------
#endif /* __cplusplus */
//-----------------------------------------------------------------------------
#define BMESSAGEQUEUE_DEF
#endif /* BHAPI_MESSAGE_QUEUE_H */
//-----------------------------------------------------------------------------

