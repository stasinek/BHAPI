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
 * File: PrivateHandler.cpp
 *
 * --------------------------------------------------------------------------*/

#include "PrivateHandler.h"
#include "PrivateApplication.h"

#include "../kernel/Kernel.h"
#include "../support/ClassInfo.h"
#include "../support/Autolock.h"
#include "../support/Errors.h"
#include "../support/Locker.h"
#include "../app/Looper.h"

BLocker* bhapi::get_handler_operator_locker()
{
	return bhapi::app_connector->HandlersDepot()->Locker();
}


b_uint64 bhapi::get_handler_token(const BHandler *handler)
{
	return((handler == NULL || handler->fToken == NULL) ? B_MAXUINT64 : handler->fToken->Token());
}


b_uint64 bhapi::get_ref_handler_token(const BHandler *handler)
{
	b_uint64 retVal = B_MAXUINT64;

	BAutolock <BTokensDepot>autolock(bhapi::app_connector->HandlersDepot());

	BToken *aToken = bhapi::app_connector->HandlersDepot()->FetchToken(bhapi::get_handler_token(handler));
	if(aToken != NULL)
	{
		b_uint64 vitalities = aToken->Vitalities();
		aToken->operator++();
		if(aToken->Vitalities() != vitalities) retVal = aToken->Token();
	}

	return retVal;
}


BHandler* bhapi::get_handler(b_uint64 token)
{
	BHandler *retVal = NULL;

	BAutolock <BTokensDepot>autolock(bhapi::app_connector->HandlersDepot());

	BToken *aToken = bhapi::app_connector->HandlersDepot()->FetchToken(token);
	if(aToken != NULL) retVal = reinterpret_cast<BHandler*>(aToken->Data());

	return retVal;
}


b_bigtime_t bhapi::get_handler_create_time_stamp(b_uint64 token)
{
	b_bigtime_t retVal = B_MAXINT64;

	BAutolock <BTokensDepot>autolock(bhapi::app_connector->HandlersDepot());

	BToken *aToken = bhapi::app_connector->HandlersDepot()->FetchToken(token);
	if(aToken != NULL) retVal = aToken->TimeStamp();

	return retVal;
}


BLooper* bhapi::get_handler_looper(b_uint64 token)
{
	BAutolock <BTokensDepot>autolock(bhapi::app_connector->HandlersDepot());

	BHandler *handler = bhapi::get_handler(token);
	return(handler == NULL ? NULL : handler->Looper());
}


b_uint64 bhapi::get_ref_looper_token(b_uint64 token)
{
	BAutolock <BTokensDepot>autolock(bhapi::app_connector->HandlersDepot());

	BHandler *handler = bhapi::get_handler(token);
	return(handler == NULL ? B_MAXUINT64 : bhapi::get_ref_handler_token(handler->Looper()));
}


b_status_t bhapi::lock_looper_of_handler(b_uint64 token, b_bigtime_t timeout)
{
	BLocker *handlers_locker = bhapi::get_handler_operator_locker();

	handlers_locker->Lock();

	BLooper *looper = bhapi::get_handler_looper(token);
	BLooper *looper_proxy = (looper != NULL ? looper->_Proxy() : NULL);
    void *locker = ((looper == NULL || looper->fLocker == NULL) ? NULL : bhapi::clone_locker(looper->fLocker));

	if(locker == NULL)
	{
		handlers_locker->Unlock();
		return B_BAD_VALUE;
	}

	b_int64 save_count = handlers_locker->CountLocks();
	while(handlers_locker->CountLocks() > 0) handlers_locker->Unlock();

    b_status_t retVal = bhapi::lock_locker_etc(locker, B_TIMEOUT, timeout);
	if(retVal == B_OK)
	{
		handlers_locker->Lock();
		if(looper != bhapi::get_handler_looper(token) || looper_proxy != looper->_Proxy()) retVal = B_MISMATCHED_VALUES;
		if(save_count-- == 1) handlers_locker->Unlock();
        if(retVal != B_OK) bhapi::unlock_locker(locker);
	}
    bhapi::delete_locker(locker);

	while(save_count-- > 1) handlers_locker->Lock();

	return retVal;
}


bool bhapi::is_current_at_looper_thread(b_uint64 token)
{
	BAutolock <BTokensDepot>autolock(bhapi::app_connector->HandlersDepot());

    BLooper *looper = cast_as(bhapi::get_handler(token), BLooper);
	if(looper == NULL) return false;

	bool retVal = (looper->Thread() == bhapi::get_current_thread_id() ? true : false);

	return retVal;
}


bool bhapi::ref_handler(b_uint64 token)
{
	bool retVal = false;

	BAutolock <BTokensDepot>autolock(bhapi::app_connector->HandlersDepot());

	BToken *aToken = bhapi::app_connector->HandlersDepot()->FetchToken(token);
	if(aToken != NULL)
	{
		b_uint64 vitalities = aToken->Vitalities();
		aToken->operator++();
		if(aToken->Vitalities() != vitalities) retVal = true;
	}

	return retVal;
}


void bhapi::unref_handler(b_uint64 token)
{
	BAutolock <BTokensDepot>autolock(bhapi::app_connector->HandlersDepot());

	BToken *aToken = bhapi::app_connector->HandlersDepot()->FetchToken(token);
	if(aToken != NULL) aToken->operator--();
}

