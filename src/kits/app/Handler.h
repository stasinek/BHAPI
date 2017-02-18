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
 * File: Handler.h
 * Description: Basic object model for Application Kit
 *
 * --------------------------------------------------------------------------*/

#ifndef BHAPI_HANDLER__H
#define BHAPI_HANDLER__H
#include <kits/support/SupportDefs.h>

#ifdef __cplusplus /* Just for C++ */

#define B_OBSERVE_WHAT_CHANGE		"etk:observe_change_what"
#define B_OBSERVE_ORIGINAL_WHAT		"etk:observe_orig_what"
#define B_OBSERVER_OBSERVE_ALL	    B_MAXUINT32

class BLooper;
class BMessage;
class BMessageFilter;
class BMessenger;
class BToken;
class BList;
#include "../support/Archivable.h"
class IMPEXP_BHAPI BHandler : public BArchivable {
public:
    BHandler(const char *name = NULL);
	// Archiving
	BHandler(const BMessage *from);
    virtual ~BHandler();

    virtual b_status_t  Archive(BMessage *into, bool deep = true) const;
	static BArchivable *Instantiate(const BMessage *from);

    void                SetName(const char *name);
    const char         *Name() const;

	virtual void		MessageReceived(BMessage *message);

    BLooper             *Looper() const;

	virtual void		SetNextHandler(BHandler *handler);
    BHandler           *NextHandler() const;

    bool                LockLooper();
    b_status_t          LockLooperWithTimeout(b_bigtime_t microseconds_timeout);
    void                UnlockLooper();

	// Observer calls
    b_status_t          StartWatching(BMessenger msgr, b_uint32 what);
    b_status_t          StartWatchingAll(BMessenger msgr);
    b_status_t          StopWatching(BMessenger msgr, b_uint32 what);
    b_status_t          StopWatchingAll(BMessenger msgr);

    b_status_t          StartWatching(BHandler *handler, b_uint32 what);
    b_status_t          StartWatchingAll(BHandler *handler);
    b_status_t          StopWatching(BHandler *handler, b_uint32 what);
    b_status_t          StopWatchingAll(BHandler *handler);

	// Notifier calls
    virtual void		SendNotices(b_uint32 what, const BMessage *msg = NULL);
    bool                IsWatched(b_uint32 what = B_OBSERVER_OBSERVE_ALL) const;

	// Message Filtering
	virtual bool		AddFilter(BMessageFilter *filter);
	virtual bool		RemoveFilter(BMessageFilter *filter);
	virtual bool		SetFilterList(const BList *filterList);
    const BList        *FilterList() const;

	// Scripting
    virtual BHandler   *ResolveSpecifier(BMessage *msg, b_int32 index, BMessage *specifier, b_int32 what, const char *property);
	virtual b_status_t	GetSupportedSuites(BMessage *data);

private:
	friend class BLooper;
	friend class BMessage;
    friend b_uint64 bhapi_get_handler_token(const BHandler *handler);

    BToken *fToken;
	char *fName;
	BLooper *fLooper;
	BHandler *fPrevHandler;
	BHandler *fNextHandler;
	void *fObserverList;
	BList *fFilters;
};
#endif /* __cplusplus */
#endif /* BHAPI_HANDLER__H */
