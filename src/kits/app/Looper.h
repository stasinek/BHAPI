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
 * File: Looper.h
 * Description: Looper for waiting/dispatching message
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_LOOPER_H
#define BHAPI_LOOPER_H
//-----------------------------------------------------------------------------
#ifdef __cplusplus /* Just for C++ */
//-----------------------------------------------------------------------------
#ifndef BAPPLICATION_DEF
#define BAPPLICATION_DEF
class BApplication;
#endif
//-----------------------------------------------------------------------------
#ifndef BMESSENGER_DEF
#define BMESSENGER_DEF
class BMessenger;
#endif
//-----------------------------------------------------------------------------
#ifndef BMESSAGE_DEF
#define BMESSAGE_DEF
class BMessage;
#endif
//-----------------------------------------------------------------------------
#ifndef BMESSAGEFILTER_DEF
#define BMESSAGEFILTER_DEF
class BMessageFilter;
#endif
//-----------------------------------------------------------------------------
#ifndef BMESSAGEQUEUE_DEF
#define BMESSAGEQUEUE_DEF
class BMessageQueue;
#endif
//-----------------------------------------------------------------------------
#ifndef BMESSAGEQUEUE_DEF
#define BMESSAGEQUEUE_DEF
class BLoooper;
#endif
//-----------------------------------------------------------------------------
#endif // __cplusplus
//-----------------------------------------------------------------------------
#include "../support/List.h"
#include "Handler.h"
#include <kernel/OS.h>
//-----------------------------------------------------------------------------
#ifdef __cplusplus
//-----------------------------------------------------------------------------
class BHAPI_IMPEXP BLooper : public BHandler {
public:
//	BLooper(const char *name = NULL,
//		__be_int32 priority = B_NORMAL_PRIORITY);
    void Init(const char *name,  __be_int32 priority);
    BLooper(const char *name);
    BLooper(__be_int32 priority);
    BLooper(const char *name,
         __be_int32 priority);
    virtual ~BLooper();

    // Archiving
    BLooper(const BMessage *from);
    virtual status_t Archive(BMessage *into, bool deep = true) const;
    static BArchivable *Instantiate(const BMessage *from);

    void		AddHandler(BHandler *handler);
    bool		RemoveHandler(BHandler *handler);
     __be_int32		CountHandlers() const;
    BHandler	*HandlerAt(__be_int32 index) const;
     __be_int32		IndexOf(BHandler *handler) const;

    BHandler	*PreferredHandler() const;
    void		SetPreferredHandler(BHandler *handler);

    bool		IsRunning() const;
    virtual void	*Run();
    virtual void	Quit();
    virtual bool	QuitRequested();
    BLooper*	Proxy() const;
    bool		ProxyBy(BLooper *proxy);

    thread_id	Thread() const;

    bool		Lock();
    void		Unlock();
    status_t	LockWithTimeout(bigtime_t microseconds_timeout);

     __be_int64		CountLocks() const;
    bool		IsLockedByCurrentThread() const;

    virtual void	DispatchMessage(BMessage *msg, BHandler *target);

    // Empty functions BEGIN --- just for derivative class
    virtual void	MessageReceived(BMessage *msg);
    // Empty functions END

    BMessage	*CurrentMessage() const;
    BMessage	*DetachCurrentMessage();
    BMessageQueue	*MessageQueue() const;

    status_t	PostMessage(__be_uint32 command);
    status_t	PostMessage(const BMessage *message);
    status_t	PostMessage(__be_uint32 command,
                    BHandler *handler,
                    BHandler *reply_to = NULL);
    status_t	PostMessage(const BMessage *message,
                    BHandler *handler,
                    BHandler *reply_to = NULL);

    virtual bool	AddCommonFilter(BMessageFilter *filter);
    virtual bool	RemoveCommonFilter(BMessageFilter *filter);
    virtual bool	SetCommonFilterList(const BList *filterList);
    const BList	*CommonFilterList() const;

    static BLooper	*LooperForThread(thread_id tid);

protected:
    // NextLooperMessage & DispatchLooperMessage: called from task of looper, like below
    //	while(true)
    //	{
    //		...
    //		BMessage *aMsg = NextLooperMessage(B_INFINITE_TIMEOUT);
    //
    //		if(aMsg == NULL) /* after "this->QuitRequested()" return "true" or proxy deconstructing. */
    //		{
    //			...
    //			break;
    //		}
    //		else
    //		{
    //			/* leaked memory unless "DispatchLooperMessage" applied or "delete" instead */
    //			DispatchLooperMessage(aMsg);
    //		}
    //		...
    //	}
//	BMessage	*NextLooperMessage(bigtime_t timeout = B_INFINITE_TIMEOUT);
    BMessage	*NextLooperMessage(bigtime_t timeout);
    void		DispatchLooperMessage(BMessage *msg);

private:
    friend class BHandler;
    friend class BApplication;
    friend class BMessenger;
    friend status_t bhapi::lock_looper_of_handler(__be_uint64 token, bigtime_t timeout);

    bool fDeconstructing;
    BLooper *fProxy;
    BList fClients;

     __be_int32 fThreadPriority;

     __be_int32 fHandlersCount;
    BHandler *fPreferredHandler;

    void *fLocker;
     __be_int64 fLocksCount;

    void *fThread;
    sem_id fSem;

    BMessageQueue *fMessageQueue;
    BMessage *fCurrentMessage;

    static status_t _task(void*);
    static status_t _taskLooper(BLooper*, void*);
    static void _taskError(void*);

    static BList sLooperList;

    BHandler *_MessageTarget(const BMessage *msg, bool *preferred);
    status_t _PostMessage(const BMessage *msg,  __be_uint64 handlerToken,  __be_uint64 replyToken, bigtime_t timeout);

    BLooper *_Proxy() const;
    bool _ProxyBy(BLooper *proxy);
    BLooper *_GetNextClient(BLooper *client) const;

    bool *fThreadExited;

    BList fCommonFilters;
    void _FilterAndDispatchMessage(BMessage *msg, BHandler *target);

    virtual bool	IsDependsOnOthersWhenQuitRequested() const;
};
//-----------------------------------------------------------------------------
#endif /* __cplusplus */
//-----------------------------------------------------------------------------
#define BLOOPER_DEF
#endif /* BHAPI_LOOPER_H */
//-----------------------------------------------------------------------------
