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
 * File: Handler.cpp
 *
 * --------------------------------------------------------------------------*/

#include "./../kernel/Kernel.h"
#include "./../support/String.h"
#include "./../support/List.h"
#include "./../support/Locker.h"
#include "./../support/Autolock.h"

#include "./../private/PrivateApplication.h"
#include "./../private/PrivateHandler.h"

#include "Handler.h"
#include "Looper.h"
#include "Messenger.h"
#include "MessageFilter.h"


class _LOCAL EObserverList {
public:
	EObserverList();
	~EObserverList();

	e_status_t	AddWatching(EMessenger msgr, euint32 what);
	e_status_t	RemoveWatching(EMessenger msgr, euint32 what);
	bool		IsWatched(euint32 what) const;
	EList		*GetObserverList(euint32 what) const;

private:
	EList fListWatching;
	EList fListWatchingAll;
};


class _LOCAL EWatchingInfo {
private:
	EMessenger fMessenger;
	EList fWhats;

public:
	EWatchingInfo(EMessenger msgr)
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

	EMessenger *Messenger() const
	{
		return const_cast<EMessenger*>(&fMessenger);
	}

	bool IsSameMessenger(EMessenger msgr) const
	{
		return(fMessenger == msgr);
	}

	bool AddWhat(euint32 what)
	{
		if(what == E_OBSERVER_OBSERVE_ALL) return false;
		return fWhats.AddItem(reinterpret_cast<void*>(what));
	}

	bool RemoveWhat(euint32 what)
	{
		if(what == E_OBSERVER_OBSERVE_ALL) return false;

		eint32 save_count = fWhats.CountItems();

		for(eint32 i = 0; i < fWhats.CountItems(); i++)
		{
			if(reinterpret_cast<euint32>(fWhats.ItemAt(i)) == what)
			{
				fWhats.RemoveItem(i);
				break;
			}
		}

		return(save_count > fWhats.CountItems());
	}

	bool HasWhat(euint32 what) const
	{
		if(what == E_OBSERVER_OBSERVE_ALL) return false;

		for(eint32 i = 0; i < fWhats.CountItems(); i++)
		{
			if(reinterpret_cast<euint32>(fWhats.ItemAt(i)) == what) return true;
		}
		return false;
	}

	eint32 CountWhats() const
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
	for(eint32 i = 0; i < fListWatching.CountItems(); i++)
	{
		delete (EWatchingInfo*)fListWatching.ItemAt(i);
	}

	for(eint32 i = 0; i < fListWatchingAll.CountItems(); i++)
	{
		delete (EWatchingInfo*)fListWatchingAll.ItemAt(i);
	}
}


e_status_t
EObserverList::AddWatching(EMessenger msgr, euint32 what)
{
	if(msgr.IsValid() == false) return E_BAD_HANDLER;

	EWatchingInfo *info = NULL;
	eint32 index_single = -1, index_all = -1;

	for(eint32 i = 0; i < fListWatching.CountItems(); i++)
	{
		if(((EWatchingInfo*)fListWatching.ItemAt(i))->IsSameMessenger(msgr))
		{
			index_single = i;
			break;
		}
	}
	for(eint32 i = 0; i < fListWatchingAll.CountItems(); i++)
	{
		if(((EWatchingInfo*)fListWatchingAll.ItemAt(i))->IsSameMessenger(msgr))
		{
			index_all = i;
			break;
		}
	}

	if(what == E_OBSERVER_OBSERVE_ALL)
	{
		if(index_all < 0)
		{
			if(fListWatchingAll.AddItem(info = new EWatchingInfo(msgr)) == false)
			{
				delete info;
				return E_ERROR;
			}
		}
		else
		{
			((EWatchingInfo*)fListWatchingAll.ItemAt(index_all))->MakeEmpty();
		}

		if(index_single >= 0)
			delete (EWatchingInfo*)fListWatching.RemoveItem(index_single);

		return E_OK;
	}
	else if(index_all >= 0)
	{
		((EWatchingInfo*)fListWatchingAll.ItemAt(index_all))->RemoveWhat(what);
		return E_OK;
	}

	if((info = (EWatchingInfo*)fListWatching.ItemAt(index_single)) == NULL)
	{
		if(fListWatching.AddItem(info = new EWatchingInfo(msgr)) == false)
		{
			delete info;
			return E_ERROR;
		}
		return E_OK;
	}
	else
	{
		return((info->HasWhat(what) || info->AddWhat(what)) ? E_OK : E_ERROR);
	}
}


