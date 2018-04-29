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
 * File: Looper.cpp
 *
 * --------------------------------------------------------------------------*/

#include <kits/netLooper.h>
#include <kits/netAppDefs.h>
#include <kits/netApplication.h>
#include <kits/app/Messenger.h>
#include <kits/app/Handler.h>
#include <kits/netMessageQueue.h>
#include <kits/netMessageFilter.h>
#include <kits/netMessage.h>
#include <kits/netAppDefs.h>

#include <os/debug.h>
#include <os/kernel.h>
#include <os/kernel.h>
#include <kits/support/Locker.h>
#include <kits/debug/Errors.h>
#include <kits/support/Autolock.h>
#include <kits/support/ClassInfo.h>
#include <kits/private/support/Token.h>
#include <kits/private/app/PrivateHandler.h>




#ifdef BHAPI_BUILD_LIBRARY
BHAPI_EXPORT BList BLooper::sLooperList;
#endif // BHAPI_BUILD_LIBRARY

/*BLooper::BLooper(const char *name)
    : BHandler(name), fDeconstructing(false), fProxy(NULL), fHandlersCount(1), fPreferredHandler(NULL), fLocker(NULL), fLocksCount(B_INT64_CONSTANT(0)), fThread(NULL), fSem(NULL), fMessageQueue(NULL), fCurrentMessage(NULL), fThreadExited(NULL)
{
    this->Init(name,B_NORMAL_PRIORITY);
}
*/

/*BLooper::BLooper(int32 priority)
    : BHandler(""), fDeconstructing(false), fProxy(NULL), fHandlersCount(1), fPreferredHandler(NULL), fLocker(NULL), fLocksCount(B_INT64_CONSTANT(0)), fThread(NULL), fSem(NULL), fMessageQueue(NULL), fCurrentMessage(NULL), fThreadExited(NULL)
{
    this->Init(NULL,priority);
}
*/

/*BLooper::BLooper(const char *name,  int32 priority)
    : BHandler(name), fDeconstructing(false), fProxy(NULL), fHandlersCount(1), fPreferredHandler(NULL), fLocker(NULL), fLocksCount(B_INT64_CONSTANT(0)), fThread(NULL), fSem(NULL), fMessageQueue(NULL), fCurrentMessage(NULL), fThreadExited(NULL)
{
    this->Init(name,priority);
}
*/

void BLooper::Init(const char *name,  int32 priority)
{
    BLocker *hLocker = bhapi::get_handler_operator_locker();
    BAutolock <BLocker>autolock(hLocker);

    if((fLocker = bhapi::create_locker()) == NULL)
        BHAPI_ERROR("[APP]: %s --- Unable to create locker for looper.", __PRETTY_FUNCTION__);

    fPrevHandler = fNextHandler = this;
    fLooper = this;

    fMessageQueue = new BMessageQueue();
    if(fMessageQueue) fSem = bhapi::create_sem(B_INT64_CONSTANT(0), NULL);

    fThreadPriority = priority;

    sLooperList.AddItem(this);
}

BLooper::~BLooper()
{
    for(BHandler *handler = HandlerAt(1); RemoveHandler(handler); handler = HandlerAt(1)) delete handler;

    if(fProxy) ProxyBy(NULL);
    while(fClients.CountItems() > 0)
    {
        BLooper *client = (BLooper*)fClients.ItemAt(0);
        client->Lock();
        client->ProxyBy(NULL);
        client->Unlock();
    }

    while(fCommonFilters->CountItems() > 0)
    {
        BMessageFilter *filter = (BMessageFilter*)fCommonFilters->ItemAt(0);
        BLooper::RemoveCommonFilter(filter);
        delete filter;
    }

    BLocker *hLocker = bhapi::get_handler_operator_locker();
    BAutolock <BLocker>autolock(hLocker);

    if(fMessageQueue) delete fMessageQueue;
    if(fSem) bhapi::delete_sem(fSem);
    if(fCurrentMessage) delete fCurrentMessage;
    if(fThread) bhapi::delete_thread(fThread);

    sLooperList.RemoveItem(this);

    if(BHandler::fToken != NULL) BHandler::fToken->MakeEmpty();

    if(fLocker)
    {
        bhapi::close_locker(fLocker);
        bhapi::delete_locker(fLocker);
    }
}


