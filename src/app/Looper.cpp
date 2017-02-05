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
 * File: Looper.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdlib.h>

#include "./../kernel/Kernel.h>
#include "./../support/Locker.h>
#include "./../support/Autolock.h>
#include "./../support/ClassInfo.h>

#include "./../private/Token.h>
#include "./../private/PrivateHandler.h>

#include "AppDefs.h"
#include "Application.h"
#include "Messenger.h"
#include "Looper.h"


EList ELooper::sLooperList;


ELooper::ELooper(const char *name, eint32 priority)
	: EHandler(name), fDeconstructing(false), fProxy(NULL), fHandlersCount(1), fPreferredHandler(NULL), fLocker(NULL), fLocksCount(E_INT64_CONSTANT(0)), fThread(NULL), fSem(NULL), fMessageQueue(NULL), fCurrentMessage(NULL), fThreadExited(NULL)
{
	ELocker *hLocker = etk_get_handler_operator_locker();
	EAutolock <ELocker>autolock(hLocker);

	if((fLocker = etk_create_locker()) == NULL)
		ETK_ERROR("[APP]: %s --- Unable to create locker for looper.", __PRETTY_FUNCTION__);

	fPrevHandler = fNextHandler = this;
	fLooper = this;

	fMessageQueue = new EMessageQueue();
	if(fMessageQueue) fSem = etk_create_sem(E_INT64_CONSTANT(0), NULL);

	fThreadPriority = priority;

	sLooperList.AddItem(this);
}


ELooper::~ELooper()
{
	for(EHandler *handler = HandlerAt(1); RemoveHandler(handler); handler = HandlerAt(1)) delete handler;

	if(fProxy) ProxyBy(NULL);
	while(fClients.CountItems() > 0)
	{
		ELooper *client = (ELooper*)fClients.ItemAt(0);
		client->Lock();
		client->ProxyBy(NULL);
		client->Unlock();
	}

	while(fCommonFilters.CountItems() > 0)
	{
		EMessageFilter *filter = (EMessageFilter*)fCommonFilters.ItemAt(0);
		ELooper::RemoveCommonFilter(filter);
		delete filter;
	}

	ELocker *hLocker = etk_get_handler_operator_locker();
	EAutolock <ELocker>autolock(hLocker);

	if(fMessageQueue) delete fMessageQueue;
	if(fSem) etk_delete_sem(fSem);
	if(fCurrentMessage) delete fCurrentMessage;
	if(fThread) etk_delete_thread(fThread);

	sLooperList.RemoveItem(this);

	if(EHandler::fToken != NULL) EHandler::fToken->MakeEmpty();

	if(fLocker)
	{
		etk_close_locker(fLocker);
		etk_delete_locker(fLocker);
	}
}


ELooper::ELooper(const EMessage *from)
	: EHandler(from), fDeconstructing(false), fProxy(NULL), fThreadPriority(E_NORMAL_PRIORITY), fHandlersCount(1), fPreferredHandler(NULL), fLocker(NULL), fLocksCount(E_INT64_CONSTANT(0)), fThread(NULL), fSem(NULL), fMessageQueue(NULL), fCurrentMessage(NULL), fThreadExited(NULL)
{
	ELocker *hLocker = etk_get_handler_operator_locker();
	EAutolock <ELocker>autolock(hLocker);

	if((fLocker = etk_create_locker()) == NULL)
		ETK_ERROR("[APP]: %s --- Unable to create locker for looper.", __PRETTY_FUNCTION__);

	fPrevHandler = fNextHandler = this;
	fLooper = this;

	fMessageQueue = new EMessageQueue();
	if(fMessageQueue) fSem = etk_create_sem(E_INT64_CONSTANT(0), NULL);

	sLooperList.AddItem(this);
}


e_status_t
ELooper::Archive(EMessage *into, bool deep) const
{
	if(!into) return E_ERROR;

	EHandler::Archive(into, deep);
	into->AddString("class", "ELooper");

	// TODO

	return E_OK;
}


EArchivable*
ELooper::Instantiate(const EMessage *from)
{
	if(e_validate_instantiation(from, "ELooper"))
		return new ELooper(from);
	return NULL;
}