e_status_t
EObserverList::RemoveWatching(EMessenger msgr, euint32 what)
{
	if(msgr.IsValid() == false) return E_BAD_HANDLER;

	EWatchingInfo *info = NULL;
	eint32 index_single = -1, index_all = -1;

	for(eint32 i = 0; i < fListWatching.CountItems(); i++)
	{
		if(((EWatchingInfo*)fListWatching.ItemAt(i))->IsSameMessenger(msgr))
		{
			index_single = i;
			break;
		}
	}
	for(eint32 i = 0; i < fListWatchingAll.CountItems(); i++)
	{
		if(((EWatchingInfo*)fListWatchingAll.ItemAt(i))->IsSameMessenger(msgr))
		{
			index_all = i;
			break;
		}
	}

	if(what == E_OBSERVER_OBSERVE_ALL)
	{
		if(index_single >= 0) delete (EWatchingInfo*)fListWatching.RemoveItem(index_single);
		if(index_all >= 0) delete (EWatchingInfo*)fListWatchingAll.RemoveItem(index_all);
	}
	else
	{
		if(index_all >= 0)
		{
			info = (EWatchingInfo*)fListWatchingAll.ItemAt(index_all);
			if(!(info->HasWhat(what) || info->AddWhat(what))) return E_ERROR;
		}
		if(index_single >= 0)
		{
			info = (EWatchingInfo*)fListWatching.ItemAt(index_single);
			info->RemoveWhat(what);
			if(info->CountWhats() <= 0) delete (EWatchingInfo*)fListWatching.RemoveItem(index_single);
		}
	}

	return E_OK;
}


