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
 * File: Looper.h
 * Description: Looper for waiting/dispatching message
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_LOOPER_H__
#define __ETK_LOOPER_H__

#include "./../support/List.h"
#include "./../kernel/OS.h"
#include "./../app/AppDefs.h"
#include "./../app/Handler.h"
#include "./../app/MessageQueue.h"
#include "./../app/MessageFilter.h"

#ifdef __cplusplus /* Just for C++ */

class EApplication;
class EMessenger;

class _IMPEXP_ETK ELooper : public EHandler {
public:
	ELooper(const char *name = NULL,
		eint32 priority = E_NORMAL_PRIORITY);
	virtual ~ELooper();

	// Archiving
	ELooper(const EMessage *from);
	virtual e_status_t Archive(EMessage *into, bool deep = true) const;
	static EArchivable *Instantiate(const EMessage *from);

	void		AddHandler(EHandler *handler);
	bool		RemoveHandler(EHandler *handler);
	eint32		CountHandlers() const;
	EHandler	*HandlerAt(eint32 index) const;
	eint32		IndexOf(EHandler *handler) const;

	EHandler	*PreferredHandler() const;
	void		SetPreferredHandler(EHandler *handler);

	bool		IsRunning() const;
	virtual void	*Run();
	virtual void	Quit();
	virtual bool	QuitRequested();
	ELooper*	Proxy() const;
	bool		ProxyBy(ELooper *proxy);

	e_thread_id	Thread() const;

	bool		Lock();
	void		Unlock();
	e_status_t	LockWithTimeout(e_bigtime_t microseconds_timeout);

	eint64		CountLocks() const;
	bool		IsLockedByCurrentThread() const;

	virtual void	DispatchMessage(EMessage *msg, EHandler *target);

	// Empty functions BEGIN --- just for derivative class
	virtual void	MessageReceived(EMessage *msg);
	// Empty functions END

	EMessage	*CurrentMessage() const;
	EMessage	*DetachCurrentMessage();
	EMessageQueue	*MessageQueue() const;

	e_status_t	PostMessage(euint32 command);
	e_status_t	PostMessage(const EMessage *message);
	e_status_t	PostMessage(euint32 command,
				    EHandler *handler,
				    EHandler *reply_to = NULL);
	e_status_t	PostMessage(const EMessage *message,
				    EHandler *handler,
				    EHandler *reply_to = NULL);

	virtual bool	AddCommonFilter(EMessageFilter *filter);
	virtual bool	RemoveCommonFilter(EMessageFilter *filter);
	virtual bool	SetCommonFilterList(const EList *filterList);
	const EList	*CommonFilterList() const;

	static ELooper	*LooperForThread(e_thread_id tid);

protected:
	// NextLooperMessage & DispatchLooperMessage: called from task of looper, like below
	//	while(true)
	//	{
	//		...
	//		EMessage *aMsg = NextLooperMessage(E_INFINITE_TIMEOUT);
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
	EMessage	*NextLooperMessage(e_bigtime_t timeout = E_INFINITE_TIMEOUT);
	void		DispatchLooperMessage(EMessage *msg);

private:
	friend class EHandler;
	friend class EApplication;
	friend class EMessenger;
	friend e_status_t etk_lock_looper_of_handler(euint64 token, e_bigtime_t timeout);

	bool fDeconstructing;
	ELooper *fProxy;
	EList fClients;

	eint32 fThreadPriority;

	eint32 fHandlersCount;
	EHandler *fPreferredHandler;

	void *fLocker;
	eint64 fLocksCount;

	void *fThread;
	void *fSem;

	EMessageQueue *fMessageQueue;
	EMessage *fCurrentMessage;

	static e_status_t _task(void*);
	static e_status_t _taskLooper(ELooper*, void*);
	static void _taskError(void*);

	static EList sLooperList;

	EHandler *_MessageTarget(const EMessage *msg, bool *preferred);
	e_status_t _PostMessage(const EMessage *msg, euint64 handlerToken, euint64 replyToken, e_bigtime_t timeout);

	ELooper *_Proxy() const;
	bool _ProxyBy(ELooper *proxy);
	ELooper *_GetNextClient(ELooper *client) const;

	bool *fThreadExited;

	EList fCommonFilters;
	void _FilterAndDispatchMessage(EMessage *msg, EHandler *target);

	virtual bool	IsDependsOnOthersWhenQuitRequested() const;
};

#endif /* __cplusplus */

#endif /* __ETK_LOOPER_H__ */

