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
 * File: MessageQueue.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../kernel/Kernel.h"
#include "./../support/Locker.h"
#include "./../support/List.h"

#include "./../private/PrivateHandler.h"

#include "MessageQueue.h"


EMessageQueue::EMessageQueue()
{
	if((fLocker = etk_create_locker()) == NULL)
		ETK_ERROR("[APP]: %s --- Unable to create locker for looper.", __PRETTY_FUNCTION__);
}


EMessageQueue::~EMessageQueue()
{
	for(eint32 i = 0; i < fMessagesList.CountItems(); i++) delete (EMessage*)fMessagesList.ItemAt(i);
	if(fLocker != NULL)
	{
		etk_close_locker(fLocker);
		etk_delete_locker(fLocker);
	}
}


e_status_t
EMessageQueue::LockWithTimeout(e_bigtime_t timeout)
{
	ELocker *handlers_locker = etk_get_handler_operator_locker();

	handlers_locker->Lock();

	void *locker = etk_clone_locker(fLocker);
	if(locker == NULL)
	{
		handlers_locker->Unlock();
		return E_ERROR;
	}

	eint64 save_count = handlers_locker->CountLocks();
	while(handlers_locker->CountLocks() > 0) handlers_locker->Unlock();

	e_status_t retVal = etk_lock_locker_etc(fLocker, E_TIMEOUT, timeout);
	etk_delete_locker(locker);

	while(save_count-- > 1) handlers_locker->Lock();

	return retVal;
}


bool
EMessageQueue::Lock()
{
	return(LockWithTimeout(E_INFINITE_TIMEOUT) == E_OK);
}


void
EMessageQueue::Unlock()
{
	if(etk_count_locker_locks(fLocker) <= 0)
	{
		ETK_WARNING("[APP]: %s -- MessageQueue wasn't locked by current thread.", __PRETTY_FUNCTION__);
		return;
	}

	etk_unlock_locker(fLocker);
}


eint32
EMessageQueue::CountMessages() const
{
	return fMessagesList.CountItems();
}


bool
EMessageQueue::IsEmpty() const
{
	return fMessagesList.IsEmpty();
}


bool
EMessageQueue::AddMessage(EMessage *an_event)
{
	if(an_event == NULL) return false;

	if(fMessagesList.AddItem(an_event)) return true;

	delete an_event;
	return false;
}


bool
EMessageQueue::RemoveMessage(EMessage *an_event)
{
	if(an_event == NULL) return false;

	if(fMessagesList.RemoveItem(an_event) == false) return false;

	delete an_event;
	return true;
}


EMessage*
EMessageQueue::NextMessage()
{
	return((EMessage*)fMessagesList.RemoveItem(0));
}


EMessage*
EMessageQueue::FindMessage(eint32 index) const
{
	return((EMessage*)fMessagesList.ItemAt(index));
}


EMessage*
EMessageQueue::FindMessage(euint32 what, eint32 fromIndex) const
{
	for(eint32 i = fromIndex; i < fMessagesList.CountItems(); i++)
	{
		EMessage *msg = (EMessage*)fMessagesList.ItemAt(i);
		if(msg->what == what) return msg;
	}

	return NULL;
}


EMessage*
EMessageQueue::FindMessage(euint32 what, eint32 fromIndex, eint32 count) const
{
	for(eint32 i = fromIndex, j = 0; i < fMessagesList.CountItems() && j < count; i++, j++)
	{
		EMessage *msg = (EMessage*)fMessagesList.ItemAt(i);
		if(msg->what == what) return msg;
	}

	return NULL;
}


eint32
EMessageQueue::IndexOfMessage(EMessage *an_event) const
{
	return fMessagesList.IndexOf(an_event);
}

