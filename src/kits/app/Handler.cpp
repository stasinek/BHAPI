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
 * File: Handler.cpp
 *
 * --------------------------------------------------------------------------*/

#include "Handler.h"
#include "Looper.h"
#include "Messenger.h"
#include "MessageFilter.h"
#include "Message.h"
#include "MessageQueue.h"
#include "AppDefs.h"

#include "../kernel/Kernel.h"
#include "../kernel/Debug.h"
#include "../support/Errors.h"
#include "../support/StringMe.h"
#include "../support/List.h"
#include "../support/Locker.h"
#include "../support/Autolock.h"
#include "../support/List.h"
#include "../private/PrivateApplication.h"
#include "../private/PrivateHandler.h"


class _LOCAL EObserverList {
public:
	EObserverList();
	~EObserverList();

	b_status_t	AddWatching(BMessenger msgr, b_uint32 what);
	b_status_t	RemoveWatching(BMessenger msgr, b_uint32 what);
	bool		IsWatched(b_uint32 what) const;
	BList		*GetObserverList(b_uint32 what) const;

private:
	BList fListWatching;
	BList fListWatchingAll;
};


class _LOCAL EWatchingInfo {
private:
	BMessenger fMessenger;
	BList fWhats;

public:
	EWatchingInfo(BMessenger msgr)
		: fMessenger(msgr)
	{
	}

	~EWatchingInfo()
	{
	}

	bool IsValid() const
	{
		return fMessenger.IsValid();
	}

	BMessenger *Messenger() const
	{
		return const_cast<BMessenger*>(&fMessenger);
	}

	bool IsSameMessenger(BMessenger msgr) const
	{
		return(fMessenger == msgr);
	}

	bool AddWhat(b_uint32 what)
	{
		if(what == B_OBSERVER_OBSERVE_ALL) return false;
        return fWhats.AddItem(reinterpret_cast<void*>(what));
	}

	bool RemoveWhat(b_uint32 what)
	{
		if(what == B_OBSERVER_OBSERVE_ALL) return false;

		b_int32 save_count = fWhats.CountItems();

		for(b_int32 i = 0; i < fWhats.CountItems(); i++)
		{
            if(reinterpret_cast<b_uint32>(fWhats.ItemAt(i)) == what)
			{
				fWhats.RemoveItem(i);
				break;
			}
		}

		return(save_count > fWhats.CountItems());
	}

	bool HasWhat(b_uint32 what) const
	{
		if(what == B_OBSERVER_OBSERVE_ALL) return false;

		for(b_int32 i = 0; i < fWhats.CountItems(); i++)
		{
            if(reinterpret_cast<b_uint32>(fWhats.ItemAt(i)) == what) return true;
		}
		return false;
	}

	b_int32 CountWhats() const
	{
		return fWhats.CountItems();
	}

	void MakeEmpty()
	{
		fWhats.MakeEmpty();
	}
};


EObserverList::EObserverList()
{
}


EObserverList::~EObserverList()
{
	for(b_int32 i = 0; i < fListWatching.CountItems(); i++)
	{
		delete (EWatchingInfo*)fListWatching.ItemAt(i);
	}

	for(b_int32 i = 0; i < fListWatchingAll.CountItems(); i++)
	{
		delete (EWatchingInfo*)fListWatchingAll.ItemAt(i);
	}
}


b_status_t
EObserverList::AddWatching(BMessenger msgr, b_uint32 what)
{
	if(msgr.IsValid() == false) return B_BAD_HANDLER;

	EWatchingInfo *info = NULL;
	b_int32 index_single = -1, index_all = -1;

	for(b_int32 i = 0; i < fListWatching.CountItems(); i++)
	{
		if(((EWatchingInfo*)fListWatching.ItemAt(i))->IsSameMessenger(msgr))
		{
			index_single = i;
			break;
		}
	}
	for(b_int32 i = 0; i < fListWatchingAll.CountItems(); i++)
	{
		if(((EWatchingInfo*)fListWatchingAll.ItemAt(i))->IsSameMessenger(msgr))
		{
			index_all = i;
			break;
		}
	}

	if(what == B_OBSERVER_OBSERVE_ALL)
	{
		if(index_all < 0)
		{
			if(fListWatchingAll.AddItem(info = new EWatchingInfo(msgr)) == false)
			{
				delete info;
				return B_ERROR;
			}
		}
		else
		{
			((EWatchingInfo*)fListWatchingAll.ItemAt(index_all))->MakeEmpty();
		}

		if(index_single >= 0)
			delete (EWatchingInfo*)fListWatching.RemoveItem(index_single);

		return B_OK;
	}
	else if(index_all >= 0)
	{
		((EWatchingInfo*)fListWatchingAll.ItemAt(index_all))->RemoveWhat(what);
		return B_OK;
	}

	if((info = (EWatchingInfo*)fListWatching.ItemAt(index_single)) == NULL)
	{
		if(fListWatching.AddItem(info = new EWatchingInfo(msgr)) == false)
		{
			delete info;
			return B_ERROR;
		}
		return B_OK;
	}
	else
	{
		return((info->HasWhat(what) || info->AddWhat(what)) ? B_OK : B_ERROR);
	}
}


