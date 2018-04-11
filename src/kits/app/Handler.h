/* --------------------------------------------------------------------------
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
 * File: Handler.h
 * Description: Basic object model for Application Kit
 *
 * --------------------------------------------------------------------------*/
#ifndef BHAPI_HANDLER_H
#define BHAPI_HANDLER_H
//-----------------------------------------------------------------------------
#include "private/PrivateHandler.h"
//-----------------------------------------------------------------------------
#ifdef __cplusplus /* Just for C++ */
//-----------------------------------------------------------------------------
#define B_OBSERVE_WHAT_CHANGE		"BHAPI:observe_change_what"
#define B_OBSERVE_ORIGINAL_WHAT		"BHAPI:observe_orig_what"
#define B_OBSERVER_OBSERVE_ALL	    B_MAXUINT32
//-----------------------------------------------------------------------------
#ifndef BLOOPER_DEF
#define BLOOPER_DEF
class BLooper;
#endif
//-----------------------------------------------------------------------------
#ifndef BMESSAGE_DEF
#define BMESSAGE_DEF
class BMessage;
#endif
//-----------------------------------------------------------------------------
#ifndef BMESSAGEFILTER_DEF
#define BMESSAGEFILTER_DEF
class BMessageFilter;
#endif
//-----------------------------------------------------------------------------
#ifndef BMESSANGER_DEF
#define BMESSANGER_DEF
class BMessenger;
#endif
//-----------------------------------------------------------------------------
#ifndef BTOKEN_DEF
#define BTOKEN_DEF
class BToken;
#endif
//-----------------------------------------------------------------------------
#ifndef BLIST_DEF
#define BLIST_DEF
class BList;
#endif
//-----------------------------------------------------------------------------
#endif // __cplusplus
//-----------------------------------------------------------------------------
#include "../support/Archivable.h"
#include <kernel/OS.h>
//-----------------------------------------------------------------------------
#ifdef __cplusplus /* Just for C++ */
//-----------------------------------------------------------------------------
using namespace bhapi;
class BHAPI_IMPEXP BHandler : public BArchivable {
public:
    BHandler(const char *name = NULL);
    // Archiving
    BHandler(const BMessage *from);
    virtual ~BHandler();

    virtual status_t  Archive(BMessage *into, bool deep = true) const;
    static BArchivable *Instantiate(const BMessage *from);

    void                SetName(const char *name);
    const char         *Name() const;

    virtual void		MessageReceived(BMessage *message);

    BLooper             *Looper() const;

    virtual void		SetNextHandler(BHandler *handler);
    BHandler           *NextHandler() const;

    bool                LockLooper();
    status_t          LockLooperWithTimeout(bigtime_t microseconds_timeout);
    void                UnlockLooper();

    // Observer calls
    status_t          StartWatching(BMessenger msgr,  uint32 what);
    status_t          StartWatchingAll(BMessenger msgr);
    status_t          StopWatching(BMessenger msgr,  uint32 what);
    status_t          StopWatchingAll(BMessenger msgr);

    status_t          StartWatching(BHandler *handler,  uint32 what);
    status_t          StartWatchingAll(BHandler *handler);
    status_t          StopWatching(BHandler *handler,  uint32 what);
    status_t          StopWatchingAll(BHandler *handler);

    // Notifier calls
    virtual void		SendNotices(uint32 what, const BMessage *msg = NULL);
    bool                IsWatched(uint32 what = B_OBSERVER_OBSERVE_ALL) const;

    // Message Filtering
    virtual bool		AddFilter(BMessageFilter *filter);
    virtual bool		RemoveFilter(BMessageFilter *filter);
    virtual bool		SetFilterList(const BList *filterList);
    const BList        *FilterList() const;

    // Scripting
    virtual BHandler   *ResolveSpecifier(BMessage *msg,  int32 index, BMessage *specifier,  int32 what, const char *property);
    virtual status_t	GetSupportedSuites(BMessage *data);

private:
    friend class BLooper;
    friend class BMessage;
    friend  uint64 bhapi::get_handler_token(const BHandler *handler);

    BToken *fToken;
    char *fName;
    BLooper *fLooper;
    BHandler *fPrevHandler;
    BHandler *fNextHandler;
    void *fObserverList;
    BList *fFilters;
};
//-----------------------------------------------------------------------------
#endif /* __cplusplus */
//-----------------------------------------------------------------------------
#endif /* BHAPI_HANDLER_H */
//-----------------------------------------------------------------------------
