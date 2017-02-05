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
 * File: PrivateHandler.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../kernel/Kernel.h"
#include "./../support/ClassInfo.h"
#include "./../support/Autolock.h"

#include "PrivateApplication.h"
#include "PrivateHandler.h"


ELocker* etk_get_handler_operator_locker()
{
	return etk_app_connector->HandlersDepot()->Locker();
}


euint64 etk_get_handler_token(const EHandler *handler)
{
	return((handler == NULL || handler->fToken == NULL) ? E_MAXUINT64 : handler->fToken->Token());
}


euint64 etk_get_ref_handler_token(const EHandler *handler)
{
	euint64 retVal = E_MAXUINT64;

	EAutolock <ETokensDepot>autolock(etk_app_connector->HandlersDepot());

	EToken *aToken = etk_app_connector->HandlersDepot()->FetchToken(etk_get_handler_token(handler));
	if(aToken != NULL)
	{
		euint64 vitalities = aToken->Vitalities();
		aToken->operator++();
		if(aToken->Vitalities() != vitalities) retVal = aToken->Token();
	}

	return retVal;
}


EHandler* etk_get_handler(euint64 token)
{
	EHandler *retVal = NULL;

	EAutolock <ETokensDepot>autolock(etk_app_connector->HandlersDepot());

	EToken *aToken = etk_app_connector->HandlersDepot()->FetchToken(token);
	if(aToken != NULL) retVal = reinterpret_cast<EHandler*>(aToken->Data());

	return retVal;
}


e_bigtime_t etk_get_handler_create_time_stamp(euint64 token)
{
	e_bigtime_t retVal = E_MAXINT64;

	EAutolock <ETokensDepot>autolock(etk_app_connector->HandlersDepot());

	EToken *aToken = etk_app_connector->HandlersDepot()->FetchToken(token);
	if(aToken != NULL) retVal = aToken->TimeStamp();

	return retVal;
}


ELooper* etk_get_handler_looper(euint64 token)
{
	EAutolock <ETokensDepot>autolock(etk_app_connector->HandlersDepot());

	EHandler *handler = etk_get_handler(token);
	return(handler == NULL ? NULL : handler->Looper());
}


euint64 etk_get_ref_looper_token(euint64 token)
{
	EAutolock <ETokensDepot>autolock(etk_app_connector->HandlersDepot());

	EHandler *handler = etk_get_handler(token);
	return(handler == NULL ? E_MAXUINT64 : etk_get_ref_handler_token(handler->Looper()));
}


e_status_t etk_lock_looper_of_handler(euint64 token, e_bigtime_t timeout)
{
	ELocker *handlers_locker = etk_get_handler_operator_locker();

	handlers_locker->Lock();

	ELooper *looper = etk_get_handler_looper(token);
	ELooper *looper_proxy = (looper != NULL ? looper->_Proxy() : NULL);
	void *locker = ((looper == NULL || looper->fLocker == NULL) ? NULL : etk_clone_locker(looper->fLocker));

	if(locker == NULL)
	{
		handlers_locker->Unlock();
		return E_BAD_VALUE;
	}

	eint64 save_count = handlers_locker->CountLocks();
	while(handlers_locker->CountLocks() > 0) handlers_locker->Unlock();

	e_status_t retVal = etk_lock_locker_etc(locker, E_TIMEOUT, timeout);
	if(retVal == E_OK)
	{
		handlers_locker->Lock();
		if(looper != etk_get_handler_looper(token) || looper_proxy != looper->_Proxy()) retVal = E_MISMATCHED_VALUES;
		if(save_count-- == 1) handlers_locker->Unlock();
		if(retVal != E_OK) etk_unlock_locker(locker);
	}
	etk_delete_locker(locker);

	while(save_count-- > 1) handlers_locker->Lock();

	return retVal;
}


bool etk_is_current_at_looper_thread(euint64 token)
{
	EAutolock <ETokensDepot>autolock(etk_app_connector->HandlersDepot());

	ELooper *looper = e_cast_as(etk_get_handler(token), ELooper);
	if(looper == NULL) return false;

	bool retVal = (looper->Thread() == etk_get_current_thread_id() ? true : false);

	return retVal;
}


bool etk_ref_handler(euint64 token)
{
	bool retVal = false;

	EAutolock <ETokensDepot>autolock(etk_app_connector->HandlersDepot());

	EToken *aToken = etk_app_connector->HandlersDepot()->FetchToken(token);
	if(aToken != NULL)
	{
		euint64 vitalities = aToken->Vitalities();
		aToken->operator++();
		if(aToken->Vitalities() != vitalities) retVal = true;
	}

	return retVal;
}


void etk_unref_handler(euint64 token)
{
	EAutolock <ETokensDepot>autolock(etk_app_connector->HandlersDepot());

	EToken *aToken = etk_app_connector->HandlersDepot()->FetchToken(token);
	if(aToken != NULL) aToken->operator--();
}