BLooper::BLooper(const BMessage *from)
    : BHandler(from), fDeconstructing(false), fProxy(NULL), fThreadPriority(B_NORMAL_PRIORITY), fHandlersCount(1), fPreferredHandler(NULL), fLocker(NULL), fLocksCount(B_INT64_CONSTANT(0)), fThread(NULL), fSem(NULL), fMessageQueue(NULL), fCurrentMessage(NULL), fThreadExited(NULL)
{
    BLocker *hLocker = bhapi::get_handler_operator_locker();
    BAutolock <BLocker>autolock(hLocker);

    if((fLocker = bhapi::create_locker()) == NULL)
        BHAPI_ERROR("[APP]: %s --- Unable to create locker for looper.", __PRETTY_FUNCTION__);

    fPrevHandler = fNextHandler = this;
    fLooper = this;

    fMessageQueue = new BMessageQueue();
    if(fMessageQueue) fSem = bhapi::create_sem(B_INT64_CONSTANT(0), NULL);

    sLooperList.AddItem(this);
}


/*status_t
BLooper::Archive(BMessage *into, bool deep) const
{
    if(!into) return B_ERROR;

    BHandler::Archive(into, deep);
    into->AddString("class", "BLooper");

    // TODO

    return B_OK;
}
*/

BArchivable* BLooper::Instantiate(const BMessage *from)
{
    if(bhapi::validate_instantiation(from, "BLooper"))
        return new BLooper(from);
    return NULL;
}


void
BLooper::AddHandler(BHandler *handler)
{
    if(handler == NULL || handler->fLooper != NULL || fHandlersCount == B_MAXINT32) return;

    handler->fLooper = this;

    fPrevHandler->fNextHandler = handler;
    handler->fPrevHandler = fPrevHandler;
    handler->fNextHandler = this;

    fHandlersCount += 1;
    handler->SetNextHandler(this);
}


bool
BLooper::RemoveHandler(BHandler *handler)
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


int32
BLooper::CountHandlers() const
{
    return fHandlersCount;
}


BHandler*
BLooper::HandlerAt(int32 index) const
{
    if(index >= fHandlersCount) return NULL;

    if(index == 0) return const_cast<BHandler*>(cast_as(this, const BHandler));
    if(index == 1) return fNextHandler;
    if(index == fHandlersCount - 1) return fPrevHandler;

    BHandler *handler = fNextHandler;
    while(index-- > 1) handler = handler->fNextHandler;
    return handler;
}


int32
BLooper::IndexOf(BHandler *handler) const
{
    if(handler == NULL || handler->fLooper != this) return -1;

     int32 index = 0;

    const BHandler *found = this;
    while(found != handler)
    {
        found = found->fNextHandler;
        index++;
    }

    return index;
}


BHandler*
BLooper::PreferredHandler() const
{
    return fPreferredHandler;
}


void
BLooper::SetPreferredHandler(BHandler *handler)
{
    if(handler) if(handler->Looper() != this) return;
    fPreferredHandler = handler;
}


bool
BLooper::Lock()
{
    return(LockWithTimeout(B_INFINITE_TIMEOUT) == B_OK);
}


void
BLooper::Unlock()
{
    if(IsLockedByCurrentThread())
    {
        fLocksCount--;
        bhapi::unlock_locker(fLocker);
    }
    else
    {
        BHAPI_WARNING("[APP]: %s -- Looper wasn't locked by current thread.", __PRETTY_FUNCTION__);
    }
}


status_t
BLooper::LockWithTimeout(bigtime_t microseconds_timeout)
{
     uint64 token = bhapi::get_handler_token(this);

    status_t retVal = bhapi::lock_looper_of_handler(token, microseconds_timeout);

    if(retVal == B_OK) fLocksCount++;

    return retVal;
}


