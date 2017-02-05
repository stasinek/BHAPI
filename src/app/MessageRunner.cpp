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
 * File: MessageRunner.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../kernel/Kernel.h>
#include "./../support/Locker.h>
#include "./../support/Autolock.h>

#include "./../private/PrivateHandler.h>

#include "Application.h"
#include "MessageRunner.h"

extern ELocker* etk_get_handler_operator_locker();

EMessageRunner::EMessageRunner(const EMessenger &target, const EMessage *msg, e_bigtime_t interval, eint32 count)
	: fToken(-1), fTarget(NULL), fReplyTo(NULL), fMessage(NULL), fPrevSendTime(E_INT64_CONSTANT(-1))
{
	if(!(msg == NULL || (fMessage = new EMessage(*msg)) != NULL)) return;
	if(target.IsValid())
	{
		fTarget = new EMessenger(target);
		if(fTarget == NULL || fTarget->IsValid() == false) return;
	}
	fInterval = interval;
	fCount = count;

	ELocker *hLocker = etk_get_handler_operator_locker();
	EAutolock <ELocker>autolock(hLocker);

	eint32 token = EApplication::sRunnerList.IndexOf(NULL);
	if(token >= 0)
	{
		if(EApplication::sRunnerList.ReplaceItem(token, this) == false) return;
		fToken = token;
	}
	else
	{
		if(EApplication::sRunnerList.AddItem(this) == false) return;
		fToken = EApplication::sRunnerList.CountItems() - 1;
	}

	if(fCount != 0 && fInterval > E_INT64_CONSTANT(0) &&
	   !(fTarget == NULL || fTarget->IsValid() == false) && fMessage != NULL && etk_app != NULL)
	{
		EApplication::sRunnerMinimumInterval = E_INT64_CONSTANT(-1);
		etk_app->PostMessage(_EVENTS_PENDING_, etk_app);
	}
}


EMessageRunner::EMessageRunner(const EMessenger &target, const EMessage *msg, e_bigtime_t interval, eint32 count, const EMessenger &replyTo)
	: fToken(-1), fTarget(NULL), fReplyTo(NULL), fMessage(NULL), fPrevSendTime(E_INT64_CONSTANT(-1))
{
	if(!(msg == NULL || (fMessage = new EMessage(*msg)) != NULL)) return;
	if(target.IsValid())
	{
		fTarget = new EMessenger(target);
		if(fTarget == NULL || fTarget->IsValid() == false) return;
	}
	if(replyTo.IsValid())
	{
		fReplyTo = new EMessenger(replyTo);
		if(fReplyTo == NULL || fReplyTo->IsValid() == false) return;
	}
	fInterval = interval;
	fCount = count;

	ELocker *hLocker = etk_get_handler_operator_locker();
	EAutolock <ELocker>autolock(hLocker);

	eint32 token = EApplication::sRunnerList.IndexOf(NULL);
	if(token >= 0)
	{
		if(EApplication::sRunnerList.ReplaceItem(token, this) == false) return;
		fToken = token;
	}
	else
	{
		if(EApplication::sRunnerList.AddItem(this) == false) return;
		fToken = EApplication::sRunnerList.CountItems() - 1;
	}

	if(fCount != 0 && fInterval > E_INT64_CONSTANT(0) &&
	   !(fTarget == NULL || fTarget->IsValid() == false) && fMessage != NULL && etk_app != NULL)
	{
		EApplication::sRunnerMinimumInterval = E_INT64_CONSTANT(-1);
		etk_app->PostMessage(_EVENTS_PENDING_, etk_app);
	}
}


EMessageRunner::~EMessageRunner()
{
	ELocker *hLocker = etk_get_handler_operator_locker();
	EAutolock <ELocker>autolock(hLocker);

	if(fToken >= 0)
	{
		void *oldItem = NULL;
		bool removed = (fToken == EApplication::sRunnerList.CountItems() - 1 ?
					(EApplication::sRunnerList.RemoveItem(fToken) == (void*)this) :
					!(EApplication::sRunnerList.ReplaceItem(fToken, NULL, &oldItem) == false || oldItem != (void*)this));
		if(!removed)
			ETK_ERROR("[APP]: %s --- Unable to remove runner, it must something error.", __PRETTY_FUNCTION__);
	}
	if(fTarget) delete fTarget;
	if(fReplyTo) delete fReplyTo;
	if(fMessage) delete fMessage;
}


bool
EMessageRunner::IsValid() const
{
	return(fToken >= 0);
}