void
ELooper::AddHandler(EHandler *handler)
{
	if(handler == NULL || handler->fLooper != NULL || fHandlersCount == E_MAXINT32) return;

	handler->fLooper = this;

	fPrevHandler->fNextHandler = handler;
	handler->fPrevHandler = fPrevHandler;
	handler->fNextHandler = this;

	fHandlersCount += 1;
	handler->SetNextHandler(this);
}


bool
ELooper::RemoveHandler(EHandler *handler)
{
	if(handler == NULL || handler == this) return false;
	if(handler->fLooper != this) return false;

	handler->fPrevHandler->fNextHandler = handler->fNextHandler;
	handler->fNextHandler->fPrevHandler = handler->fPrevHandler;
	handler->fPrevHandler = handler->fNextHandler = NULL;

	fHandlersCount -= 1;

	handler->fLooper = NULL;
	handler->SetNextHandler(NULL);

	return true;
}


eint32
ELooper::CountHandlers() const
{
	return fHandlersCount;
}


EHandler*
ELooper::HandlerAt(eint32 index) const
{
	if(index >= fHandlersCount) return NULL;

	if(index == 0) return const_cast<EHandler*>(e_cast_as(this, const EHandler));
	if(index == 1) return fNextHandler;
	if(index == fHandlersCount - 1) return fPrevHandler;

	EHandler *handler = fNextHandler;
	while(index-- > 1) handler = handler->fNextHandler;
	return handler;
}


eint32
ELooper::IndexOf(EHandler *handler) const
{
	if(handler == NULL || handler->fLooper != this) return -1;

	eint32 index = 0;

	const EHandler *found = this;
	while(found != handler)
	{
		found = found->fNextHandler;
		index++;
	}

	return index;
}


EHandler*
ELooper::PreferredHandler() const
{
	return fPreferredHandler;
}


void
ELooper::SetPreferredHandler(EHandler *handler)
{
	if(handler) if(handler->Looper() != this) return;
	fPreferredHandler = handler;
}


bool
ELooper::Lock()
{
	return(LockWithTimeout(E_INFINITE_TIMEOUT) == E_OK);
}


void
ELooper::Unlock()
{
	if(IsLockedByCurrentThread())
	{
		fLocksCount--;
		etk_unlock_locker(fLocker);
	}
	else
	{
		ETK_WARNING("[APP]: %s -- Looper wasn't locked by current thread.", __PRETTY_FUNCTION__);
	}
}


e_status_t
ELooper::LockWithTimeout(e_bigtime_t microseconds_timeout)
{
	euint64 token = etk_get_handler_token(this);

	e_status_t retVal = etk_lock_looper_of_handler(token, microseconds_timeout);

	if(retVal == E_OK) fLocksCount++;

	return retVal;
}


eint64
ELooper::CountLocks() const
{
	return fLocksCount;
}


bool
ELooper::IsLockedByCurrentThread() const
{
	return(CountLocks() > E_INT64_CONSTANT(0));
}


e_status_t
ELooper::PostMessage(euint32 command)
{
	EMessage msg(command);
	return PostMessage(&msg, this, NULL);
}


e_status_t
ELooper::PostMessage(const EMessage *message)
{
	return PostMessage(message, this, NULL);
}


e_status_t
ELooper::PostMessage(euint32 command, EHandler *handler, EHandler *reply_to)
{
	EMessage msg(command);
	return PostMessage(&msg, handler, reply_to);
}


e_status_t
ELooper::PostMessage(const EMessage *_message, EHandler *handler, EHandler *reply_to)
{
	if(_message == NULL)
	{
		ETK_WARNING("[APP]: %s --- Can't post empty message.", __PRETTY_FUNCTION__);
		return E_BAD_VALUE;
	}

	euint64 handlerToken = etk_get_handler_token(handler);
	euint64 replyToken = etk_get_handler_token(reply_to);

	EMessage aMsg(*_message);
	aMsg.fIsReply = false;
	if(aMsg.fSource != NULL)
	{
		etk_delete_port(aMsg.fSource);
		aMsg.fSource = NULL;
	}

	return _PostMessage(&aMsg, handlerToken, replyToken, E_INFINITE_TIMEOUT);
}


