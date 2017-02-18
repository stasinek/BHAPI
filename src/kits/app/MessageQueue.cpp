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
 * File: MessageQueue.cpp
 *
 * --------------------------------------------------------------------------*/

#include "MessageQueue.h"
#include "Message.h"

#include "../kernel/Kernel.h"
#include "../kernel/Debug.h"
#include "../support/Locker.h"
#include "../support/Errors.h"
#include "../support/List.h"
#include "../private/PrivateHandler.h"

BMessageQueue::BMessageQueue()
{
	if((fLocker = bhapi_create_locker()) == NULL)
		BHAPI_ERROR("[APP]: %s --- Unable to create locker for looper.", __PRETTY_FUNCTION__);
}


BMessageQueue::~BMessageQueue()
{
	for(b_int32 i = 0; i < fMessagesList.CountItems(); i++) delete (BMessage*)fMessagesList.ItemAt(i);
	if(fLocker != NULL)
	{
		bhapi_close_locker(fLocker);
		bhapi_delete_locker(fLocker);
	}
}


b_status_t
BMessageQueue::LockWithTimeout(b_bigtime_t timeout)
{
	BLocker *handlers_locker = bhapi_get_handler_operator_locker();

	handlers_locker->Lock();

	void *locker = bhapi_clone_locker(fLocker);
	if(locker == NULL)
	{
		handlers_locker->Unlock();
		return B_ERROR;
	}

	b_int64 save_count = handlers_locker->CountLocks();
	while(handlers_locker->CountLocks() > 0) handlers_locker->Unlock();

	b_status_t retVal = bhapi_lock_locker_etc(fLocker, B_TIMEOUT, timeout);
	bhapi_delete_locker(locker);

	while(save_count-- > 1) handlers_locker->Lock();

	return retVal;
}


bool
BMessageQueue::Lock()
{
	return(LockWithTimeout(B_INFINITE_TIMEOUT) == B_OK);
}


void
BMessageQueue::Unlock()
{
	if(bhapi_count_locker_locks(fLocker) <= 0)
	{
		BHAPI_WARNING("[APP]: %s -- MessageQueue wasn't locked by current thread.", __PRETTY_FUNCTION__);
		return;
	}

	bhapi_unlock_locker(fLocker);
}


b_int32
BMessageQueue::CountMessages() const
{
	return fMessagesList.CountItems();
}


bool
BMessageQueue::IsEmpty() const
{
	return fMessagesList.IsEmpty();
}


bool
BMessageQueue::AddMessage(BMessage *an_event)
{
	if(an_event == NULL) return false;

	if(fMessagesList.AddItem(an_event)) return true;

	delete an_event;
	return false;
}


bool
BMessageQueue::RemoveMessage(BMessage *an_event)
{
	if(an_event == NULL) return false;

	if(fMessagesList.RemoveItem(an_event) == false) return false;

	delete an_event;
	return true;
}


BMessage*
BMessageQueue::NextMessage()
{
	return((BMessage*)fMessagesList.RemoveItem(0));
}


BMessage*
BMessageQueue::FindMessage(b_int32 index) const
{
	return((BMessage*)fMessagesList.ItemAt(index));
}


BMessage*
BMessageQueue::FindMessage(b_uint32 what, b_int32 fromIndex) const
{
	for(b_int32 i = fromIndex; i < fMessagesList.CountItems(); i++)
	{
		BMessage *msg = (BMessage*)fMessagesList.ItemAt(i);
		if(msg->what == what) return msg;
	}

	return NULL;
}


BMessage*
BMessageQueue::FindMessage(b_uint32 what, b_int32 fromIndex, b_int32 count) const
{
	for(b_int32 i = fromIndex, j = 0; i < fMessagesList.CountItems() && j < count; i++, j++)
	{
		BMessage *msg = (BMessage*)fMessagesList.ItemAt(i);
		if(msg->what == what) return msg;
	}

	return NULL;
}


b_int32
BMessageQueue::IndexOfMessage(BMessage *an_event) const
{
	return fMessagesList.IndexOf(an_event);
}

