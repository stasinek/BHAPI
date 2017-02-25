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
 * File: Messenger.cpp
 *
 * --------------------------------------------------------------------------*/

#include "Messenger.h"
#include "Message.h"

#include "../app/Looper.h"
#include "../app/AppDefs.h"
#include "../kernel/Kernel.h"
#include "../kernel/Debug.h"
#include "../kernel/OS.h"
#include "../support/ClassInfo.h"
#include "../support/Locker.h"
#include "../support/Errors.h"
#include "../support/Autolock.h"
#include "../private/PrivateHandler.h"

#include <stdlib.h>

BMessenger::BMessenger()
	: fHandlerToken(B_MAXUINT64), fLooperToken(B_MAXUINT64),
	  fPort(NULL), fSem(NULL), fTargetTeam(B_INT64_CONSTANT(0))
{
}


BMessenger::BMessenger(const char *signature, b_int64 team, b_status_t *perr)
	: fHandlerToken(B_MAXUINT64), fLooperToken(B_MAXUINT64),
	  fPort(NULL), fSem(NULL), fTargetTeam(B_INT64_CONSTANT(0))
{
	// TODO
	BHAPI_WARNING("[APP]: %s --- Remote target unsupported yet.", __PRETTY_FUNCTION__);
	if(perr) *perr = B_ERROR;
}


BMessenger::BMessenger(const BHandler *handler,
		       const BLooper *looper,
		       b_status_t *perr)
	: fHandlerToken(B_MAXUINT64), fLooperToken(B_MAXUINT64),
	  fPort(NULL), fSem(NULL), fTargetTeam(B_INT64_CONSTANT(0))
{
	InitData(handler, looper, perr);
}


BMessenger::BMessenger(b_int64 targetTeam, b_uint64 targetToken, b_bigtime_t timestamp, b_status_t *perr)
	: fHandlerToken(B_MAXUINT64), fLooperToken(B_MAXUINT64),
	  fPort(NULL), fSem(NULL), fTargetTeam(B_INT64_CONSTANT(0))
{
	if(targetTeam != bhapi_get_current_team_id())
	{
		BHAPI_WARNING("[APP]: %s --- Remote target unsupported yet.", __PRETTY_FUNCTION__);
		if(perr) *perr = B_ERROR;
		return;
	}

	if(bhapi_ref_handler(targetToken) == false)
	{
		if(perr) *perr = B_ERROR;
		return;
	}

	b_uint64 looperToken = B_MAXUINT64;

	if(bhapi_get_handler_create_time_stamp(targetToken) != timestamp ||
	   (looperToken = bhapi_get_ref_looper_token(targetToken)) == B_MAXUINT64)
	{
		bhapi_unref_handler(targetToken);
		if(perr) *perr = B_ERROR;
		return;
	}

	fTargetTeam = targetTeam;
	fHandlerToken = targetToken;
	fLooperToken = looperToken;

	if(perr) *perr = B_OK;
}


void
BMessenger::InitData(const BHandler *handler, const BLooper *looper, b_status_t *perr)
{
	if(fHandlerToken != B_MAXUINT64) bhapi_unref_handler(fHandlerToken);
	if(fLooperToken != B_MAXUINT64) bhapi_unref_handler(fLooperToken);

	fHandlerToken = B_MAXUINT64; fLooperToken = B_MAXUINT64;

	if(fSem) bhapi_delete_sem(fSem);
	if(fPort) bhapi_delete_port(fPort);

	fSem = NULL; fPort = NULL; fTargetTeam = B_INT64_CONSTANT(0);

	if(perr) *perr = B_BAD_HANDLER;

	if(handler)
	{
		b_uint64 handlerToken = bhapi_get_ref_handler_token(handler);
		if(handlerToken == B_MAXUINT64) return;

		b_uint64 looperToken = bhapi_get_ref_looper_token(handlerToken);
		if(looperToken == B_MAXUINT64) {bhapi_unref_handler(handlerToken); return;}

		fTargetTeam = bhapi_get_current_team_id();
		fHandlerToken = handlerToken;
		fLooperToken = looperToken;

		if(perr) *perr = B_OK;
	}
	else if(looper)
	{
		b_uint64 looperToken = bhapi_get_ref_handler_token(looper);
		if(looperToken == B_MAXUINT64) return;

		fTargetTeam = bhapi_get_current_team_id();
		fHandlerToken = B_MAXUINT64;
		fLooperToken = looperToken;

		if(perr) *perr = B_OK;
	}
}


