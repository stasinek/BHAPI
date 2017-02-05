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
 * File: Invoker.h
 * Description: Invoke message to any target
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_INVOKER_H__
#define __ETK_INVOKER_H__

#include "./../app/Messenger.h"
#include "./../support/List.h"

#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_ETK EInvoker {
public:
	EInvoker();
	EInvoker(EMessage *message,
		 const EHandler *handler,
		 const ELooper *looper = NULL);
	EInvoker(EMessage *message, EMessenger target);

	virtual ~EInvoker();

	virtual e_status_t	SetMessage(EMessage *message);
	EMessage*		Message() const;
	euint32			Command() const;

	virtual e_status_t	SetTarget(const EHandler *handler, const ELooper *looper = NULL);
	virtual e_status_t	SetTarget(EMessenger messenger);

	bool			IsTargetLocal() const;
	EHandler*		Target(ELooper **looper = NULL) const;
	EMessenger		Messenger() const;

	virtual e_status_t	SetHandlerForReply(const EHandler *handler);
	EHandler*		HandlerForReply() const;

	virtual e_status_t	Invoke(const EMessage *msg = NULL);
	e_status_t		InvokeNotify(const EMessage *msg, euint32 kind = E_CONTROL_INVOKED);

	e_status_t		SetTimeout(e_bigtime_t timeout);
	e_bigtime_t		Timeout() const;

protected:
	/* Return the change code for a notification.  This is either
	   E_CONTROL_INVOKED for raw Invoke() calls, or the kind
	   supplied to InvokeNotify().  In addition, 'notify' will be
	   set to true if this was an InvokeNotify() call, else false. */
	euint32			InvokeKind(bool* notify = NULL);

	/* Start and end an InvokeNotify context around an Invoke() call.
	   These are only needed for writing custom methods that
	   emulate the standard InvokeNotify() call. */
	void			BeginInvokeNotify(euint32 kind = E_CONTROL_INVOKED);
	void			EndInvokeNotify();

private:
	EMessage *fMessage;
	EMessenger fMessenger;
	euint64 fReplyHandlerToken;

	e_bigtime_t fTimeout;

	euint32 fNotifyKind;
	bool fNotifyCalled;

	EList fNotifyStatesList;
};

#endif /* __cplusplus */

#endif /* __ETK_INVOKER_H__ */