EList*
EObserverList::GetObserverList(euint32 what) const
{
	EList *list = new EList();

	if(what == E_OBSERVER_OBSERVE_ALL)
	{
		for(eint32 i = 0; i < fListWatchingAll.CountItems(); i++)
			list->AddItem(((EWatchingInfo*)fListWatchingAll.ItemAt(i))->Messenger());

		for(eint32 i = 0; i < fListWatching.CountItems(); i++)
			list->AddItem(((EWatchingInfo*)fListWatching.ItemAt(i))->Messenger());
	}
	else
	{
		for(eint32 i = 0; i < fListWatching.CountItems(); i++)
		{
			EWatchingInfo *aInfo = (EWatchingInfo*)fListWatching.ItemAt(i);
			if(aInfo->HasWhat(what)) list->AddItem(aInfo->Messenger());
		}

		for(eint32 i = 0; i < fListWatchingAll.CountItems(); i++)
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
EObserverList::IsWatched(euint32 what) const
{
	if(what == E_OBSERVER_OBSERVE_ALL) return(fListWatching.IsEmpty() == false || fListWatchingAll.IsEmpty() == false);

	for(eint32 i = 0; i < fListWatching.CountItems(); i++)
	{
		if(((EWatchingInfo*)fListWatching.ItemAt(i))->HasWhat(what)) return true;
	}

	eint32 exclude_times = 0;
	for(eint32 i = 0; i < fListWatchingAll.CountItems(); i++)
	{
		if(((EWatchingInfo*)fListWatchingAll.ItemAt(i))->HasWhat(what)) exclude_times++;
	}
	return(exclude_times != fListWatchingAll.CountItems());
}


EHandler::EHandler(const char *name)
	: EArchivable(),
	  fLooper(NULL),
	  fPrevHandler(NULL), fNextHandler(NULL),
	  fObserverList(NULL), fFilters(NULL)
{
	fName = EStrdup(name);
	fToken = etk_app_connector->HandlersDepot()->CreateToken(reinterpret_cast<void*>(this));
}


EHandler::~EHandler()
{
	if(fName != NULL) delete[] fName;
	if(fToken != NULL) delete fToken;
	if(fObserverList != NULL) delete reinterpret_cast<EObserverList*>(fObserverList);
	if(fFilters != NULL)
	{
		EHandler::SetFilterList(NULL);
		delete fFilters;
	}
}


EHandler::EHandler(const EMessage *from)
	: EArchivable(from),
	  fLooper(NULL),
	  fPrevHandler(NULL), fNextHandler(NULL),
	  fObserverList(NULL), fFilters(NULL)
{
	const char *name = NULL;
	if(from != NULL) from->FindString("_name", &name);

	fName = EStrdup(name);
	fToken = etk_app_connector->HandlersDepot()->CreateToken(reinterpret_cast<void*>(this));
}


e_status_t
EHandler::Archive(EMessage *into, bool deep) const
{
	if(!into) return E_ERROR;

	EArchivable::Archive(into, deep);
	into->AddString("class", "EHandler");
	into->AddString("_name", fName);

	return E_OK;
}


EArchivable*
EHandler::Instantiate(const EMessage *from)
{
	return(e_validate_instantiation(from, "EHandler") == false ? NULL : new EHandler(from));
}


void
EHandler::SetName(const char *name)
{
	if(fName != NULL) delete[] fName;
	fName = EStrdup(name);
}


const char*
EHandler::Name() const
{
	return fName;
}


void
EHandler::MessageReceived(EMessage *message)
{
	if(message == NULL || message->IsSystem() || fNextHandler == NULL || fNextHandler == fLooper) return;
	fNextHandler->MessageReceived(message);
}


void
EHandler::SetNextHandler(EHandler *handler)
{
	if(fLooper == NULL || handler == this || fNextHandler == handler) return;

	if(handler == NULL) ETK_ERROR("[APP]: %s --- Invalid operation", __PRETTY_FUNCTION__);

	if(handler == fLooper)
	{
		// Before:	fLooper ... fPrevHandler, this, fNextHandler ...
		// After:	fLooper ... fPrevHandler, fNextHandler ... this
		fPrevHandler->fNextHandler = fNextHandler;
		fNextHandler->fPrevHandler = fPrevHandler;
		fLooper->fPrevHandler->fNextHandler = this;

		EHandler *save_handler = fPrevHandler;

		fPrevHandler = fLooper->fPrevHandler;
		fNextHandler = fLooper;

		// call virtual function
		fPrevHandler->SetNextHandler(fPrevHandler->fNextHandler);
		save_handler->SetNextHandler(save_handler->fNextHandler);
	}
	else if(handler != NULL)
	{
		if(handler->fLooper != fLooper) return;

		EHandler *last_handler = handler;
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

		EHandler *save_handler = handler->fPrevHandler;

		handler->fPrevHandler = this;
		fNextHandler = handler;

		// call virtual function
		last_handler->SetNextHandler(last_handler->fNextHandler);
		save_handler->SetNextHandler(save_handler->fNextHandler);
	}
}


EHandler*
EHandler::NextHandler() const
{
	return fNextHandler;
}


ELooper*
EHandler::Looper() const
{
	return fLooper;
}


bool
EHandler::LockLooper()
{
	return(LockLooperWithTimeout(E_INFINITE_TIMEOUT) == E_OK);
}


e_status_t
EHandler::LockLooperWithTimeout(e_bigtime_t microseconds_timeout)
{
	EAutolock <ELocker>autolock(etk_get_handler_operator_locker());
	return(fLooper == NULL ? E_BAD_VALUE : fLooper->LockWithTimeout(microseconds_timeout));
}


void
EHandler::UnlockLooper()
{
	if(fLooper) fLooper->Unlock();
}


e_status_t
EHandler::StartWatching(EMessenger msgr, euint32 what)
{
	if(fObserverList == NULL) fObserverList = reinterpret_cast<void*>(new EObserverList());
	return reinterpret_cast<EObserverList*>(fObserverList)->AddWatching(msgr, what);
}


e_status_t
EHandler::StartWatchingAll(EMessenger msgr)
{
	return StartWatching(msgr, E_OBSERVER_OBSERVE_ALL);
}


e_status_t
EHandler::StopWatching(EMessenger msgr, euint32 what)
{
	if(fObserverList == NULL) return E_ERROR;
	return reinterpret_cast<EObserverList*>(fObserverList)->RemoveWatching(msgr, what);
}


e_status_t
EHandler::StopWatchingAll(EMessenger msgr)
{
	return StopWatching(msgr, E_OBSERVER_OBSERVE_ALL);
}


e_status_t
EHandler::StartWatching(EHandler *handler, euint32 what)
{
	e_status_t status;
	EMessenger msgr(handler, NULL, &status);
	if(status != E_OK) return status;

	return StartWatching(msgr, what);
}


e_status_t
EHandler::StartWatchingAll(EHandler *handler)
{
	e_status_t status;
	EMessenger msgr(handler, NULL, &status);
	if(status != E_OK) return status;

	return StartWatchingAll(msgr);
}


e_status_t
EHandler::StopWatching(EHandler *handler, euint32 what)
{
	e_status_t status;
	EMessenger msgr(handler, NULL, &status);
	if(status != E_OK) return status;

	return StopWatching(msgr, what);
}


e_status_t
EHandler::StopWatchingAll(EHandler *handler)
{
	e_status_t status;
	EMessenger msgr(handler, NULL, &status);
	if(status != E_OK) return status;

	return StopWatchingAll(msgr);
}


void
EHandler::SendNotices(euint32 what, const EMessage *message)
{
	if(fObserverList == NULL) return;

	EList *msgrsList = reinterpret_cast<EObserverList*>(fObserverList)->GetObserverList(what);
	if(msgrsList == NULL) return;

	EMessage msg(E_OBSERVER_NOTICE_CHANGE);
	if(message != NULL)
	{
		msg = *message;
		msg.what = E_OBSERVER_NOTICE_CHANGE;
		msg.AddInt32(E_OBSERVE_ORIGINAL_WHAT, message->what);
	}
	msg.AddInt32(E_OBSERVE_WHAT_CHANGE, what);

	for(eint32 i = 0; i < msgrsList->CountItems(); i++)
	{
		EMessenger *aMsgr = (EMessenger*)msgrsList->ItemAt(i);

		if(aMsgr->SendMessage(&msg, (EHandler*)NULL, 50000) != E_OK)
		{
			if(aMsgr->IsTargetLocal())
			{
				ELooper *looper = NULL;
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
EHandler::IsWatched(euint32 what) const
{
	if(fObserverList == NULL) return false;
	return reinterpret_cast<EObserverList*>(fObserverList)->IsWatched(what);
}


bool
EHandler::AddFilter(EMessageFilter *filter)
{
	if(filter == NULL || filter->fHandler != NULL) return false;
	if(fFilters == NULL) fFilters = new EList();
	if(fFilters->AddItem(filter) == false) return false;
	filter->fHandler = this;
	return true;
}


bool
EHandler::RemoveFilter(EMessageFilter *filter)
{
	if(fFilters == NULL || filter == NULL || filter->fHandler != this || fFilters->RemoveItem(filter) == false) return false;
	filter->fHandler = NULL;
	return true;
}


const EList*
EHandler::FilterList() const
{
	return fFilters;
}


bool
EHandler::SetFilterList(const EList *filterList)
{
	if(fFilters != NULL)
	{
		// Here we delete all filters without calling "RemoveFilter",
		// if you care about this, you should inherit this function.
		for(eint32 i = 0; i < fFilters->CountItems(); i++)
		{
			EMessageFilter *filter = (EMessageFilter*)fFilters->ItemAt(i);
			filter->fHandler = NULL;
			delete filter;
		}

		fFilters->MakeEmpty();
	}

	if(filterList != NULL)
	{
		for(eint32 i = 0; i < filterList->CountItems(); i++) AddFilter((EMessageFilter*)filterList->ItemAt(i));
	}

	return true;
}


EHandler*
EHandler::ResolveSpecifier(EMessage *msg, eint32 index, EMessage *specifier, eint32 what, const char *property)
{
	// TODO
	ETK_WARNING("[APP]: %s --- TODO", __PRETTY_FUNCTION__);
	return NULL;
}


e_status_t
EHandler::GetSupportedSuites(EMessage *data)
{
	// TODO
	ETK_WARNING("[APP]: %s --- TODO", __PRETTY_FUNCTION__);
	return E_ERROR;
}

