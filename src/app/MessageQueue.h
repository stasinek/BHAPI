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
 * File: MessageQueue.h
 * Description: The first-in/first-out queue of messages for Looper
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_MESSAGE_QUEUE_H__
#define __ETK_MESSAGE_QUEUE_H__

#include "./../app/Message.h>

#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_ETK EMessageQueue {
public:
	EMessageQueue();
	virtual ~EMessageQueue();

	// add "an_event" to the queue and delete it automatically when FAILED
	bool		AddMessage(EMessage *an_event);

	// remove "an_event" from the queue and delete it automatically when FOUNDED
	bool		RemoveMessage(EMessage *an_event);

	// return the FIRST message and detach from the queue, you should "delete" by yourself
	EMessage	*NextMessage();

	EMessage	*FindMessage(eint32 index) const;
	EMessage	*FindMessage(euint32 what, eint32 fromIndex = 0) const;
	EMessage	*FindMessage(euint32 what, eint32 fromIndex, eint32 count) const;
	eint32		IndexOfMessage(EMessage *an_event) const;

	eint32		CountMessages() const;
	bool		IsEmpty() const;

	bool		Lock();
	void		Unlock();
	e_status_t	LockWithTimeout(e_bigtime_t microseconds_timeout);

private:
	EList fMessagesList;
	void *fLocker;
};

#endif /* __cplusplus */

#endif /* __ETK_MESSAGE_QUEUE_H__ */

