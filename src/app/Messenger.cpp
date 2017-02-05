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
 * File: Messenger.cpp
 *
 * --------------------------------------------------------------------------*/

#include <stdlib.h>

#include "./../kernel/Kernel.h>
#include "./../support/ClassInfo.h>
#include "./../support/Locker.h>
#include "./../support/Autolock.h>

#include "./../private/PrivateHandler.h>

#include "Messenger.h"


EMessenger::EMessenger()
	: fHandlerToken(E_MAXUINT64), fLooperToken(E_MAXUINT64),
	  fPort(NULL), fSem(NULL), fTargetTeam(E_INT64_CONSTANT(0))
{
}


EMessenger::EMessenger(const char *signature, eint64 team, e_status_t *perr)
	: fHandlerToken(E_MAXUINT64), fLooperToken(E_MAXUINT64),
	  fPort(NULL), fSem(NULL), fTargetTeam(E_INT64_CONSTANT(0))
{
	// TODO
	ETK_WARNING("[APP]: %s --- Remote target unsupported yet.", __PRETTY_FUNCTION__);
	if(perr) *perr = E_ERROR;
}


EMessenger::EMessenger(const EHandler *handler,
		       const ELooper *looper,
		       e_status_t *perr)
	: fHandlerToken(E_MAXUINT64), fLooperToken(E_MAXUINT64),
	  fPort(NULL), fSem(NULL), fTargetTeam(E_INT64_CONSTANT(0))
{
	InitData(handler, looper, perr);
}


EMessenger::EMessenger(eint64 targetTeam, euint64 targetToken, e_bigtime_t timestamp, e_status_t *perr)
	: fHandlerToken(E_MAXUINT64), fLooperToken(E_MAXUINT64),
	  fPort(NULL), fSem(NULL), fTargetTeam(E_INT64_CONSTANT(0))
{
	if(targetTeam != etk_get_current_team_id())
	{
		ETK_WARNING("[APP]: %s --- Remote target unsupported yet.", __PRETTY_FUNCTION__);
		if(perr) *perr = E_ERROR;
		return;
	}

	if(etk_ref_handler(targetToken) == false)
	{
		if(perr) *perr = E_ERROR;
		return;
	}

	euint64 looperToken = E_MAXUINT64;

	if(etk_get_handler_create_time_stamp(targetToken) != timestamp ||
	   (looperToken = etk_get_ref_looper_token(targetToken)) == E_MAXUINT64)
	{
		etk_unref_handler(targetToken);
		if(perr) *perr = E_ERROR;
		return;
	}

	fTargetTeam = targetTeam;
	fHandlerToken = targetToken;
	fLooperToken = looperToken;

	if(perr) *perr = E_OK;
}


void
EMessenger::InitData(const EHandler *handler, const ELooper *looper, e_status_t *perr)
{
	if(fHandlerToken != E_MAXUINT64) etk_unref_handler(fHandlerToken);
	if(fLooperToken != E_MAXUINT64) etk_unref_handler(fLooperToken);

	fHandlerToken = E_MAXUINT64; fLooperToken = E_MAXUINT64;

	if(fSem) etk_delete_sem(fSem);
	if(fPort) etk_delete_port(fPort);

	fSem = NULL; fPort = NULL; fTargetTeam = E_INT64_CONSTANT(0);

	if(perr) *perr = E_BAD_HANDLER;

	if(handler)
	{
		euint64 handlerToken = etk_get_ref_handler_token(handler);
		if(handlerToken == E_MAXUINT64) return;

		euint64 looperToken = etk_get_ref_looper_token(handlerToken);
		if(looperToken == E_MAXUINT64) {etk_unref_handler(handlerToken); return;}

		fTargetTeam = etk_get_current_team_id();
		fHandlerToken = handlerToken;
		fLooperToken = looperToken;

		if(perr) *perr = E_OK;
	}
	else if(looper)
	{
		euint64 looperToken = etk_get_ref_handler_token(looper);
		if(looperToken == E_MAXUINT64) return;

		fTargetTeam = etk_get_current_team_id();
		fHandlerToken = E_MAXUINT64;
		fLooperToken = looperToken;

		if(perr) *perr = E_OK;
	}
}