b_status_t
EObserverList::RemoveWatching(BMessenger msgr, b_uint32 what)
{
	if(msgr.IsValid() == false) return B_BAD_HANDLER;

	EWatchingInfo *info = NULL;
	b_int32 index_single = -1, index_all = -1;

	for(b_int32 i = 0; i < fListWatching.CountItems(); i++)
	{
		if(((EWatchingInfo*)fListWatching.ItemAt(i))->IsSameMessenger(msgr))
		{
			index_single = i;
			break;
		}
	}
	for(b_int32 i = 0; i < fListWatchingAll.CountItems(); i++)
	{
		if(((EWatchingInfo*)fListWatchingAll.ItemAt(i))->IsSameMessenger(msgr))
		{
			index_all = i;
			break;
		}
	}

	if(what == B_OBSERVER_OBSERVE_ALL)
	{
		if(index_single >= 0) delete (EWatchingInfo*)fListWatching.RemoveItem(index_single);
		if(index_all >= 0) delete (EWatchingInfo*)fListWatchingAll.RemoveItem(index_all);
	}
	else
	{
		if(index_all >= 0)
		{
			info = (EWatchingInfo*)fListWatchingAll.ItemAt(index_all);
			if(!(info->HasWhat(what) || info->AddWhat(what))) return B_ERROR;
		}
		if(index_single >= 0)
		{
			info = (EWatchingInfo*)fListWatching.ItemAt(index_single);
			info->RemoveWhat(what);
			if(info->CountWhats() <= 0) delete (EWatchingInfo*)fListWatching.RemoveItem(index_single);
		}
	}

	return B_OK;
}


BList*
EObserverList::GetObserverList(b_uint32 what) const
{
	BList *list = new BList();

	if(what == B_OBSERVER_OBSERVE_ALL)
	{
		for(b_int32 i = 0; i < fListWatchingAll.CountItems(); i++)
			list->AddItem(((EWatchingInfo*)fListWatchingAll.ItemAt(i))->Messenger());

		for(b_int32 i = 0; i < fListWatching.CountItems(); i++)
			list->AddItem(((EWatchingInfo*)fListWatching.ItemAt(i))->Messenger());
	}
	else
	{
		for(b_int32 i = 0; i < fListWatching.CountItems(); i++)
		{
			EWatchingInfo *aInfo = (EWatchingInfo*)fListWatching.ItemAt(i);
			if(aInfo->HasWhat(what)) list->AddItem(aInfo->Messenger());
		}

		for(b_int32 i = 0; i < fListWatchingAll.CountItems(); i++)
		{
			EWatchingInfo *aInfo = (EWatchingInfo*)fListWatchingAll.ItemAt(i);
			if(aInfo->HasWhat(what) == false) list->AddItem(aInfo->Messenger());
		}
	}

	if(list->IsEmpty())
	{
		delete list;
		list = NULL;
	}

	return list;
}


bool
EObserverList::IsWatched(b_uint32 what) const
{
	if(what == B_OBSERVER_OBSERVE_ALL) return(fListWatching.IsEmpty() == false || fListWatchingAll.IsEmpty() == false);

	for(b_int32 i = 0; i < fListWatching.CountItems(); i++)
	{
		if(((EWatchingInfo*)fListWatching.ItemAt(i))->HasWhat(what)) return true;
	}

	b_int32 exclude_times = 0;
	for(b_int32 i = 0; i < fListWatchingAll.CountItems(); i++)
	{
		if(((EWatchingInfo*)fListWatchingAll.ItemAt(i))->HasWhat(what)) exclude_times++;
	}
	return(exclude_times != fListWatchingAll.CountItems());
}


BHandler::BHandler(const char *name)
	: BArchivable(),
	  fLooper(NULL),
	  fPrevHandler(NULL), fNextHandler(NULL),
	  fObserverList(NULL), fFilters(NULL)
{
	fName = b_strdup(name);
    fToken = bhapi_app_connector->HandlersDepot()->CreateToken(reinterpret_cast<void*>(this));
}


BHandler::~BHandler()
{
	if(fName != NULL) delete[] fName;
	if(fToken != NULL) delete fToken;
    if(fObserverList != NULL) delete reinterpret_cast<EObserverList*>(fObserverList);
	if(fFilters != NULL)
	{
		BHandler::SetFilterList(NULL);
		delete fFilters;
	}
}