e_status_t
ELooper::_PostMessage(const EMessage *_message, euint64 handlerToken, euint64 replyToken, e_bigtime_t timeout)
{
	if(fMessageQueue == NULL || _message == NULL) return E_ERROR;

	euint64 selfToken = etk_get_handler_token(this);
	e_bigtime_t handlerTokenTimestamp = etk_get_handler_create_time_stamp(handlerToken);
	e_bigtime_t replyTokenTimestamp = etk_get_handler_create_time_stamp(replyToken);

	if(fMessageQueue->LockWithTimeout(timeout) != E_OK) return E_ERROR;

	e_status_t retVal = E_ERROR;

	if(fSem != NULL)
	{
		if(_message->what == _EVENTS_PENDING_ && handlerToken == selfToken)
		{
			retVal = (_message->fNoticeSource ? E_ERROR : E_OK);
		}
		else
		{
			EMessage *message = new EMessage(*_message);

			message->fTeam = etk_get_current_team_id();
			message->fTargetToken = handlerToken;
			message->fTargetTokenTimestamp = handlerTokenTimestamp;

			if(replyToken != E_MAXUINT64)
			{
				message->fReplyToken = replyToken;
				message->fReplyTokenTimestamp = replyTokenTimestamp;
				if(message->fSource)
				{
					etk_delete_port(message->fSource);
					message->fSource = NULL;
				}
			}

			if(fMessageQueue->AddMessage(message))
			{
				message->fNoticeSource = _message->fNoticeSource;
				retVal = E_OK;
			}
		}

		etk_release_sem(fSem);
	}

	fMessageQueue->Unlock();

	return retVal;
}


void
ELooper::DispatchMessage(EMessage *msg, EHandler *target)
{
	if(target == NULL) target = fPreferredHandler;
	if(!target || target->Looper() != this) return;

	if(msg->what == E_QUIT_REQUESTED && target == this)
	{
		if(QuitRequested()) PostMessage(_QUIT_);
	}
	else
	{
		target->MessageReceived(msg);
	}
}


void
ELooper::MessageReceived(EMessage *msg)
{
}


bool
ELooper::IsRunning() const
{
	ELocker *hLocker = etk_get_handler_operator_locker();
	EAutolock <ELocker>autolock(hLocker);

	if(fProxy != NULL) return _Proxy()->IsRunning();
	if(fThread == NULL || etk_get_thread_run_state(fThread) == ETK_THREAD_READY) return false;

	return true;
}


void*
ELooper::Run()
{
	ELocker *hLocker = etk_get_handler_operator_locker();
	EAutolock <ELocker>autolock(hLocker);

	if(fProxy)
	{
		ETK_WARNING("[APP]: %s --- The Looper has proxy, run aborted.", __PRETTY_FUNCTION__);
		return NULL;
	}

	if(!fThread)
	{
		if((fThread = etk_create_thread(_task, fThreadPriority, this, NULL)) == NULL)
			ETK_ERROR("[APP]: %s -- Unable to create thread!", __PRETTY_FUNCTION__);
	}
	else if(etk_get_thread_run_state(fThread) != ETK_THREAD_READY)
	{
		ETK_ERROR("[APP]: %s --- Thread must run only one time!", __PRETTY_FUNCTION__);
	}

	if(etk_resume_thread(fThread) == E_OK)
		return fThread;
	else
		return NULL;
}


bool
ELooper::QuitRequested()
{
	return true;
}


