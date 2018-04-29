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
 * File: MessageQueue.cpp
 *
 * --------------------------------------------------------------------------*/

#include <kits/netMessageQueue.h>
#include <kits/netMessage.h>

#include <os/kernel.h>
#include <os/debug.h>
#include <kits/support/Locker.h>
#include <kits/debug/Errors.h>
#include <kits/support/List.h>
#include <kits/private/app/PrivateHandler.h>

BMessageQueue::BMessageQueue()
{
    if((fLocker = bhapi::create_locker()) == NULL)
        BHAPI_ERROR("[APP]: %s --- Unable to create locker for looper.", __PRETTY_FUNCTION__);
}


BMessageQueue::~BMessageQueue()
{
    for(int32 i = 0; i < fMessagesList.CountItems(); i++) delete (BMessage*)fMessagesList.ItemAt(i);
    if(fLocker != NULL)
    {
        bhapi::close_locker(fLocker);
        bhapi::delete_locker(fLocker);
    }
}


status_t 
BMessageQueue::LockWithTimeout(bigtime_t timeout)
{
    BLocker *handlers_locker = bhapi::get_handler_operator_locker();

    handlers_locker->Lock();

    void *locker = bhapi::clone_locker(fLocker);
    if(locker == NULL)
    {
        handlers_locker->Unlock();
        return B_ERROR;
    }

     int64 save_count = handlers_locker->CountLocks();
    while(handlers_locker->CountLocks() > 0) handlers_locker->Unlock();

    status_t retVal = bhapi::lock_locker_etc(fLocker, B_TIMEOUT, timeout);
    bhapi::delete_locker(locker);

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
    if(bhapi::count_locker_locks(fLocker) <= 0)
    {
        BHAPI_WARNING("[APP]: %s -- MessageQueue wasn't locked by current thread.", __PRETTY_FUNCTION__);
        return;
    }

    bhapi::unlock_locker(fLocker);
}


int32
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
    return((BMessage*)fMessagesList.RemoveItems(0,1));
}


BMessage*
BMessageQueue::FindMessage(int32 index) const
{
    return((BMessage*)fMessagesList.ItemAt(index));
}


BMessage*
BMessageQueue::FindMessage(uint32 what,  int32 fromIndex) const
{
    for(int32 i = fromIndex; i < fMessagesList.CountItems(); i++)
    {
        BMessage *msg = (BMessage*)fMessagesList.ItemAt(i);
        if(msg->what == what) return msg;
    }

    return NULL;
}


BMessage*
BMessageQueue::FindMessage(uint32 what,  int32 fromIndex,  int32 count) const
{
    for(int32 i = fromIndex, j = 0; i < fMessagesList.CountItems() && j < count; i++, j++)
    {
        BMessage *msg = (BMessage*)fMessagesList.ItemAt(i);
        if(msg->what == what) return msg;
    }

    return NULL;
}


int32
BMessageQueue::IndexOfMessage(BMessage *an_event) const
{
    return fMessagesList.IndexOf(an_event);
}