e_status_t
EMessageRunner::SetTarget(const EMessenger &target)
{
	if(fToken < 0) return E_ERROR;

	ELocker *hLocker = etk_get_handler_operator_locker();
	EAutolock <ELocker>autolock(hLocker);

	if(target.IsValid())
	{
		EMessenger *msgr = new EMessenger(target);
		if(msgr == NULL || msgr->IsValid() == false)
		{
			if(msgr) delete msgr;
			return E_ERROR;
		}
		if(fTarget) delete fTarget;
		fTarget = msgr;
	}
	else
	{
		if(fTarget) delete fTarget;
		fTarget = NULL;
	}

	fPrevSendTime = E_INT64_CONSTANT(-1);

	if(fCount != 0 && fInterval > E_INT64_CONSTANT(0) &&
	   !(fTarget == NULL || fTarget->IsValid() == false) && fMessage != NULL && etk_app != NULL)
	{
		EApplication::sRunnerMinimumInterval = E_INT64_CONSTANT(-1);
		etk_app->PostMessage(_EVENTS_PENDING_, etk_app);
	}

	return E_OK;
}


e_status_t
EMessageRunner::SetReplyTo(const EMessenger &replyTo)
{
	if(fToken < 0) return E_ERROR;

	ELocker *hLocker = etk_get_handler_operator_locker();
	EAutolock <ELocker>autolock(hLocker);

	if(replyTo.IsValid())
	{
		EMessenger *msgr = new EMessenger(replyTo);
		if(msgr == NULL || msgr->IsValid() == false)
		{
			if(msgr) delete msgr;
			return E_ERROR;
		}
		if(fReplyTo) delete fReplyTo;
		fReplyTo = msgr;
	}
	else
	{
		if(fReplyTo) delete fReplyTo;
		fReplyTo = NULL;
	}

	fPrevSendTime = E_INT64_CONSTANT(-1);

	return E_OK;
}


e_status_t
EMessageRunner::SetMessage(const EMessage *msg)
{
	EMessage *aMsg = NULL;
	if(fToken < 0 || !(msg == NULL || (aMsg = new EMessage(*msg)) != NULL)) return E_ERROR;

	ELocker *hLocker = etk_get_handler_operator_locker();
	EAutolock <ELocker>autolock(hLocker);

	if(fMessage) delete fMessage;
	fMessage = aMsg;

	fPrevSendTime = E_INT64_CONSTANT(-1);

	if(fCount != 0 && fInterval > E_INT64_CONSTANT(0) &&
	   !(fTarget == NULL || fTarget->IsValid() == false) && fMessage != NULL && etk_app != NULL)
	{
		EApplication::sRunnerMinimumInterval = E_INT64_CONSTANT(-1);
		etk_app->PostMessage(_EVENTS_PENDING_, etk_app);
	}

	return E_OK;
}


e_status_t
EMessageRunner::SetInterval(e_bigtime_t interval)
{
	if(fToken < 0) return E_ERROR;

	ELocker *hLocker = etk_get_handler_operator_locker();
	EAutolock <ELocker>autolock(hLocker);

	fInterval = interval;
	fPrevSendTime = E_INT64_CONSTANT(-1);

	if(fCount != 0 && fInterval > E_INT64_CONSTANT(0) &&
	   !(fTarget == NULL || fTarget->IsValid() == false) && fMessage != NULL && etk_app != NULL)
	{
		EApplication::sRunnerMinimumInterval = E_INT64_CONSTANT(-1);
		etk_app->PostMessage(_EVENTS_PENDING_, etk_app);
	}

	return E_OK;
}


e_status_t
EMessageRunner::SetCount(eint32 count)
{
	if(fToken < 0) return E_ERROR;

	ELocker *hLocker = etk_get_handler_operator_locker();
	EAutolock <ELocker>autolock(hLocker);

	fCount = count;
	fPrevSendTime = E_INT64_CONSTANT(-1);

	if(fCount != 0 && fInterval > E_INT64_CONSTANT(0) &&
	   !(fTarget == NULL || fTarget->IsValid() == false) && fMessage != NULL && etk_app != NULL)
	{
		EApplication::sRunnerMinimumInterval = E_INT64_CONSTANT(-1);
		etk_app->PostMessage(_EVENTS_PENDING_, etk_app);
	}

	return E_OK;
}


e_status_t
EMessageRunner::GetInfo(e_bigtime_t *interval, eint32 *count) const
{
	if(fToken < 0 || (!interval && !count)) return E_ERROR;

	ELocker *hLocker = etk_get_handler_operator_locker();
	EAutolock <ELocker>autolock(hLocker);

	if(interval) *interval = fInterval;
	if(count) *count = fCount;

	return E_OK;
}


e_status_t
EMessageRunner::GetInfo(EMessenger *target, EMessage *msg, e_bigtime_t *interval, eint32 *count, EMessenger *replyTo) const
{
	if(fToken < 0 || (!target && !msg && interval && !count && !replyTo)) return E_ERROR;

	ELocker *hLocker = etk_get_handler_operator_locker();
	EAutolock <ELocker>autolock(hLocker);

	if(target) *target = (fTarget ? *fTarget : EMessenger());
	if(replyTo) *replyTo = (fReplyTo ? *fReplyTo : EMessenger());
	if(msg) {msg->MakeEmpty(); msg->what = 0; if(fMessage) *msg = *fMessage;}
	if(interval) *interval = fInterval;
	if(count) *count = fCount;

	return E_OK;
}