void
ELooper::Quit()
{
	if(!IsLockedByCurrentThread())
		ETK_ERROR("[APP]: %s --- Looper must LOCKED before this call!", __PRETTY_FUNCTION__);

	if(fDeconstructing) return;

	ELocker *hLocker = etk_get_handler_operator_locker();
	hLocker->Lock();
	if(etk_get_thread_id(fThread) == etk_get_current_thread_id() && Proxy() == this)
		ETK_ERROR("\n\
**************************************************************************\n\
*                           [APP]: ELooper                               *\n\
*                                                                        *\n\
*      Task must call \"PostMessage(E_QUIT_REQUESTED)\" instead of         *\n\
*      \"Quit()\" within the looper!!!                                     *\n\
*                                                                        *\n\
**************************************************************************\n\n");
	void *thread = NULL;
	if(fThread)
	{
		if((thread = etk_open_thread(etk_get_thread_id(fThread))) == NULL)
			ETK_ERROR("[APP]: %s --- Unable to duplicate the thread!", __PRETTY_FUNCTION__);
	}
	hLocker->Unlock();

	if(thread)
	{
		fDeconstructing = true;

		if(PostMessage(_QUIT_) != E_OK)
			ETK_ERROR("[APP]: %s --- Send \"_QUIT_\" to looper error!", __PRETTY_FUNCTION__);
		euint64 token = etk_get_handler_token(this);
		fLocksCount = E_INT64_CONSTANT(0);
		eint64 locksCount = etk_count_locker_locks(fLocker);
		while((locksCount--) > E_INT64_CONSTANT(0)) etk_unlock_locker(fLocker);

		e_status_t status;
		etk_wait_for_thread(thread, &status);
		if(etk_get_thread_run_state(thread) != ETK_THREAD_EXITED)
			if(etk_lock_looper_of_handler(token, E_INFINITE_TIMEOUT) == E_OK) delete this;
		etk_delete_thread(thread);
	}
	else
	{
		delete this;
	}
}


void
ELooper::_taskError(void *data)
{
	bool *exited = (bool*)data;
	bool showError = (!exited || *exited == false);
	if(exited) delete exited;
	if(showError)
	{
		ETK_ERROR("\n\
**************************************************************************\n\
*                           [APP]: ELooper                               *\n\
*                                                                        *\n\
*      Task must call \"PostMessage(E_QUIT_REQUESTED)\" instead of         *\n\
*      \"etk_exit_thread\" within the looper!!!                            *\n\
*                                                                        *\n\
**************************************************************************\n\n");
	}
}


EHandler*
ELooper::_MessageTarget(const EMessage *msg, bool *preferred)
{
	if(msg == NULL || msg->fTeam != etk_get_current_team_id()) return NULL;
	EHandler *handler = NULL;
	if(etk_ref_handler(msg->fTargetToken))
	{
		if(etk_get_handler_create_time_stamp(msg->fTargetToken) == msg->fTargetTokenTimestamp &&
		   etk_get_handler_looper(msg->fTargetToken) == this) handler = etk_get_handler(msg->fTargetToken);
		etk_unref_handler(msg->fTargetToken);
	}
	if(preferred) *preferred = (msg->fTargetToken == E_MAXUINT64);
	return handler;
}


e_status_t
ELooper::_task(void *arg)
{
	ELooper *self = (ELooper*)arg;
	if(self == NULL) return E_ERROR;

	void *sem = etk_clone_sem_by_source(self->fSem);
	if(!sem) return E_ERROR;

	bool *threadExited = new bool;
	if(!threadExited)
	{
		etk_delete_sem(sem);
		return E_NO_MEMORY;
	}
	*threadExited = false;

	if(etk_on_exit_thread(_taskError, (void*)threadExited) != E_OK)
	{
		delete threadExited;
		etk_delete_sem(sem);
		return E_ERROR;
	}
	self->fThreadExited = threadExited;

	if(etk_on_exit_thread((void (*)(void*))etk_delete_sem, sem) != E_OK)
	{
		*threadExited = true;
		etk_delete_sem(sem);
		return E_ERROR;
	}

	e_status_t status = _taskLooper(self, sem);

	*threadExited = true;

	return status;
}


ELooper*
ELooper::_GetNextClient(ELooper *client) const
{
	if(client == NULL || _Proxy() != client->_Proxy()) return NULL;
	if(client->fClients.CountItems() > 0) return (ELooper*)client->fClients.FirstItem();
	if(client->fProxy)
	{
		if(client->fProxy->fClients.LastItem() == (void*)client)
		{
			ELooper *retVal = NULL;
			while(true)
			{
				client = client->fProxy;
				if(client == NULL || client->fProxy == NULL) break;
				retVal = (ELooper*)client->fProxy->fClients.ItemAt(client->fProxy->fClients.IndexOf(client) + 1);
				if(retVal != NULL) break;
			}
			return retVal;
		}
		return((ELooper*)client->fProxy->fClients.ItemAt(client->fProxy->fClients.IndexOf(client) + 1));
	}

	return NULL;
}