BMessenger::BMessenger(const BMessenger &from)
	: fHandlerToken(B_MAXUINT64), fLooperToken(B_MAXUINT64),
	  fPort(NULL), fSem(NULL), fTargetTeam(B_INT64_CONSTANT(0))
{
	*this = from;
}


BMessenger&
BMessenger::operator=(const BMessenger &from)
{
	if(fHandlerToken != B_MAXUINT64) bhapi_unref_handler(fHandlerToken);
	if(fLooperToken != B_MAXUINT64) bhapi_unref_handler(fLooperToken);

	fHandlerToken = B_MAXUINT64; fLooperToken = B_MAXUINT64;

	if(fSem) bhapi_delete_sem(fSem);
	if(fPort) bhapi_delete_port(fPort);

	fSem = NULL; fPort = NULL; fTargetTeam = B_INT64_CONSTANT(0);

	if(!from.IsValid()) return *this;

	if(!from.IsTargetLocal())
	{
		// TODO
		return *this;
	}

	if(from.fHandlerToken != B_MAXUINT64) if(bhapi_ref_handler(from.fHandlerToken)) fHandlerToken = from.fHandlerToken;
	if(from.fLooperToken != B_MAXUINT64) if(bhapi_ref_handler(from.fLooperToken)) fLooperToken = from.fLooperToken;

	if(fLooperToken == B_MAXUINT64 || fHandlerToken != from.fHandlerToken)
	{
		if(fHandlerToken != B_MAXUINT64) bhapi_unref_handler(fHandlerToken);
		if(fLooperToken != B_MAXUINT64) bhapi_unref_handler(fLooperToken);

		fHandlerToken = B_MAXUINT64; fLooperToken = B_MAXUINT64;

		return *this;
	}

	fTargetTeam = from.fTargetTeam;

	return *this;
}


bool
BMessenger::operator==(const BMessenger &other) const
{
	if(fTargetTeam != other.fTargetTeam) return false;
	return(fHandlerToken == other.fHandlerToken && fLooperToken == other.fLooperToken);
}


bool
BMessenger::operator!=(const BMessenger &other) const
{
	if(fTargetTeam != other.fTargetTeam) return true;
	return((fHandlerToken == other.fHandlerToken && fLooperToken == other.fLooperToken) ? false : true);
}


BMessenger::~BMessenger()
{
	InitData(NULL, NULL, NULL);
}


bool
BMessenger::IsTargetLocal() const
{
	return(fTargetTeam == bhapi_get_current_team_id());
}


bool
BMessenger::IsAtTargetLooperThread() const
{
	if(fTargetTeam != bhapi_get_current_team_id()) return false;
	return bhapi_is_current_at_looper_thread(fLooperToken);
}


BHandler*
BMessenger::Target(BLooper **looper) const
{
	if(looper) *looper = NULL;
	if(!IsTargetLocal()) return NULL;

	BHandler *handler = bhapi_get_handler(fHandlerToken);
	if(looper) *looper = b_cast_as(bhapi_get_handler(fLooperToken), BLooper);
	return handler;
}


bool
BMessenger::LockTarget() const
{
	return(LockTargetWithTimeout(B_INFINITE_TIMEOUT) == B_OK);
}


b_status_t
BMessenger::LockTargetWithTimeout(b_bigtime_t timeout) const
{
	if(!IsTargetLocal()) return B_ERROR;

	BLocker *hLocker = bhapi_get_handler_operator_locker();
	BAutolock <BLocker>autolock(hLocker);
	BLooper *looper = bhapi_get_handler_looper(fLooperToken);

	return(looper ? looper->LockWithTimeout(timeout) : B_ERROR);
}


bool
BMessenger::IsValid() const
{
	if(IsTargetLocal()) return(fLooperToken != B_MAXUINT64);
	else return(fPort != NULL && fSem != NULL);
}


b_status_t
BMessenger::SendMessage(b_uint32 command) const
{
    BMessage msg(command);
    return SendMessage(&msg, NULL, B_INFINITE_TIMEOUT);
}

b_status_t
BMessenger::SendMessage(b_uint32 command, BHandler *reply_to = NULL) const
{
	BMessage msg(command);
	return SendMessage(&msg, reply_to, B_INFINITE_TIMEOUT);
}