/*int64
BLooper::CountLocks() const
{
    return fLocksCount;
}
*/

bool
BLooper::IsLockedByCurrentThread() const
{
    return(CountLocks() > B_INT64_CONSTANT(0));
}


status_t
BLooper::PostMessage(uint32 command)
{
    BMessage msg(command);
    return PostMessage(&msg, this, NULL);
}


status_t
BLooper::PostMessage(const BMessage *message)
{
    return PostMessage(message, this, NULL);
}


status_t
BLooper::PostMessage(uint32 command, BHandler *handler, BHandler *reply_to)
{
    BMessage msg(command);
    return PostMessage(&msg, handler, reply_to);
}


status_t
BLooper::PostMessage(const BMessage *_message, BHandler *handler, BHandler *reply_to)
{
    if(_message == NULL)
    {
        BHAPI_WARNING("[APP]: %s --- Can't post empty message.", __PRETTY_FUNCTION__);
        return B_BAD_VALUE;
    }

     uint64 handlerToken = bhapi::get_handler_token(handler);
     uint64 replyToken = bhapi::get_handler_token(reply_to);

    BMessage aMsg(*_message);
    aMsg.fIsReply = false;
    if(aMsg.fSource != NULL)
    {
        bhapi::delete_port(aMsg.fSource);
        aMsg.fSource = NULL;
    }

    return _PostMessage(&aMsg, handlerToken, replyToken, B_INFINITE_TIMEOUT);
}


status_t
BLooper::_PostMessage(const BMessage *_message,  uint64 handlerToken,  uint64 replyToken, bigtime_t timeout)
{
    if(fMessageQueue == NULL || _message == NULL) return B_ERROR;

     uint64 selfToken = bhapi::get_handler_token(this);
    bigtime_t handlerTokenTimestamp = bhapi::get_handler_create_time_stamp(handlerToken);
    bigtime_t replyTokenTimestamp = bhapi::get_handler_create_time_stamp(replyToken);

    if(fMessageQueue->LockWithTimeout(timeout) != B_OK) return B_ERROR;

    status_t retVal = B_ERROR;

    if(fSem != NULL)
    {
        if(_message->what == _EVENTS_PENDING_ && handlerToken == selfToken)
        {
            retVal = (_message->fNoticeSource ? B_ERROR : B_OK);
        }
        else
        {
            BMessage *message = new BMessage(*_message);

            message->fTeam = bhapi::get_current_team_id();
            message->fTargetToken = handlerToken;
            message->fTargetTokenTimestamp = handlerTokenTimestamp;

            if(replyToken != B_MAXUINT64)
            {
                message->fReplyToken = replyToken;
                message->fReplyTokenTimestamp = replyTokenTimestamp;
                if(message->fSource)
                {
                    bhapi::delete_port(message->fSource);
                    message->fSource = NULL;
                }
            }

            if(fMessageQueue->AddMessage(message))
            {
                message->fNoticeSource = _message->fNoticeSource;
                retVal = B_OK;
            }
        }

        bhapi::release_sem(fSem);
    }

    fMessageQueue->Unlock();

    return retVal;
}


void
BLooper::DispatchMessage(BMessage *msg, BHandler *target)
{
    if(target == NULL) target = fPreferredHandler;
    if(!target || target->Looper() != this) return;

    if(msg->what == B_QUIT_REQUESTED && target == this)
    {
        if(QuitRequested()) PostMessage(_QUIT_);
    }
    else
    {
        target->MessageReceived(msg);
    }
}


void
BLooper::MessageReceived(BMessage *msg)
{
}


bool
BLooper::IsRunning() const
{
    BLocker *hLocker = bhapi::get_handler_operator_locker();
    BAutolock <BLocker>autolock(hLocker);

    if(fProxy != NULL) return _Proxy()->IsRunning();
    if(fThread == NULL || bhapi::get_thread_run_state(fThread) == BHAPI_THREAD_READY) return false;

    return true;
}