e_status_t
ELooper::_taskLooper(ELooper *self, void *sem)
{
	ELocker *hLocker = etk_get_handler_operator_locker();

	hLocker->Lock();

	if(self == NULL || self->fThread == NULL || etk_get_thread_id(self->fThread) != etk_get_current_thread_id() || sem == NULL)
	{
		hLocker->Unlock();
		return E_ERROR;
	}

	hLocker->Unlock();

	euint8 flags = 0; // 0 --- normal, 1 --- continue, >= 2 --- break
	ELooper *looper = NULL;
	EMessageQueue *queue = NULL;
	while(flags < 2)
	{
		looper = self;

		self->Lock();
		queue = self->fMessageQueue;
		while(looper != NULL && queue != NULL)
		{
			EMessage *aMsg = NULL;

			queue->Lock();
			if(queue->IsEmpty() == false)
			{
				aMsg = queue->FindMessage((eint32)0);
				if(aMsg->what == _QUIT_)
				{
					queue->Unlock();
					flags = (looper == self ? 2 : 1);

					looper->Lock();
					if(looper->fDeconstructing == false)
					{
						looper->fDeconstructing = true;
						looper->Quit();
					}
					delete looper;

					break;
				}
				aMsg = queue->NextMessage();
			}
			queue->Unlock();

			if(aMsg)
			{
				bool preferred = false;
				EHandler *handler = looper->_MessageTarget(aMsg, &preferred);
				if(handler == NULL && !preferred)
				{
					delete aMsg;
					continue;
				}

				looper->Lock();
				looper->_FilterAndDispatchMessage(aMsg, handler);
				bool isClient = (looper->Proxy() == self ? true : false);
				looper->Unlock();

				if(isClient) continue;

				flags = 1;
				break;
			}

			looper = self->_GetNextClient(looper);
			queue = (looper == NULL ? NULL : looper->fMessageQueue);
		}

		if(flags >= 2) break;

		self->Unlock();
		if(flags > 0)
		{
			if(flags == 1) flags = 0;
			continue;
		}

		etk_sem_info sem_info;
		if(etk_acquire_sem(sem) != E_OK || etk_get_sem_info(sem, &sem_info) != E_OK) sem_info.closed = true;

		if(sem_info.closed) break;
	}

	return E_OK;
}