b_status_t
BMessenger::SendMessage(const BMessage *a_message) const
{
return BMessenger::SendMessage(a_message,NULL,B_INFINITE_TIMEOUT);

}

b_status_t
BMessenger::SendMessage(const BMessage *a_message,
            BHandler *reply_to = NULL,
            b_bigtime_t timeout = B_INFINITE_TIMEOUT) const
{
	if(a_message == NULL)
	{
		BHAPI_WARNING("[APP]: %s --- Can't post empty message.", __PRETTY_FUNCTION__);
		return B_BAD_VALUE;
	}

	b_uint64 replyToken = bhapi_get_handler_token(reply_to);

	BMessage aMsg(*a_message);
	aMsg.fIsReply = false;
	if(aMsg.fSource != NULL)
	{
		bhapi_delete_port(aMsg.fSource);
		aMsg.fSource = NULL;
	}

	return _SendMessage(&aMsg, replyToken, timeout);
}

b_status_t
BMessenger::SendMessage(const BMessage *a_message, BMessage *reply_message) const
{
return BMessenger::SendMessage(a_message,reply_message,B_INFINITE_TIMEOUT,B_INFINITE_TIMEOUT);
}

b_status_t
BMessenger::SendMessage(const BMessage *a_message, BMessage *reply_message, b_bigtime_t sendTimeout = B_INFINITE_TIMEOUT, b_bigtime_t replyTimeout = B_INFINITE_TIMEOUT) const
{
	if(a_message == NULL || reply_message == NULL)
	{
		BHAPI_WARNING("[APP]: %s --- Can't post empty message or \"reply_message\" assigned to be \"NULL\".", __PRETTY_FUNCTION__);
		return B_BAD_VALUE;
	}

	void *port = bhapi_create_port(1, NULL, BHAPI_AREA_ACCESS_OWNER);
	if(port == NULL) return B_NO_MORE_PORTS;

	BMessage *aMsg = new BMessage(*a_message);
	if(aMsg->fSource != NULL) bhapi_delete_port(aMsg->fSource);
	aMsg->fTeam = bhapi_get_current_team_id();
	aMsg->fIsReply = false;
	aMsg->fReplyToken = B_MAXUINT64;
	aMsg->fReplyTokenTimestamp = B_MAXINT64;
	aMsg->fNoticeSource = true; // auto close the port when deleted
	aMsg->fSource = port; // auto delete the port when deleted

	b_status_t status = _SendMessage(aMsg, B_MAXUINT64, sendTimeout);
	if(status == B_OK)
	{
		BMessage *reply = _GetMessageFromPort(port, B_TIMEOUT, replyTimeout, &status);
		if(reply != NULL)
		{
			*reply_message = *reply;
			delete reply;
		}
		else
		{
			reply_message->what = B_NO_REPLY;
		}
	}

	delete aMsg;
	return status;
}


b_status_t
BMessenger::_SendMessage(const BMessage *a_message,
			 b_uint64 replyToken,
			 b_bigtime_t timeout) const
{
	if(a_message == NULL) return B_BAD_VALUE;
	if(!IsValid()) return B_ERROR;

	if(!IsTargetLocal())
	{
		// TODO
		return B_ERROR;
	}

	b_status_t retVal = B_ERROR;

	BLocker *hLocker = bhapi_get_handler_operator_locker();
	BAutolock <BLocker>autolock(hLocker);

	BLooper *looper = b_cast_as(bhapi_get_handler(fLooperToken), BLooper);
	if(looper) retVal = looper->_PostMessage(a_message, fHandlerToken, replyToken, timeout);

	return retVal;
}


b_status_t
BMessenger::_SendMessageToPort(void *port, const BMessage *msg, b_uint32 flags, b_bigtime_t timeout)
{
	if(!port || !msg) return B_ERROR;

	size_t flattenedSize = msg->FlattenedSize();
	if(flattenedSize <= 0)
	{
		BHAPI_WARNING("[APP]: Faltten size little than 1. (%s:%d)", __FILE__, __LINE__);
		return B_ERROR;
	}

	char *buffer = (char*)malloc(flattenedSize);
	if(!buffer)
	{
		BHAPI_WARNING("[APP]: Buffer malloc failed. (%s:%d)", __FILE__, __LINE__);
		return B_NO_MEMORY;
	}

	if(msg->Flatten(buffer, flattenedSize) == false)
	{
		free(buffer);
		BHAPI_WARNING("[APP]: Flatten message failed. (%s:%d)", __FILE__, __LINE__);
		return B_ERROR;
	}

	b_status_t status;
	if((status = bhapi_write_port_etc(port, _EVENTS_PENDING_, buffer, flattenedSize, flags, timeout)) != B_OK)
	{
		free(buffer);
		BHAPI_WARNING("[APP]: write port %s. (%s:%d)", status == B_TIMEOUT ? "time out" : "failed", __FILE__, __LINE__);
		return status;
	}

	free(buffer);

	return B_OK;
}