EMessenger::EMessenger(const EMessenger &from)
	: fHandlerToken(E_MAXUINT64), fLooperToken(E_MAXUINT64),
	  fPort(NULL), fSem(NULL), fTargetTeam(E_INT64_CONSTANT(0))
{
	*this = from;
}


EMessenger&
EMessenger::operator=(const EMessenger &from)
{
	if(fHandlerToken != E_MAXUINT64) etk_unref_handler(fHandlerToken);
	if(fLooperToken != E_MAXUINT64) etk_unref_handler(fLooperToken);

	fHandlerToken = E_MAXUINT64; fLooperToken = E_MAXUINT64;

	if(fSem) etk_delete_sem(fSem);
	if(fPort) etk_delete_port(fPort);

	fSem = NULL; fPort = NULL; fTargetTeam = E_INT64_CONSTANT(0);

	if(!from.IsValid()) return *this;

	if(!from.IsTargetLocal())
	{
		// TODO
		return *this;
	}

	if(from.fHandlerToken != E_MAXUINT64) if(etk_ref_handler(from.fHandlerToken)) fHandlerToken = from.fHandlerToken;
	if(from.fLooperToken != E_MAXUINT64) if(etk_ref_handler(from.fLooperToken)) fLooperToken = from.fLooperToken;

	if(fLooperToken == E_MAXUINT64 || fHandlerToken != from.fHandlerToken)
	{
		if(fHandlerToken != E_MAXUINT64) etk_unref_handler(fHandlerToken);
		if(fLooperToken != E_MAXUINT64) etk_unref_handler(fLooperToken);

		fHandlerToken = E_MAXUINT64; fLooperToken = E_MAXUINT64;

		return *this;
	}

	fTargetTeam = from.fTargetTeam;

	return *this;
}


bool
EMessenger::operator==(const EMessenger &other) const
{
	if(fTargetTeam != other.fTargetTeam) return false;
	return(fHandlerToken == other.fHandlerToken && fLooperToken == other.fLooperToken);
}


bool
EMessenger::operator!=(const EMessenger &other) const
{
	if(fTargetTeam != other.fTargetTeam) return true;
	return((fHandlerToken == other.fHandlerToken && fLooperToken == other.fLooperToken) ? false : true);
}


EMessenger::~EMessenger()
{
	InitData(NULL, NULL, NULL);
}


bool
EMessenger::IsTargetLocal() const
{
	return(fTargetTeam == etk_get_current_team_id());
}


bool
EMessenger::IsAtTargetLooperThread() const
{
	if(fTargetTeam != etk_get_current_team_id()) return false;
	return etk_is_current_at_looper_thread(fLooperToken);
}


EHandler*
EMessenger::Target(ELooper **looper) const
{
	if(looper) *looper = NULL;
	if(!IsTargetLocal()) return NULL;

	EHandler *handler = etk_get_handler(fHandlerToken);
	if(looper) *looper = e_cast_as(etk_get_handler(fLooperToken), ELooper);
	return handler;
}


bool
EMessenger::LockTarget() const
{
	return(LockTargetWithTimeout(E_INFINITE_TIMEOUT) == E_OK);
}


e_status_t
EMessenger::LockTargetWithTimeout(e_bigtime_t timeout) const
{
	if(!IsTargetLocal()) return E_ERROR;

	ELocker *hLocker = etk_get_handler_operator_locker();
	EAutolock <ELocker>autolock(hLocker);
	ELooper *looper = etk_get_handler_looper(fLooperToken);

	return(looper ? looper->LockWithTimeout(timeout) : E_ERROR);
}


bool
EMessenger::IsValid() const
{
	if(IsTargetLocal()) return(fLooperToken != E_MAXUINT64);
	else return(fPort != NULL && fSem != NULL);
}


e_status_t
EMessenger::SendMessage(euint32 command, EHandler *reply_to) const
{
	EMessage msg(command);
	return SendMessage(&msg, reply_to, E_INFINITE_TIMEOUT);
}