void*
BLooper::Run()
{
    BLocker *hLocker = bhapi::get_handler_operator_locker();
    BAutolock <BLocker>autolock(hLocker);

    if(fProxy)
    {
        BHAPI_WARNING("[APP]: %s --- The Looper has proxy, run aborted.", __PRETTY_FUNCTION__);
        return NULL;
    }

    if(!fThread)
    {
        if((fThread = bhapi::create_thread(_task, fThreadPriority, this, NULL)) == NULL)
            BHAPI_ERROR("[APP]: %s -- Unable to create thread!", __PRETTY_FUNCTION__);
    }
    else if(bhapi::get_thread_run_state(fThread) != BHAPI_THREAD_READY)
    {
        BHAPI_ERROR("[APP]: %s --- Thread must run only one time!", __PRETTY_FUNCTION__);
    }

    if(bhapi::resume_thread(bhapi::get_thread_id(fThread)) == B_OK)
        return fThread;
    else
        return NULL;
}


bool
BLooper::QuitRequested()
{
    return true;
}


void
BLooper::Quit()
{
    if(!IsLockedByCurrentThread())
        BHAPI_ERROR("[APP]: %s --- Looper must LOCKED before this call!", __PRETTY_FUNCTION__);

    if(fDeconstructing) return;

    BLocker *hLocker = bhapi::get_handler_operator_locker();
    hLocker->Lock();
    if(bhapi::get_thread_id(fThread) == bhapi::get_current_thread_id() && Proxy() == this)
        BHAPI_ERROR("\n\
**************************************************************************\n\
*                           [APP]: BLooper                               *\n\
*                                                                        *\n\
*      Task must call \"PostMessage(B_QUIT_REQUESTED)\" instead of         *\n\
*      \"Quit()\" within the looper!!!                                     *\n\
*                                                                        *\n\
**************************************************************************\n\n");
    void *thread = NULL;
    if(fThread)
    {
        if((thread = bhapi::open_thread(bhapi::get_thread_id(fThread))) == NULL)
            BHAPI_ERROR("[APP]: %s --- Unable to duplicate the thread!", __PRETTY_FUNCTION__);
    }
    hLocker->Unlock();

    if(thread)
    {
        fDeconstructing = true;

        if(PostMessage(_QUIT_) != B_OK)
            BHAPI_ERROR("[APP]: %s --- Send \"_QUIT_\" to looper error!", __PRETTY_FUNCTION__);
         uint64 token = bhapi::get_handler_token(this);
        fLocksCount = B_INT64_CONSTANT(0);
         int64 locksCount = bhapi::count_locker_locks(fLocker);
        while((locksCount--) > B_INT64_CONSTANT(0)) bhapi::unlock_locker(fLocker);

        status_t status;
        bhapi::wait_for_thread(bhapi::get_thread_id(thread), &status);
        if(bhapi::get_thread_run_state(thread) != BHAPI_THREAD_EXITED)
            if(bhapi::lock_looper_of_handler(token, B_INFINITE_TIMEOUT) == B_OK) delete this;
        bhapi::delete_thread(thread);
    }
    else
    {
        delete this;
    }
}


void
BLooper::_taskError(void *data)
{
    bool *exited = (bool*)data;
    bool showError = (!exited || *exited == false);
    if(exited) delete exited;
    if(showError)
    {
        BHAPI_ERROR("\n\
**************************************************************************\n\
*                           [APP]: BLooper                               *\n\
*                                                                        *\n\
*      Task must call \"PostMessage(B_QUIT_REQUESTED)\" instead of         *\n\
*      \"b_exit_thread\" within the looper!!!                            *\n\
*                                                                        *\n\
**************************************************************************\n\n");
    }
}


BHandler*
BLooper::_MessageTarget(const BMessage *msg, bool *preferred)
{
    if(msg == NULL || msg->fTeam != bhapi::get_current_team_id()) return NULL;
    BHandler *handler = NULL;
    if(bhapi::ref_handler(msg->fTargetToken))
    {
        if(bhapi::get_handler_create_time_stamp(msg->fTargetToken) == msg->fTargetTokenTimestamp &&
           bhapi::get_handler_looper(msg->fTargetToken) == this) handler = bhapi::get_handler(msg->fTargetToken);
        bhapi::unref_handler(msg->fTargetToken);
    }
    if(preferred) *preferred = (msg->fTargetToken == B_MAXUINT64);
    return handler;
}