BMessage*
BMessenger::_GetMessageFromPort(void *port, b_uint32 flags, b_bigtime_t timeout, b_status_t *err)
{
	b_status_t retErr = B_OK;
	BMessage* retMsg = NULL;

	do{
        b_int64 bufferSize = bhapi_port_buffer_size_etc(port, flags, timeout);
		if(bufferSize == 0)
		{
			b_int32 code;
			retErr = bhapi_read_port_etc(port, &code, NULL, 0, B_TIMEOUT, B_INT64_CONSTANT(0));
//			if(retErr != B_OK) BHAPI_DEBUG("[APP]: Port read failed(0x%x). (%s:%d)", retErr, __FILE__, __LINE__);
			break;
		}
		else if(bufferSize < 0)
		{
			retErr = bufferSize;
//			if(!(retErr == B_WOULD_BLOCK || retErr == B_TIMED_OUT))
//				BHAPI_DEBUG("[APP]: Port read failed(0x%x). (%s:%d)", retErr, __FILE__, __LINE__);
			break;
		}

		char *buffer = (char*)malloc((size_t)bufferSize);
		if(!buffer)
		{
			retErr = B_NO_MEMORY;
			BHAPI_WARNING("[APP]: Memory alloc failed. (%s:%d)", __FILE__, __LINE__);
			break;
		}
		bzero(buffer, (size_t)bufferSize);

		b_int32 code;
		if((retErr = bhapi_read_port_etc(port, &code, buffer, bufferSize, B_TIMEOUT, B_INT64_CONSTANT(0))) != B_OK)
		{
//			BHAPI_DEBUG("[APP]: Port read failed(0x%x). (%s:%d)", retErr, __FILE__, __LINE__);
			free(buffer);
			break;
		}
		if(code != _EVENTS_PENDING_ || (size_t)bufferSize < sizeof(size_t))
		{
			BHAPI_WARNING("[APP]: Message is invalid. (%s:%d)", __FILE__, __LINE__);
			free(buffer);
			retErr = B_ERROR;
			break;
		}

		size_t msgBufferSize = 0;
		memcpy(&msgBufferSize, buffer, sizeof(size_t));
		if(bufferSize != (b_size_t)msgBufferSize) /* the first "size_t" == FlattenedSize() */
		{
			BHAPI_WARNING("[APP]: Message length is invalid. (%s:%d)", __FILE__, __LINE__);
			free(buffer);
			retErr = B_ERROR;
			break;
		}

		if((retMsg = new BMessage()) == NULL)
		{
			BHAPI_WARNING("[APP]: Memory alloc failed. (%s:%d)", __FILE__, __LINE__);
			free(buffer);
			retErr = B_NO_MEMORY;
			break;
		}

		if(retMsg->Unflatten(buffer, msgBufferSize) == false)
		{
			BHAPI_WARNING("[APP]: Message unflatten failed. (%s:%d)", __FILE__, __LINE__);
			delete retMsg;
			retMsg = NULL;
			retErr = B_ERROR;
		}

		free(buffer);
	}while(false);

	if(err) *err = retErr;
	return retMsg;
}


size_t
BMessenger::FlattenedSize() const
{
	return(2 * sizeof(b_uint64) + 2 * sizeof(b_bigtime_t) + sizeof(b_int64));
}


bool
BMessenger::Flatten(char *buffer, size_t bufferSize) const
{
	if(buffer == NULL || bufferSize < FlattenedSize()) return false;

	b_bigtime_t handler_stamp = bhapi_get_handler_create_time_stamp(fHandlerToken);
	b_bigtime_t looper_stamp = bhapi_get_handler_create_time_stamp(fLooperToken);

	memcpy(buffer, &fTargetTeam, sizeof(b_int64)); buffer += sizeof(b_int64);
	memcpy(buffer, &fHandlerToken, sizeof(b_uint64)); buffer += sizeof(b_uint64);
	memcpy(buffer, &handler_stamp, sizeof(b_bigtime_t)); buffer += sizeof(b_bigtime_t);
	memcpy(buffer, &fLooperToken, sizeof(b_uint64)); buffer += sizeof(b_uint64);
	memcpy(buffer, &looper_stamp, sizeof(b_bigtime_t));

	return true;
}


