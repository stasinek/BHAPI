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
 * File: Clipboard.cpp
 * Description: an interface to a clipboard
 *
 * --------------------------------------------------------------------------*/

#include "./../kernel/Kernel.h"
#include "./../support/String.h"
#include "./../support/List.h"
#include "./../support/SimpleLocker.h"

#include "Clipboard.h"
#include "Application.h"


class _LOCAL _ESystemClipboard {
public:
	ESimpleLocker fLocker;
	EMessage fData;
	EList fWatchingList;

	_ESystemClipboard()
	{
	}

	~_ESystemClipboard()
	{
		EMessenger *msgr;
		while((msgr = (EMessenger*)fWatchingList.RemoveItem((eint32)0)) != NULL) delete msgr;
	}

	bool Lock()
	{
		return fLocker.Lock();
	}

	void Unlock()
	{
		fLocker.Unlock();
	}

	const EMessage& Data()
	{
		return fData;
	}

	euint32 Count()
	{
		return(fData.IsEmpty() ? 0 : 1);
	}

	e_status_t AddWatching(const EMessenger &target)
	{
		if(target.IsValid() == false) return E_ERROR;

		for(eint32 i = 0; i < fWatchingList.CountItems(); i++)
		{
			EMessenger *msgr = (EMessenger*)fWatchingList.ItemAt(i);
			if(*msgr == target) return E_ERROR;
		}

		EMessenger *msgr = new EMessenger(target);
		if(msgr->IsValid() == false || fWatchingList.AddItem(msgr) == false)
		{
			delete msgr;
			return E_ERROR;
		}

		return E_OK;
	}

	e_status_t RemoveWatching(const EMessenger &target)
	{
		for(eint32 i = 0; i < fWatchingList.CountItems(); i++)
		{
			EMessenger *msgr = (EMessenger*)fWatchingList.ItemAt(i);
			if(*msgr == target)
			{
				fWatchingList.RemoveItem(msgr);
				delete msgr;
				return E_OK;
			}
		}

		return E_ERROR;
	}

	e_status_t Commit(EMessage *msg)
	{
		if(msg == NULL || msg->IsEmpty()) return E_ERROR;

		fData = *msg;

		EMessage aMsg(E_CLIPBOARD_CHANGED);
		aMsg.AddInt64("when", e_real_time_clock_usecs());
		aMsg.AddString("name", "system");

		for(eint32 i = 0; i < fWatchingList.CountItems(); i++)
		{
			EMessenger *msgr = (EMessenger*)fWatchingList.ItemAt(i);
			msgr->SendMessage(&aMsg);
		}

		return E_OK;
	}
};

static _ESystemClipboard __etk_system_clipboard__;


EClipboard::EClipboard(const char *name)
	: fName(NULL), fData(NULL)
{
	if(name == NULL || strcmp(name, "system") != 0)
	{
		ETK_WARNING("[APP]: %s --- Only \"system\" supported yet, waiting for implementing!", __PRETTY_FUNCTION__);
		return;
	}

	fName = (name == NULL ? NULL : EStrdup(name));
	fData = new EMessage();
}


EClipboard::~EClipboard()
{
	if(fName) delete[] fName;
	if(fData) delete fData;
}


const char*
EClipboard::Name() const
{
	return fName;
}


bool
EClipboard::Lock()
{
	// TODO
	if(fName == NULL) return fLocker.Lock();

	if(fLocker.Lock() == false) return false;

	if(fLocker.CountLocks() == E_INT64_CONSTANT(1))
	{
		__etk_system_clipboard__.Lock();
		*fData = __etk_system_clipboard__.Data();
		__etk_system_clipboard__.Unlock();
	}

	return true;
}


void
EClipboard::Unlock()
{
	fLocker.Unlock();
}


eint64
EClipboard::CountLocks() const
{
	return fLocker.CountLocks();
}


e_status_t
EClipboard::Clear()
{
	if(fLocker.CountLocks() <= E_INT64_CONSTANT(0) || fData == NULL) return E_ERROR;
	fData->MakeEmpty();
	return E_OK;
}


e_status_t
EClipboard::Commit()
{
	// TODO
	if(fName == NULL) return E_ERROR;

	if(fLocker.CountLocks() <= E_INT64_CONSTANT(0) || fData == NULL || fData->IsEmpty()) return E_ERROR;

	__etk_system_clipboard__.Lock();
	__etk_system_clipboard__.Commit(fData);
	__etk_system_clipboard__.Unlock();

	return E_OK;
}


e_status_t
EClipboard::Revert()
{
	// TODO
	if(fName == NULL) return E_ERROR;

	if(fLocker.CountLocks() <= E_INT64_CONSTANT(0) || fData == NULL) return E_ERROR;

	__etk_system_clipboard__.Lock();
	*fData = __etk_system_clipboard__.Data();
	__etk_system_clipboard__.Unlock();

	return E_OK;
}


EMessenger
EClipboard::DataSource() const
{
	// TODO
	if(fName == NULL) return EMessenger();

	return etk_app_messenger;
}


EMessage*
EClipboard::Data() const
{
	return fData;
}


euint32
EClipboard::LocalCount() const
{
	// TODO
	return SystemCount();
}


euint32
EClipboard::SystemCount() const
{
	// TODO
	if(fName == NULL) return 0;

	__etk_system_clipboard__.Lock();
	euint32 retVal = __etk_system_clipboard__.Count();
	__etk_system_clipboard__.Unlock();

	return retVal;
}


e_status_t
EClipboard::StartWatching(const EMessenger &target)
{
	// TODO
	if(fName == NULL) return E_ERROR;

	if(target.IsValid() == false) return E_ERROR;
	__etk_system_clipboard__.Lock();
	e_status_t retVal = __etk_system_clipboard__.AddWatching(target);
	__etk_system_clipboard__.Unlock();

	return retVal;
}


e_status_t
EClipboard::StopWatching(const EMessenger &target)
{
	// TODO
	if(fName == NULL) return E_ERROR;

	__etk_system_clipboard__.Lock();
	e_status_t retVal = __etk_system_clipboard__.RemoveWatching(target);
	__etk_system_clipboard__.Unlock();

	return retVal;
}