BHandler::BHandler(const BMessage *from)
	: BArchivable(from),
	  fLooper(NULL),
	  fPrevHandler(NULL), fNextHandler(NULL),
	  fObserverList(NULL), fFilters(NULL)
{
	const char *name = NULL;
    if(from != NULL) from->FindString("_name", &name);

	fName = b_strdup(name);
    fToken = bhapi_app_connector->HandlersDepot()->CreateToken(reinterpret_cast<void*>(this));
}


b_status_t
BHandler::Archive(BMessage *into, bool deep) const
{
	if(!into) return B_ERROR;

	BArchivable::Archive(into, deep);
	into->AddString("class", "BHandler");
	into->AddString("_name", fName);

	return B_OK;
}


BArchivable*
BHandler::Instantiate(const BMessage *from)
{
	return(b_validatb_instantiation(from, "BHandler") == false ? NULL : new BHandler(from));
}


void
BHandler::SetName(const char *name)
{
	if(fName != NULL) delete[] fName;
	fName = b_strdup(name);
}


const char*
BHandler::Name() const
{
	return fName;
}


void
BHandler::MessageReceived(BMessage *message)
{
	if(message == NULL || message->IsSystem() || fNextHandler == NULL || fNextHandler == fLooper) return;
	fNextHandler->MessageReceived(message);
}


void
BHandler::SetNextHandler(BHandler *handler)
{
	if(fLooper == NULL || handler == this || fNextHandler == handler) return;

	if(handler == NULL) BHAPI_ERROR("[APP]: %s --- Invalid operation", __PRETTY_FUNCTION__);

	if(handler == fLooper)
	{
		// Before:	fLooper ... fPrevHandler, this, fNextHandler ...
		// After:	fLooper ... fPrevHandler, fNextHandler ... this
		fPrevHandler->fNextHandler = fNextHandler;
		fNextHandler->fPrevHandler = fPrevHandler;
		fLooper->fPrevHandler->fNextHandler = this;

		BHandler *save_handler = fPrevHandler;

		fPrevHandler = fLooper->fPrevHandler;
		fNextHandler = fLooper;

		// call virtual function
		fPrevHandler->SetNextHandler(fPrevHandler->fNextHandler);
		save_handler->SetNextHandler(save_handler->fNextHandler);
	}
	else if(handler != NULL)
	{
		if(handler->fLooper != fLooper) return;

		BHandler *last_handler = handler;
		while(!(last_handler->fNextHandler == fLooper ||
		        last_handler->fNextHandler == this)) last_handler = last_handler->fNextHandler;

		fNextHandler->fPrevHandler = last_handler;
		last_handler->fNextHandler = fNextHandler;

		if(last_handler->fNextHandler == fLooper)
		{
			// Before:	fLooper ... this, fNextHandler ... handler ... last_handler
			// After:	fLooper ... this, handler ... last_handler, fNextHandler ...
			handler->fPrevHandler->fNextHandler = fLooper;
			fLooper->fPrevHandler = handler->fPrevHandler;
		}
		else // last_handler->fNextHandler == this
		{
			// Before:	fLooper ... handler ... last_handler, this, fNextHandler ...
			// After:	fLooper ... this, handler ... last_handler, fNextHandler ...
			handler->fPrevHandler->fNextHandler = this;
			fPrevHandler = handler->fPrevHandler;
		}

		BHandler *save_handler = handler->fPrevHandler;

		handler->fPrevHandler = this;
		fNextHandler = handler;

		// call virtual function
		last_handler->SetNextHandler(last_handler->fNextHandler);
		save_handler->SetNextHandler(save_handler->fNextHandler);
	}
}


BHandler*
BHandler::NextHandler() const
{
	return fNextHandler;
}


BLooper*
BHandler::Looper() const
{
	return fLooper;
}


bool
BHandler::LockLooper()
{
	return(LockLooperWithTimeout(B_INFINITE_TIMEOUT) == B_OK);
}


b_status_t
BHandler::LockLooperWithTimeout(b_bigtime_t microseconds_timeout)
{
	BAutolock <BLocker>autolock(bhapi_get_handler_operator_locker());
	return(fLooper == NULL ? B_BAD_VALUE : fLooper->LockWithTimeout(microseconds_timeout));
}


void
BHandler::UnlockLooper()
{
	if(fLooper) fLooper->Unlock();
}


b_status_t
BHandler::StartWatching(BMessenger msgr, b_uint32 what)
{
    if(fObserverList == NULL) fObserverList = reinterpret_cast<void*>(new EObserverList());
    return reinterpret_cast<EObserverList*>(fObserverList)->AddWatching(msgr, what);
}