bool
BMessenger::Unflatten(const char *buffer, size_t bufferSize)
{
	if(buffer == NULL || bufferSize < FlattenedSize()) return false;

	b_bigtime_t handler_stamp = B_MAXINT64;
	b_bigtime_t looper_stamp = B_MAXINT64;

	b_int64 target_team = B_INT64_CONSTANT(0);
	b_uint64 handler_token = B_MAXUINT64;
	b_uint64 looper_token = B_MAXUINT64;

	memcpy(&target_team, buffer, sizeof(b_int64)); buffer += sizeof(b_int64);
	memcpy(&handler_token, buffer, sizeof(b_uint64)); buffer += sizeof(b_uint64);
	memcpy(&handler_stamp, buffer, sizeof(b_bigtime_t)); buffer += sizeof(b_bigtime_t);
	memcpy(&looper_token, buffer, sizeof(b_uint64)); buffer += sizeof(b_uint64);
	memcpy(&looper_stamp, buffer, sizeof(b_bigtime_t));

	if(fHandlerToken != B_MAXUINT64) bhapi_unref_handler(fHandlerToken);
	if(fLooperToken != B_MAXUINT64) bhapi_unref_handler(fLooperToken);

	fHandlerToken = B_MAXUINT64; fLooperToken = B_MAXUINT64;

	if(fSem) bhapi_delete_sem(fSem);
	if(fPort) bhapi_delete_port(fPort);

	fSem = NULL; fPort = NULL; fTargetTeam = B_INT64_CONSTANT(0);

	do{
		if(target_team != bhapi_get_current_team_id())
		{
			// TODO
			BHAPI_DEBUG("[APP]: %s --- Remote target unsupported.", __PRETTY_FUNCTION__);
			break;
		}

		if(handler_token == B_MAXUINT64 && looper_token == B_MAXUINT64) break;
		if(handler_stamp == B_MAXINT64 && looper_stamp == B_MAXINT64) break;

		if(handler_token != B_MAXUINT64) if(bhapi_ref_handler(handler_token)) fHandlerToken = handler_token;
		if(looper_token != B_MAXUINT64) if(bhapi_ref_handler(looper_token)) fLooperToken = looper_token;

		if(fLooperToken == B_MAXUINT64 || fHandlerToken != handler_token ||
		   (fHandlerToken == B_MAXUINT64 ? false : bhapi_get_handler_create_time_stamp(fHandlerToken) != handler_stamp) ||
		   (fLooperToken == B_MAXUINT64 ? false : bhapi_get_handler_create_time_stamp(fLooperToken) != looper_stamp))
		{
			if(fHandlerToken != B_MAXUINT64) bhapi_unref_handler(fHandlerToken);
			if(fLooperToken != B_MAXUINT64) bhapi_unref_handler(fLooperToken);
			fHandlerToken = B_MAXUINT64; fLooperToken = B_MAXUINT64;
			BHAPI_DEBUG("[APP]: %s --- Invalid target.", __PRETTY_FUNCTION__);
			break;
		}

		fTargetTeam = target_team;
	} while(false);

	return true;
}


void
BMessenger::PrintToStream() const
{
	BHAPI_OUTPUT("******** BMessenger Debug Output **********\n");

	BHAPI_OUTPUT("\tTarget team: %I64i\n", fTargetTeam);
	BHAPI_OUTPUT("\tToken of target handler: ");

	if(fHandlerToken == B_MAXUINT64) BHAPI_OUTPUT("E_MAXUINT64\n");
	else BHAPI_OUTPUT("%I64u\n", fHandlerToken);

	BHAPI_OUTPUT("\tToken of target looper: ");
	if(fLooperToken == B_MAXUINT64) BHAPI_OUTPUT("E_MAXUINT64\n");
	else BHAPI_OUTPUT("%I64u - %p\n", fLooperToken, bhapi_get_handler(fLooperToken));

	if(fPort == NULL) BHAPI_OUTPUT("\tPort invalid.\n");
	if(fSem == NULL) BHAPI_OUTPUT("\tSemaphore invalid.\n");

	BHAPI_OUTPUT("*******************************************\n");
}