e_status_t
EMessenger::SendMessage(const EMessage *a_message,
			EHandler *reply_to,
			e_bigtime_t timeout) const
{
	if(a_message == NULL)
	{
		ETK_WARNING("[APP]: %s --- Can't post empty message.", __PRETTY_FUNCTION__);
		return E_BAD_VALUE;
	}

	euint64 replyToken = etk_get_handler_token(reply_to);

	EMessage aMsg(*a_message);
	aMsg.fIsReply = false;
	if(aMsg.fSource != NULL)
	{
		etk_delete_port(aMsg.fSource);
		aMsg.fSource = NULL;
	}

	return _SendMessage(&aMsg, replyToken, timeout);
}


e_status_t
EMessenger::SendMessage(const EMessage *a_message, EMessage *reply_message, e_bigtime_t sendTimeout, e_bigtime_t replyTimeout) const
{
	if(a_message == NULL || reply_message == NULL)
	{
		ETK_WARNING("[APP]: %s --- Can't post empty message or \"reply_message\" assigned to be \"NULL\".", __PRETTY_FUNCTION__);
		return E_BAD_VALUE;
	}

	void *port = etk_create_port(1, NULL, ETK_AREA_ACCESS_OWNER);
	if(port == NULL) return E_NO_MORE_PORTS;

	EMessage *aMsg = new EMessage(*a_message);
	if(aMsg->fSource != NULL) etk_delete_port(aMsg->fSource);
	aMsg->fTeam = etk_get_current_team_id();
	aMsg->fIsReply = false;
	aMsg->fReplyToken = E_MAXUINT64;
	aMsg->fReplyTokenTimestamp = E_MAXINT64;
	aMsg->fNoticeSource = true; // auto close the port when deleted
	aMsg->fSource = port; // auto delete the port when deleted

	e_status_t status = _SendMessage(aMsg, E_MAXUINT64, sendTimeout);
	if(status == E_OK)
	{
		EMessage *reply = _GetMessageFromPort(port, E_TIMEOUT, replyTimeout, &status);
		if(reply != NULL)
		{
			*reply_message = *reply;
			delete reply;
		}
		else
		{
			reply_message->what = E_NO_REPLY;
		}
	}

	delete aMsg;
	return status;
}


e_status_t
EMessenger::_SendMessage(const EMessage *a_message,
			 euint64 replyToken,
			 e_bigtime_t timeout) const
{
	if(a_message == NULL) return E_BAD_VALUE;
	if(!IsValid()) return E_ERROR;

	if(!IsTargetLocal())
	{
		// TODO
		return E_ERROR;
	}

	e_status_t retVal = E_ERROR;

	ELocker *hLocker = etk_get_handler_operator_locker();
	EAutolock <ELocker>autolock(hLocker);

	ELooper *looper = e_cast_as(etk_get_handler(fLooperToken), ELooper);
	if(looper) retVal = looper->_PostMessage(a_message, fHandlerToken, replyToken, timeout);

	return retVal;
}


e_status_t
EMessenger::_SendMessageToPort(void *port, const EMessage *msg, euint32 flags, e_bigtime_t timeout)
{
	if(!port || !msg) return E_ERROR;

	size_t flattenedSize = msg->FlattenedSize();
	if(flattenedSize <= 0)
	{
		ETK_WARNING("[APP]: Faltten size little than 1. (%s:%d)", __FILE__, __LINE__);
		return E_ERROR;
	}

	char *buffer = (char*)malloc(flattenedSize);
	if(!buffer)
	{
		ETK_WARNING("[APP]: Buffer malloc failed. (%s:%d)", __FILE__, __LINE__);
		return E_NO_MEMORY;
	}

	if(msg->Flatten(buffer, flattenedSize) == false)
	{
		free(buffer);
		ETK_WARNING("[APP]: Flatten message failed. (%s:%d)", __FILE__, __LINE__);
		return E_ERROR;
	}

	e_status_t status;
	if((status = etk_write_port_etc(port, _EVENTS_PENDING_, buffer, flattenedSize, flags, timeout)) != E_OK)
	{
		free(buffer);
		ETK_WARNING("[APP]: write port %s. (%s:%d)", status == E_TIMEOUT ? "time out" : "failed", __FILE__, __LINE__);
		return status;
	}

	free(buffer);

	return E_OK;
}


