/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
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
 * File: Clipboard.cpp
 * Description: an interface to a clipboard
 *
 * --------------------------------------------------------------------------*/

#include "Clipboard.h"
#include "Application.h"

#include "../kernel/Kernel.h"
#include "../kernel/Debug.h"
#include "../support/StringClass.h"
#include "../support/List.h"
#include "../support/SimpleLocker.h"
#include "../support/Errors.h"
#include "../app/Message.h"
#include "../app/AppDefs.h"

class BHAPI_LOCAL BSystemClipboard {
public:
	BSimpleLocker fLocker;
	BMessage fData;
	BList fWatchingList;

    BSystemClipboard()
	{
	}

    ~BSystemClipboard()
	{
		BMessenger *msgr;
		while((msgr = (BMessenger*)fWatchingList.RemoveItem((__be_int32)0)) != NULL) delete msgr;
	}

	bool Lock()
	{
		return fLocker.Lock();
	}

	void Unlock()
	{
		fLocker.Unlock();
	}

	const BMessage& Data()
	{
		return fData;
	}

	__be_uint32 Count()
	{
		return(fData.IsEmpty() ? 0 : 1);
	}

	status_t AddWatching(const BMessenger &target)
	{
		if(target.IsValid() == false) return B_ERROR;

		for(__be_int32 i = 0; i < fWatchingList.CountItems(); i++)
		{
			BMessenger *msgr = (BMessenger*)fWatchingList.ItemAt(i);
			if(*msgr == target) return B_ERROR;
		}

		BMessenger *msgr = new BMessenger(target);
		if(msgr->IsValid() == false || fWatchingList.AddItem(msgr) == false)
		{
			delete msgr;
			return B_ERROR;
		}

		return B_OK;
	}

	status_t RemoveWatching(const BMessenger &target)
	{
		for(__be_int32 i = 0; i < fWatchingList.CountItems(); i++)
		{
			BMessenger *msgr = (BMessenger*)fWatchingList.ItemAt(i);
			if(*msgr == target)
			{
				fWatchingList.RemoveItem(msgr);
				delete msgr;
				return B_OK;
			}
		}

		return B_ERROR;
	}

	status_t Commit(BMessage *msg)
	{
		if(msg == NULL || msg->IsEmpty()) return B_ERROR;

		fData = *msg;

		BMessage aMsg(B_CLIPBOARD_CHANGED);
		aMsg.AddInt64("when", b_real_time_clock_usecs());
		aMsg.AddString("name", "system");

		for(__be_int32 i = 0; i < fWatchingList.CountItems(); i++)
		{
			BMessenger *msgr = (BMessenger*)fWatchingList.ItemAt(i);
			msgr->SendMessage(&aMsg);
		}

		return B_OK;
	}
};

static BSystemClipboard __bhapi_system_clipboard__;

BClipboard::BClipboard(const char *name)
	: fName(NULL), fData(NULL)
{
	if(name == NULL || strcmp(name, "system") != 0)
	{
		BHAPI_WARNING("[APP]: %s --- Only \"system\" supported yet, waiting for implementing!", __PRETTY_FUNCTION__);
		return;
	}

	fName = (name == NULL ? NULL : bhapi::strdup(name));
	fData = new BMessage();
}

BClipboard::~BClipboard()
{
	if(fName) delete[] fName;
	if(fData) delete fData;
}

const char* BClipboard::Name() const
{
	return fName;
}

bool BClipboard::Lock()
{
	// TODO
	if(fName == NULL) return fLocker.Lock();

	if(fLocker.Lock() == false) return false;

	if(fLocker.CountLocks() == B_INT64_CONSTANT(1))
	{
		__bhapi_system_clipboard__.Lock();
		*fData = __bhapi_system_clipboard__.Data();
		__bhapi_system_clipboard__.Unlock();
	}

	return true;
}

void BClipboard::Unlock()
{
	fLocker.Unlock();
}

__be_int64 BClipboard::CountLocks() const
{
	return fLocker.CountLocks();
}

status_t BClipboard::Clear()
{
	if(fLocker.CountLocks() <= B_INT64_CONSTANT(0) || fData == NULL) return B_ERROR;
	fData->MakeEmpty();
	return B_OK;
}

status_t BClipboard::Commit()
{
	// TODO
	if(fName == NULL) return B_ERROR;

	if(fLocker.CountLocks() <= B_INT64_CONSTANT(0) || fData == NULL || fData->IsEmpty()) return B_ERROR;

	__bhapi_system_clipboard__.Lock();
	__bhapi_system_clipboard__.Commit(fData);
	__bhapi_system_clipboard__.Unlock();

	return B_OK;
}

status_t BClipboard::Revert()
{
	// TODO
	if(fName == NULL) return B_ERROR;

	if(fLocker.CountLocks() <= B_INT64_CONSTANT(0) || fData == NULL) return B_ERROR;

	__bhapi_system_clipboard__.Lock();
	*fData = __bhapi_system_clipboard__.Data();
	__bhapi_system_clipboard__.Unlock();

	return B_OK;
}

BMessenger BClipboard::DataSource() const
{
	// TODO
	if(fName == NULL) return BMessenger();

	return bhapi::__be_app_messenger;
}

BMessage* BClipboard::Data() const
{
	return fData;
}

__be_uint32 BClipboard::LocalCount() const
{
	// TODO
	return SystemCount();
}

__be_uint32 BClipboard::SystemCount() const
{
	// TODO
	if(fName == NULL) return 0;

	__bhapi_system_clipboard__.Lock();
	__be_uint32 retVal = __bhapi_system_clipboard__.Count();
	__bhapi_system_clipboard__.Unlock();

	return retVal;
}

status_t BClipboard::StartWatching(const BMessenger &target)
{
	// TODO
	if(fName == NULL) return B_ERROR;

	if(target.IsValid() == false) return B_ERROR;
	__bhapi_system_clipboard__.Lock();
	status_t retVal = __bhapi_system_clipboard__.AddWatching(target);
	__bhapi_system_clipboard__.Unlock();

	return retVal;
}

status_t BClipboard::StopWatching(const BMessenger &target)
{
	// TODO
	if(fName == NULL) return B_ERROR;

	__bhapi_system_clipboard__.Lock();
	status_t retVal = __bhapi_system_clipboard__.RemoveWatching(target);
	__bhapi_system_clipboard__.Unlock();

	return retVal;
}

