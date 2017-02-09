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
 * File: Looper.h
 * Description: Looper for waiting/dispatching message
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_LOOPER__H
#define BHAPI_LOOPER__H
#include "./../support/SupportDefs.h"

#ifdef __cplusplus /* Just for C++ */

class BApplication;
class BMessenger;
class BMessage;
class BMessageFilter;
class BMessageQueue;
class BLoooper;
#include "./../support/BList.h"
#include "Handler.h"
class IMPEXP_BHAPI BLooper : public BHandler {
public:
//	BLooper(const char *name = NULL,
//		b_int32 priority = B_NORMAL_PRIORITY);
    BLooper(const char *name);
    BLooper(const char *name,
        b_int32 priority);
    virtual ~BLooper();

	// Archiving
	BLooper(const BMessage *from);
	virtual b_status_t Archive(BMessage *into, bool deep = true) const;
	static BArchivable *Instantiate(const BMessage *from);

	void		AddHandler(BHandler *handler);
	bool		RemoveHandler(BHandler *handler);
	b_int32		CountHandlers() const;
	BHandler	*HandlerAt(b_int32 index) const;
	b_int32		IndexOf(BHandler *handler) const;

	BHandler	*PreferredHandler() const;
	void		SetPreferredHandler(BHandler *handler);

	bool		IsRunning() const;
	virtual void	*Run();
	virtual void	Quit();
	virtual bool	QuitRequested();
	BLooper*	Proxy() const;
	bool		ProxyBy(BLooper *proxy);

	b_thread_id	Thread() const;

	bool		Lock();
	void		Unlock();
	b_status_t	LockWithTimeout(b_bigtime_t microseconds_timeout);

	b_int64		CountLocks() const;
	bool		IsLockedByCurrentThread() const;

	virtual void	DispatchMessage(BMessage *msg, BHandler *target);

	// Empty functions BEGIN --- just for derivative class
	virtual void	MessageReceived(BMessage *msg);
	// Empty functions END

	BMessage	*CurrentMessage() const;
	BMessage	*DetachCurrentMessage();
	BMessageQueue	*MessageQueue() const;

	b_status_t	PostMessage(b_uint32 command);
	b_status_t	PostMessage(const BMessage *message);
	b_status_t	PostMessage(b_uint32 command,
				    BHandler *handler,
				    BHandler *reply_to = NULL);
	b_status_t	PostMessage(const BMessage *message,
				    BHandler *handler,
				    BHandler *reply_to = NULL);

	virtual bool	AddCommonFilter(BMessageFilter *filter);
	virtual bool	RemoveCommonFilter(BMessageFilter *filter);
	virtual bool	SetCommonFilterList(const BList *filterList);
	const BList	*CommonFilterList() const;

	static BLooper	*LooperForThread(b_thread_id tid);

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
//	BMessage	*NextLooperMessage(b_bigtime_t timeout = B_INFINITE_TIMEOUT);
    BMessage	*NextLooperMessage(b_bigtime_t timeout);
    void		DispatchLooperMessage(BMessage *msg);

private:
	friend class BHandler;
	friend class BApplication;
	friend class BMessenger;
	friend b_status_t bhapi_lock_looper_of_handler(b_uint64 token, b_bigtime_t timeout);

	bool fDeconstructing;
	BLooper *fProxy;
	BList fClients;

	b_int32 fThreadPriority;

	b_int32 fHandlersCount;
	BHandler *fPreferredHandler;

	void *fLocker;
	b_int64 fLocksCount;

	void *fThread;
	void *fSem;

	BMessageQueue *fMessageQueue;
	BMessage *fCurrentMessage;

	static b_status_t _task(void*);
	static b_status_t _taskLooper(BLooper*, void*);
	static void _taskError(void*);

	static BList sLooperList;

	BHandler *_MessageTarget(const BMessage *msg, bool *preferred);
	b_status_t _PostMessage(const BMessage *msg, b_uint64 handlerToken, b_uint64 replyToken, b_bigtime_t timeout);

	BLooper *_Proxy() const;
	bool _ProxyBy(BLooper *proxy);
	BLooper *_GetNextClient(BLooper *client) const;

	bool *fThreadExited;

	BList fCommonFilters;
	void _FilterAndDispatchMessage(BMessage *msg, BHandler *target);

	virtual bool	IsDependsOnOthersWhenQuitRequested() const;
};

#endif /* __cplusplus */

#endif /* BHAPI_LOOPER__H */

