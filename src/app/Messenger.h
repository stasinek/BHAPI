/* --------------------------------------------------------------------------
 *
 * BHAPI++ previously named ETK++, The Easy Toolkit for C++ programing
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
 * File: Messenger.h
 * Description: Sending message
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_MESSENGER__H
#define BHAPI_MESSENGER__H
#include <support/SupportDefs.h>

#ifdef __cplusplus /* Just for C++ */

class BMessage;
class BHandler;
class BLooper;
class IMPEXP_BHAPI BMessenger {
public:
	BMessenger();
    BMessenger(const char *signature, b_int64 team = 0, b_status_t *perr = NULL);
	BMessenger(const BHandler *handler, const BLooper *looper = NULL, b_status_t *perr = NULL);

	BMessenger(const BMessenger &msgr);
	~BMessenger();

	bool		IsTargetLocal() const;
	bool		IsAtTargetLooperThread() const;
	BHandler*	Target(BLooper **looper) const;

	bool		LockTarget() const;
    b_status_t	LockTargetWithTimeout(b_bigtime_t timeout) const;

//    b_status_t	SendMessage(b_uint32 command, BHandler *reply_to = NULL) const;
    b_status_t	SendMessage(b_uint32 command) const;
    b_status_t	SendMessage(b_uint32 command, BHandler *reply_to) const;
//	b_status_t	SendMessage(const BMessage *a_message, BHandler *reply_to = NULL,
//				    b_bigtime_t timeout = B_INFINITE_TIMEOUT) const;
    b_status_t	SendMessage(const BMessage *a_message) const;
    b_status_t	SendMessage(const BMessage *a_message, BHandler *reply_to,
                    b_bigtime_t timeout) const;
//	b_status_t	SendMessage(const BMessage *a_message, BMessage *reply_message,
//				    b_bigtime_t sendTimeout = B_INFINITE_TIMEOUT,
//				    b_bigtime_t replyTimeout = B_INFINITE_TIMEOUT) const;
    b_status_t	SendMessage(const BMessage *a_message, BMessage *reply_message) const;
    b_status_t	SendMessage(const BMessage *a_message, BMessage *reply_message,
                    b_bigtime_t sendTimeout,
                    b_bigtime_t replyTimeout) const;

	bool		IsValid() const;

	BMessenger	&operator=(const BMessenger &from);
	bool		operator==(const BMessenger &other) const;
	bool		operator!=(const BMessenger &other) const;

	size_t		FlattenedSize() const;
	bool		Flatten(char *buffer, size_t bufferSize) const;
	bool		Unflatten(const char *buffer, size_t bufferSize);

	void		PrintToStream() const;

private:
	friend class BMessage;
	friend class BInvoker;

    BMessenger(b_int64 targetTeam, b_uint64 targetToken, b_bigtime_t timestamp, b_status_t *perr);

    b_uint64 fHandlerToken;
    b_uint64 fLooperToken;

	void *fPort;
	void *fSem;

    b_int64 fTargetTeam;

	void InitData(const BHandler *handler, const BLooper *looper, b_status_t *perr);

    static b_status_t _SendMessageToPort(void *port, const BMessage *msg, b_uint32 flags, b_bigtime_t timeout);
    static BMessage* _GetMessageFromPort(void *port, b_uint32 flags, b_bigtime_t timeout, b_status_t *err);

    b_status_t _SendMessage(const BMessage *a_message, b_uint64 replyToken, b_bigtime_t timeout) const;
};

#endif /* __cplusplus */

#endif /* BHAPI_MESSENGER__H */

