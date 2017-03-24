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
 * File: Invoker.cpp
 *
 * --------------------------------------------------------------------------*/

#include "Invoker.h"
#include "Application.h"
#include "Messenger.h"
#include "Message.h"
#include "AppDefs.h"

#include "../kernel/KERNEL.h"
#include "../kernel/Debug.h"
#include "../support/Errors.h"
#include "../../private/app/PrivateHandler.h"

BInvoker::BInvoker()
    : fMessage(NULL), fReplyHandlerToken(B_MAXUINT64), fTimeout(B_INFINITE_TIMEOUT), fNotifyKind(B_CONTROL_INVOKED), fNotifyCalled(false)
{
    SetHandlerForReply(bhapi::be_app);
}

BInvoker::BInvoker(BMessage *message, const BHandler *handler, const BLooper *looper)
    : fMessage(NULL), fReplyHandlerToken(B_MAXUINT64), fTimeout(B_INFINITE_TIMEOUT), fNotifyKind(B_CONTROL_INVOKED), fNotifyCalled(false)
{
    fMessage = message;
    BMessenger msgr(handler, looper, NULL);
    fMessenger = msgr;

    SetHandlerForReply(bhapi::be_app);
}


BInvoker::BInvoker(BMessage *message, BMessenger target)
    : fMessage(NULL), fReplyHandlerToken(B_MAXUINT64), fTimeout(B_INFINITE_TIMEOUT), fNotifyKind(B_CONTROL_INVOKED), fNotifyCalled(false)
{
    fMessage = message;
    fMessenger = target;

    SetHandlerForReply(bhapi::be_app);
}


BInvoker::~BInvoker()
{
    if(fMessage) delete fMessage;
    if(fReplyHandlerToken != B_MAXUINT64) bhapi::unref_handler(fReplyHandlerToken);
    if(!fNotifyStatesList.IsEmpty())
    {
        for(int32 i = 0; i < fNotifyStatesList.CountItems(); i++)
        {
            bhapi::invoker_notify_state *state = (bhapi::invoker_notify_state*)fNotifyStatesList.ItemAt(i);
            if(state) delete state;
        }
        fNotifyStatesList.MakeEmpty();
    }
}

status_t
BInvoker::SetMessage(BMessage *message)
{
    if(fMessage) delete fMessage;
    fMessage = message;

    return B_OK;
}


BMessage*
BInvoker::Message() const
{
    return fMessage;
}


uint32
BInvoker::Command() const
{
    return fMessage ? fMessage->what : 0;
}


status_t
BInvoker::SetTarget(const BHandler *handler, const BLooper *looper)
{
    status_t status;
    BMessenger msgr(handler, looper, &status);
    if(status != B_OK && !(handler == NULL && looper == NULL)) return status;

    fMessenger = msgr;
    return B_OK;
}


status_t
BInvoker::SetTarget(BMessenger messenger)
{
    fMessenger = messenger;
    return B_OK;
}


bool
BInvoker::IsTargetLocal() const
{
    return fMessenger.IsTargetLocal();
}


BHandler*
BInvoker::Target(BLooper **looper) const
{
    return fMessenger.Target(looper);
}


BMessenger
BInvoker::Messenger() const
{
    return BMessenger(fMessenger);
}


status_t
BInvoker::SetHandlerForReply(const BHandler *handler)
{
    if(fReplyHandlerToken != B_MAXUINT64) bhapi::unref_handler(fReplyHandlerToken);

    fReplyHandlerToken = bhapi::get_ref_handler_token(handler);

    return B_OK;
}


BHandler*
BInvoker::HandlerForReply() const
{
    return(bhapi::get_handler(fReplyHandlerToken));
}


status_t
BInvoker::Invoke(const BMessage *msg)
{
    const BMessage *message = (msg ? msg : fMessage);
    if(!message) return B_BAD_VALUE;

    return fMessenger._SendMessage(message, fReplyHandlerToken, fTimeout);
}


status_t
BInvoker::InvokeNotify(const BMessage *msg)
{
return BInvoker::InvokeNotify(msg,B_CONTROL_INVOKED);
}

status_t
BInvoker::InvokeNotify(const BMessage *msg,  uint32 kind = B_CONTROL_INVOKED)
{
    status_t status = B_BAD_VALUE;

    if(msg)
    {
        BeginInvokeNotify(kind);
        status = Invoke(msg);
        EndInvokeNotify();
    }

    return status;
}


uint32
BInvoker::InvokeKind(bool* notify)
{
    if(notify) *notify = fNotifyCalled;
    return fNotifyKind;
}


void
BInvoker::BeginInvokeNotify(void)
{
BInvoker::BeginInvokeNotify(B_CONTROL_INVOKED);
}
void
BInvoker::BeginInvokeNotify(uint32 kind)
{
    bhapi::invoker_notify_state *state = new bhapi::invoker_notify_state;
    if(state)
    {
        state->kind = fNotifyKind;
        state->called = fNotifyCalled;
        if(!fNotifyStatesList.AddItem((void*)state)) delete state;
    }

    fNotifyKind = kind;
    fNotifyCalled = true;
}


void
BInvoker::EndInvokeNotify()
{
    fNotifyKind = B_CONTROL_INVOKED;
    fNotifyCalled = false;

    bhapi::invoker_notify_state *state = (bhapi::invoker_notify_state*)fNotifyStatesList.RemoveItem(fNotifyStatesList.CountItems() - 1);
    if(state)
    {
        fNotifyKind = state->kind;
        fNotifyCalled = state->called;
        delete state;
    }
}


status_t
BInvoker::SetTimeout(bigtime_t timeout)
{
    fTimeout = timeout;
    return B_OK;
}


bigtime_t
BInvoker::Timeout() const
{
    return fTimeout;
}