status_t
BLooper::_task(void *arg)
{
    BLooper *self = (BLooper*)arg;
    if(self == NULL) return B_ERROR;

    void *sem = bhapi::clone_sem_by_source((void*)self->fSem);
    if(!sem) return B_ERROR;

    bool *threadExited = new bool;
    if(!threadExited)
    {
        bhapi::delete_sem((bhapi::sem_id)sem);
        return B_NO_MEMORY;
    }
    *threadExited = false;

    if(bhapi::on_exit_thread(_taskError, (void*)threadExited) != B_OK)
    {
        delete threadExited;
        bhapi::delete_sem((bhapi::sem_id)sem);
        return B_ERROR;
    }
    self->fThreadExited = threadExited;

    if(bhapi::on_exit_thread((void (*)(void*))bhapi::delete_sem, sem) != B_OK)
    {
        *threadExited = true;
        bhapi::delete_sem((bhapi::sem_id)sem);
        return B_ERROR;
    }

    status_t status = _taskLooper(self, sem);

    *threadExited = true;

    return status;
}


BLooper*
BLooper::_GetNextClient(BLooper *client) const
{
    if(client == NULL || _Proxy() != client->_Proxy()) return NULL;
    if(client->fClients.CountItems() > 0) return (BLooper*)client->fClients.FirstItem();
    if(client->fProxy)
    {
        if(client->fProxy->fClients.LastItem() == (void*)client)
        {
            BLooper *retVal = NULL;
            while(true)
            {
                client = client->fProxy;
                if(client == NULL || client->fProxy == NULL) break;
                retVal = (BLooper*)client->fProxy->fClients.ItemAt(client->fProxy->fClients.IndexOf(client) + 1);
                if(retVal != NULL) break;
            }
            return retVal;
        }
        return((BLooper*)client->fProxy->fClients.ItemAt(client->fProxy->fClients.IndexOf(client) + 1));
    }

    return NULL;
}