EMessage*
EMessenger::_GetMessageFromPort(void *port, euint32 flags, e_bigtime_t timeout, e_status_t *err)
{
	e_status_t retErr = E_OK;
	EMessage* retMsg = NULL;

	do{
		ssize_t bufferSize = etk_port_buffer_size_etc(port, flags, timeout);
		if(bufferSize == 0)
		{
			eint32 code;
			retErr = etk_read_port_etc(port, &code, NULL, 0, E_TIMEOUT, E_INT64_CONSTANT(0));
//			if(retErr != E_OK) ETK_DEBUG("[APP]: Port read failed(0x%x). (%s:%d)", retErr, __FILE__, __LINE__);
			break;
		}
		else if(bufferSize < 0)
		{
			retErr = bufferSize;
//			if(!(retErr == E_WOULD_BLOCK || retErr == E_TIMED_OUT))
//				ETK_DEBUG("[APP]: Port read failed(0x%x). (%s:%d)", retErr, __FILE__, __LINE__);
			break;
		}

		char *buffer = (char*)malloc((size_t)bufferSize);
		if(!buffer)
		{
			retErr = E_NO_MEMORY;
			ETK_WARNING("[APP]: Memory alloc failed. (%s:%d)", __FILE__, __LINE__);
			break;
		}
		bzero(buffer, (size_t)bufferSize);

		eint32 code;
		if((retErr = etk_read_port_etc(port, &code, buffer, bufferSize, E_TIMEOUT, E_INT64_CONSTANT(0))) != E_OK)
		{
//			ETK_DEBUG("[APP]: Port read failed(0x%x). (%s:%d)", retErr, __FILE__, __LINE__);
			free(buffer);
			break;
		}
		if(code != _EVENTS_PENDING_ || (size_t)bufferSize < sizeof(size_t))
		{
			ETK_WARNING("[APP]: Message is invalid. (%s:%d)", __FILE__, __LINE__);
			free(buffer);
			retErr = E_ERROR;
			break;
		}

		size_t msgBufferSize = 0;
		memcpy(&msgBufferSize, buffer, sizeof(size_t));
		if(bufferSize != (ssize_t)msgBufferSize) /* the first "size_t" == FlattenedSize() */
		{
			ETK_WARNING("[APP]: Message length is invalid. (%s:%d)", __FILE__, __LINE__);
			free(buffer);
			retErr = E_ERROR;
			break;
		}

		if((retMsg = new EMessage()) == NULL)
		{
			ETK_WARNING("[APP]: Memory alloc failed. (%s:%d)", __FILE__, __LINE__);
			free(buffer);
			retErr = E_NO_MEMORY;
			break;
		}

		if(retMsg->Unflatten(buffer, msgBufferSize) == false)
		{
			ETK_WARNING("[APP]: Message unflatten failed. (%s:%d)", __FILE__, __LINE__);
			delete retMsg;
			retMsg = NULL;
			retErr = E_ERROR;
		}

		free(buffer);
	}while(false);

	if(err) *err = retErr;
	return retMsg;
}


size_t
EMessenger::FlattenedSize() const
{
	return(2 * sizeof(euint64) + 2 * sizeof(e_bigtime_t) + sizeof(eint64));
}


bool
EMessenger::Flatten(char *buffer, size_t bufferSize) const
{
	if(buffer == NULL || bufferSize < FlattenedSize()) return false;

	e_bigtime_t handler_stamp = etk_get_handler_create_time_stamp(fHandlerToken);
	e_bigtime_t looper_stamp = etk_get_handler_create_time_stamp(fLooperToken);

	memcpy(buffer, &fTargetTeam, sizeof(eint64)); buffer += sizeof(eint64);
	memcpy(buffer, &fHandlerToken, sizeof(euint64)); buffer += sizeof(euint64);
	memcpy(buffer, &handler_stamp, sizeof(e_bigtime_t)); buffer += sizeof(e_bigtime_t);
	memcpy(buffer, &fLooperToken, sizeof(euint64)); buffer += sizeof(euint64);
	memcpy(buffer, &looper_stamp, sizeof(e_bigtime_t));

	return true;
}


