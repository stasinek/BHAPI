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
 * File: Messenger.h
 * Description: Sending message
 *
 * --------------------------------------------------------------------------*/

#ifndef __ETK_MESSENGER_H__
#define __ETK_MESSENGER_H__

#include "./../app/Looper.h"

#ifdef __cplusplus /* Just for C++ */

class _IMPEXP_ETK EMessenger {
public:
	EMessenger();
	EMessenger(const char *signature, eint64 team = 0, e_status_t *perr = NULL);
	EMessenger(const EHandler *handler, const ELooper *looper = NULL, e_status_t *perr = NULL);

	EMessenger(const EMessenger &msgr);
	~EMessenger();

	bool		IsTargetLocal() const;
	bool		IsAtTargetLooperThread() const;
	EHandler*	Target(ELooper **looper) const;

	bool		LockTarget() const;
	e_status_t	LockTargetWithTimeout(e_bigtime_t timeout) const;

	e_status_t	SendMessage(euint32 command, EHandler *reply_to = NULL) const;
	e_status_t	SendMessage(const EMessage *a_message, EHandler *reply_to = NULL,
				    e_bigtime_t timeout = E_INFINITE_TIMEOUT) const;
	e_status_t	SendMessage(const EMessage *a_message, EMessage *reply_message,
				    e_bigtime_t sendTimeout = E_INFINITE_TIMEOUT,
				    e_bigtime_t replyTimeout = E_INFINITE_TIMEOUT) const;

	bool		IsValid() const;

	EMessenger	&operator=(const EMessenger &from);
	bool		operator==(const EMessenger &other) const;
	bool		operator!=(const EMessenger &other) const;

	size_t		FlattenedSize() const;
	bool		Flatten(char *buffer, size_t bufferSize) const;
	bool		Unflatten(const char *buffer, size_t bufferSize);

	void		PrintToStream() const;

private:
	friend class EMessage;
	friend class EInvoker;

	EMessenger(eint64 targetTeam, euint64 targetToken, e_bigtime_t timestamp, e_status_t *perr);

	euint64 fHandlerToken;
	euint64 fLooperToken;

	void *fPort;
	void *fSem;

	eint64 fTargetTeam;

	void InitData(const EHandler *handler, const ELooper *looper, e_status_t *perr);

	static e_status_t _SendMessageToPort(void *port, const EMessage *msg, euint32 flags, e_bigtime_t timeout);
	static EMessage* _GetMessageFromPort(void *port, euint32 flags, e_bigtime_t timeout, e_status_t *err);

	e_status_t _SendMessage(const EMessage *a_message, euint64 replyToken, e_bigtime_t timeout) const;
};

#endif /* __cplusplus */

#endif /* __ETK_MESSENGER_H__ */

