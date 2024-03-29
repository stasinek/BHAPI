﻿/* --------------------------------------------------------------------------
 *
 * BHAPI++ Copyright (C) 2017, Stanislaw Stasiak, based on Haiku & ETK++, The Easy Toolkit for C++ programing
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

#include <kits/app/Handler.h>
#include <kits/netLooper.h>
#include <kits/app/Messenger.h>
#include <kits/netMessageFilter.h>
#include <kits/netMessage.h>
#include <kits/netMessageQueue.h>
#include <kits/netAppDefs.h>

#include <os/kernel.h>
#include <os/debug.h>
#include <kits/debug/Errors.h>
#include <kits/support/String.h>
#include <kits/support/List.h>
#include <kits/support/Locker.h>
#include <kits/support/Autolock.h>
#include <kits/support/List.h>
#include <kits/private/app/PrivateApplication.h>
#include <kits/private/app/PrivateHandler.h>


class BHAPI_LOCAL BObserverList {
public:
    BObserverList();
    ~BObserverList();

    status_t	AddWatching(BMessenger msgr,  uint32_t what);
    status_t	RemoveWatching(BMessenger msgr,  uint32_t what);
    bool		IsWatched(uint32_t what) const;
    BList		*GetObserverList(uint32_t what) const;

private:
    BList fListWatching;
    BList fListWatchingAll;
};


class BHAPI_LOCAL BWatchingInfo {
private:
    BMessenger fMessenger;
    BList fWhats;

public:
    BWatchingInfo(BMessenger msgr)
        : fMessenger(msgr)
    {
    }

    ~BWatchingInfo()
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

    bool AddWhat(uint32_t what)
    {
        if(what == B_OBSERVER_OBSERVE_ALL) return false;
        return fWhats.AddItem(reinterpret_cast<void*>(what));
    }

    bool RemoveWhat(uint32_t what)
    {
        if(what == B_OBSERVER_OBSERVE_ALL) return false;

         int32_t save_count = fWhats.CountItems();

        for(int32_t i = 0; i < fWhats.CountItems(); i++)
        {
            if(reinterpret_cast<uint32_t>(fWhats.ItemAt(i)) == what)
            {
                fWhats.RemoveItem(i);
                break;
            }
        }

        return(save_count > fWhats.CountItems());
    }

    bool HasWhat(uint32_t what) const
    {
        if(what == B_OBSERVER_OBSERVE_ALL) return false;

        for(int32_t i = 0; i < fWhats.CountItems(); i++)
        {
            if(reinterpret_cast<uint32_t>(fWhats.ItemAt(i)) == what) return true;
        }
        return false;
    }

     int32_t CountWhats() const
    {
        return fWhats.CountItems();
    }

    void MakeEmpty()
    {
        fWhats.MakeEmpty();
    }
};

BObserverList::BObserverList()
{
}

BObserverList::~BObserverList()
{
    for(int32_t i = 0; i < fListWatching.CountItems(); i++)
    {
        delete (BWatchingInfo*)fListWatching.ItemAt(i);
    }

    for(int32_t i = 0; i < fListWatchingAll.CountItems(); i++)
    {
        delete (BWatchingInfo*)fListWatchingAll.ItemAt(i);
    }
}

status_t BObserverList::AddWatching(BMessenger msgr,  uint32_t what)
{
    if(msgr.IsValid() == false) return B_BAD_HANDLER;

    BWatchingInfo *info = NULL;
     int32_t index_single = -1, index_all = -1;

    for(int32_t i = 0; i < fListWatching.CountItems(); i++)
    {
        if(((BWatchingInfo*)fListWatching.ItemAt(i))->IsSameMessenger(msgr))
        {
            index_single = i;
            break;
        }
    }
    for(int32_t i = 0; i < fListWatchingAll.CountItems(); i++)
    {
        if(((BWatchingInfo*)fListWatchingAll.ItemAt(i))->IsSameMessenger(msgr))
        {
            index_all = i;
            break;
        }
    }

    if(what == B_OBSERVER_OBSERVE_ALL)
    {
        if(index_all < 0)
        {
            if(fListWatchingAll.AddItem(info = new BWatchingInfo(msgr)) == false)
            {
                delete info;
                return B_ERROR;
            }
        }
        else
        {
            ((BWatchingInfo*)fListWatchingAll.ItemAt(index_all))->MakeEmpty();
        }

        if(index_single >= 0)
            delete (BWatchingInfo*)fListWatching.RemoveItem(index_single);

        return B_OK;
    }
    else if(index_all >= 0)
    {
        ((BWatchingInfo*)fListWatchingAll.ItemAt(index_all))->RemoveWhat(what);
        return B_OK;
    }

    if((info = (BWatchingInfo*)fListWatching.ItemAt(index_single)) == NULL)
    {
        if(fListWatching.AddItem(info = new BWatchingInfo(msgr)) == false)
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

status_t BObserverList::RemoveWatching(BMessenger msgr,  uint32_t what)
{
    if(msgr.IsValid() == false) return B_BAD_HANDLER;

    BWatchingInfo *info = NULL;
     int32_t index_single = -1, index_all = -1;

    for(int32_t i = 0; i < fListWatching.CountItems(); i++)
    {
        if(((BWatchingInfo*)fListWatching.ItemAt(i))->IsSameMessenger(msgr))
        {
            index_single = i;
            break;
        }
    }
    for(int32_t i = 0; i < fListWatchingAll.CountItems(); i++)
    {
        if(((BWatchingInfo*)fListWatchingAll.ItemAt(i))->IsSameMessenger(msgr))
        {
            index_all = i;
            break;
        }
    }

    if(what == B_OBSERVER_OBSERVE_ALL)
    {
        if(index_single >= 0) delete (BWatchingInfo*)fListWatching.RemoveItem(index_single);
        if(index_all >= 0) delete (BWatchingInfo*)fListWatchingAll.RemoveItem(index_all);
    }
    else
    {
        if(index_all >= 0)
        {
            info = (BWatchingInfo*)fListWatchingAll.ItemAt(index_all);
            if(!(info->HasWhat(what) || info->AddWhat(what))) return B_ERROR;
        }
        if(index_single >= 0)
        {
            info = (BWatchingInfo*)fListWatching.ItemAt(index_single);
            info->RemoveWhat(what);
            if(info->CountWhats() <= 0) delete (BWatchingInfo*)fListWatching.RemoveItem(index_single);
        }
    }

    return B_OK;
}


BList* BObserverList::GetObserverList(uint32_t what) const
{
    BList *list = new BList();

    if(what == B_OBSERVER_OBSERVE_ALL)
    {
        for(int32_t i = 0; i < fListWatchingAll.CountItems(); i++)
            list->AddItem(((BWatchingInfo*)fListWatchingAll.ItemAt(i))->Messenger());

        for(int32_t i = 0; i < fListWatching.CountItems(); i++)
            list->AddItem(((BWatchingInfo*)fListWatching.ItemAt(i))->Messenger());
    }
    else
    {
        for(int32_t i = 0; i < fListWatching.CountItems(); i++)
        {
            BWatchingInfo *aInfo = (BWatchingInfo*)fListWatching.ItemAt(i);
            if(aInfo->HasWhat(what)) list->AddItem(aInfo->Messenger());
        }

        for(int32_t i = 0; i < fListWatchingAll.CountItems(); i++)
        {
            BWatchingInfo *aInfo = (BWatchingInfo*)fListWatchingAll.ItemAt(i);
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

bool BObserverList::IsWatched(uint32_t what) const
{
    if(what == B_OBSERVER_OBSERVE_ALL) return(fListWatching.IsEmpty() == false || fListWatchingAll.IsEmpty() == false);

    for(int32_t i = 0; i < fListWatching.CountItems(); i++)
    {
        if(((BWatchingInfo*)fListWatching.ItemAt(i))->HasWhat(what)) return true;
    }

     int32_t exclude_times = 0;
    for(int32_t i = 0; i < fListWatchingAll.CountItems(); i++)
    {
        if(((BWatchingInfo*)fListWatchingAll.ItemAt(i))->HasWhat(what)) exclude_times++;
    }
    return(exclude_times != fListWatchingAll.CountItems());
}

BHandler::BHandler(const char *name)
    : BArchivable(),
      fLooper(NULL),
      fPrevHandler(NULL), fNextHandler(NULL),
      fObserverList(NULL), fFilters(NULL)
{
    fName = bhapi::strdup(name);
    fToken = bhapi::be_app_connector->HandlersDepot()->CreateToken(reinterpret_cast<void*>(this));
}

BHandler::~BHandler()
{
    if(fName != NULL) delete[] fName;
    if(fToken != NULL) delete fToken;
    if(fObserverList != NULL) delete reinterpret_cast<BObserverList*>(fObserverList);
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

    fName = bhapi::strdup(name);
    fToken = bhapi::be_app_connector->HandlersDepot()->CreateToken(reinterpret_cast<void*>(this));
}

status_t BHandler::Archive(BMessage *into, bool deep) const
{
    if(!into) return B_ERROR;

    BArchivable::Archive(into, deep);
    into->AddString("class", "BHandler");
    into->AddString("_name", fName);

    return B_OK;
}

BArchivable* BHandler::Instantiate(const BMessage *from)
{
    return(bhapi::validate_instantiation(from, "BHandler") == false ? NULL : new BHandler(from));
}

void BHandler::SetName(const char *name)
{
    if(fName != NULL) delete[] fName;
    fName = bhapi::strdup(name);
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


void BHandler::SetNextHandler(BHandler *handler)
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


status_t
BHandler::LockLooperWithTimeout(bigtime_t microseconds_timeout)
{
    BAutolock <BLocker>autolock(bhapi::get_handler_operator_locker());
    return(fLooper == NULL ? B_BAD_VALUE : fLooper->LockWithTimeout(microseconds_timeout));
}


void
BHandler::UnlockLooper()
{
    if(fLooper) fLooper->Unlock();
}


status_t
BHandler::StartWatching(BMessenger msgr,  uint32_t what)
{
    if(fObserverList == NULL) fObserverList = reinterpret_cast<void*>(new BObserverList());
    return reinterpret_cast<BObserverList*>(fObserverList)->AddWatching(msgr, what);
}


status_t
BHandler::StartWatchingAll(BMessenger msgr)
{
    return StartWatching(msgr, B_OBSERVER_OBSERVE_ALL);
}


status_t
BHandler::StopWatching(BMessenger msgr,  uint32_t what)
{
    if(fObserverList == NULL) return B_ERROR;
    return reinterpret_cast<BObserverList*>(fObserverList)->RemoveWatching(msgr, what);
}


status_t BHandler::StopWatchingAll(BMessenger msgr)
{
    return StopWatching(msgr, B_OBSERVER_OBSERVE_ALL);
}


status_t BHandler::StartWatching(BHandler *handler,  uint32_t what)
{
    status_t status;
    BMessenger msgr(handler, NULL, &status);
    if(status != B_OK) return status;

    return StartWatching(msgr, what);
}


status_t BHandler::StartWatchingAll(BHandler *handler)
{
    status_t status;
    BMessenger msgr(handler, NULL, &status);
    if(status != B_OK) return status;

    return StartWatchingAll(msgr);
}

status_t BHandler::StopWatching(BHandler *handler,  uint32_t what)
{
    status_t status;
    BMessenger msgr(handler, NULL, &status);
    if(status != B_OK) return status;

    return StopWatching(msgr, what);
}

status_t BHandler::StopWatchingAll(BHandler *handler)
{
    status_t status;
    BMessenger msgr(handler, NULL, &status);
    if(status != B_OK) return status;

    return StopWatchingAll(msgr);
}

void BHandler::SendNotices(uint32_t what, const BMessage *message)
{
    if(fObserverList == NULL) return;

    BList *msgrsList = reinterpret_cast<BObserverList*>(fObserverList)->GetObserverList(what);
    if(msgrsList == NULL) return;

    BMessage msg(B_OBSERVER_NOTICE_CHANGE);
    if(message != NULL)
    {
        msg = *message;
        msg.what = B_OBSERVER_NOTICE_CHANGE;
        msg.AddInt32(B_OBSERVE_ORIGINAL_WHAT, message->what);
    }
    msg.AddInt32(B_OBSERVE_WHAT_CHANGE, what);

    for(int32_t i = 0; i < msgrsList->CountItems(); i++)
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


bool BHandler::IsWatched(uint32_t what) const
{
    if(fObserverList == NULL) return false;
    return reinterpret_cast<BObserverList*>(fObserverList)->IsWatched(what);
}


bool BHandler::AddFilter(BMessageFilter *filter)
{
    if(filter == NULL || filter->fHandler != NULL) return false;
    if(fFilters == NULL) fFilters = new BList();
    if(fFilters->AddItem(filter) == false) return false;
    filter->fHandler = this;
    return true;
}

bool BHandler::RemoveFilter(BMessageFilter *filter)
{
    if(fFilters == NULL || filter == NULL || filter->fHandler != this || fFilters->RemoveItem(filter) == false) return false;
    filter->fHandler = NULL;
    return true;
}

const BList* BHandler::FilterList() const
{
    return fFilters;
}

bool BHandler::SetFilterList(const BList *filterList)
{
    if(fFilters != NULL)
    {
        // Here we delete all filters without calling "RemoveFilter",
        // if you care about this, you should inherit this function.
        for(int32_t i = 0; i < fFilters->CountItems(); i++)
        {
            BMessageFilter *filter = (BMessageFilter*)fFilters->ItemAt(i);
            filter->fHandler = NULL;
            delete filter;
        }

        fFilters->MakeEmpty();
    }

    if(filterList != NULL)
    {
        for(int32_t i = 0; i < filterList->CountItems(); i++) AddFilter((BMessageFilter*)filterList->ItemAt(i));
    }

    return true;
}


BHandler* BHandler::ResolveSpecifier(BMessage *msg,  int32_t index, BMessage *specifier,  int32_t what, const char *property)
{
    // TODO
    BHAPI_WARNING("[APP]: %s --- TODO", __PRETTY_FUNCTION__);
    return NULL;
}

status_t BHandler::GetSupportedSuites(BMessage *data)
{
    // TODO
    BHAPI_WARNING("[APP]: %s --- TODO", __PRETTY_FUNCTION__);
    return B_ERROR;
}