EMessage*
ELooper::NextLooperMessage(e_bigtime_t timeout)
{
	e_bigtime_t prevTime = etk_real_time_clock_usecs();

	if(!IsLockedByCurrentThread())
		ETK_ERROR("[APP]: %s --- Looper must LOCKED before this call!", __PRETTY_FUNCTION__);

	ELocker *hLocker = etk_get_handler_operator_locker();
	hLocker->Lock();
	void *sem = NULL;
	ELooper *proxy = _Proxy();
	if(proxy == NULL || proxy->fThread == NULL || etk_get_thread_id(proxy->fThread) != etk_get_current_thread_id() ||
	   (sem = etk_clone_sem_by_source(proxy->fSem)) == NULL)
	{
		hLocker->Unlock();
		return NULL;
	}
	hLocker->Unlock();

	euint8 flags = 0; // <= 0 --- normal, 1 --- continue, >= 2 --- break
	ELooper *looper = NULL;
	EMessageQueue *queue = NULL;
	EMessage *retVal = NULL;

	while(true)
	{
		looper = proxy;

		proxy->Lock();
		queue = proxy->fMessageQueue;
		while(looper != NULL && queue != NULL)
		{
			if(proxy == etk_app) EApplication::etk_dispatch_message_runners();

			EMessage *aMsg = NULL;

			queue->Lock();
			if(queue->IsEmpty() == false)
			{
				aMsg = queue->FindMessage((eint32)0);
				if(aMsg->what == _QUIT_)
				{
					queue->Unlock();
					flags = ((looper == proxy || looper == this) ? 2 : 1);

					if(flags == 1)
					{
						looper->Lock();
						if(looper->fLocksCount == E_INT64_CONSTANT(1))
						{
							if(looper->fDeconstructing == false)
							{
								looper->fDeconstructing = true;
								looper->Quit();
							}
							delete looper;
						}
						else
						{
							looper->Unlock();
							looper = proxy->_GetNextClient(looper);
							queue = (looper == NULL ? NULL : looper->fMessageQueue);
							continue;
						}
					}

					break;
				}
				aMsg = queue->NextMessage();
			}
			queue->Unlock();

			if(aMsg)
			{
				bool preferred = false;
				EHandler *handler = looper->_MessageTarget(aMsg, &preferred);
				if(handler == NULL && !preferred)
				{
					delete aMsg;
					continue;
				}

				if(looper == this)
				{
					retVal = aMsg;
					flags = 2;
					break;
				}

				looper->Lock();
				looper->_FilterAndDispatchMessage(aMsg, handler);
				bool isClient = (looper->Proxy() == proxy ? true : false);
				looper->Unlock();

				if(isClient && Proxy() == proxy) continue;

				flags = 1;
				break;
			}

			looper = proxy->_GetNextClient(looper);
			queue = (looper == NULL ? NULL : looper->fMessageQueue);
		}
		proxy->Unlock();

		if(flags >= 2) break;
		if(Proxy() != proxy)
		{
			etk_release_sem(sem); // to push back semaphore
			break;
		}
		if(flags == 1)
		{
			flags = 0;
			continue;
		}

		etk_sem_info sem_info;
		e_status_t status = E_ERROR;
		e_bigtime_t waitTime = timeout;
		if(timeout >= E_INT64_CONSTANT(0))
		{
			if(proxy == etk_app)
			{
				hLocker->Lock();
				waitTime = min_c(timeout, (EApplication::sRunnerMinimumInterval == E_INT64_CONSTANT(0) ?
								E_INFINITE_TIMEOUT :
								max_c(EApplication::sRunnerMinimumInterval, E_INT64_CONSTANT(50000))));
				hLocker->Unlock();
			}

			status = etk_acquire_sem_etc(sem, E_INT64_CONSTANT(1), E_TIMEOUT, waitTime);
		}
		if(etk_get_sem_info(sem, &sem_info) != E_OK) sem_info.closed = true;

		if(sem_info.closed || !(status == E_OK || status == E_TIMED_OUT)) break;
		if(status == E_TIMED_OUT && waitTime == timeout) break;
		if(timeout != E_INFINITE_TIMEOUT)
		{
			e_bigtime_t curTime = etk_real_time_clock_usecs();
			timeout -= (curTime - prevTime);
			prevTime = curTime;
		}
	}

	etk_delete_sem(sem);
	return retVal;
}


void
ELooper::DispatchLooperMessage(EMessage *msg)
{
	if(msg == NULL) return;

	if(!IsLockedByCurrentThread())
		ETK_ERROR("[APP]: %s --- Looper must LOCKED before this call!", __PRETTY_FUNCTION__);
	if(Thread() != etk_get_current_thread_id())
		ETK_ERROR("[APP]: %s --- Looper must call this within the task of looper!", __PRETTY_FUNCTION__);

	bool preferred = false;
	EHandler *handler = _MessageTarget(msg, &preferred);
	if(handler == NULL && !preferred)
	{
		delete msg;
		return;
	}

	_FilterAndDispatchMessage(msg, handler);
}