status_t
BLooper::_taskLooper(BLooper *self, void *sem)
{
    BLocker *hLocker = bhapi::get_handler_operator_locker();

    hLocker->Lock();

    if(self == NULL || self->fThread == NULL || bhapi::get_thread_id(self->fThread) != bhapi::get_current_thread_id() || sem == NULL)
    {
        hLocker->Unlock();
        return B_ERROR;
    }

    hLocker->Unlock();

     uint8 flags = 0; // 0 --- normal, 1 --- continue, >= 2 --- break
    BLooper *looper = NULL;
    BMessageQueue *queue = NULL;
    while(flags < 2)
    {
        looper = self;

        self->Lock();
        queue = self->fMessageQueue;
        while(looper != NULL && queue != NULL)
        {
            BMessage *aMsg = NULL;

            queue->Lock();
            if(queue->IsEmpty() == false)
            {
                aMsg = queue->FindMessage((int32)0);
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
                BHandler *handler = looper->_MessageTarget(aMsg, &preferred);
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

        bhapi::sem_info sem_info;
        if(bhapi::acquire_sem((bhapi::sem_id)sem) != B_OK || bhapi::get_sem_info((bhapi::sem_id)sem, &sem_info) != B_OK) sem_info.closed = true;

        if(sem_info.closed) break;
    }

    return B_OK;
}


BMessage* BLooper::NextLooperMessage(bigtime_t timeout = B_INFINITE_TIMEOUT)
{
    bigtime_t prevTime = b_real_time_clock_usecs();

    if(!IsLockedByCurrentThread())
        BHAPI_ERROR("[APP]: %s --- Looper must LOCKED before this call!", __PRETTY_FUNCTION__);

    BLocker *hLocker = bhapi::get_handler_operator_locker();
    hLocker->Lock();
    void *sem = NULL;
    BLooper *proxy = _Proxy();
    if(proxy == NULL || proxy->fThread == NULL || bhapi::get_thread_id(proxy->fThread) != bhapi::get_current_thread_id() ||
       (sem = bhapi::clone_sem_by_source((void*)proxy->fSem)) == NULL)
    {
        hLocker->Unlock();
        return NULL;
    }
    hLocker->Unlock();

     uint8 flags = 0; // <= 0 --- normal, 1 --- continue, >= 2 --- break
    BLooper *looper = NULL;
    BMessageQueue *queue = NULL;
    BMessage *retVal = NULL;

    while(true)
    {
        looper = proxy;

        proxy->Lock();
        queue = proxy->fMessageQueue;
        while(looper != NULL && queue != NULL)
        {
            if(proxy == bhapi::be_app) BApplication::dispatch_message_runners();

            BMessage *aMsg = NULL;

            queue->Lock();
            if(queue->IsEmpty() == false)
            {
                aMsg = queue->FindMessage((int32)0);
                if(aMsg->what == _QUIT_)
                {
                    queue->Unlock();
                    flags = ((looper == proxy || looper == this) ? 2 : 1);

                    if(flags == 1)
                    {
                        looper->Lock();
                        if(looper->fLocksCount == B_INT64_CONSTANT(1))
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
                BHandler *handler = looper->_MessageTarget(aMsg, &preferred);
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
            bhapi::release_sem((bhapi::sem_id)sem); // to push back semaphore
            break;
        }
        if(flags == 1)
        {
            flags = 0;
            continue;
        }

        bhapi::sem_info sem_info;
        status_t status = B_ERROR;
        bigtime_t waitTime = timeout;
        if(timeout >= B_INT64_CONSTANT(0))
        {
            if(proxy == bhapi::be_app)
            {
                hLocker->Lock();
                waitTime = min_c(timeout, (BApplication::sRunnerMinimumInterval == B_INT64_CONSTANT(0) ?
                                B_INFINITE_TIMEOUT :
                                max_c(BApplication::sRunnerMinimumInterval, B_INT64_CONSTANT(50000))));
                hLocker->Unlock();
            }

            status = bhapi::acquire_sem_etc((bhapi::sem_id)sem, B_INT64_CONSTANT(1), B_TIMEOUT, waitTime);
        }
        if(bhapi::get_sem_info((bhapi::sem_id)sem, &sem_info) != B_OK) sem_info.closed = true;

        if(sem_info.closed || !(status == B_OK || status == B_TIMED_OUT)) break;
        if(status == B_TIMED_OUT && waitTime == timeout) break;
        if(timeout != B_INFINITE_TIMEOUT)
        {
            bigtime_t curTime = b_real_time_clock_usecs();
            timeout -= (curTime - prevTime);
            prevTime = curTime;
        }
    }

    bhapi::delete_sem((bhapi::sem_id)sem);
    return retVal;
}


void
BLooper::DispatchLooperMessage(BMessage *msg)
{
    if(msg == NULL) return;

    if(!IsLockedByCurrentThread())
        BHAPI_ERROR("[APP]: %s --- Looper must LOCKED before this call!", __PRETTY_FUNCTION__);
    if(Thread() != bhapi::get_current_thread_id())
        BHAPI_ERROR("[APP]: %s --- Looper must call this within the task of looper!", __PRETTY_FUNCTION__);

    bool preferred = false;
    BHandler *handler = _MessageTarget(msg, &preferred);
    if(handler == NULL && !preferred)
    {
        delete msg;
        return;
    }

    _FilterAndDispatchMessage(msg, handler);
}


void
BLooper::_FilterAndDispatchMessage(BMessage *msg, BHandler *_target)
{
    if(msg == NULL) return;

    bhapi::filter_result status = B_DISPATCH_MESSAGE;
    BHandler *handler = _target;

    if(msg->what != _QUIT_) // (!(msg->what == B_QUIT_REQUESTED || msg->what == _QUIT_))
    {
        for(int32 i = 0; i < fCommonFilters->CountItems(); i++)
        {
            BMessageFilter *filter = (BMessageFilter*)fCommonFilters->ItemAt(i);
            if((status = filter->doFilter(msg, &handler)) == B_SKIP_MESSAGE) break;
        }

        BHandler *target = (handler == NULL ? fPreferredHandler : handler);
        if(!(status == B_SKIP_MESSAGE || target == NULL || target->fFilters == NULL))
        {
            for(int32 i = 0; i < target->fFilters->CountItems(); i++)
            {
                BMessageFilter *filter = (BMessageFilter*)target->fFilters->ItemAt(i);
                if((status = filter->doFilter(msg, &handler)) == B_SKIP_MESSAGE) break;
            }
        }

        if(status == B_SKIP_MESSAGE)
        {
            delete msg;
            return;
        }
    }

    BMessage *oldMsg = fCurrentMessage;
    fCurrentMessage = msg;
    DispatchMessage(msg, handler);
    if(fCurrentMessage != NULL) delete fCurrentMessage;
    fCurrentMessage = oldMsg;
}


BMessage*
BLooper::CurrentMessage() const
{
    if(!fCurrentMessage || !fMessageQueue) return NULL;
    if(Thread() != bhapi::get_current_thread_id()) return NULL;

    return fCurrentMessage;
}


BMessage*
BLooper::DetachCurrentMessage()
{
    if(!fCurrentMessage || !fMessageQueue) return NULL;
    if(Thread() != bhapi::get_current_thread_id()) return NULL;

    BMessage *msg = fCurrentMessage;
    fCurrentMessage = NULL;

    return msg;
}


BMessageQueue* BLooper::MessageQueue() const
{
    return fMessageQueue;
}


thread_id BLooper::Thread() const
{
    return bhapi::get_thread_id(Proxy()->fThread);
}


BLooper* BLooper::_Proxy() const
{
    if(fProxy == NULL) return (BLooper*)this;
    return fProxy->_Proxy();
}


BLooper* BLooper::Proxy() const
{
    BLocker *hLocker = bhapi::get_handler_operator_locker();
    BAutolock <BLocker>autolock(hLocker);

    return _Proxy();
}


// Thread safe method like below:
// 	{
// 		BLooper *proxy;
// 		BLooper *looper;
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
BLooper::ProxyBy(BLooper *proxy)
{
    if(is_kind_of(this, BApplication))
    {
        BHAPI_WARNING("[APP]: %s --- Application can't proxy by other looper.", __PRETTY_FUNCTION__);
        return(proxy ? false : true);
    }

    if(IsLockedByCurrentThread() == false)
        BHAPI_ERROR("[APP]: %s --- Looper must LOCKED before this call!", __PRETTY_FUNCTION__);

    BLocker *hLocker = bhapi::get_handler_operator_locker();
    BAutolock <BLocker>autolock(hLocker);

    if(proxy == NULL ? (_Proxy() == this) : (_Proxy() == proxy->_Proxy())) return true;

    if(fThread)
    {
        BHAPI_WARNING("[APP]: %s --- The looper already run, proxy aborted.", __PRETTY_FUNCTION__);
        return false;
    }
    else if(!(proxy == NULL || proxy->IsLockedByCurrentThread()))
    {
        BHAPI_ERROR("[APP]: %s --- Proxy must LOCKED before this call!", __PRETTY_FUNCTION__);
    }

    return _ProxyBy(proxy);
}


bool
BLooper::_ProxyBy(BLooper *proxy)
{
    if(proxy != fProxy)
    {
        if(proxy) if(proxy->fClients.AddItem(this) == false) return false;
        if(fProxy) fProxy->fClients.RemoveItem((void*)this);
    }

    BLooper *oldProxy = fProxy;
    if(proxy == NULL)
    {
        fProxy = NULL;

        fMessageQueue->Lock();
        if(fSem) bhapi::delete_sem(fSem);
        fSem = bhapi::create_sem((int64)fMessageQueue->CountMessages(), NULL);
        fMessageQueue->Unlock();

        void *newLocker = NULL;
        if((newLocker = bhapi::create_locker()) == NULL)
            BHAPI_ERROR("[APP]: %s --- Unable to create locker for looper.", __PRETTY_FUNCTION__);
        for(int64 i = B_INT64_CONSTANT(0); i < fLocksCount; i++) bhapi::lock_locker(newLocker);
        void *oldLocker = fLocker;
        fLocker = newLocker;

        for(int32 i = 0; i < fClients.CountItems(); i++)
        {
            BLooper *looper = (BLooper*)fClients.ItemAt(i);
            looper->_ProxyBy(this);
        }

        for(int64 i = B_INT64_CONSTANT(0); i < fLocksCount; i++) bhapi::unlock_locker(oldLocker);
        bhapi::delete_locker(oldLocker);
    }
    else
    {
        fProxy = proxy;

        fMessageQueue->Lock();
        if(fSem) bhapi::delete_sem(fSem);
        fSem = (bhapi::sem_id)bhapi::clone_sem_by_source((void*)(proxy->fSem));
        if(fMessageQueue->CountMessages() > 0)
            bhapi::release_sem_etc(fSem, (int64)fMessageQueue->CountMessages(), 0);
        fMessageQueue->Unlock();

        void *newLocker = NULL;
        if((newLocker = bhapi::clone_locker(proxy->fLocker)) == NULL)
            BHAPI_ERROR("[APP]: %s --- Unable to create locker for looper.", __PRETTY_FUNCTION__);
        for(int64 i = B_INT64_CONSTANT(0); i < fLocksCount; i++) bhapi::lock_locker(newLocker);
        void *oldLocker = fLocker;
        fLocker = newLocker;

        for(int32 i = 0; i < fClients.CountItems(); i++)
        {
            BLooper *looper = (BLooper*)fClients.ItemAt(i);
            looper->_ProxyBy(this);
        }

        if(!oldProxy) bhapi::close_locker(oldLocker);
        else for(int64 i = B_INT64_CONSTANT(0); i < fLocksCount; i++) bhapi::unlock_locker(oldLocker);
        bhapi::delete_locker(oldLocker);
    }

    return true;
}


BLooper*
BLooper::LooperForThread(thread_id tid)
{
    void *thread = bhapi::open_thread(tid);
    if(thread == NULL) return NULL; // invalid id

    BLocker *hLocker = bhapi::get_handler_operator_locker();
    BAutolock <BLocker>autolock(hLocker);

    bhapi::delete_thread(thread);

    for(int32 i = 0; i < sLooperList.CountItems(); i++)
    {
        BLooper *looper = (BLooper*)sLooperList.ItemAt(i);
        if(bhapi::get_thread_id(looper->fThread) == tid) return looper;
    }

    return NULL;
}


/*bool
BLooper::AddCommonFilter(BMessageFilter *filter)
{
    if(filter == NULL || filter->fHandler != NULL || fCommonFilters->AddItem(filter) == false) return false;
    filter->fHandler = this;
    return true;
}
*/

bool
BLooper::RemoveCommonFilter(BMessageFilter *filter)
{
    if(filter == NULL || filter->fHandler != this || fCommonFilters->RemoveItem(filter) == false) return false;
    filter->fHandler = NULL;
    return true;
}

/*
const BList*
BLooper::CommonFilterList() const
{
    return(&fCommonFilters);
}
*/

bool
BLooper::SetCommonFilterList(const BList *filterList)
{
    while(fCommonFilters->CountItems() > 0)
    {
        BMessageFilter *filter = (BMessageFilter*)fCommonFilters->ItemAt(0);
        BLooper::RemoveCommonFilter(filter);
        delete filter;
    }

    if(filterList != NULL)
    {
        for(int32 i = 0; i < filterList->CountItems(); i++) AddCommonFilter((BMessageFilter*)filterList->ItemAt(i));
    }

    return true;
}


bool
BLooper::IsDependsOnOthersWhenQuitRequested() const
{
    return false;
}