b_status_t
BHandler::StartWatchingAll(BMessenger msgr)
{
	return StartWatching(msgr, B_OBSERVER_OBSERVE_ALL);
}


b_status_t
BHandler::StopWatching(BMessenger msgr, b_uint32 what)
{
	if(fObserverList == NULL) return B_ERROR;
    return reinterpret_cast<EObserverList*>(fObserverList)->RemoveWatching(msgr, what);
}


b_status_t
BHandler::StopWatchingAll(BMessenger msgr)
{
	return StopWatching(msgr, B_OBSERVER_OBSERVE_ALL);
}


b_status_t
BHandler::StartWatching(BHandler *handler, b_uint32 what)
{
	b_status_t status;
	BMessenger msgr(handler, NULL, &status);
	if(status != B_OK) return status;

	return StartWatching(msgr, what);
}


b_status_t
BHandler::StartWatchingAll(BHandler *handler)
{
	b_status_t status;
	BMessenger msgr(handler, NULL, &status);
	if(status != B_OK) return status;

	return StartWatchingAll(msgr);
}


b_status_t
BHandler::StopWatching(BHandler *handler, b_uint32 what)
{
	b_status_t status;
	BMessenger msgr(handler, NULL, &status);
	if(status != B_OK) return status;

	return StopWatching(msgr, what);
}


b_status_t
BHandler::StopWatchingAll(BHandler *handler)
{
	b_status_t status;
	BMessenger msgr(handler, NULL, &status);
	if(status != B_OK) return status;

	return StopWatchingAll(msgr);
}


void
BHandler::SendNotices(b_uint32 what, const BMessage *message)
{
	if(fObserverList == NULL) return;

    BList *msgrsList = reinterpret_cast<EObserverList*>(fObserverList)->GetObserverList(what);
	if(msgrsList == NULL) return;

	BMessage msg(B_OBSERVER_NOTICE_CHANGE);
	if(message != NULL)
	{
		msg = *message;
		msg.what = B_OBSERVER_NOTICE_CHANGE;
		msg.AddInt32(B_OBSERVE_ORIGINAL_WHAT, message->what);
	}
	msg.AddInt32(B_OBSERVE_WHAT_CHANGE, what);

	for(b_int32 i = 0; i < msgrsList->CountItems(); i++)
	{
		BMessenger *aMsgr = (BMessenger*)msgrsList->ItemAt(i);

		if(aMsgr->SendMessage(&msg, (BHandler*)NULL, 50000) != B_OK)
		{
			if(aMsgr->IsTargetLocal())
			{
				BLooper *looper = NULL;
				aMsgr->Target(&looper);
				if(looper == NULL) StopWatchingAll(*aMsgr);
			}
			else
			{
				// TODO
			}
		}
	}

	delete msgrsList;
}


bool
BHandler::IsWatched(b_uint32 what) const
{
	if(fObserverList == NULL) return false;
    return reinterpret_cast<EObserverList*>(fObserverList)->IsWatched(what);
}


bool
BHandler::AddFilter(BMessageFilter *filter)
{
	if(filter == NULL || filter->fHandler != NULL) return false;
	if(fFilters == NULL) fFilters = new BList();
	if(fFilters->AddItem(filter) == false) return false;
	filter->fHandler = this;
	return true;
}


bool
BHandler::RemoveFilter(BMessageFilter *filter)
{
	if(fFilters == NULL || filter == NULL || filter->fHandler != this || fFilters->RemoveItem(filter) == false) return false;
	filter->fHandler = NULL;
	return true;
}


const BList*
BHandler::FilterList() const
{
	return fFilters;
}


bool
BHandler::SetFilterList(const BList *filterList)
{
	if(fFilters != NULL)
	{
		// Here we delete all filters without calling "RemoveFilter",
		// if you care about this, you should inherit this function.
		for(b_int32 i = 0; i < fFilters->CountItems(); i++)
		{
			BMessageFilter *filter = (BMessageFilter*)fFilters->ItemAt(i);
			filter->fHandler = NULL;
			delete filter;
		}

		fFilters->MakeEmpty();
	}

	if(filterList != NULL)
	{
		for(b_int32 i = 0; i < filterList->CountItems(); i++) AddFilter((BMessageFilter*)filterList->ItemAt(i));
	}

	return true;
}


BHandler*
BHandler::ResolveSpecifier(BMessage *msg, b_int32 index, BMessage *specifier, b_int32 what, const char *property)
{
	// TODO
	BHAPI_WARNING("[APP]: %s --- TODO", __PRETTY_FUNCTION__);
	return NULL;
}


b_status_t
BHandler::GetSupportedSuites(BMessage *data)
{
	// TODO
	BHAPI_WARNING("[APP]: %s --- TODO", __PRETTY_FUNCTION__);
	return B_ERROR;
}