void
ELooper::_FilterAndDispatchMessage(EMessage *msg, EHandler *_target)
{
	if(msg == NULL) return;

	e_filter_result status = E_DISPATCH_MESSAGE;
	EHandler *handler = _target;

	if(msg->what != _QUIT_) // (!(msg->what == E_QUIT_REQUESTED || msg->what == _QUIT_))
	{
		for(eint32 i = 0; i < fCommonFilters.CountItems(); i++)
		{
			EMessageFilter *filter = (EMessageFilter*)fCommonFilters.ItemAt(i);
			if((status = filter->doFilter(msg, &handler)) == E_SKIP_MESSAGE) break;
		}

		EHandler *target = (handler == NULL ? fPreferredHandler : handler);
		if(!(status == E_SKIP_MESSAGE || target == NULL || target->fFilters == NULL))
		{
			for(eint32 i = 0; i < target->fFilters->CountItems(); i++)
			{
				EMessageFilter *filter = (EMessageFilter*)target->fFilters->ItemAt(i);
				if((status = filter->doFilter(msg, &handler)) == E_SKIP_MESSAGE) break;
			}
		}

		if(status == E_SKIP_MESSAGE)
		{
			delete msg;
			return;
		}
	}

	EMessage *oldMsg = fCurrentMessage;
	fCurrentMessage = msg;
	DispatchMessage(msg, handler);
	if(fCurrentMessage != NULL) delete fCurrentMessage;
	fCurrentMessage = oldMsg;
}


EMessage*
ELooper::CurrentMessage() const
{
	if(!fCurrentMessage || !fMessageQueue) return NULL;
	if(Thread() != etk_get_current_thread_id()) return NULL;

	return fCurrentMessage;
}


EMessage*
ELooper::DetachCurrentMessage()
{
	if(!fCurrentMessage || !fMessageQueue) return NULL;
	if(Thread() != etk_get_current_thread_id()) return NULL;

	EMessage *msg = fCurrentMessage;
	fCurrentMessage = NULL;

	return msg;
}


EMessageQueue*
ELooper::MessageQueue() const
{
	return fMessageQueue;
}


eint64
ELooper::Thread() const
{
	return etk_get_thread_id(Proxy()->fThread);
}


ELooper*
ELooper::_Proxy() const
{
	if(fProxy == NULL) return (ELooper*)this;
	return fProxy->_Proxy();
}


ELooper*
ELooper::Proxy() const
{
	ELocker *hLocker = etk_get_handler_operator_locker();
	EAutolock <ELocker>autolock(hLocker);

	return _Proxy();
}


// Thread safe method like below:
// 	{
// 		ELooper *proxy;
// 		ELooper *looper;
// 		...
// 		looper->Lock();
// 		if(looper->ProxyBy(NULL)) // To clean previous proxy and unlock the original locker automatically.
// 		{
// 			proxy->Lock(); // At the time, looper has alone locker, so thus prevent to be a dead lock.
// 			looper->ProxyBy(proxy);
// 			proxy->Unlock();
// 		}
// 		looper->Unlock();
// 		...
// 	}
bool
ELooper::ProxyBy(ELooper *proxy)
{
	if(e_is_kind_of(this, EApplication))
	{
		ETK_WARNING("[APP]: %s --- Application can't proxy by other looper.", __PRETTY_FUNCTION__);
		return(proxy ? false : true);
	}

	if(IsLockedByCurrentThread() == false)
		ETK_ERROR("[APP]: %s --- Looper must LOCKED before this call!", __PRETTY_FUNCTION__);

	ELocker *hLocker = etk_get_handler_operator_locker();
	EAutolock <ELocker>autolock(hLocker);

	if(proxy == NULL ? (_Proxy() == this) : (_Proxy() == proxy->_Proxy())) return true;

	if(fThread)
	{
		ETK_WARNING("[APP]: %s --- The looper already run, proxy aborted.", __PRETTY_FUNCTION__);
		return false;
	}
	else if(!(proxy == NULL || proxy->IsLockedByCurrentThread()))
	{
		ETK_ERROR("[APP]: %s --- Proxy must LOCKED before this call!", __PRETTY_FUNCTION__);
	}

	return _ProxyBy(proxy);
}


