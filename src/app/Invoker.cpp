/* --------------------------------------------------------------------------
 *
 * ETK++ --- The Easy Toolkit for C++ programing
 * Copyright (C) 2004-2006, Anthony Lee, All Rights Reserved
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
 * File: Invoker.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../private/PrivateHandler.h"

#include "Application.h"
#include "Invoker.h"


typedef struct etk_invoker_notify_state {
	euint32 kind;
	bool called;
} etk_invoker_notify_state;


EInvoker::EInvoker()
	: fMessage(NULL), fReplyHandlerToken(E_MAXUINT64), fTimeout(E_INFINITE_TIMEOUT), fNotifyKind(E_CONTROL_INVOKED), fNotifyCalled(false)
{
	SetHandlerForReply(etk_app);
}


EInvoker::EInvoker(EMessage *message, const EHandler *handler, const ELooper *looper)
	: fMessage(NULL), fReplyHandlerToken(E_MAXUINT64), fTimeout(E_INFINITE_TIMEOUT), fNotifyKind(E_CONTROL_INVOKED), fNotifyCalled(false)
{
	fMessage = message;
	EMessenger msgr(handler, looper, NULL);
	fMessenger = msgr;

	SetHandlerForReply(etk_app);
}


EInvoker::EInvoker(EMessage *message, EMessenger target)
	: fMessage(NULL), fReplyHandlerToken(E_MAXUINT64), fTimeout(E_INFINITE_TIMEOUT), fNotifyKind(E_CONTROL_INVOKED), fNotifyCalled(false)
{
	fMessage = message;
	fMessenger = target;

	SetHandlerForReply(etk_app);
}


EInvoker::~EInvoker()
{
	if(fMessage) delete fMessage;
	if(fReplyHandlerToken != E_MAXUINT64) etk_unref_handler(fReplyHandlerToken);
	if(!fNotifyStatesList.IsEmpty())
	{
		for(eint32 i = 0; i < fNotifyStatesList.CountItems(); i++)
		{
			etk_invoker_notify_state *state = (etk_invoker_notify_state*)fNotifyStatesList.ItemAt(i);
			if(state) delete state;
		}
		fNotifyStatesList.MakeEmpty();
	}
}


e_status_t
EInvoker::SetMessage(EMessage *message)
{
	if(fMessage) delete fMessage;
	fMessage = message;

	return E_OK;
}


EMessage*
EInvoker::Message() const
{
	return fMessage;
}


euint32
EInvoker::Command() const
{
	return fMessage ? fMessage->what : 0;
}


e_status_t
EInvoker::SetTarget(const EHandler *handler, const ELooper *looper)
{
	e_status_t status;
	EMessenger msgr(handler, looper, &status);
	if(status != E_OK && !(handler == NULL && looper == NULL)) return status;

	fMessenger = msgr;
	return E_OK;
}


e_status_t
EInvoker::SetTarget(EMessenger messenger)
{
	fMessenger = messenger;
	return E_OK;
}


bool
EInvoker::IsTargetLocal() const
{
	return fMessenger.IsTargetLocal();
}


EHandler*
EInvoker::Target(ELooper **looper) const
{
	return fMessenger.Target(looper);
}


EMessenger
EInvoker::Messenger() const
{
	return EMessenger(fMessenger);
}


e_status_t
EInvoker::SetHandlerForReply(const EHandler *handler)
{
	if(fReplyHandlerToken != E_MAXUINT64) etk_unref_handler(fReplyHandlerToken);

	fReplyHandlerToken = etk_get_ref_handler_token(handler);

	return E_OK;
}


EHandler*
EInvoker::HandlerForReply() const
{
	return(etk_get_handler(fReplyHandlerToken));
}


e_status_t
EInvoker::Invoke(const EMessage *msg)
{
	const EMessage *message = (msg ? msg : fMessage);
	if(!message) return E_BAD_VALUE;

	return fMessenger._SendMessage(message, fReplyHandlerToken, fTimeout);
}


e_status_t
EInvoker::InvokeNotify(const EMessage *msg, euint32 kind)
{
	e_status_t status = E_BAD_VALUE;

	if(msg)
	{
		BeginInvokeNotify(kind);
		status = Invoke(msg);
		EndInvokeNotify();
	}

	return status;
}


euint32
EInvoker::InvokeKind(bool* notify)
{
	if(notify) *notify = fNotifyCalled;
	return fNotifyKind;
}


void
EInvoker::BeginInvokeNotify(euint32 kind)
{
	etk_invoker_notify_state *state = new etk_invoker_notify_state;
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
EInvoker::EndInvokeNotify()
{
	fNotifyKind = E_CONTROL_INVOKED;
	fNotifyCalled = false;

	etk_invoker_notify_state *state = (etk_invoker_notify_state*)fNotifyStatesList.RemoveItem(fNotifyStatesList.CountItems() - 1);
	if(state)
	{
		fNotifyKind = state->kind;
		fNotifyCalled = state->called;
		delete state;
	}
}


e_status_t
EInvoker::SetTimeout(e_bigtime_t timeout)
{
	fTimeout = timeout;
	return E_OK;
}


e_bigtime_t
EInvoker::Timeout() const
{
	return fTimeout;
}