bool
EMessenger::Unflatten(const char *buffer, size_t bufferSize)
{
	if(buffer == NULL || bufferSize < FlattenedSize()) return false;

	e_bigtime_t handler_stamp = E_MAXINT64;
	e_bigtime_t looper_stamp = E_MAXINT64;

	eint64 target_team = E_INT64_CONSTANT(0);
	euint64 handler_token = E_MAXUINT64;
	euint64 looper_token = E_MAXUINT64;

	memcpy(&target_team, buffer, sizeof(eint64)); buffer += sizeof(eint64);
	memcpy(&handler_token, buffer, sizeof(euint64)); buffer += sizeof(euint64);
	memcpy(&handler_stamp, buffer, sizeof(e_bigtime_t)); buffer += sizeof(e_bigtime_t);
	memcpy(&looper_token, buffer, sizeof(euint64)); buffer += sizeof(euint64);
	memcpy(&looper_stamp, buffer, sizeof(e_bigtime_t));

	if(fHandlerToken != E_MAXUINT64) etk_unref_handler(fHandlerToken);
	if(fLooperToken != E_MAXUINT64) etk_unref_handler(fLooperToken);

	fHandlerToken = E_MAXUINT64; fLooperToken = E_MAXUINT64;

	if(fSem) etk_delete_sem(fSem);
	if(fPort) etk_delete_port(fPort);

	fSem = NULL; fPort = NULL; fTargetTeam = E_INT64_CONSTANT(0);

	do{
		if(target_team != etk_get_current_team_id())
		{
			// TODO
			ETK_DEBUG("[APP]: %s --- Remote target unsupported.", __PRETTY_FUNCTION__);
			break;
		}

		if(handler_token == E_MAXUINT64 && looper_token == E_MAXUINT64) break;
		if(handler_stamp == E_MAXINT64 && looper_stamp == E_MAXINT64) break;

		if(handler_token != E_MAXUINT64) if(etk_ref_handler(handler_token)) fHandlerToken = handler_token;
		if(looper_token != E_MAXUINT64) if(etk_ref_handler(looper_token)) fLooperToken = looper_token;

		if(fLooperToken == E_MAXUINT64 || fHandlerToken != handler_token ||
		   (fHandlerToken == E_MAXUINT64 ? false : etk_get_handler_create_time_stamp(fHandlerToken) != handler_stamp) ||
		   (fLooperToken == E_MAXUINT64 ? false : etk_get_handler_create_time_stamp(fLooperToken) != looper_stamp))
		{
			if(fHandlerToken != E_MAXUINT64) etk_unref_handler(fHandlerToken);
			if(fLooperToken != E_MAXUINT64) etk_unref_handler(fLooperToken);
			fHandlerToken = E_MAXUINT64; fLooperToken = E_MAXUINT64;
			ETK_DEBUG("[APP]: %s --- Invalid target.", __PRETTY_FUNCTION__);
			break;
		}

		fTargetTeam = target_team;
	} while(false);

	return true;
}


void
EMessenger::PrintToStream() const
{
	ETK_OUTPUT("******** EMessenger Debug Output **********\n");

	ETK_OUTPUT("\tTarget team: %I64i\n", fTargetTeam);
	ETK_OUTPUT("\tToken of target handler: ");

	if(fHandlerToken == E_MAXUINT64) ETK_OUTPUT("E_MAXUINT64\n");
	else ETK_OUTPUT("%I64u\n", fHandlerToken);

	ETK_OUTPUT("\tToken of target looper: ");
	if(fLooperToken == E_MAXUINT64) ETK_OUTPUT("E_MAXUINT64\n");
	else ETK_OUTPUT("%I64u - %p\n", fLooperToken, etk_get_handler(fLooperToken));

	if(fPort == NULL) ETK_OUTPUT("\tPort invalid.\n");
	if(fSem == NULL) ETK_OUTPUT("\tSemaphore invalid.\n");

	ETK_OUTPUT("*******************************************\n");
}