bool
ELooper::_ProxyBy(ELooper *proxy)
{
	if(proxy != fProxy)
	{
		if(proxy) if(proxy->fClients.AddItem(this) == false) return false;
		if(fProxy) fProxy->fClients.RemoveItem((void*)this);
	}

	ELooper *oldProxy = fProxy;
	if(proxy == NULL)
	{
		fProxy = NULL;

		fMessageQueue->Lock();
		if(fSem) etk_delete_sem(fSem);
		fSem = etk_create_sem((eint64)fMessageQueue->CountMessages(), NULL);
		fMessageQueue->Unlock();

		void *newLocker = NULL;
		if((newLocker = etk_create_locker()) == NULL)
			ETK_ERROR("[APP]: %s --- Unable to create locker for looper.", __PRETTY_FUNCTION__);
		for(eint64 i = E_INT64_CONSTANT(0); i < fLocksCount; i++) etk_lock_locker(newLocker);
		void *oldLocker = fLocker;
		fLocker = newLocker;

		for(eint32 i = 0; i < fClients.CountItems(); i++)
		{
			ELooper *looper = (ELooper*)fClients.ItemAt(i);
			looper->_ProxyBy(this);
		}

		for(eint64 i = E_INT64_CONSTANT(0); i < fLocksCount; i++) etk_unlock_locker(oldLocker);
		etk_delete_locker(oldLocker);
	}
	else
	{
		fProxy = proxy;

		fMessageQueue->Lock();
		if(fSem) etk_delete_sem(fSem);
		fSem = etk_clone_sem_by_source(proxy->fSem);
		if(fMessageQueue->CountMessages() > 0)
			etk_release_sem_etc(fSem, (eint64)fMessageQueue->CountMessages(), 0);
		fMessageQueue->Unlock();

		void *newLocker = NULL;
		if((newLocker = etk_clone_locker(proxy->fLocker)) == NULL)
			ETK_ERROR("[APP]: %s --- Unable to create locker for looper.", __PRETTY_FUNCTION__);
		for(eint64 i = E_INT64_CONSTANT(0); i < fLocksCount; i++) etk_lock_locker(newLocker);
		void *oldLocker = fLocker;
		fLocker = newLocker;

		for(eint32 i = 0; i < fClients.CountItems(); i++)
		{
			ELooper *looper = (ELooper*)fClients.ItemAt(i);
			looper->_ProxyBy(this);
		}

		if(!oldProxy) etk_close_locker(oldLocker);
		else for(eint64 i = E_INT64_CONSTANT(0); i < fLocksCount; i++) etk_unlock_locker(oldLocker);
		etk_delete_locker(oldLocker);
	}

	return true;
}


ELooper*
ELooper::LooperForThread(e_thread_id tid)
{
	void *thread = etk_open_thread(tid);
	if(thread == NULL) return NULL; // invalid id

	ELocker *hLocker = etk_get_handler_operator_locker();
	EAutolock <ELocker>autolock(hLocker);

	etk_delete_thread(thread);

	for(eint32 i = 0; i < sLooperList.CountItems(); i++)
	{
		ELooper *looper = (ELooper*)sLooperList.ItemAt(i);
		if(etk_get_thread_id(looper->fThread) == tid) return looper;
	}

	return NULL;
}


bool
ELooper::AddCommonFilter(EMessageFilter *filter)
{
	if(filter == NULL || filter->fHandler != NULL || fCommonFilters.AddItem(filter) == false) return false;
	filter->fHandler = this;
	return true;
}


bool
ELooper::RemoveCommonFilter(EMessageFilter *filter)
{
	if(filter == NULL || filter->fHandler != this || fCommonFilters.RemoveItem(filter) == false) return false;
	filter->fHandler = NULL;
	return true;
}


const EList*
ELooper::CommonFilterList() const
{
	return(&fCommonFilters);
}


bool
ELooper::SetCommonFilterList(const EList *filterList)
{
	while(fCommonFilters.CountItems() > 0)
	{
		EMessageFilter *filter = (EMessageFilter*)fCommonFilters.ItemAt(0);
		ELooper::RemoveCommonFilter(filter);
		delete filter;
	}

	if(filterList != NULL)
	{
		for(eint32 i = 0; i < filterList->CountItems(); i++) AddCommonFilter((EMessageFilter*)filterList->ItemAt(i));
	}

	return true;
}


bool
ELooper::IsDependsOnOthersWhenQuitRequested() const
{
	return false;
}

